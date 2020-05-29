# SSL Server
##Introduction
This is test sslserver using 1 port to support No-SSL, SSL, Mutual SSL. The decision of SSL and Mutual SSL is based on the domain name. 
The code is POC, the socket implementation will have multiple try-outs:
- poll()
- epoll() (TODO)
- kqueue  (TODO)
- OpenSSL BIO : Why have multiple socket abstractions.  (TODO)
- Boost Asio

The goal is to write a server which is small and capable to handle above 5k connections. I didn't succeed it with 
boost::asio, so I want to start digging into this matter.

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

In the docker plugin, you have to config as '--cap-add=SYS_PTRACE'

##Methods of IO
There are some methods to do IO and the first version of this POC is using poll(). Next we try to use epoll() on linux and lastly we will use kqueue to run this on MacOSX.

## Using poll()
This is supported on all UNIX like systems as long as you don't use the special extensions. Though I am using some 
extensions. The max number of connections I can handle use computer is around 1000 per thread. I'm not convinced yet, 
because I see in the timings that the number connections don't go up to 1000, which means poll() is not giving me all 
the connection or gatling perf-test is not working as it should. I run everything on 1 PC.
Running 5 servers, I reach 5000 connections with a loss of less 1% due to timeouts. 

## Using epoll()

##Use io_uring

```
https://github.com/frevib/io_uring-echo-server/blob/master/io_uring_echo_server.c
```