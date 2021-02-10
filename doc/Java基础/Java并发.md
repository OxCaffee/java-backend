# Java并发编程

## 线程

### 线程的生命周期

<div align=center><img src="/assets/thread.png"/></div>

### 终止线程的4种方式

* 程序正常结束
* 标志控制线程结束，通常标志由volatile关键字标记
* interrupt()结束:

1. 情况1: 线程处于阻塞状态: 直接调用interrupt()会抛出一个异常，捕获这个异常同时跳出循环，线程才能结束，**只有调用interrupt()捕获异常，break跳出循环，线程才能正常退出run()**

```java
Thread t = new Thread(() -> {
            while (!Thread.currentThread().isInterrupted()) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                    break;       //quit loop
                }
            }
        });
```

2. 情况2: 线程处于非阻塞状态: 调用interrupt()后，isInterrupted()会返回true，和上面一样跳出循环

* stop()强制终止线程，这种终止线程的方法不是线程安全的，因为当调用stop()方法的时候，所有子模块的锁都会被释放，可能给系统带来无法承受的问题

### _wait()_

* 需要synchronized同步块，调用该方法的线程进入阻塞状态，直到有线程唤醒
* **释放锁**，所以要在同步块中，不然没有锁何谈释放一说
* 一般是Object调用wait()，阻塞当前线程

### _notify()_

* 与wait()搭配起来用
* **随机唤醒一个线程，这个随机性由jdk版本决定**
* 也需要在同步块中

### _yield()_

* 不释放锁
* 让出当前的CPU时间片，使线程回到Runnable就绪状态，可与其他**同优先级** 的线程竞争时间片
* native方法

### _join()_

* 不释放锁
* 在一个线程中调用t.join()就是等t线程先执行完再继续执行本线程
* 可将并行程序实现为串行程序

### _sleep()_

* 不释放锁
* 无法被唤醒，一般sleep时间到才醒来

## 线程之间的数据共享

1. 互斥锁

```java
class Data{
    int data = 0;

    synchronized void increase(){
        this.data++;
    }

    synchronized void decrease(){
        this.data--;
    }
}
```

2. 当所有对象的操作都是一样的时候，共享同一个Runnable

## 阻塞队列

### 什么是阻塞队列

阻塞队列（BlockingQueue）是一个支持两个附加操作的队列。这两个附加的操作是：在队列为空时，获取元素的线程会等待队列变为非空。当队列满时，存储元素的线程会等待队列可用。阻塞队列常用于生产者和消费者的场景，生产者是往队列里添加元素的线程，消费者是从队列里拿元素的线程。阻塞队列就是生产者存放元素的容器，而消费者也只从容器里拿元素。

|方法\处理方式|抛出异常|返回特殊值|一直阻塞|超时退出|
|:---:|:---:|:---:|:---:|:---:|
|插入方法|add(e)| offer(e)| put(e)| offer(e,time,unit)|
|移除方法| remove()| poll()| take()| poll(time,unit)|
|检查方法 |element() |peek() |不可用 |不可用|

### Java中的阻塞队列

* **ArrayBlockingQueue**：是一个用数组实现的有界阻塞队列，此队列按照先进先出（FIFO）的原则对元素进行排序。支持公平锁和非公平锁。【注：每一个线程在获取锁的时候可能都会排队等待，如果在等待时间上，先获取锁的线程的请求一定先被满足，那么这个锁就是公平的。反之，这个锁就是不公平的。公平的获取锁，也就是当前等待时间最长的线程先获取锁】
* **LinkedBlockingQueue**：一个由链表结构组成的有界队列，此队列的长度为Integer.MAX_VALUE。此队列按照先进先出的顺序进行排序。
* **PriorityBlockingQueue**： 一个支持线程优先级排序的无界队列，默认自然序进行排序，也可以自定义实现compareTo()方法来指定元素排序规则，不能保证同优先级元素的顺序。
* **DelayQueue**： 一个实现PriorityBlockingQueue实现延迟获取的无界队列，在创建元素时，可以指定多久才能从队列中获取当前元素。只有延时期满后才能从队列中获取元素。（DelayQueue可以运用在以下应用场景：
  * 1.缓存系统的设计：可以用DelayQueue保存缓存元素的有效期，使用一个线程循环查询DelayQueue，一旦能从 DelayQueue中获取元素时，表示缓存有效期到了。
  * 2.定时任务调度。使用DelayQueue保存当天将会执行的任务和执行时间，一旦从DelayQueue中获取到任务就开始执行，从比如TimerQueue就是使用DelayQueue实现的。）
