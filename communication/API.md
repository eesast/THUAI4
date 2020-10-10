## Server
```c#
public void Listen(ushort port);//开始工作，监听某个端口
public event OnReceiveCallback OnReceive;//这两个event在Agent连接或发送信息时触发
public event OnConnectCallback OnConnect;//目前传参IntPtr connId，但似乎也没什么用？
public void Send(byte[] bytes);//广播
public bool Dequeue(out byte[] data);//队列里取一个，失败返回0
public void Stop();
public void Dispose();
```

## Client

```c#
public bool Connect(string IP, ushort port)；//连接Agent
public event OnReceiveCallback OnReceive;//收到信息时触发，传参是收到的信息bytes[]
public bool Send(byte[] bytes);//向Agent发送
public bool Stop();
public void Dispose();
```

## Agent

Agent 

-s|--server game server endpoint

-p|--port agent port