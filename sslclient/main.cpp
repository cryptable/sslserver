/*
 * Copyright 2013-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#if 0
int main(int argc, char **argv)
{
    BIO *sbio = NULL, *out = NULL;
    int len;
    char tmpbuf[1024];
    SSL_CTX *ctx;
    SSL_CONF_CTX *cctx;
    SSL *ssl;
    char **args = argv + 1;
    const char *connect_str = "localhost:2000";
    int nargs = argc - 1;

    ctx = SSL_CTX_new(TLS_client_method());
    cctx = SSL_CONF_CTX_new();
    SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_CLIENT);
    SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
    while (*args && **args == '-') {
        int rv;
        /* Parse standard arguments */
        rv = SSL_CONF_cmd_argv(cctx, &nargs, &args);
        if (rv == -3) {
            fprintf(stderr, "Missing argument for %s\n", *args);
            goto end;
        }
        if (rv < 0) {
            fprintf(stderr, "Error in command %s\n", *args);
            ERR_print_errors_fp(stderr);
            goto end;
        }
        /* If rv > 0 we processed something so proceed to next arg */
        if (rv > 0)
            continue;
        /* Otherwise application specific argument processing */
        if (strcmp(*args, "-connect") == 0) {
            connect_str = args[1];
            if (connect_str == NULL) {
                fprintf(stderr, "Missing -connect argument\n");
                goto end;
            }
            args += 2;
            nargs -= 2;
            continue;
        } else {
            fprintf(stderr, "Unknown argument %s\n", *args);
            goto end;
        }
    }

    if (!SSL_CONF_CTX_finish(cctx)) {
        fprintf(stderr, "Finish error\n");
        ERR_print_errors_fp(stderr);
        goto end;
    }

    /*
     * We'd normally set some stuff like the verify paths and * mode here
     * because as things stand this will connect to * any server whose
     * certificate is signed by any CA.
     */

    sbio = BIO_new_ssl_connect(ctx);

    BIO_get_ssl(sbio, &ssl);

    if (!ssl) {
        fprintf(stderr, "Can't locate SSL pointer\n");
        goto end;
    }

    /* Don't want any retries */
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* We might want to do other things with ssl here */

    BIO_set_conn_hostname(sbio, connect_str);

    out = BIO_new_fp(stdout, BIO_NOCLOSE);
    if (BIO_do_connect(sbio) <= 0) {
        fprintf(stderr, "Error connecting to server\n");
        ERR_print_errors_fp(stderr);
        goto end;
    }

    if (BIO_do_handshake(sbio) <= 0) {
        fprintf(stderr, "Error establishing SSL connection\n");
        ERR_print_errors_fp(stderr);
        goto end;
    }

    /* Could examine ssl here to get connection info */

    BIO_puts(sbio, "GET / HTTP/1.0\n\n");
    for (;;) {
        len = BIO_read(sbio, tmpbuf, 1024);
        if (len <= 0)
            break;
        BIO_write(out, tmpbuf, len);
    }
    end:
    SSL_CONF_CTX_free(cctx);
    BIO_free_all(sbio);
    BIO_free(out);
    return 0;
}

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PASSWORD "system"
int pem_passwd_cb(char *buf, int size, int rwflag, void *userdata) {
    printf("Get Password");

    strncpy(buf, PASSWORD, size);
    buf[size-1] = '\0';
    return strlen(buf);
}


