# Netty源码解析——EventLoop

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [ _EventExecutor_](#_EventExecutor_)
* 3. [_OrderedEventExecutor_](#OrderedEventExecutor_)
* 4. [_EventLoop_](#EventLoop_)
* 5. [_AbstarctEventExecutor_](#AbstarctEventExecutor_)
	* 5.1. [Constructor](#Constructor)
	* 5.2. [_#newPromise_](#newPromise_)
	* 5.3. [_#newProgressivePromise_](#newProgressivePromise_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

Netty使用EventLoop来处理连接上的读写事件，而一个连接上的所有请求都保证在一个EventLoop中被处理，一个EventLoop中只有一个Thread，所以也就实现了一个连接上的所有事件只会在一个线程中被执行。一个EventLoopGroup包含多个EventLoop，可以把一个EventLoop当做是Reactor线程模型中的一个线程，而一个EventLoopGroup类似于一个ExecutorService，当然，这只是为了更好的理解Netty的线程模型，它们之间是没有等价关系的，后面的分析中会详细讲到。下面的图片展示了Netty的线程模型：

<div align=center><img src="/assets/netty29.png"/></div>

在NI/O非阻塞模式下，Netty将负责为每个Channel分配一个EventLoop，一旦一个EventLoop呗分配给了一个Channel，那么在它的整个生命周期中都使用这个EventLoop，但是多个Channel将可能共享一个EventLoop，所以和Thread相关的ThreadLocal的使用就要特别注意，因为有多个Channel在使用该Thread来处理读写时间。在阻塞IO模式下，考虑到一个Channel将会阻塞，所以不太可能将一个EventLoop共用于多个Channel之间，所以，每一个Channel都将被分配一个EventLoop，并且反过来也成立，也就是一个EventLoop将只会被绑定到一个Channel上来处理这个Channel上的读写事件。无论是非阻塞模式还是阻塞模式，一个Channel都将会保证一个Channel上的所有读写事件都只会在一个EventLoop上被处理。

* **NIO模式分配EventLoop** :

<div align=center><img src="/assets/netty30.png"/></div>

* **BIO模式下分配EventLoop** :

<div align=center><img src="/assets/netty31.png"/></div>

在Netty中，EventLoop的类图关系如下:

<div align=center><img src="/assets/netty28.png"></div>

从EventLoop的类图中可以发现，其实EventLoop继承了Java的ScheduledExecutorService，也就是调度线程池，所以，EventLoop应当有ScheduledExecutorService提供的所有功能。那为什么需要继承ScheduledExecutorService呢，也就是为什么需要延时调度功能，那是因为，在Netty中，有可能用户线程和Netty的I/O线程同时操作网络资源，而为了减少并发锁竞争，Netty将用户线程的任务包装成Netty的task，然后向Netty的I/O任务一样去执行它们。有些时候我们需要延时执行任务，或者周期性执行任务，那么就需要调度功能。这是Netty在设计上的考虑，为我们极大的简化的编程方法。

大概了解了EventLoop的一些基础信息，下面就可以开始剖析EventLoop的源码了。

##  2. <a name='_EventExecutor_'></a> _EventExecutor_

EventExecutor继承自EventExecutorGroup。是事件执行器的接口，定义了事件执行器的一些基本操作，源码如下:

```java
//返回自己的ref
@Override
EventExecutor next();

//返回当前EventExecutor所属的EventExecutorGroup
EventExecutorGroup parent();

//当前线程是否在EventLoop线程中
boolean inEventLoop();

//返回给定线程是否在EventLoop线程中
boolean inEventLoop(Thread thread);

//创建一个Promise对象
<V> Promise<V> newPromise();

//创建一个ProgressivePromise对象
<V> ProgressivePromise newProgressivePromise();

//返回成功结果的Future对象
<V> Future<V> newSucceededFuture(V result);

//返回失败结果的Future对象
<V> Future<V> newFailedFuture(Throwable cause);
```

##  3. <a name='OrderedEventExecutor_'></a>_OrderedEventExecutor_

继承 EventExecutor 接口，有序的事件执行器接口。内部没有任何新增实现。

##  4. <a name='EventLoop_'></a>_EventLoop_

```java
public interface EventLoop extends OrderedEventExecutor, EventLoopGroup {

    @Override
    EventLoopGroup parent();

}
```

**EventLoop 将会处理注册在其上的 Channel 的所有 IO 操作**

##  5. <a name='AbstarctEventExecutor_'></a>_AbstarctEventExecutor_

实现 EventExecutor 接口，继承 AbstractExecutorService 抽象类，EventExecutor 抽象类。

###  5.1. <a name='Constructor'></a>Constructor

```java
//所属 EventExecutorGroup
private final EventExecutorGroup parent;

//EventExecutor 数组。只包含自己，用于遍历
private final Collection<EventExecutor> selfCollection = Collections.<EventExecutor>singleton(this);

protected AbstractEventExecutor() {
    this(null);
}

protected AbstractEventExecutor(EventExecutorGroup parent) {
    this.parent = parent;
}
```

###  5.2. <a name='newPromise_'></a>_#newPromise_

创建DefaultPromise对象，在此之前，我们必须去了解什么是Promise，Promise和Future之间有什么关系和区别。

**Netty是一个异步网络处理框架，在实现中大量使用了Future机制，并在Java自带Future的基础上，增加了Promise机制。这两者的目的都是使异步编程更加方便使用。**

* **Future** :使用Future机制时，我们调用耗时任务会立刻返回一个Future实例，使用该实例能够以阻塞的方式或者在未来某刻获得耗时任务的执行结果，还可以添加监听事件设置后续程序。

```java
Future asynchronousFunction(String arg){
  Future future = new Future(new Callable(){
      public Object call(){
        return null;
      }
  });
  return future;
}
 ReturnHandler handler = asynchronousFunction(); //  耗时函数，但会立即返回一个句柄
 handler.getResult(); // 通过句柄可以等待结果
 handler.addListener(); //通过句柄可以添加完成后执行的事件
 handler.cancel(); // 通过句柄取消耗时任务
```

* **Promise** :在Future机制中，业务逻辑所在任务执行的状态（成功或失败）是在Future中实现的，而在Promise中，可以在业务逻辑控制任务的执行结果，相比Future，更加灵活。

```java
// 异步的耗时任务接收一个promise
Promise asynchronousFunction(String arg){
    Promise  promise = new PromiseImpl();
    Object result = null;
    result = search()  //业务逻辑,
    if(success){
        promise.setSuccess(result); // 通知promise当前异步任务成功了，并传入结果
    }else if(failed){
        promise.setFailure(reason); //// 通知promise当前异步任务失败了
    }else if(error){
        promise.setFailure(error); //// 通知promise当前异步任务发生了异常
    }
}

// 调用异步的耗时任务
Promise promise = asynchronousFunction(promise) ；//会立即返回promise
//添加成功处理/失败处理/异常处理等事件
promise.addListener();// 例如，可以添加成功后执行的事件
doOtherThings() ; //　继续做其他事件，不需要理会asynchronousFunction何时结束
```
下面我们继续来看源码，`#newPromise`创建一个DefaultPromise对象，源码如下:

```java
public <V> Promise<V> newPromise() {
    return new DefaultPromise<V>(this);
}
```

至于DefaultPromise的源码解析，限于篇幅问题，我们将会留在单独的一篇文章来分析——[Netty源码解析正式篇——Promise](/doc/Netty/Promise.md)

###  5.3. <a name='newProgressivePromise_'></a>_#newProgressivePromise_

创建一个DefaultProgressivePromise对象:

```java
public <V> ProgressivePromise<V> newProgressivePromise() {
    return new DefaultProgressivePromise<V>(this);
}
```

