# SSL Server
##Introduction
This is test sslserver using 1 port to support No-SSL, SSL, Mutual SSL. The decision of SSL and Mutual SSL is based on the domain name. 
The code is POC, the socket implementation will have multiple try-outs:
- poll()
- epoll() (TODO)
- kqueue  (TODO)
- OpenSSL BIO : Why have multiple socket abstractions.  (TODO)
- Boost Asio

The goal is to write a server which is small and capable to handle above 5k connections.

##Usage for developemnt
- cmake
- openssl
- using docker to build and test on MacOSX
- vagrant to built and test on FreeBSD/NetBSD/OpenBSD (TODO)
- Gatling to test the multiple connections

##Project setup
Before building the project, you should run 1-setup-dev.sh. It will download openssl, compile it and install it in the common directory, which will be created in the directory.
 
```shell script
./1-setup-dev.sh
```

###Perform docker build with CLion
Integrate the docker tools in CLion and follow the guideline on the blog of IntelliJ [Using Docker with CLion](https://blog.jetbrains.com/clion/2020/01/using-docker-with-clion/)

```
docker build -t clion/remote-cpp-env:0.2 -f Dockerfile.remote-cpp-env .
docker run -d --cap-add sys_ptrace -p127.0.0.1:2222:22 --name clion_remote_env clion/remote-cpp-env:0.2
```

##Methods of IO
There are some metdods to do IO and the first version of this POC is using poll(). Next we try to use epoll() on linux and lastly we will use kqueue to run this on MacOSX.

## Using poll()
This is supported on all UNIX like systems as long as you don't use the special extensions.

##Use io_uring

```
https://github.com/frevib/io_uring-echo-server/blob/master/io_uring_echo_server.c
```