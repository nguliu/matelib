**A simple and easy to use high performance C + + Network Library**
##  
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
- 使用Epoll边沿触发的IO多路复用技术实现Reactor模式，使用非阻塞IO+应用层缓冲区
- 使用IO thread pool + compute thread pool充分发挥多核CPU性能，每个IO线程一个event loop（即one loop per thred）
- 线程模型将划分为主线程、IO线程和worker线程，主线程接收客户端连接（accept），并通过Round-Robin策略分发给IO线程，IO线程负责连接管理（即事件监听和读写操作），worker线程负责业务计算任务（即对数据进行处理，应用层处理复杂的时候可以开启）
- 使用双缓冲区实现异步日志系统，避免生产者线程写磁盘的开销。另外，通过时间驱动或文件大小驱动实现日志文件的滚动
- 使用eventfd实现了线程的异步唤醒
- 为了避免内存泄漏，使用了智能指针等RAII机制管理对象资源
- 使用有线状态机解析HTTP请求，支持HTTP长连接
- 支持优雅关闭连接
## Concurrent model
![Image text](https://github.com/Canna011/myWebServer/blob/master/dec%26img/IO%E6%A8%A1%E5%9E%8B.png) 
其中，mainReadtor只负责accept新客户端的连接（如果只有mainReactor它也负责IO和compute），mainReactor建立一个新连接之后采用Round-Robin方式将其分发给
其他sub Reactor，每个连接只属于一个Reactor，由所属线程负责IO和compute。
