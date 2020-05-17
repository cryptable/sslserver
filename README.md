# sslserver
This is test sslserver using 1 port to support No-SSL, SSL, Mutual SSL. The decision of SSL and Mutual SSL is based on the domain name. Code is POC and dirty, the socket implementation should go to the openSSL BIO implementation. Don't like, but why have multiple asynchronous socket implementations.
