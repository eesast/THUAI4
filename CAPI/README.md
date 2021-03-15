# THUAI4 选手接口

THUAI4 原电子系第 22 届队式程序设计大赛

## 设计思路（参考）

C++ 语言标准：C++17

#### 多线程设计

- 通信线程：监听代理 Agent 推送的信息更新并发送信息
- AI 线程：死循环

#### 重点

- Protobuf 的使用
- 资源的互斥访问（避免选手使用数据的同时通信线程修改它）

#### 难点（饼）

- 设计良好的 debug 接口

## 开发进度（参考）

用很 naive 的 AI 策略进行了测试

提供了基础的 Debug 接口

还有很多不想填了的饼

### Windows:

配置好 VS2019 工程文件 Release/Debug模式均能成功生成

（HPSocket x86 x64的库还不一样，目前云盘上传的只有win32）

win10 电脑测试

### Linux: 

提供了 makefile

Ubuntu 18.04 X86_64 架构服务器上可以正常生成、运行

（同上，云盘上的库是x64的）

### Mac:

无

## 静态/动态库

由于它们（尤其静态库）比较大，放在了[清华云盘](https://cloud.tsinghua.edu.cn/library/362b57c2-33e0-49f7-b6b9-ded5ea1c0cac/THUAI4%E6%96%87%E4%BB%B6/)上。CAPI4Linux.rar 是 Linux 上的工程（虽然rar是windows的），so.rar、a.rar、dll.rar、lib.rar 分别是 Linux/Windows 所需动态库静态库。

### 目录结构

 - CAPI
   - CAPI
     - include
       - win
       - linux
     - src
     - lib (for Windows)
     - dll (for Windows)
     - CAPI.vcproj (for Windows)
     - CAPI.vcproj.filters (for Windows)
     - a (for Linux)
     - so (for Linux)
     - makefile (for Linux)
   - CAPI.sln（for Windows）



四个库目录没有在github上上传，需要从云盘上下载。VS工程文件和makefile视情况删除。

### 关于运行时动态库的位置

- windows 下 VS 项目->属性->配置属性->调试->环境里把 dll 文件夹加进去了，所以用 VS 调试可以找到动态库。但命令行调用还得把 dll 放到和 CAPI.exe 一个位置。
- Linux 里 g++ 编译时传了 -Wl, -rpath=./so​，所以可执行文件要和 so 文件夹在同一个位置。



## 开发组成员

何思成、刘雪枫
