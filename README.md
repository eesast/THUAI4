# THUAI4
THUAI4 原电子系第22届队式程序设计大赛

## 赛题简介

待定，头脑风暴中

## 框架简介

待定，C/S架构，实时游戏，同时支持人/AI，支持多种部署方式（局域网、服务器、本地）

## 仓库说明

本仓库为所有开发工作共用仓库，请勿上传大文件，每个目录均已配置好Visual Studio开发时的`gitignore`忽略规则，请根据需要自行修改，不要修改主目录下的`gitignore`忽略规则。

#### 目录分配

- CAPI：选手接口，生成控制台可执行文件
- communication：通信组件，生成`.NET Standard 2.0`，` .NET Core 3.1`类库与exe程序（Agent）
- config：配置文件，主要是游戏中数值等信息
- dependency：需要使用的各种依赖与proto文件
- installer：下载/更新器，生成Windows GUI程序
- interaction：玩家交互，生成Unity程序
- launcher：游戏启动器，生成Windows GUI程序。
- logic：游戏逻辑，生成控制台可执行文件（`.Net Core 3.1`）
- playback：回放组件，生成`.NET Standard 2.0`和` .NET Core 3.1`类库
- resource：用于主目录下markdown文件的图片等

#### 分支

- master：工作较稳定的版本或阶段性成果，需要2 reviewers
- dev：各个小组开发工作最新进展，需要1 reviewer

## 开发注意事项
- 认真填写PR与Commit信息，建议使用中文，禁止使用无意义的内容，科协仓库仅master与dev分支设有保护，可以自行创建其他分支用于小范围的团队合作
- 仍需要大家先将仓库fork到自己的仓库中进行开发
- 尽量不要上传大文件
- 多了解其他小组的工作

## 开发组成员

逻辑组：刘雪枫，李晨阳，谢童欣

交互组：张书源，潘修睿

通信组：何思成，李泽润，刘芳甫

赛事组：史益鸣，叶召城

端茶倒水：李翔宇