int main(int argc, char *argv[]) {
    struct sockaddr_in addr;
    bool   ssl_support = false;
    bool   ssl_client_support = false;
    char   *channelid = NULL;

    if (argc == 3) {
        channelid = argv[1];
        if (strncmp(argv[2], "clear", 5) == 0){
            ssl_support = false;
            ssl_client_support = false;
        }
        if (strncmp(argv[2], "ssl", 3) == 0){
            ssl_support = true;
        }
        if (strncmp(argv[2], "sslclient", 8) == 0) {
            ssl_support = true;
            ssl_client_support = true;
        }
    }
    else {
        printf("Error missing <channel id> and/or ssl/sslclient\n");
        exit(EXIT_FAILURE);
    };

    /* Initialize openSSL */
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    /* Create the TLS context */
    SSL_CTX *ssl_ctx_1way = SSL_CTX_new(TLS_client_method());
    if (!SSL_CTX_set_min_proto_version(ssl_ctx_1way, TLS1_3_VERSION)) {
        perror("SSL_CTX_set_min_proto_version failed");
        exit(EXIT_FAILURE);
    }
    if (ssl_client_support) {
        SSL_CTX_set_default_passwd_cb(ssl_ctx_1way, pem_passwd_cb);
        if (!SSL_CTX_use_certificate_chain_file(ssl_ctx_1way, "../../sslclient/client_chain.pem")) {
            perror("SSL_CTX_use_certificate_chain_file 1way failed");
            exit(EXIT_FAILURE);
        }
        if (!SSL_CTX_use_PrivateKey_file(ssl_ctx_1way, "../../sslclient/client_key.pem", SSL_FILETYPE_PEM)) {
            printf("SSLError [%s]\n", ERR_error_string(ERR_get_error(), NULL));
            perror("SSL_CTX_use_PrivateKey_file 1way failed");
            exit(EXIT_FAILURE);
        }
    }

    /* Create socket and connect to server */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror( "socket creation failed");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(2000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int connfd = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (connfd < 0) {
        perror( "socket connection failed");
        exit(EXIT_FAILURE);
    }

    SSL *ssl = NULL;
    if (ssl_support) {
        ssl = SSL_new(ssl_ctx_1way);
        if (!SSL_set_fd(ssl, sockfd)) {
            perror("SSL_set_fd Accept failed: ");
            close(sockfd);
            SSL_free(ssl);
            exit(EXIT_FAILURE);
        }
        if (ssl_client_support) {
            if (!SSL_set_tlsext_host_name(ssl, "ssl2.localhost")) {
                perror("SSL_set_tlsext_host_name failed: ");
                close(sockfd);
                SSL_free(ssl);
                exit(EXIT_FAILURE);
            }
        }
        else {
            if (!SSL_set_tlsext_host_name(ssl, "ssl1.localhost.be")) {
                perror("SSL_set_tlsext_host_name failed: ");
                close(sockfd);
                SSL_free(ssl);
                exit(EXIT_FAILURE);
            }
        }
        if (SSL_connect(ssl) <= 0) {
            printf("SSLError [%s]\n", ERR_error_string(ERR_get_error(), NULL));
            perror("SSL_connect failed: ");
            close(sockfd);
            SSL_free(ssl);
            exit(EXIT_FAILURE);
        }
    }

    printf("Client connected (%d)\n", connfd);

    int cntr = 0;
    while (1) {
        char out_buf[128] = {0};
        char in_buf[128] = {0};
        sprintf(out_buf, "Hello server though %s for %d time (conn:%d)\n", channelid, cntr++, connfd);
        sleep(1);
        if (ssl) {
            int resw = SSL_write(ssl, out_buf, strlen(out_buf));
            if (resw <= 0) {
                printf("SSLError [%s]\n", ERR_error_string(SSL_get_error(ssl, resw), NULL));
                perror("SSL_write failed: ");
                close(sockfd);
                SSL_free(ssl);
                exit(EXIT_FAILURE);
            }
        }
        else {
            int resw = write(sockfd, out_buf, strlen(out_buf));
            if (resw <= 0) {
                perror("write failed: ");
            }
        }
        printf("server out %s", out_buf);
        if (ssl) {
            int resr = SSL_read(ssl, in_buf, (sizeof(in_buf) - 1));
            if (resr <= 0) {
                printf("SSLError [%s]\n", ERR_error_string(SSL_get_error(ssl, resr), NULL));
                perror("SSL_read failed: ");
                close(sockfd);
                SSL_free(ssl);
                exit(EXIT_FAILURE);
            }
        }
        else {
            int resr = read(sockfd, in_buf, (sizeof(in_buf) - 1));
        }
        printf("server in %s", in_buf);
    }

    close(connfd);
    exit(EXIT_SUCCESS);
}
#endif