* **SynchronousQueue**： 一个不存储元素的阻塞队列，每一个put操作必须等待take操作，否则不能添加元素。支持公平锁和非公平锁。SynchronousQueue的一个使用场景是在线程池里。Executors.newCachedThreadPool()就使用了SynchronousQueue，这个线程池根据需要（新任务到来时）创建新的线程，如果有空闲线程则会重复使用，线程空闲了60秒后会被回收。
* **LinkedTransferQueue**： 一个由链表结构组成的无界阻塞队列，相当于其它队列，LinkedTransferQueue队列多了transfer和tryTransfer方法。
* **LinkedBlockingDeque**： 一个由链表结构组成的双向阻塞队列。队列头部和尾部都可以添加和移除元素，多线程并发时，可以将锁的竞争最多降到一半。

## _CountDownLatch_ 和 _Join()_

* 二者都是阻塞当前线程待其他线程执行完毕之后继续执行本线程
* CountDownLatch要比join()更灵活，因为join()只有等待线程执行完成，而CountDownLatch只需计数至0就可以了

## _synchronized_ 锁

### _synchronized_ 锁的是什么

* 作用于普通方法的时候，是给当前调用该方法的实例(this)加锁
* 作用于静态方法的时候，是给对应的Class的所有实例加锁:**因为Class的相关数据存储在metaspace(jdk1.8)，metaspace是全局共享的，因此静态方法锁会锁柱所有调用该方法的线程**
* **锁住的是以该对象为锁的代码块**，该锁有多个队列，当有多个线程同时访问这个锁，会被放到不同的容器中

### _synchronized_ 锁的核心组件

* Wait Set: 哪些线程调用了wait()被阻塞
* Contention List: 所有请求锁的线程会首先放在这个容器中
* Entry List: Contention List中有资格竞争锁的线程被放到这个容器当中
* OnDeck: 任何一个时候，最多一个线程有资格称为锁的持有者，这个线程被称为OnDeck
* Owner: 当前应获得锁的线程
* !Owner: 释放锁

### _synchronized_ 的实现流程

<div align=center><img src="/assets/syn1.png"/></div>

* synchronized是非公平锁，在线程进入Contention List之前，首先先自旋尝试获得锁，也就是“我先试试看插个队”
* 由于可能会有多个线程同时请求锁，会从Contention List中选取一些能够有资格竞争锁的线程进入Entry List，也称“辛德勒的名单”
* Entry List中只有一个最终会得到锁，称为OnDeck，“天选之子”，但是这个“天子”可能随时被还没有进入Contention List的线程自旋时“篡位”，通俗一点就是，“天选之子”还没登基，就被“篡位了”
* Owner线程表示“现任天子”，调用了wait()，会进入Wait Set，即“打入凡间”
* Wait Set中的线程在被notify()或者notifyAll()时，进入Entry Set重新竞争锁

## _CyclicBarrier_ 循环栅栏

* CyclicBarrier与CountDownLatch实现的功能是一样的，但是CyclicBarrier更加精简
* CountDownLatch通过countDown()来使计数器减，而CyclicBarrier通过await()使计数器减
* CountDownLatch基于AQS共享模式设计而成，而CyclicBarrier基于ReentrantLock和Condition设计而成
* CyclicBarrier的流程如下:

<div align=center><img src="/assets/cycba.png"></div>

```java
    //内置线程全部都执行完毕之后需要执行的后置任务
    Runnable task = new Runnable() {
        @Override
        public void run() {
            System.out.println("5 threads step forth together...");
        }
    };

    CyclicBarrier barrier = new CyclicBarrier(5, task);

    //计数减用barrier.await()
```

