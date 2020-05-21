# sslserver
This is test sslserver using 1 port to support No-SSL, SSL, Mutual SSL. The decision of SSL and Mutual SSL is based on the domain name. Code is POC and dirty, the socket implementation should go to the openSSL BIO implementation. Don't like, but why have multiple asynchronous socket implementations.

##Clion configuration

##Perform docker build with CLion

```
docker build -t clion/remote-cpp-env:0.2 -f Dockerfile.remote-cpp-env .
docker run -d --cap-add sys_ptrace -p127.0.0.1:2222:22 --name clion_remote_env clion/remote-cpp-env:0.2
```

###Use io_uring

```
https://github.com/frevib/io_uring-echo-server/blob/master/io_uring_echo_server.c
```