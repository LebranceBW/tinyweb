# 单线程小型web服务器tinyweb

## 简介:tea:
 tinyweb是一个epoll + 协程实现的一个小的web服务器。

## 进度:memo:
- [x] epoll监听与事件分发
- [x] 简单的helloworld界面
- [x] 关闭连接用的close，看看怎么改成shutdown
- [x] 看看nginx和apache怎么做的
- [x] 排除连接数过多之后崩溃的:bug:
- [x] 压力测试
- [ ] 单元测试
- [ ] http报文解析层

## 压力测试:watch:
工具:wrench: [kalvin在线工具](https://tools.kalvinbg.cn/dev/stress/testing)
条件:hammer:: 60个线程循环发送20个http报文

## docker仓库:whale:
[Docker镜像](https://hub.docker.com/repository/docker/lebrancebw/tinyweb)
```bash
docker pull lebrancebw/tinyweb:1.0
docker run -d -p 8080:80 lebrancebw/tinyweb
```
## 参考
* [SimpleEpollServer](https://github.com/MalwareTech/SimpleEpollServer)