## _Semaphore_ 信号量

### _Semaphore_ 与 _ReentrantLock_ 的区别

* Semaphore信号量为多个线程协作提供了更强大的控制方法，无论是ReentrantLock还是Synchronized一次都只允许一个线程访问一个资源。信号量允许多个线程同时访问同一个资源
* Semaphore.acquire()默认为可响应中断锁，与ReentrantLock.lockInterruptibly()作用一致
* Semaphore还有可轮询锁和定时锁的功能

### _Semaphore Demo_

```java
    Semaphore semaphore = new Semaphore(5, true);   //公平锁

    private class Task extends Thread{
        @Override
        public void run(){
            try {
                semaphore.acquire();
                System.out.println(Thread.currentThread().getName() + " acquired a lock.");
                //Thread.sleep(3000);
                /**
                 * logic code
                 */
            } catch (InterruptedException e) {
                e.printStackTrace();
            }finally {
                semaphore.release();
                System.out.println(Thread.currentThread().getName() + " released a lock.");
            }
        }
    }
```

## _ReadWriteLock_ 读写锁

synchronized读与读互斥，但是读写锁可以保证仅有一个线程在写，其他线程可以同时读

### _ReentrantReadWriteLock_

#### 获取锁的方式

* 公平模式
* 非公平模式

#### 可重入性

什么是可重入锁，不可重入锁呢？"重入"字面意思已经很明显了，就是可以重新进入。可重入锁，就是说一个线程在获取某个锁后，还可以继续获取该锁，即允许一个线程多次获取同一个锁。比如synchronized内置锁就是可重入的，如果A类有2个synchornized方法method1和method2，那么method1调用method2是允许的。显然重入锁给编程带来了极大的方便。假如内置锁不是可重入的，那么导致的问题是：1个类的synchornized方法不能调用本类其他synchornized方法，也不能调用父类中的synchornized方法。与内置锁对应，JDK提供的显示锁ReentrantLock也是可以重入的，这里通过一个例子着重说下可重入锁的释放需要的事儿。

```java
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class MyReentrantReadWriteLock {

    public static void main(String[] args) throws InterruptedException {
        final ReentrantReadWriteLock  lock = new ReentrantReadWriteLock ();
        Thread t = new Thread(new Runnable() {
            @Override
            public void run() {
                lock.writeLock().lock();
                System.out.println("Thread real execute");
                lock.writeLock().unlock();
            }
        });

        lock.writeLock().lock();
        lock.writeLock().lock();
        t.start();
        Thread.sleep(200);

        System.out.println("release once");
        lock.writeLock().unlock();
    }

}
```

测试结果

```java
release once
```

因为上面写锁只释放了一次，因此主线程死锁了

#### 锁降级

```markdown
在不允许中间写入的情况下，写入锁可以降级为读锁吗？读锁是否可以升级为写锁，优先于其他等待的读取或写入操作？简言之就是说，锁降级：从写锁变成读锁；锁升级：从读锁变成写锁，ReadWriteLock是否支持呢？
```

* 锁升级demo

```java
public void doDownGrades(){
        lock.writeLock().lock();
        System.out.println("write lock...");
        lock.readLock().lock();
        System.out.println("read lock...");
        lock.readLock().unlock();
        lock.writeLock().unlock();
    }
```

测试结果

```java
write lock...
```

很显然，不能进行锁升级，否则会卡死线程

* 锁降级demo

```java
public void doDownGrades(){
        lock.writeLock().lock();
        System.out.println("write lock...");
        lock.readLock().lock();
        System.out.println("read lock...");
        lock.readLock().unlock();
        lock.writeLock().unlock();
    }
```

测试结果

```java
write lock...
read lock...
```

可以进行锁降级

#### 读写测试

