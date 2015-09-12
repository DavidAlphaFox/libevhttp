<pre>libevhttp 是基于 libev 网络库开发的一套 HTTP Server 端服务框架，是使用c++开发的，使用此libevhttp可方便的创建http应用程序。<br>
<br>
<br>
如果你有好的意见建议可通过(trywen@qq.com)联系我。<br>
<br>
当前在CentOS和ubuntu比较新的版本中编译通过, 其它linux环境没试过，如果有需要可联系本人<br>
<br>
注意：<br>
1. 安装此http网络库之前必须先安装libev，http://dist.schmorp.de/libev/ 有下载<br>
2. libev和libevhttp安装完成后，需要:ldconfig /usr/local/lib<br>
<br>
[更新说明 v0.0.4]<br>
1. 解决在ubuntu中不能正常编译通过的问题<br>
2. 优化其它功能<br>
<br>
<br>
[更新说明 v0.0.3]<br>
1. IIOWriter类型<br>
1) 增加transport(IIOPump* iopumper)方法，可以将IIOPump写入目标设备<br>
2) write方法中增加bool copy=true参数, 原型为:<br>
virtual bool write(const char* buff, size_t n, bool copy=true, off_t offset = -1)<br>
增加了copy参数，如果设置copy=true, 将会生成buff的复本， 否则，将直接引用buff<br>
3)  增加start()方法，修改为手动启动，以前是在提交写请求时自动启动写服务，现在需要手动调用start()方法启动。<br>
同时对应的也增加stop()方法来停止写， 这样在使用IIOWriter及所有实现类型时，需要手动调用start()方法来写<br>
数据。<br>
2. IIOReader类型<br>
1) read()方法中参数bool full参数，指示是否将buff读满时再触发读完成事件, 现在方法原型为：<br>
virtual bool read(char* buff, size_t n, bool full = false, off_t offset = -1)<br>
2. Response类型<br>
1) 增加sendfile方法，可以按照一定速率发送文件, 内部采用的是SendfileIOPump实现, 方法原型为:<br>
bool sendfile(FD fd, off_t readOffset, size_t len, unsigned int speed, size_t maxSendSize=DEFAULT_DATA_SIZE)<br>
2) evhttpd_example-0.0.3.zip的FileServerHttpServlet.h演示了Response.sendfile的用法<br>
3) 优化部份方法<br>
4. 优化HttpHandler, Socket的读写, 使用IIOReader和IIOWriter的SocketNIOReader和SocketNIOWriter实现来读写数据<br>
5. 优化HttpProcess<br>
6. 新增IIOWriter和IIOWriter的Socket读写实现，SocketNIOReader,SocketNIOWriter<br>
7. 移除AIOWriter和AIOReader两个类型<br>
8. 优化了某些代码<br>
<br>
<br>
[更新说明 v0.0.2]<br>
1. libevhttp-0.0.1.tar.gz中的HttpHandler.cpp中误include了event.h，有可能导致EV_READ和EV_WRITE的值混乱，在libevhttp-0.0.2.tar.gz中已经将其去掉<br>
</pre>