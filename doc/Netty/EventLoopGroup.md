# Netty源码解析——EventLoopGroup

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_EventExecutorGroup_](#EventExecutorGroup_)
* 3. [_AbstractEventExecutorGroup_](#AbstractEventExecutorGroup_)
	* 3.1. [_#submit_](#submit_)
	* 3.2. [_#schedule_](#schedule_)
	* 3.3. [_#execute_](#execute_)
	* 3.4. [_#invokeAll_](#invokeAll_)
* 4. [_MultithreadEventExecutorGroup_](#MultithreadEventExecutorGroup_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->、

##  1. <a name=''></a>前言

* `Channel` 为Netty 网络操作抽象类。
* `EventLoop` 负责处理注册到其上的 Channel 处理 I/O 操作，两者配合参与 I/O 操作。
* `EventLoopGroup` 是一个 EventLoop 的分组，它可以获取到一个或者多个 EventLoop 对象，因此它提供了迭代出 EventLoop 对象的方法。

`EventLoop`和`EventGroup`的类图如下所示:

<div align=center><img src="/assets/netty17.png"/></div>

* 红框部分，为 EventLoopGroup 相关的类关系。其他部分，为 EventLoop 相关的类关系。
* 因为我们实际上使用的是 `NioEventLoopGroup` 和 `NioEventLoop` ，所以在此省略了其它相关的类，例如 `OioEventLoopGroup` 、 `EmbeddedEventLoop` 等等。

##  2. <a name='EventExecutorGroup_'></a>_EventExecutorGroup_

* 选择一个 _Executor_ 对象，_EventLoopGroup_ 本身不执行任务，而是通过`#submit`和`#schedule`交给自己管理的`EventExecutor`完成

<div align=center><img src="/assets/netty19.png"/></div>

* 至于选择哪一个_Executor_ ，可以通过`#next`来选择一个

<div align=center><img src="/assets/netty18.png"/></div>

##  3. <a name='AbstractEventExecutorGroup_'></a>_AbstractEventExecutorGroup_

`AbstractEventExecutorGroup`是`EventExecutorGroup`的默认实现

###  3.1. <a name='submit_'></a>_#submit_

<div align=center><img src="/assets/netty20.png"/></div>

###  3.2. <a name='schedule_'></a>_#schedule_

<div align=center><img src="/assets/netty21.png"/></div>

###  3.3. <a name='execute_'></a>_#execute_

<div align=center><img src="/assets/netty22.png"/></div>

###  3.4. <a name='invokeAll_'></a>_#invokeAll_

<div align=center><img src="/assets/netty23.png"/></div>

##  4. <a name='MultithreadEventExecutorGroup_'></a>_MultithreadEventExecutorGroup_

`io.netty.util.concurrent.MultithreadEventExecutorGroup` ，继承 `AbstractEventExecutorGroup` 抽象类，基于多线程的 EventExecutor ( 事件执行器 )的分组抽象类。

### Constructor

<div align=center><img src="/assets/netty24.png"/></div>

## _NioEventLoopGroup_

`io.netty.channel.nio.NioEventLoopGroup` ，继承 `MultithreadEventLoopGroup` 抽象类，`NioEventLoop` 的分组实现类。

### Constructor

构造方法比较多，**主要是明确了父构造方法的 Object ... args 方法参数:**
* 第一个参数，`selectorProvider` ， `java.nio.channels.spi.SelectorProvider`  ，用于创建 Java NIO Selector 对象。
* 第二个参数，`selectStrategyFactory` ， `io.netty.channel.SelectStrategyFactory` ，选择策略工厂。详细解析，见后续文章。
* 第三个参数，`rejectedExecutionHandler` ， `io.netty.channel.SelectStrategyFactory` ，拒绝执行处理器。详细解析，见后续文章。

### _#newChild_

<div align=center><img src="/assets/netty25.png"/></div>

### _#setIoRatio_

<div align=center><img src="/assets/netty26.png"/></div>

### _#rebuildSelectors_

因为 JDK 有 epoll 100% CPU Bug 。实际上，NioEventLoop 当触发该 Bug 时，也会自动调用 `NioEventLoop#rebuildSelector()` 方法，进行重建 Selector 对象，以修复该问题。

<div align=center><img src="/assets/netty27.png"/></div>

