# 单线程小型web服务器tinyweb

## 简介:tea:
 tinyweb是一个epoll + 协程实现的一个小的web服务器。

## 进度:memo:
- [x] epoll监听与时间分发
- [x] 简单的helloworld界面
- [ ] 看看nginx和apache怎么做的
- [ ] 排除连接数过多之后崩溃的:bug:
- [ ] 单元测试
- [ ] http报文解析层
- [ ] 压力测试

## 压力测试:watch:
工具:wrench: JMeter
条件:hammer:: 600个线程循环发送20个http报文
