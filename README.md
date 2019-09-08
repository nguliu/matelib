# My WebServer
## introduce
本项目为C++11编写的基于Epoll LT模式的多线程服务器，在实现的过程中参考了muduo网络库、libevent网络库和网上许多优秀的博客、开源项目等，支持同步日志和异步日志记录服务器状态，应用层实现了HTTP的Get和Head请求方法解析，支持静态资源的获取和HTTP长连接，另外可基于底层框架实现其他应用层协议。
## Project purposes
在之前学习了许多优秀书籍后，对Linux网络编程有了一定的理论知识储备，但实践应用偏少，在后来学习了muduo网络库和陈硕的Linux多线程服务端编程，决定写一个自己
的类似的服务器，对之前的理论知识进行巩固和应用。
- 主要用到的知识有：C++11特性和编程规范、Linux环境编程、Linux网络编程、IO模型、多线程、TCP/IP、HTTP、性能分析工具等。
- 主要参考的书籍有：C++ Primer、Effective C++、Unix环境高级编程、Unix网络编程 卷I、计算机网络、操作系统、Linux多线程服务端编程。

| Part Ⅰ | Part Ⅱ | Part Ⅲ | Part Ⅳ |
| :--------: | :---------: | :---------: | :---------: |
| [并发模型](https://github.com/Canna011/HighPerformance-server/blob/master/%E5%B9%B6%E5%8F%91%E6%A8%A1%E5%9E%8B)|[连接处理](https://github.com/Canna011/HighPerformance-server/blob/master/%E8%BF%9E%E6%8E%A5%E5%A4%84%E7%90%86)|[网络IO](https://github.com/Canna011/HighPerformance-server/blob/master/%E7%BD%91%E7%BB%9CIO) | [测试与改进](https://github.com/Canna011/HighPerformance-server/blob/master/%E6%B5%8B%E8%AF%95%E4%B8%8E%E6%94%B9%E8%BF%9B)
