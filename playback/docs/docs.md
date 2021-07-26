# docs  

本项目用于提供游戏回放功能  

注意：阅读此文档前请务必确保已认真阅读 logic 内的 docs！！！  

## 回放文件格式  

回放文件以 .thuaipb（取 THUAI Playback 缩写）为后缀名，内容如下：  

第 1~2 个字节一次为 P、B 两个字母的 Unicode 码，即第 1 个字节为 80，第 2 个字节为 66；第 3~4 个字节为保留字节，无实际意义，必须为零。第 5~8 共四个字节储存队伍总数，第 9~12 共四个字节储存每支队伍的玩家数；从下一个字节开始储存游戏的信息。  

游戏信息为每次服务器向客户端发送的当前游戏信息经 Protobuf 进行序列化后再经 GZip 压缩后形成的二进制字节序列。  

## 关于项目  

### playback、playbackForUnity  

用于写入与读取回放文件，使用的第三方库为 `Google.Protobuf`  

其中，playback 的目标平台为 .NET Core 3.1，供服务器与选手代码客户端与建议调试窗口客户端使用；playbackForUnity 目标平台为 .NET Standard 2.0，供 Unity 客户端使用    

### PlayBackPlayerDll、PlayBackPlayerResources 与 PlayBackPlayer  

此三个项目为 C++/CLI 项目，供游戏回放可以不通过 Server 进行观看。此项目是由 logic 内的调试项目 LocalUI 与 LocalUIDll 改造而成，因此也强烈不建议后来开发者再使用此技术进行开发，原因在 logic 的 docs 内已经写出，恕不赘述  