```java
public void doRead(Thread t){
        lock.readLock().lock();
        boolean isWriteLock = lock.isWriteLocked();
        if(!isWriteLock){
            System.out.println("current lock is read lock...");
        }

        try{
            for(int i = 0; i < 5; i++){
                Thread.sleep(2000);
                System.out.println(t.getName() + " is reading...");
            }
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            lock.readLock().unlock();
            System.out.println("release read lock");
        }
    }

public void doWrite(Thread t){
        lock.writeLock().lock();
        boolean isWriteLock = lock.isWriteLocked();
        if(isWriteLock){
            System.out.println("current lock is write lock...");
        }

        try {
            Thread.sleep(2000);
            System.out.println(t.getName() + " is writing....");
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            lock.writeLock().unlock();
            System.out.println("release write lock...");
        }
    }

public static void main(String[] args){
        MyReadWriteLock myReadWriteLock = new MyReadWriteLock();

        ExecutorService service = Executors.newCachedThreadPool();

        service.execute(new Thread(()->{
            myReadWriteLock.doRead(Thread.currentThread());
        }));

        service.execute(new Thread(()->{
            myReadWriteLock.doRead(Thread.currentThread());
        }));

        service.execute(new Thread(()->{
            myReadWriteLock.doRead(Thread.currentThread());
        }));

        service.execute(new Thread(()->{
            myReadWriteLock.doWrite(Thread.currentThread());
        }));

        service.execute(new Thread(()->{
            myReadWriteLock.doWrite(Thread.currentThread());
        }));
    }
```

测试结果

```java
current lock is read lock...
current lock is read lock...
current lock is read lock...
pool-1-thread-1 is reading...
pool-1-thread-2 is reading...
pool-1-thread-3 is reading...
pool-1-thread-1 is reading...
pool-1-thread-3 is reading...
pool-1-thread-2 is reading...
pool-1-thread-1 is reading...
pool-1-thread-2 is reading...
pool-1-thread-3 is reading...
pool-1-thread-1 is reading...
pool-1-thread-3 is reading...
pool-1-thread-2 is reading...
pool-1-thread-1 is reading...
release read lock
pool-1-thread-3 is reading...
pool-1-thread-2 is reading...
release read lock
release read lock
current lock is write lock...
pool-1-thread-4 is writing....
release write lock...
current lock is write lock...
pool-1-thread-5 is writing....
release write lock...
```

### 读/写锁之间的互斥关系

读锁和写锁之间是互斥的关系，也就是说，读线程一定可以看到上一次写锁释放后更新的内容

## _AQS_ 和 _CAS_

### _AQS_

AQS抽象同步队列是许多同步器的核心，其内部维护了一个state和FIFO队列，当多线程争夺锁的时候，失败线程会进入到这个FIFO队列中等待唤醒

获取state的方法只有三种:

* getState()
* setState()
* compareAndSetState()

AQS还定义了两种资源共享的方式:

* 独占模式Exclusive: ReentrantLock就是个例子
* 共享模式Share: Semaphore和CountDownLatch

AQS实现是ABS核心(以state状态为计数)，以ReentrantLock为例，初始设置state为0，当lock()时，会调用tryAcquire()来使state加1，tryRelease()会使state减1，当state=0时才表示锁当前是空闲的状态，要记住，**重入多少次就释放多少次，否则state不归零会使程序死锁**

### _CAS_

CAS(Compare And Swap/Set)，其包含三个参数CAS(V,E, N)，V表示要更新的数值，E表示期望的数值，N表示更新后的数值，只有比较V和E相等，才会更新为N

CAS是抱着乐观的心态进行操作，即认为每次操作都会成功，但是多个线程同时对一个数据进行CAS操作的时候，只有一个数据会操作成功，其他的线程并不会因此终止而是可以继续进行CAS尝试

AtomicXXX里面对于多线程同时操作一个实例时，已经取得实例操作权的线程具有排他性，也就是说，**其他取得操作权失败的线程会自旋来等待**

相比于synchronized，CAS是一种非阻塞的方法

## _ABA_ 问题

CAS操作会导致ABA问题，部分乐观锁为了解决ABA问题，将每个实例加上了Version，即版本号，版本号只会大不会小，从而解决了ABA问题
