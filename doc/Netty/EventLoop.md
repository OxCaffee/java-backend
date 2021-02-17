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
* 6. [_AbstractScheduledEventExecutor_](#AbstractScheduledEventExecutor_)
	* 6.1. [_ScheduledFutureTask_](#ScheduledFutureTask_)
		* 6.1.1. [成员属性](#-1)
		* 6.1.2. [_#nanoTime_](#nanoTime_)
		* 6.1.3. [_#compareTo_](#compareTo_)
		* 6.1.4. [_#run_](#run_)
		* 6.1.5. [_#cancel_](#cancel_)
	* 6.2. [_#schedule_](#schedule_)
	* 6.3. [_#removeScheduled_](#removeScheduled_)
	* 6.4. [_#hasScheduledTasks_](#hasScheduledTasks_)
	* 6.5. [_#nextScheduledTaskNano_](#nextScheduledTaskNano_)
	* 6.6. [_#cancelScheduledTasks_](#cancelScheduledTasks_)
* 7. [_SingleThreadEventExecutor_](#SingleThreadEventExecutor_)
	* 7.1. [成员属性](#-1)
	* 7.2. [_#fetchFromScheduledTaskQueue_](#fetchFromScheduledTaskQueue_)
	* 7.3. [_#reject_](#reject_)
	* 7.4. [_#execute_](#execute_)

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

##  6. <a name='AbstractScheduledEventExecutor_'></a>_AbstractScheduledEventExecutor_

继承 AbstractEventExecutor 抽象类，**支持定时任务**的 EventExecutor 的抽象类。

AbstractScheduledExecutor有几个非常重要的关键属性:

1. **ScheduledFutureTask比较器**

```java
private static final Comparator<ScheduledFutureTask<?>> SCHEDULED_FUTURE_TASK_COMPARATOR =
   			new Comparator<ScheduledFutureTask<?>>() {
                @Override
                public int compare(ScheduledFutureTask<?> o1, ScheduledFutureTask<?> o2) {
                    return o1.compareTo(o2);
                }
            };
```

2. **Wake up Task**

```java
static final Runnable WAKEUP_TASK = new Runnable() {
       @Override
       public void run() { } // Do nothing
    };
```

3. **调度队列scheduledTaskQueue**

```java
PriorityQueue<ScheduledFutureTask<?>> scheduledTaskQueue;
```

4. **下一个Task的id**

```java
long nextTaskId;
```

在开始分析AbstractScheduleEventExecutor的源码之前，我们需要先了解一些各个成员属性。

###  6.1. <a name='ScheduledFutureTask_'></a>_ScheduledFutureTask_

`io.netty.util.concurrent.ScheduledFutureTask` ，实现 ScheduledFuture、PriorityQueueNode 接口，继承 PromiseTask 抽象类，Netty 定时任务。

####  6.1.1. <a name='-1'></a>成员属性

```java
//调度的开始基准时间，因为该FutureTask基于时间差而不是目标时间，所以操作系统的时钟改变并不会影响调度结果
private static final long START_TIME = System.nanoTime();

//该ScheduledFutureTask的id，初始化时赋值，之后不会改变
private long id;

//调度截至时间，即时间差最大值
private long deadlineNanos;

//调度的阶段，源码这里没怎么看懂
private final long periodNanos;

//在调度队列中的索引index，初始时刻为不在队列中
private int queueIndex = INDEX_NOT_IN_QUEUE;
```

####  6.1.2. <a name='nanoTime_'></a>_#nanoTime_

获得当前调度的时间， **指的是从开始时间算起一共过了多少时间，是相对值**

```java
static long nanoTime() {
    return System.nanoTime() - START_TIME;
}
```

####  6.1.3. <a name='compareTo_'></a>_#compareTo_

该方法是比较两个ScheduledFutureTask，因为在AbstractScheduledEventExecutor中声明了比较器，因此要重写此方法。假如两个ScheduledFutureTask对象，分别为o1, o2，则比较方法如下：

1. `o1==o2` ，返回二者相等
2. `o1.deadlineNanos - o2.deadlineNanos > 0` ，返回`o1>o2`
3. `o1.deadlineNanos - o2.deadlineNanos < 0` ，返回`o1<o2`
4. `o1.deadlineNanos == o2.deadlineNanos` 且 `o1.id < o2.id` ，返回`o1<o2`

```java
    @Override
    public int compareTo(Delayed o) {
        if (this == o) {
            return 0;
        }

        ScheduledFutureTask<?> that = (ScheduledFutureTask<?>) o;
        long d = deadlineNanos() - that.deadlineNanos();
        if (d < 0) {
            return -1;
        } else if (d > 0) {
            return 1;
        } else if (id < that.id) {
            return -1;
        } else {
            assert id != that.id;
            return 1;
        }
    }
```

####  6.1.4. <a name='run_'></a>_#run_

```java
    @Override
    public void run() {
        //校验当前task必须在EventLoop线程中
        assert executor().inEventLoop();
        try {
            if (delayNanos() > 0L) {
                // 如果任务没有被取消，从任务队列中移除
                if (isCancelled()) {
                    scheduledExecutor().scheduledTaskQueue().removeTyped(this);
                } else {
                    scheduledExecutor().scheduleFromEventLoop(this);
                }
                return;
            }
            //判断当前任务为仅执行一次的任务
            if (periodNanos == 0) {
                // 设置任务不可取消
                if (setUncancellableInternal()) {
                    // 执行任务
                    V result = runTask();
                    //通知任务执行成功
                    setSuccessInternal(result);
                }
            } else { //当前任务不止执行一次
                // 判断任务并未取消
                if (!isCancelled()) {
                    //执行任务
                    runTask();
                    if (!executor().isShutdown()) {
                        // 计算下次执行时间
                        if (periodNanos > 0) {
                            deadlineNanos += periodNanos;
                        } else {
                            deadlineNanos = nanoTime() - periodNanos;
                        }
                        // 判断任务并未取消
                        if (!isCancelled()) {
                            // 重新添加到任务队列，等待下次定时执行
                            scheduledExecutor().scheduledTaskQueue().add(this);
                        }
                    }
                }
            }
        } catch (Throwable cause) {
            setFailureInternal(cause);
        }
    }
```

####  6.1.5. <a name='cancel_'></a>_#cancel_

有两个方法，可以取消定时任务。代码如下：

```java
@Override
public boolean cancel(boolean mayInterruptIfRunning) {
    boolean canceled = super.cancel(mayInterruptIfRunning);
    // 取消成功，移除出定时任务队列
    if (canceled) {
        ((AbstractScheduledEventExecutor) executor()).removeScheduled(this);
    }
    return canceled;
}

// 移除任务
boolean cancelWithoutRemove(boolean mayInterruptIfRunning) {
    return super.cancel(mayInterruptIfRunning);
}
```

差别在于，是否 调用 `AbstractScheduledEventExecutor#removeScheduled(ScheduledFutureTask)` 方法，从定时任务队列移除自己

###  6.2. <a name='schedule_'></a>_#schedule_

提交定时任务， **必须在 EventLoop 的线程中，才能添加到定时任务到队列中**

```java
<V> ScheduledFuture<V> schedule(final ScheduledFutureTask<V> task) {
    if (inEventLoop()) {
        // 添加到定时任务队列
        scheduledTaskQueue().add(task);
    } else {
        // 通过 EventLoop 的线程，添加到定时任务队列
        execute(new Runnable() {
            @Override
            public void run() {
                scheduledTaskQueue().add(task);
            }
        });
    }
    return task;
}
```

###  6.3. <a name='removeScheduled_'></a>_#removeScheduled_

移除出定时任务队列， **必须在EventLoop线程中，才能移出定时任务队列**

```java
final void removeScheduled(final ScheduledFutureTask<?> task) {
    if (inEventLoop()) {
        // 移除出定时任务队列
        scheduledTaskQueue().removeTyped(task);
    } else {
        // 通过 EventLoop 的线程，移除出定时任务队列
        execute(new Runnable() {
            @Override
            public void run() {
                removeScheduled(task);
            }
        });
    }
}
```

###  6.4. <a name='hasScheduledTasks_'></a>_#hasScheduledTasks_

```java
protected final boolean hasScheduledTasks() {
    Queue<ScheduledFutureTask<?>> scheduledTaskQueue = this.scheduledTaskQueue;
    // 获得队列首个定时任务。不会从队列中，移除该任务
    ScheduledFutureTask<?> scheduledTask = scheduledTaskQueue == null ? null : scheduledTaskQueue.peek();
    // 判断该任务是否到达可执行的时间
    return scheduledTask != null && scheduledTask.deadlineNanos() <= nanoTime();
}
```

###  6.5. <a name='nextScheduledTaskNano_'></a>_#nextScheduledTaskNano_

获得定时任务队列，距离当前时间，还要多久可执行。

- 若队列**为空**，则返回 `-1` 。
- 若队列**非空**，若为负数，直接返回 0 。实际等价，ScheduledFutureTask#delayNanos() 方法。

```java
protected final long nextScheduledTaskNano() {
    Queue<ScheduledFutureTask<?>> scheduledTaskQueue = this.scheduledTaskQueue;
    // 获得队列首个定时任务。不会从队列中，移除该任务
    ScheduledFutureTask<?> scheduledTask = scheduledTaskQueue == null ? null : scheduledTaskQueue.peek();
    if (scheduledTask == null) {
        return -1;
    }
    // 距离当前时间，还要多久可执行。若为负数，直接返回 0 。实际等价，ScheduledFutureTask#delayNanos() 方法。
    return Math.max(0, scheduledTask.deadlineNanos() - nanoTime());
}
```

###  6.6. <a name='cancelScheduledTasks_'></a>_#cancelScheduledTasks_

取消定时任务队列的所有任务

```java
protected void cancelScheduledTasks() {
    assert inEventLoop();

    // 若队列为空，直接返回
    PriorityQueue<ScheduledFutureTask<?>> scheduledTaskQueue = this.scheduledTaskQueue;
    if (isNullOrEmpty(scheduledTaskQueue)) {
        return;
    }

    // 循环，取消所有任务
    final ScheduledFutureTask<?>[] scheduledTasks = scheduledTaskQueue.toArray(new ScheduledFutureTask<?>[0]);
    for (ScheduledFutureTask<?> task : scheduledTasks) {
        task.cancelWithoutRemove(false);
    }

    scheduledTaskQueue.clearIgnoringIndexes();
}

private static boolean isNullOrEmpty(Queue<ScheduledFutureTask<?>> queue) {
    return queue == null || queue.isEmpty();
}
```

##  7. <a name='SingleThreadEventExecutor_'></a>_SingleThreadEventExecutor_

实现 OrderedEventExecutor 接口，继承 AbstractScheduledEventExecutor 抽象类，基于单线程的 EventExecutor 抽象类，**即一个 EventExecutor 对应一个线程**。

###  7.1. <a name='-1'></a>成员属性

```java
//字段的原子更新器
private static final AtomicIntegerFieldUpdater<SingleThreadEventExecutor> STATE_UPDATER =AtomicIntegerFieldUpdater.newUpdater(SingleThreadEventExecutor.class, "state");

//字段的原子更新器
private static final AtomicReferenceFieldUpdater<SingleThreadEventExecutor, ThreadProperties> PROPERTIES_UPDATER = AtomicReferenceFieldUpdater.newUpdater(SingleThreadEventExecutor.class, ThreadProperties.class, "threadProperties");

//任务队列
private final Queue<Runnable> taskQueue;

//线程
private volatile Thread thread;

//线程属性
@SuppressWarnings("unused")
private volatile ThreadProperties threadProperties;

//执行器
private final Executor executor;

//线程是否已经打断
private volatile boolean interrupted;

//EventLoop 优雅关闭
private final Semaphore threadLock = new Semaphore(0);

//EventLoop 优雅关闭
private final Set<Runnable> shutdownHooks = new LinkedHashSet<Runnable>();

//添加任务时，是否唤醒线程
private final boolean addTaskWakesUp;

//最大等待执行任务数量，即taskQueue的队列大小
private final int maxPendingTasks;

//拒绝执行处理器
private final RejectedExecutionHandler rejectedExecutionHandler;

//最后执行时间
private long lastExecutionTime;

//状态
@SuppressWarnings({ "FieldMayBeFinal", "unused" })
private volatile int state = ST_NOT_STARTED;

//优雅关闭
private volatile long gracefulShutdownQuietPeriod;

//优雅关闭超时时间，单位：毫秒 EventLoop 优雅关闭
private volatile long gracefulShutdownTimeout;

//优雅关闭开始时间，单位：毫秒 EventLoop 优雅关闭
private long gracefulShutdownStartTime;

//EventLoop 优雅关闭
private final Promise<?> terminationFuture = new DefaultPromise<Void>(GlobalEventExecutor.INSTANCE);

//state的五种类型
private static final int ST_NOT_STARTED = 1; // 未开始
private static final int ST_STARTED = 2; // 已开始
private static final int ST_SHUTTING_DOWN = 3; // 正在关闭中
private static final int ST_SHUTDOWN = 4; // 已关闭
private static final int ST_TERMINATED = 5; // 已经终止

protected SingleThreadEventExecutor(
        EventExecutorGroup parent, ThreadFactory threadFactory, boolean addTaskWakesUp) {
    this(parent, new ThreadPerTaskExecutor(threadFactory), addTaskWakesUp);
}

protected SingleThreadEventExecutor(
        EventExecutorGroup parent, ThreadFactory threadFactory,
        boolean addTaskWakesUp, int maxPendingTasks, RejectedExecutionHandler rejectedHandler) {
    this(parent, new ThreadPerTaskExecutor(threadFactory), addTaskWakesUp, maxPendingTasks, rejectedHandler);
}

protected SingleThreadEventExecutor(EventExecutorGroup parent, Executor executor, boolean addTaskWakesUp) {
    this(parent, executor, addTaskWakesUp, DEFAULT_MAX_PENDING_EXECUTOR_TASKS, RejectedExecutionHandlers.reject());
}

protected SingleThreadEventExecutor(EventExecutorGroup parent, Executor executor,
                                    boolean addTaskWakesUp, int maxPendingTasks,
                                    RejectedExecutionHandler rejectedHandler) {
    super(parent);
    this.addTaskWakesUp = addTaskWakesUp;
    this.maxPendingTasks = Math.max(16, maxPendingTasks);
    this.executor = ObjectUtil.checkNotNull(executor, "executor");
    taskQueue = newTaskQueue(this.maxPendingTasks);
    rejectedExecutionHandler = ObjectUtil.checkNotNull(rejectedHandler, "rejectedHandler");
}
```

###  7.2. <a name='fetchFromScheduledTaskQueue_'></a>_#fetchFromScheduledTaskQueue_

将定时任务队列 `scheduledTaskQueue` 到达可执行的任务，添加到任务队列 `taskQueue` 中。代码如下:

```java
private boolean fetchFromScheduledTaskQueue() {
    // 获得当前时间
    long nanoTime = AbstractScheduledEventExecutor.nanoTime();
    // 获得指定时间内，定时任务队列首个可执行的任务，并且从队列中移除。
    Runnable scheduledTask  = pollScheduledTask(nanoTime);
    // 不断从定时任务队列中，获得
    while (scheduledTask != null) {
        // 将定时任务添加到 taskQueue 中。若添加失败，则结束循环，返回 false ，表示未获取完所有课执行的定时任务
        if (!taskQueue.offer(scheduledTask)) {
            // 将定时任务添加回 scheduledTaskQueue 中
            // No space left in the task queue add it back to the scheduledTaskQueue so we pick it up again.
            scheduledTaskQueue().add((ScheduledFutureTask<?>) scheduledTask);
            return false;
        }
        // 获得指定时间内，定时任务队列**首个**可执行的任务，并且从队列中移除。
        scheduledTask  = pollScheduledTask(nanoTime);
    }
    // 返回 true ，表示获取完所有可执行的定时任务
    return true;
}
```

###  7.3. <a name='reject_'></a>_#reject_

拒绝执行任务

```java
protected final void reject(Runnable task) {
    rejectedExecutionHandler.rejected(task, this);
}
```

`io.netty.util.concurrent.RejectedExecutionHandler` ，拒绝执行处理器接口。代码如下：

```java
//RejectedExecutionHandler
void rejected(Runnable task, SingleThreadEventExecutor executor);
```

###  7.4. <a name='execute_'></a>_#execute_

执行一个任务。但是方法名无法很完整的体现出具体的方法实现，甚至有一些出入，所以我们直接看源码，代码如下：

```java
    private void execute(Runnable task, boolean immediate) {
        //execute必须在EventLoop线程中
        boolean inEventLoop = inEventLoop();
        //添加任务
        addTask(task);
        //不在EventLoop中
        if (!inEventLoop) {
            //开启线程
            startThread();
            if (isShutdown()) {
                boolean reject = false;
                try {
                    if (removeTask(task)) {
                        reject = true;
                    }
                } catch (UnsupportedOperationException e) {
                }
                if (reject) {
                    reject();
                }
            }
        }

        if (!addTaskWakesUp && immediate) {
            wakeup(inEventLoop);
        }
    }
```

