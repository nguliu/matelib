**A simple and easy to use high performance C + + Network Library**  
[![Build Status](https://travis-ci.org/inmail/matelib.svg?branch=master)](https://travis-ci.org/inmail/matelib)
##  
## Introduction
本项目为C++11编写的（RPC实现中使用了少量的C++14特性）基于epoll的多线程服务端网络库，在实现的过程中参考了[muduo网络库](https://github.com/chenshuo/muduo)、[libevent网络库](https://github.com/libevent/libevent)和网上许多优秀的博客、开源项目等，该库的主要作用是运用Linux系统调用处理网络连接、网络IO、日志记录、定时器等相关编程细节，使用该库可以使编程人员脱离服务端编程中与操作系统接轨的细节，只需要实现应用层业务逻辑并设置几个相应的事件处理函数即可。另外，实现了高效的异步日志系统，支持日志文件滚动等功能，应用层实现了简易的HTTP服务器和RPC服务器，另外可基于底层框架实现其他应用层服务。
## Project Purpose
在之前学习了许多优秀书籍后，对C++、Linux环境编程、网络编程有了一定的理论知识储备，但实践应用偏少，在后来学习了muduo网络库和陈硕的Linux多线程服务端编
程，于是决定开始动手进行实践，也是对之前的理论知识进行巩固和应用。
- 主要用到的知识：C++11特性和编程规范、Linux环境编程、Linux网络编程、并发IO模型、多线程、TCP/IP、HTTP、RPC服务相关、性能分析工具。
- 主要参考的书籍：C++ Primer、Effective C++、Unix环境高级编程、Unix网络编程 卷I、计算机网络、操作系统、Linux多线程服务端编程（陈硕）、Linux高性能服务器编程（游双）。
## Environment
- OS: Ubuntu 18.04
- Kernel: 5.0.0-27-generic
- Complier: g++ 7.4.0
## Build & Run
./build.sh  
./bin/httpserver_test  
./bin/rpcserver_test
## Technical Points
- 使用Epoll水平触发的IO多路复用技术实现Reactor模式，使用非阻塞IO+应用层缓冲区
- 使用IO线程池（兼计算线程池）充分发挥多核CPU性能，每个IO线程都是一个事件循环（即one loop per thred模式），另外计算线程可独立出去作为一个计算线程池
- 线程模型将划分为主线程+IO线程池+可选的Worker线程池，主线程接收客户端连接（accept），并通过Round-Robin策略分发给IO线程池中的线程，IO线程负责连接管理与网络IO，Worker线程负责业务计算任务（如果没有开启Worker线程池则由IO线程进行计算）
- 使用双缓冲区实现异步日志系统，避免生产者线程写磁盘的开销。另外，通过时间驱动或文件大小驱动实现日志文件的滚动
- 基于时间堆实现了高效的定时器，通过timerfd将定时器事件转化为IO事件，实现了统一事件源，通过Reactor进行统一管理
- 使用eventfd将线程异步唤醒转化为IO事件，实现了统一事件源，通过Reactor进行统一管理
- 为了避免内存泄漏，使用了智能指针等RAII机制管理对象资源
- 支持优雅关闭连接
- 基于网络传输服务实现HTTP服务，使用有限状态机解析HTTP请求，支持静态资源的获取和HTTP长连接
- 基于网络传输服务实现RPC服务，对参数和函数调用结果序列化传输和反序列化，支持远程自由函数、成员函数的调用
## Concurrent Model
![Image text](https://github.com/Canna011/myWebServer/blob/master/dec%26img/IO%E6%A8%A1%E5%9E%8B.png) 
其中，mainReadtor只负责accept新客户端的连接（如果只有mainReactor它也负责IO和compute），mainReactor建立一个新连接之后采用Round-Robin方式将其分发给
其他sub Reactor，每个连接只属于一个Reactor，由所属线程负责IO和compute。
