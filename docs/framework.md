
# framework  

## 机制和组成  
解耦设计，底层提供基本功能，上层封装应用功能


三层
HAL encapsulate devices+EtherCAT master+Other instruments.  
Server with websocket+eventloop.  
Client with GUI, sending req, receive rsp.  

### websockect  
websocket->sendmessage(message,function<void(message)>cb)  



## deps  
spdlog
soem
libhv/libuv(readcb)




