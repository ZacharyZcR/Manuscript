## FastCGI简介

FastCGI是通用网关接口(CGI)的改进版本,是一种让交互程序与Web服务器通信的协议。它的全称是Fast Common Gateway Interface,中文名为快速通用网关接口。FastCGI的主要特点和优势包括:

1. 常驻进程模式:FastCGI使用长生存期(long-running)的进程来处理一系列的请求,而不是像CGI那样为每个请求创建新进程。这些进程由FastCGI进程管理器管理,而不是Web服务器。

2. 性能提升:由于避免了频繁创建和销毁进程,FastCGI可以显著提高Web服务器的性能,特别是在高并发环境下。其速度效率至少比CGI技术提高5倍以上。

3. 资源节约:FastCGI程序只需在启动时解析一次php.ini配置文件并加载扩展程序,进行各种初始化操作,而不是像CGI那样每次请求都重复这些步骤。

4. 分布式部署支持:FastCGI支持分布式计算,可以在Web服务器以外的主机上执行FastCGI程序,实现负载均衡。

5. 语言无关性:FastCGI是与编程语言无关的协议,任何语言编写的程序都可以通过FastCGI协议提供Web服务。

FastCGI的工作流程主要包括:

1. Web服务器启动时同时启动FastCGI Server。

2. FastCGI进程管理器创建多个CGI进程,等待Web服务器的连接。

3. 当收到Web服务器的请求时,FastCGI进程管理器选择一个已创建的CGI进程进行连接。

4. 选中的CGI子进程处理请求,生成输出和错误信息,然后通过FastCGI Server返回给Web服务器。

尽管FastCGI相比CGI有许多优势,但它也存在一些缺点。由于FastCGI会同时保持多个CGI进程,因此比单个CGI进程更占用内存。每个PHP-CGI进程大约需要7MB到25MB的内存。

总的来说,FastCGI通过解决CGI的主要问题,提供了一种更高效、更灵活的Web服务器与应用程序通信方式。正如一个形象的比喻所描述的:"FastCGI就像是一群随时待命的厨师,而CGI则是每次有客人来才临时召集厨师"。这种设计使FastCGI在性能和资源利用方面都优于传统的CGI。