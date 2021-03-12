# THUAI4 选手接口

THUAI4 原电子系第22届队式程序设计大赛

## 设计思路（参考）

目标框架：C++11及以上

(使用了variant，所以要c++17)

#### 多线程设计

- 通信线程：监听代理Agent推送的信息更新并发送信息
- AI线程：死循环

#### 重点

- Protobuf的使用
- 资源的互斥访问（避免选手使用数据的同时通信线程修改它）

#### 难点（饼）

- 设计良好的debug接口

## 开发进度（参考）

### windows:

VS2019工程文件 win10电脑上可以运行

### Linux: 

makefile 在Ubuntu 18.04 X64服务器上可以运行

### Mac:

无

## 静态/动态库

有于它们（尤其静态库）比较大，放在了[清华云盘](https://cloud.tsinghua.edu.cn/library/362b57c2-33e0-49f7-b6b9-ded5ea1c0cac/THUAI4%E6%96%87%E4%BB%B6/)上。CAPI4Linux.rar是Linux上的工程（但rar不是linux的hhh），so.rar、a.rar、dll.rar、lib.rar分别是Linux/Windows所需动态库静态库。

### 最重要的是 - - - -

**可能（一定）会有BUG！！！**

（热烈欢迎大家帮我debug）

## 开发组成员

刘雪枫、何思成
