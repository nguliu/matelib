# My WebServer
## Introduce
本项目为C++11编写的基于Epoll LT模式的多线程服务器，在实现的过程中参考了muduo网络库、libevent网络库和网上许多优秀的博客、开源项目等，支持同步日志和异步日志记录服务器状态，应用层使用有限状态机对HTTP的GET和HEAD方法进行解析，支持静态资源的获取和HTTP长连接，另外可基于底层框架实现其他应用层协议。
## Project purposes
在之前学习了许多优秀书籍后，对Linux网络编程有了一定的理论知识储备，但实践应用偏少，在后来学习了muduo网络库和陈硕的Linux多线程服务端编程，决定写一个自己
的类似的服务器，对之前的理论知识进行巩固和应用。
- 主要用到的知识有：C++11特性和编程规范、Linux环境编程、Linux网络编程、IO模型、多线程、TCP/IP、HTTP、性能分析工具等。
- 主要参考的书籍有：C++ Primer、Effective C++、Unix环境高级编程、Unix网络编程 卷I、计算机网络、操作系统、Linux多线程服务端编程。
## Environment
- OS: Ubuntu 18.04
- Kernel: 5.0.0-27-generic
- Complier: g++ 7.4.0
## Build and run
./build.sh 
./bin/httpserver_test 
## Technical Points
- 使用Epoll边沿触发的IO多路复用技术实现Reactor模式，使用非阻塞IO
- 使用双缓冲区实现异步日志系统，减小生产者线程的开销
## 并发模型如下

## Other
- [性能测试]()
- [历史版本分析]()
