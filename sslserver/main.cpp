#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <poll.h>
#include <limits.h>

#define MAX_LISTEN 10
#define PORT       2000

#define clear_poll( p ) (p).fd = -1;  (p).events = 0; (p).revents = 0;
#define set_poll( p, b ) (p).fd = (b);  (p).events = POLLIN; (p).revents = 0;

struct context {
    char in_buf[128];
    int  in_buf_lg;
    int  in_buf_pos;
    char out_buf[128];
    int  out_buf_lg;
    int  out_buf_pos;
    SSL *ssl;
};

#define clear_ctx( p ) memset(&(p), 0, sizeof(context))
#define get_ctx( c, p ) (c)[(p-1)]

int verify_callback(int ok, X509_STORE_CTX *ctx) {

    return 1;
}

#define PASSWORD "system"
int pem_passwd_cb(char *buf, int size, int rwflag, void *userdata) {
    strncpy(buf, PASSWORD, size);
    buf[size-1] = '\0';
    return strlen(buf);
}

#define MUTUALSSL_DOMAIN "ssl2.localhost"
#define SERVERSSL_DOMAIN "ssl1.localhost"

int tlsext_servername_callback(SSL *ssl, int *al, void *arg) {
    const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);

    if (strncasecmp(servername, MUTUALSSL_DOMAIN, sizeof(MUTUALSSL_DOMAIN)) == 0) {
        SSL_set_verify(ssl, SSL_VERIFY_PEER, verify_callback);
    }
    else if (strncasecmp(servername, SERVERSSL_DOMAIN, sizeof(MUTUALSSL_DOMAIN)) != 0) {
        return SSL_TLSEXT_ERR_NOACK;
    }

    return SSL_TLSEXT_ERR_OK;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in addr;
    int ret = 0;

    /* Initialize openSSL */
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    /* Create the TLS context */
    SSL_CTX *ssl_ctx_1way = SSL_CTX_new(TLS_server_method());
    if (!SSL_CTX_set_min_proto_version(ssl_ctx_1way, TLS1_3_VERSION)) {
        perror("SSL_CTX_set_min_proto_version failed");
        exit(EXIT_FAILURE);
    }
    SSL_CTX_set_default_passwd_cb(ssl_ctx_1way, pem_passwd_cb);

    if (!SSL_CTX_use_certificate_chain_file(ssl_ctx_1way, "../localhost_chain.pem")) {
        perror("SSL_CTX_use_certificate_chain_file 1way failed");
        exit(EXIT_FAILURE);
    }
    if (!SSL_CTX_use_PrivateKey_file(ssl_ctx_1way, "../localhost_key.pem", SSL_FILETYPE_PEM)) {
        printf("SSLError [%s]\n", ERR_error_string(ERR_get_error(), NULL));
        perror("SSL_CTX_use_PrivateKey_file 1way failed");
        exit(EXIT_FAILURE);
    }

    if (!SSL_CTX_set_tlsext_servername_callback(ssl_ctx_1way,tlsext_servername_callback)) {
        printf("SSLError [%s]\n", ERR_error_string(ERR_get_error(), NULL));
        perror("SSL_CTX_set_tlsext_servername_callback failed");
        exit(EXIT_FAILURE);
    }

    /* Create socket and connect to server */
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0) {
        perror( "socket creation failed");
        exit(-1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, MAX_LISTEN) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    /* Handle connections */
    struct context ctxs[MAX_LISTEN];
    struct pollfd poll_ev[MAX_LISTEN+1];

    set_poll(poll_ev[0], sockfd);
    for (int i=1; i<(MAX_LISTEN+1); i++) {
        clear_poll(poll_ev[i]);
        clear_ctx( get_ctx( ctxs, i ) );
    }



    while(1) {
        int nbr_el = poll(poll_ev, MAX_LISTEN+1, 100);
        if (nbr_el < 0) {
            perror("Poll failed");
            exit(EXIT_FAILURE);
        }
        int poll_pos = 0;
        while ((nbr_el > 0) && (poll_pos < (MAX_LISTEN+1))) {
            // accept client
            if (poll_ev[poll_pos].revents != 0) {
                if (poll_pos == 0) {
                    // Accept
                    bool accepted = false;
//                    printf("Event for accept %02x\n", poll_ev[poll_pos].revents);
                    for (int i=1; i<(MAX_LISTEN+1); i++) {
                        if (poll_ev[i].fd == -1) {
                            struct sockaddr_in cl_addr;
                            uint len = sizeof(addr);
                            set_poll(poll_ev[i], accept4(sockfd, (struct sockaddr*)&cl_addr, &len, SOCK_NONBLOCK));
//                            set_poll(poll_ev[i], accept(sockfd, (struct sockaddr*)&cl_addr, &len));
                            if (poll_ev[i].fd < 0) {
                                clear_poll(poll_ev[i]);
                                perror("Accept failed: ");
                                exit(EXIT_FAILURE);
                            }
                            poll_ev[i].events = POLLIN;
                            char str[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &cl_addr.sin_addr, str, INET_ADDRSTRLEN);
                            printf("Connected from %s for %d\n", str, i);
                            sprintf(get_ctx(ctxs,i).out_buf, "Send data through channel %d\n", i);
                            get_ctx(ctxs,i).out_buf_lg = strlen(get_ctx(ctxs,i).out_buf);

                            // One way SSL
                            if (i%2 == 1) {
                                SSL *ssl = SSL_new(ssl_ctx_1way);
                                if (!SSL_set_fd(ssl, poll_ev[i].fd)) {
                                    printf("SSLError [%s]\n", ERR_error_string(ERR_get_error(), NULL));
                                    perror("SSL_set_fd failed: ");
                                    close(poll_ev[i].fd);
                                    SSL_free(ssl);
                                    clear_poll(poll_ev[i]);
                                    clear_ctx(get_ctx(ctxs,i));
                                    break;
                                }

                                do {
                                    ret = SSL_accept(ssl);
                                } while ((ret <= 0) &&
                                         ((SSL_get_error(ssl, ret) == SSL_ERROR_WANT_WRITE) ||
                                          (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ)));
                                if (ret <= 0) {
                                    printf("SSLError [%s]\n", ERR_error_string(ERR_get_error(), NULL));
                                    perror("SSL Accept failed: ");
                                    close(poll_ev[i].fd);
                                    SSL_free(ssl);
                                    clear_poll(poll_ev[i]);
                                    clear_ctx(get_ctx(ctxs,i));
                                    break;
                                }

                                // Get hostname SNI
                                printf("Connected to host: %s\n", SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name));
                                X509 *x509 = SSL_get_peer_certificate(ssl);
                                if (x509 != NULL) {
                                    char name_buffer[1024];
                                    printf("Certificate connected: %s\n", X509_NAME_oneline(X509_get_subject_name(x509), name_buffer, sizeof(name_buffer)-1));
                                }

                                // Set non blocking
                                BIO_set_nbio(SSL_get_rbio(ssl), 1);
                                BIO_set_nbio(SSL_get_wbio(ssl), 1);
                                get_ctx(ctxs,i).ssl = ssl;
                            }
                            break;
                        }
                    }
                }
                else {
//                    printf("Event for connection %d: %02x\n", poll_pos, poll_ev[poll_pos].revents);
                    if (poll_ev[poll_pos].revents & POLLIN) {
//                        printf("Data for reading %d: %02x\n", poll_pos, poll_ev[poll_pos].revents);
                        char in_buf;
                        while (1) {
                            if (get_ctx(ctxs, poll_pos).ssl) {
                                int bytes = SSL_read(get_ctx(ctxs, poll_pos).ssl, &in_buf, 1);
                                if (bytes > 0) {
                                    get_ctx(ctxs, poll_pos).in_buf[get_ctx(ctxs, poll_pos).in_buf_lg] = in_buf;
                                    get_ctx(ctxs, poll_pos).in_buf_lg++;
                                }
                                else {
                                    if (SSL_get_error(get_ctx(ctxs, poll_pos).ssl, bytes) == SSL_ERROR_WANT_READ) {
                                        printf("Received %d: %s", poll_pos, get_ctx(ctxs, poll_pos).in_buf);
                                        poll_ev[poll_pos].events = POLLOUT;
                                        break;
                                    }
                                    else {
                                        SSL_shutdown(get_ctx(ctxs,poll_pos).ssl);
                                        SSL_free(get_ctx(ctxs,poll_pos).ssl);
                                        close(poll_ev[poll_pos].fd);
                                        clear_poll(poll_ev[poll_pos]);
                                        clear_ctx(get_ctx(ctxs,poll_pos));
                                        break;
                                    }
                                }
                            }
                            else {
                                ssize_t bytes = recv(poll_ev[poll_pos].fd, &in_buf, sizeof(in_buf), MSG_DONTWAIT);
                                if (bytes > 0) {
                                    get_ctx(ctxs, poll_pos).in_buf[get_ctx(ctxs, poll_pos).in_buf_lg] = in_buf;
                                    get_ctx(ctxs, poll_pos).in_buf_lg++;
                                }
                                else {
                                    if (errno == EWOULDBLOCK) {
                                        printf("Received %d: %s", poll_pos, get_ctx(ctxs, poll_pos).in_buf);
                                        poll_ev[poll_pos].events = POLLOUT;
                                        get_ctx(ctxs,poll_pos).out_buf_pos = 0;
                                    }
                                    else {
                                        close(poll_ev[poll_pos].fd);
                                        clear_poll(poll_ev[poll_pos]);
                                        clear_ctx(get_ctx(ctxs,poll_pos));
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    if (poll_ev[poll_pos].revents & POLLOUT) {
//                        printf("Data for writing %d: %02x\n", poll_pos, poll_ev[poll_pos].revents);
                        if (get_ctx(ctxs,poll_pos).out_buf_pos < get_ctx(ctxs,poll_pos).out_buf_lg) {
                            if (get_ctx(ctxs, poll_pos).ssl) {
                                int bytes = SSL_write( get_ctx(ctxs, poll_pos).ssl, get_ctx(ctxs,poll_pos).out_buf, strlen(get_ctx(ctxs,poll_pos).out_buf));
                                if (bytes <= 0) {
                                    if (SSL_get_error(get_ctx(ctxs, poll_pos).ssl, bytes) != SSL_ERROR_WANT_WRITE) {
                                        SSL_shutdown(get_ctx(ctxs,poll_pos).ssl);
                                        SSL_free(get_ctx(ctxs,poll_pos).ssl);
                                        close(poll_ev[poll_pos].fd);
                                        clear_poll(poll_ev[poll_pos]);
                                        clear_ctx(get_ctx(ctxs,poll_pos));
                                        break;
                                    }
                                }
                                printf("Send %d: %s", poll_pos, get_ctx(ctxs, poll_pos).out_buf);
                                poll_ev[poll_pos].events = POLLIN;
                                get_ctx(ctxs, poll_pos).in_buf_lg = 0;

                            }
                            else {
                                ssize_t bytes = send(poll_ev[poll_pos].fd, get_ctx(ctxs,poll_pos).out_buf, strlen(get_ctx(ctxs,poll_pos).out_buf), MSG_DONTWAIT);
                                if (bytes <= 0) {
                                    if (errno != EWOULDBLOCK) {
                                        close(poll_ev[poll_pos].fd);
                                        clear_poll(poll_ev[poll_pos]);
                                        clear_ctx(get_ctx(ctxs,poll_pos));
                                        break;
                                    }
                                    printf("Unable to send %d: %s", poll_pos, get_ctx(ctxs, poll_pos).out_buf);
                                }
                                else {
                                    printf("Send %d: %s", poll_pos, get_ctx(ctxs, poll_pos).out_buf);
                                }
                                poll_ev[poll_pos].events = POLLIN;
                                get_ctx(ctxs, poll_pos).in_buf_lg = 0;
                            }
                        }
                        else {
                            printf("Send %d: %s", poll_pos, get_ctx(ctxs, poll_pos).out_buf);
                            poll_ev[poll_pos].events = POLLIN;
                            get_ctx(ctxs, poll_pos).in_buf_lg = 0;
                        }
                    }
                    if (poll_ev[poll_pos].revents & POLLERR) {
                        printf("Event(POLLERR) for connection %d: %02x\n", poll_pos, poll_ev[poll_pos].revents);
                        if (get_ctx(ctxs,poll_pos).ssl != NULL) {
                            SSL_shutdown(get_ctx(ctxs,poll_pos).ssl);
                            SSL_free(get_ctx(ctxs,poll_pos).ssl);
                        }
                        close(poll_ev[poll_pos].fd);
                        clear_poll(poll_ev[poll_pos]);
                        clear_ctx(get_ctx(ctxs,poll_pos));
                    }
                    if (poll_ev[poll_pos].revents & POLLRDHUP) {
                        printf("Event(POLLRDHUP) for connection %d: %02x\n", poll_pos, poll_ev[poll_pos].revents);
                        if (get_ctx(ctxs,poll_pos).ssl != NULL) {
                            SSL_shutdown(get_ctx(ctxs,poll_pos).ssl);
                            SSL_free(get_ctx(ctxs,poll_pos).ssl);
                        }
                        close(poll_ev[poll_pos].fd);
                        clear_poll(poll_ev[poll_pos]);
                        clear_ctx(get_ctx(ctxs,poll_pos));
                    }
                }
                nbr_el--;
            }
            poll_pos++;
        }
    }

//    close(client);

}
