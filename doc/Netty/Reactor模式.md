# Reactor模式

<!-- vscode-markdown-toc -->
* 1. [什么是Reactor模式](#Reactor)
* 2. [为什么要有用Reacctor](#Reacctor)
* 3. [Reacter的组成](#Reacter)
* 4. [小规模网络的恰当选择——单线程Reactor](#Reactor-1)
* 5. [单线程Reactor的改进——利用线程池](#Reactor-1)
* 6. [高并发下的多线程Reactor使用](#Reactor-1)
* 7. [多线程Reactor实现的一个服务器](#Reactor-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='Reactor'></a>什么是Reactor模式

最近在看《Netty, Redis, ZooKeeper高并发实战》一书，开篇就讲了Java NIO以及大名鼎鼎的Reactor模式，把Reactor讲的神乎其神:

**“只有彻底了解反应器的原理，才能真正构建好高性能的网络应用，才能轻松学习和掌握Netty框架”**

这几个**真正**把我的好奇心彻底勾起来了，那么在此之前，首先要先了解什么是Reactor模式，下面是摘自Reactor的Wiki解释:

```markdown
The reactor design pattern is an event handling pattern for handling service requests 
delivered concurrently to a service handler by one or more inputs. The service handler 
then demultiplexes the incoming requests and dispatches them synchronously to the 
associated request handlers
```

用一句话来概括上面关于Reactor的描述就是:Reactor就是一个事件驱动的事件分发器

##  2. <a name='Reacctor'></a>为什么要有用Reacctor

要了解Reactor的重要性，首先要先了解传统多线程OIO的致命缺陷:

```java
while(true){
  socket = accept();
  handle(socket);
}
```

上面的两句代码是传统的网络应用程序的编写方式，首先通过`accept()`阻塞应用，得到连接之后停止阻塞，继续处理，这也就说明上述代码编写的程序一次只能处理**一个请求**，放到具体应用场景会怎么样不用说了吧，写了这样的代码，就等着被裁掉吧

针对上面的问题，有人提出了一种解决方案，既然我不想服务器阻塞，那我就给每一个连接都创建一个线程不就行了吗?

问得好!怎么说呢，早期的tomcat确实是这么解决的，但是这种解决方法也存在致命的缺陷:**别忘了一个socket只能在一个时刻处理一次IO请求**，即使你CPT(Connection Per Thread)开的再多，socket该阻塞还是会阻塞，难道不是吗？

那怎么解决呢???????

这不，Reactor来了

##  3. <a name='Reacter'></a>Reacter的组成

Reactor的组成其实很简单，基本上由Reactor反应器线程，Handler线程两大部分组成

|组成部分|职责|
|:---:|:---:|
|Reactor反应器|负责IO响应，并且分发到Handler线程|
|Handler处理器|才是真正的，非阻塞的处理连接请求的线程|

##  4. <a name='Reactor-1'></a>小规模网络的恰当选择——单线程Reactor

<div align=center><img src="/assets/r1.webp"/></div>

**单线程一次Reactor的请求流程如下**:

① 服务器端的Reactor是一个线程对象，该线程会启动事件循环，并使用Selector来实现IO的多路复用。注册一个Acceptor事件处理器到Reactor中，Acceptor事件处理器所关注的事件是ACCEPT事件，这样Reactor会监听客户端向服务器端发起的连接请求事件(ACCEPT事件)。
② 客户端向服务器端发起一个连接请求，Reactor监听到了该ACCEPT事件的发生并将该ACCEPT事件派发给相应的Acceptor处理器来进行处理。Acceptor处理器通过accept()方法得到与这个客户端对应的连接(SocketChannel)，然后将该连接所关注的READ事件以及对应的READ事件处理器注册到Reactor中，这样一来Reactor就会监听该连接的READ事件了。或者当你需要向客户端发送数据时，就向Reactor注册该连接的WRITE事件和其处理器。
③ 当Reactor监听到有读或者写事件发生时，将相关的事件派发给对应的处理器进行处理。比如，读处理器会通过SocketChannel的read()方法读取数据，此时read()操作可以直接读取到数据，而不会堵塞与等待可读的数据到来。
④ 每当处理完所有就绪的感兴趣的I/O事件后，Reactor线程会再次执行select()阻塞等待新的事件就绪并将其分派给对应处理器进行处理。

**注意，Reactor的单线程模式的单线程主要是针对于I/O操作而言，也就是所以的I/O的accept()、read()、write()以及connect()操作都在一个线程上完成的。**

但在目前的单线程Reactor模式中，不仅I/O操作在该Reactor线程上，连非I/O的业务操作也在该线程上进行处理了，这可能会大大延迟I/O请求的响应。所以我们应该将非I/O的业务逻辑操作从Reactor线程上卸载，以此来加速Reactor线程对I/O请求的响应

##  5. <a name='Reactor-1'></a>单线程Reactor的改进——利用线程池

<div align=center><img src="/assets/r2.webp"/></div>

与单线程Reactor模式不同的是，添加了一个工作者线程池，并将非I/O操作从Reactor线程中移出转交给工作者线程池来执行。这样能够提高Reactor线程的I/O响应，不至于因为一些耗时的业务逻辑而延迟对后面I/O请求的处理。

使用线程池的优势：
① 通过重用现有的线程而不是创建新线程，可以在处理多个请求时分摊在线程创建和销毁过程产生的巨大开销。
② 另一个额外的好处是，当请求到达时，工作线程通常已经存在，因此不会由于等待创建线程而延迟任务的执行，从而提高了响应性。
③ 通过适当调整线程池的大小，可以创建足够多的线程以便使处理器保持忙碌状态。同时还可以防止过多线程相互竞争资源而使应用程序耗尽内存或失败

##  6. <a name='Reactor-1'></a>高并发下的多线程Reactor使用

<div align=center><img src="/assets/r3.webp"/></div>

Reactor线程池中的每一Reactor线程都会有自己的Selector、线程和分发的事件循环逻辑。
mainReactor可以只有一个，但subReactor一般会有多个。mainReactor线程主要负责接收客户端的连接请求，然后将接收到的SocketChannel传递给subReactor，由subReactor来完成和客户端的通信

**多线程Reactor的工作流程如下**:

① 注册一个Acceptor事件处理器到mainReactor中，Acceptor事件处理器所关注的事件是ACCEPT事件，这样mainReactor会监听客户端向服务器端发起的连接请求事件(ACCEPT事件)。启动mainReactor的事件循环。
② 客户端向服务器端发起一个连接请求，mainReactor监听到了该ACCEPT事件并将该ACCEPT事件派发给Acceptor处理器来进行处理。Acceptor处理器通过accept()方法得到与这个客户端对应的连接(SocketChannel)，然后将这个SocketChannel传递给subReactor线程池。
③ subReactor线程池分配一个subReactor线程给这个SocketChannel，即，将SocketChannel关注的READ事件以及对应的READ事件处理器注册到subReactor线程中。当然你也注册WRITE事件以及WRITE事件处理器到subReactor线程中以完成I/O写操作。Reactor线程池中的每一Reactor线程都会有自己的Selector、线程和分发的循环逻辑。
④ 当有I/O事件就绪时，相关的subReactor就将事件派发给响应的处理器处理。注意，这里subReactor线程只负责完成I/O的read()操作，在读取到数据后将业务逻辑的处理放入到线程池中完成，若完成业务逻辑后需要返回数据给客户端，则相关的I/O的write操作还是会被提交回subReactor线程来完成

##  7. <a name='Reactor-1'></a>多线程Reactor实现的一个服务器

```java
public class MultiThreadReactor {
    static final int SELECTOR_NUM = 5;
    static final int SUB_REACTOR_NUM = 5;
    //通道
    ServerSocketChannel serverSocketChannel;
    //使用的selector数目
    AtomicInteger next = new AtomicInteger(0);
    //多线程的性能优化，多个selector
    Selector[] selectors = new Selector[SELECTOR_NUM];
    //子反应器，同样的为了提高性能，也有好几个
    SubReactor[] subReactors = new SubReactor[SUB_REACTOR_NUM];

    MultiThreadReactor() throws Exception{
        //初始化serverSocketChannel
        serverSocketChannel = ServerSocketChannel.open();
        //设定serverSocketChannel为非阻塞的
        serverSocketChannel.configureBlocking(false);
        //设定serverSocket的端口
        serverSocketChannel.bind(new InetSocketAddress(8080));
        //初始化selector
        for(int i = 0; i < SELECTOR_NUM; i++){
            selectors[i] = Selector.open();
        }
        //初始化子reactor
        for(int i = 0; i < SUB_REACTOR_NUM; i++){
            subReactors[i] = new SubReactor(selectors[i]);
        }
        //因为一个服务器的第一个事件的发生一定是连接事件，所以初始化一个事件为ACCEPT
        //selectors[0]作为第一个selector
        SelectionKey acceptKey = serverSocketChannel.register(selectors[0], SelectionKey.OP_ACCEPT);
        //处理连接信息的我们有专门的AcceptorHandler处理器，直接交给它处理
        acceptKey.attach(new AcceptorHandler());
    }

    public void startReactorService(){
        //各个reactor开始启动
        for(int i = 0; i < SUB_REACTOR_NUM; i++){
            new Thread(subReactors[i]).run();
        }
    }

    class AcceptorHandler implements Runnable{
        public void run(){
            try {
                //获取到连接，建立通道
                SocketChannel socketChannel = serverSocketChannel.accept();
                if(socketChannel != null){
                    //确实存在连接，交给一般处理器解决
                    try {
                        new Handler(selectors[next.get()], socketChannel);
                    }catch (Exception e){
                        e.printStackTrace();
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    class Handler implements Runnable{
        SocketChannel socketChannel;
        SelectionKey selectionKey;
        ByteBuffer input = ByteBuffer.allocate(2048);
        ByteBuffer output = ByteBuffer.allocate(2048);
        static final int READ = 0, WRITE = 1;
        volatile int state = READ;
        //创建一个线程池
        ExecutorService pool = Executors.newFixedThreadPool(5);

        public Handler(Selector selector, SocketChannel socketChannel) throws Exception{
            this.socketChannel = socketChannel;
            this.socketChannel.configureBlocking(false);
            //仅仅取得事件键
            selectionKey = socketChannel.register(selector, 0);
            //将本Handler作为selectionKey的附件，方便事件一起分发
            selectionKey.attach(this);
            //向selectionKey注册Read就绪事件
            selectionKey.interestOps(SelectionKey.OP_READ);
            //唤醒selector
            selector.wakeup();
        }

        public void run(){
            //线程池开始处理异步任务
            pool.execute(new AsyncTask());
        }

        //真正的业务逻辑处理
        public synchronized void asyncRun(){
            try {
                //从通道中读取
                if(state == READ){
                    //已经读取的长度
                    int length = 0;
                    while((length = socketChannel.read(input)) > 0){
                        System.out.println(new String(input.array(), 0, length));
                    }
                    //读完之后，准备写入通道，input需要重置
                    input.reset();
                    //读完之后，注册write就绪事件
                    selectionKey.interestOps(SelectionKey.OP_WRITE);
                    //读完之后，进入发送状态
                    state = WRITE;
                }else if(state == WRITE){
                    socketChannel.write(output);
                    //写完之后，重置output
                    output.reset();
                    //进入读取状态
                    selectionKey.interestOps(SelectionKey.OP_READ);
                    state = READ;
                }
            }catch (Exception e){
                e.printStackTrace();
            }
        }

        //异步任务的内部类
        class AsyncTask implements Runnable{
            public void run(){
                Handler.this.asyncRun();
            }
        }
    }

    class SubReactor implements Runnable{
        Selector selector;

        public SubReactor(Selector selector){
            this.selector = selector;
        }

        public void run(){
            //reactor真正开始工作的地方
            try {
                while(!Thread.interrupted()){
                    //阻塞selector直到有事件被注册进来
                    this.selector.select();
                    //获取已经注册的事件
                    Set<SelectionKey> keySet = selector.selectedKeys();
                    Iterator<SelectionKey> iterator = keySet.iterator();
                    //遍历所有的事件
                    while(iterator.hasNext()){
                        SelectionKey currentKey = iterator.next();
                        //分发给响应的处理器
                        dispatch(currentKey);
                    }
                }
            }catch (Exception e){
                e.printStackTrace();
            }
        }

        public void dispatch(SelectionKey selectionKey){
            //获得selectionKey绑定的事件(Runnable)
            Runnable handler = (Runnable) selectionKey.attachment();
            //如果存在绑定的事件
            if(handler != null){
                handler.run();
            }
        }
    }
}
```