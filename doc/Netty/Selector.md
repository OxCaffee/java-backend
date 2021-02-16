# Netty源码解析——选择器Selector

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [Selector的类图](#Selector)
* 3. [创建Selector](#Selector-1)
* 4. [注册Channel到Selector中](#ChannelSelector)
* 5. [_SelectionKey_](#SelectionKey_)
	* 5.1. [事件集合](#-1)
	* 5.2. [interest set](#interestset)
	* 5.3. [ready set](#readyset)
	* 5.4. [绑定对象](#-1)
	* 5.5. [获得对象](#-1)
* 6. [获取准备就绪的Channel——_#select_](#Channel_select_)
* 7. [获取可操作的Channel——_#selectedKeys_](#Channel_selectedKeys_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

Selector ， 一般称为选择器。它是 Java NIO 核心组件中的一个，用于轮询一个或多个 NIO Channel 的状态是否处于可读、可写。如此，一个线程就可以管理多个 Channel ，也就说可以管理多个网络连接。也因此，Selector 也被称为多路复用器。

那么 Selector 是如何轮询的呢？

首先，需要将 Channel 注册到 Selector 中，这样 Selector 才知道哪些 Channel 是它需要管理的。
之后，Selector 会不断地轮询注册在其上的 Channel 。如果某个 Channel 上面发生了读或者写事件，这个 Channel 就处于就绪状态，会被 Selector 轮询出来，然后通过 SelectionKey 可以获取就绪 Channel 的集合，进行后续的 I/O 操作。
下图是一个 Selector 管理三个 Channel 的示例：

<div align=center><img src="/assets/netty10.png"/></div>

* **优点:** 使用一个线程能够处理多个 Channel 的优点是，只需要更少的线程来处理 Channel 。事实上，可以使用一个线程处理所有的 Channel 。对于操作系统来说，线程之间上下文切换的开销很大，而且每个线程都要占用系统的一些资源( 例如 CPU、内存 )。因此，使用的线程越少越好。
* **缺点:** 因为在一个线程中使用了多个 Channel ，因此会造成每个 Channel 处理效率的降低。当然，Netty 在设计实现上，通过 n 个线程处理多个 Channel ，从而很好的解决了这样的缺点。其中，n 的指的是有限的线程数，默认情况下为 CPU * 2 。

##  2. <a name='Selector'></a>Selector的类图

<div align=center><img src="/assets/netty11.png"/></div>

##  3. <a name='Selector-1'></a>创建Selector

通过`Selector#open()`方法，可以创建一个默认的Selector:

<div align=center><img src="/assets/netty12.png"/></div>

##  4. <a name='ChannelSelector'></a>注册Channel到Selector中

为了让 Selector 能够管理 Channel ，我们需要将 Channel 注册到 Selector 中。代码如下：

```java
channel.configureBlocking(false); // <1>
SelectionKey key = channel.register(selector, SelectionKey.OP_READ); //<2>
```

* **注意，如果一个 Channel 要注册到 Selector 中，那么该 Channel 必须是非阻塞，** 所以 <1> 处的 `channel.configureBlocking(false)`; 代码块。也因此，`FileChannel` 是不能够注册到 Channel 中的，因为它是阻塞的。
* 在 `#register(Selector selector, int interestSet)` 方法的第二个参数，表示一个“interest 集合”，意思是通过 Selector 监听 Channel 时，对哪些( 可以是多个 )事件感兴趣。可以监听四种不同类型的事件：
  * **Connect ：** 连接完成事件( TCP 连接 )，仅适用于客户端，对应 `SelectionKey.OP_CONNECT` 。
  * **Accept ：** 接受新连接事件，仅适用于服务端，对应 `SelectionKey.OP_ACCEPT` 。
  * **Read ：** 读事件，适用于两端，对应 `SelectionKey.OP_READ` ，表示 Buffer 可读。
  * **Write ：** 写时间，适用于两端，对应 `SelectionKey.OP_WRITE` ，表示 Buffer 可写。

Channel 触发了一个事件，意思是该事件已经就绪：

* 一个 Client Channel Channel 成功连接到另一个服务器，称为“连接就绪”。
* 一个 Server Socket Channel 准备好接收新进入的连接，称为“接收就绪”。
* 一个有数据可读的 Channel ，可以说是“读就绪”。
* 一个等待写数据的 Channel ，可以说是“写就绪”。

因为 Selector 可以对 Channel 的多个事件感兴趣，所以在我们想要注册 Channel 的多个事件到 Selector 中时，可以使用或运算 | 来组合多个事件。示例代码如下：

```java
int interestSet = SelectionKey.OP_READ | SelectionKey.OP_WRITE;
```

实际使用时，我们会有改变 Selector 对 Channel 感兴趣的事件集合，可以通过再次调用 `#register(Selector selector, int interestSet)` 方法来进行变更。示例代码如下：

```java
channel.register(selector, SelectionKey.OP_READ);
channel.register(selector, SelectionKey.OP_READ | SelectionKey.OP_WRITE);
```

初始时，Selector 仅对 Channel 的 `SelectionKey.OP_READ` 事件感兴趣。
修改后，Selector 仅对 Channel 的 `SelectionKey.OP_READ` 和 `SelectionKey.OP_WRITE)` 事件都感兴趣。

##  5. <a name='SelectionKey_'></a>_SelectionKey_

SelectionKey 在 `java.nio.channels` 包下，被定义成一个抽象类，表示一个 Channel 和一个 Selector 的注册关系

###  5.1. <a name='-1'></a>事件集合

<div align=center><img src="/assets/netty13.png"/></div>

###  5.2. <a name='interestset'></a>interest set

表示当前SelectionKey感兴趣的事件集合:

```java
int interestSet = selectionKey.interestOps();

// 判断对哪些事件感兴趣
boolean isInterestedInAccept  = interestSet & SelectionKey.OP_ACCEPT != 0;
boolean isInterestedInConnect = interestSet & SelectionKey.OP_CONNECT != 0;
boolean isInterestedInRead    = interestSet & SelectionKey.OP_READ != 0;
boolean isInterestedInWrite   = interestSet & SelectionKey.OP_WRITE != 0;
```

###  5.3. <a name='readyset'></a>ready set

通过调用 `#readyOps()` 方法，返回就绪的事件集合。示例代码如下：

```java
int readySet = selectionKey.readyOps();

// 判断哪些事件已就绪
selectionKey.isAcceptable();
selectionKey.isConnectable();
selectionKey.isReadable();
selectionKey.isWritable();
```

###  5.4. <a name='-1'></a>绑定对象

通过调用 `#attach(Object ob)` 方法，可以向 SelectionKey 添加附加对象；通过调用 `#attachment()` 方法，可以获得 SelectionKey 获得附加对象。示例代码如下：

<div align=center><img src="/assets/netty14.png"/></div>

###  5.5. <a name='-1'></a>获得对象

通过调用`#attachment`可以获得当前绑定的对象:

<div align=center><img src="/assets/netty15.png"/></div>

##  6. <a name='Channel_select_'></a>获取准备就绪的Channel——_#select_

<div align=center><img src="/assets/netty16.png"/></div>

有一点非常需要注意：**select 方法返回的 int 值，表示有多少 Channel 已经就绪。亦即，自上次调用 select 方法后有多少 Channel 变成就绪状态**。如果调用 select 方法，因为有一个 Channel 变成就绪状态则返回了 1 ；若再次调用 select 方法，如果另一个 Channel 就绪了，它会再次返回1。如果对第一个就绪的 Channel 没有做任何操作，现在就有两个就绪的 Channel ，但在每次 select 方法调用之间，只有一个 Channel 就绪了，所以才返回 1。

##  7. <a name='Channel_selectedKeys_'></a>获取可操作的Channel——_#selectedKeys_

一旦调用了 select 方法，并且返回值表明有一个或更多个 Channel 就绪了，然后可以通过调用Selector 的 `#selectedKeys()` 方法，访问“已选择键集( selected key set )”中的就绪 Channel 。示例代码所示：

```java
Set selectedKeys = selector.selectedKeys()
```
**注意，当有新增就绪的 Channel ，需要先调用 select 方法，才会添加到“已选择键集( selected key set )”中。否则，我们直接调用 #selectedKeys() 方法，是无法获得它们对应的 SelectionKey 们。**

