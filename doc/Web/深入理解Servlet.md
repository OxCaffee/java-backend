# 深入理解Servlet

<!-- vscode-markdown-toc -->
* 1. [前言](#)
	* 1.1. [什么是Servlet](#Servlet)
	* 1.2. [什么是Servlet Container](#ServletContainer)
* 2. [一个Servlet与用户交互的例子](#Servlet-1)
* 3. [Servlet的生命周期](#Servlet-1)
	* 3.1. [_#init()_](#init_)
	* 3.2. [_#service()_](#service_)
	* 3.3. [_#doGet()_](#doGet_)
	* 3.4. [_#doPost()_](#doPost_)
	* 3.5. [_#destroy()_](#destroy_)
* 4. [Servlet多线程问题](#Servlet-1)
* 5. [异步处理](#-1)
	* 5.1. [如何使用Servlet3.0的异步特性](#Servlet3.0)
	* 5.2. [异步请求案例](#-1)
	* 5.3. [Servlet异步处理流程理流程](#Servlet-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

互联网发展速度越来越快，技术的更新速度也是越来越快，各种眼花缭乱的框架让人目不暇接，但是究其本质，还是底层的一些基本的技术，只不过用了一些高级特性和设计思维，将他们整合起来，组成一个框架。对于Web开发者而言，Servlet的贯穿了开发的始终，虽然现在没有哪一个公司会用纯粹的Servlet来构建整个Web应用，从而给人一种错觉，那就是Servlet已经过时了，不需要在学习了，但是要想更好地理解各种MVC框架，Servlet的学习是必不可少的。

###  1.1. <a name='Servlet'></a>什么是Servlet

Servlet是Java的一个Web组件，由Web容器管理，和其他Java组件一样，能够被编译成平台独立的字节码(虚拟机)，并可以被动态的加载并运行在Web服务器上。Web容器(Container)，有时也被称为Servlet引擎(Servlet Engine)，是提供Servlet功能的Web Server的扩展。Servet与用户交互是基于Request和Response的。

###  1.2. <a name='ServletContainer'></a>什么是Servlet Container

Servlet Container是Web Server或者Web Application的一部分，负责提供网络服务(Network Service)，例如: 负责Request和Response的发送；解码MIME请求；规格化MIME响应等等。 **Servlet Container还负责Servlet的生命周期管理** 。

##  2. <a name='Servlet-1'></a>一个Servlet与用户交互的例子

1. 用户client(通常是浏览器Browser)访问服务器(Web Server)并发送一次HTTP请求(Request)
2. 该请求(Request)被服务器接收，并转发到Servlet Container。Servlet Container能够运行在单一服务器的单一进程，单一服务器的不同进程或者不同的服务器中，来处理该Request
3. Servlet Container根据Servlet的配置情况来决定使用哪一个Servlet来处理该Request
4. Servlet使用Request中的数据，利用相应的逻辑来决定发送什么样的Response，返还给用户
5. 当Servlet处理完Request后，Servlet Container确保Response被刷新，并返回控制权给服务器

##  3. <a name='Servlet-1'></a>Servlet的生命周期

Servlet 生命周期可被定义为从创建直到毁灭的整个过程。以下是 Servlet 遵循的过程：

- Servlet 初始化后调用 **init ()** 方法。
- Servlet 调用 **service()** 方法来处理客户端的请求。
- Servlet 销毁前调用 **destroy()** 方法。
- 最后，Servlet 是由 JVM 的垃圾回收器进行垃圾回收的。

**Servlet Container负责Servlet的加载(Loading)和初始化(Initialization)。Servlet的加载和初始化可以发生在Container创建时，也可以延迟加载到Servlet真正被需要用到的时候。**

###  3.1. <a name='init_'></a>_#init()_

init 方法被设计成只调用一次。它在第一次创建 Servlet 时被调用，在后续每次用户请求时不再调用。因此，它是用于一次性初始化，就像 Applet 的 init 方法一样。

Servlet 创建于用户第一次调用对应于该 Servlet 的 URL 时，但是您也可以指定 Servlet 在服务器第一次启动时被加载。

**当用户调用一个 Servlet 时，就会创建一个 Servlet 实例，每一个用户请求都会产生一个新的线程，适当的时候移交给 doGet 或 doPost 方法。init() 方法简单地创建或加载一些数据，这些数据将被用于 Servlet 的整个生命周期。**

init 方法的定义如下：

```java
public void init() throws ServletException {
  // 初始化代码...
}
```

###  3.2. <a name='service_'></a>_#service()_

service() 方法是执行实际任务的主要方法。Servlet 容器（即 Web 服务器）调用 service() 方法来处理来自客户端（浏览器）的请求，并把格式化的响应写回给客户端。

每次服务器接收到一个 Servlet 请求时，服务器会产生一个新的线程并调用服务。service() 方法检查 HTTP 请求类型（GET、POST、PUT、DELETE 等），并在适当的时候调用 doGet、doPost、doPut，doDelete 等方法。

下面是该方法的特征：

```java
public void service(ServletRequest request, 
                    ServletResponse response) 
      throws ServletException, IOException{
}
```

service() 方法由容器调用，service 方法在适当的时候调用 doGet、doPost、doPut、doDelete 等方法。所以，您不用对 service() 方法做任何动作，您只需要根据来自客户端的请求类型来重写 doGet() 或 doPost() 即可。

doGet() 和 doPost() 方法是每次服务请求中最常用的方法。下面是这两种方法的特征。

###  3.3. <a name='doGet_'></a>_#doGet()_

GET 请求来自于一个 URL 的正常请求，或者来自于一个未指定 METHOD 的 HTML 表单，它由 doGet() 方法处理。

```java
public void doGet(HttpServletRequest request,
                  HttpServletResponse response)
    throws ServletException, IOException {
    // Servlet 代码
}
```

###  3.4. <a name='doPost_'></a>_#doPost()_

POST 请求来自于一个特别指定了 METHOD 为 POST 的 HTML 表单，它由 doPost() 方法处理。

```java
public void doPost(HttpServletRequest request,
                   HttpServletResponse response)
    throws ServletException, IOException {
    // Servlet 代码
}
```

###  3.5. <a name='destroy_'></a>_#destroy()_

destroy() 方法只会被调用一次，在 Servlet 生命周期结束时被调用。destroy() 方法可以让您的 Servlet 关闭数据库连接、停止后台线程、把 Cookie 列表或点击计数器写入到磁盘，并执行其他类似的清理活动。

在调用 destroy() 方法之后，servlet 对象被标记为垃圾回收。destroy 方法定义如下所示：

```java
public void destroy() {
    // 终止化代码...
}
```

##  4. <a name='Servlet-1'></a>Servlet多线程问题

* Servlet采用多线程来处理多个请求同时访问，Servelet容器维护了一个线程池来服务请求。

* 线程池实际上是等待执行代码的一组线程叫做工作者线程(Worker Thread)，Servlet容器使用一个调度线程来管理工作者线程(Dispatcher Thread)。

* 当容器收到一个访问Servlet的请求，调度者线程从线程池中选出一个工作者线程，将请求传递给该线程，然后由该线程来执行Servlet的service方法。

* 当这个线程正在执行的时候，容器收到另外一个请求，调度者线程将从池中选出另外一个工作者线程来服务新的请求，容器并不关系这个请求是否访问的是同一个Servlet还是另外一个Servlet。

* 当容器同时收到对同一Servlet的多个请求，那这个Servlet的service方法将在多线程中并发的执行。
* 多线程下每个线程对**局部变量**都会有自己的一份copy，这样对局部变量的修改只会影响到自己的copy而不会对别的线程产生影响，所以这是线程安全的。
* 但是对于**实例变量**来说，由于servlet在Tomcat中是以单例模式存在的，所有的线程共享实例变量。**多个线程对共享资源的访问就造成了线程不安全问题。**

**解决Servlet多线程安全的措施如下:**

* 变量本地化，尽量使用局部变量而不是使用实例变量， **避免使用实例变量是Servlet线程安全的最佳实践!!!**
* synchronized加锁， **不推荐，会造成性能严重损耗**
* 实现SingleThreadModel接口， **已过时，因为SingleThreadModel并不能真正解决线程安全问题**
* 使用线程安全的集合，比如用Vector代替List，用Hashtable代替HashMap
* 继承HttpServlet，HttpServlet是线程安全的

##  5. <a name='-1'></a>异步处理

有时候Filter或Servlet需要等待资源或事件才能继续执行操作，例如Servlet需要等待JDBC连接或者来自其他方向的触发事件等。 **Servlet在等待这种资源来临的时候是处于阻塞的状态，这种在Servlet内部等待进而阻塞的行为往往会造成严重的资源浪费，有时候会成为系统性能的瓶颈** 。Servlet3.0新特性所支持的异步处理，能够让Servlet不需要阻塞等待而继续执行任务，直到所需要的结果返回给Servlet。

###  5.1. <a name='Servlet3.0'></a>如何使用Servlet3.0的异步特性

1. 声明Servlet，增加asyncSupported属性，开启异步支持。

   ```java
   @WebServlet(urlPatterns = "/AsyncLongRunningServlet", asyncSupported = true)
   ```

2. 通过Request获取异步上下文AsyncContext

   ```java
   AsyncContext asyncContext = request.startAsync();
   ```

3. 开启业务逻辑处理线程，并将AsyncContext传递给业务线程。

   ```java
   executor.execute(new AsyncRequestProcessor(asyncCtx, secs));
   ```

4. 在异步业务逻辑处理线程中，通过asyncContext获取request和response，处理对应的业务

5. 业务逻辑处理线程处理完成之后，调用AsyncContext的complete方法，从而结束该次异步请求

   ```java
   asyncContext.complete();
   ```

###  5.2. <a name='-1'></a>异步请求案例

1. **声明Servlet，处理前来的HTTP请求，开启异步特性**

   ```java
   //接收HTTP请求的Servlet
   @WebServlet(urlPatterns = "/async", asyncSupported = true)
   public class HTTPReceiverServelet extends HttpServlet{
       private static final long serialVersionUID = 1L;
       
       protected void doGet(HttpServletRequest request, HttpServletResponse response){
       	/** 前置业务逻辑 **/
           
           request.setAttribute("org.apache.catalina.ASYNC_SUPPORTED", true);
           //请求的时间
           int time = Integer.valueOf(request.getParameter("time"));
           //异步开始
           AsyncContext asyncContext = request.startAsync();
           //添加异步监听器，非常重要，异步处理的完成是通过监听器察觉到的
           asyncContext.addListener(new MyAsyncListener());
           //异步处理超时时间，针对有时间要求的异步请求
           asyncContext.setTimeout(1000);
           //这里就是请求隔离的的开始了，将tomcat中的线程转交给业务线程池处理
           ThreadPoolExecutor executor = (ThreadPoolExecutor) request.getServletContext().getAttrbute("executor");
           executor.execute(new MyAsyncProcessor(asyncContext, time))
       }
   }
   ```

2. **异步监听器，负责监听异步任务的完成**

   ```java
   @WebListener
   public class MyAsyncListener extends AsyncListener{
       @Override
       public void onComplete(...){}
       
       @Override
       public void onError(...){}
       
       @Override
       public void onStartAsync(...){}
       
       @Override
       public void onTimeout(...){}
   }
   ```

3. **转交给的业务处理器**

   ```java
   public class MyAsyncProcessor implements Runnable{
       private AsyncContext asyncContext;
       private int time;
       
       //构造初始化
       public MyAsyncProcessor(AsyncContext asyncContext, int time){
           this.asyncContext = asyncContext;
           this.time = time;
       }
       
       @Override
       public void run(){
           //业务逻辑处理
       }
   }
   ```

###  5.3. <a name='Servlet-1'></a>Servlet异步处理流程理流程

<div align=center><img src="/assets/servelet1.png"/></div>

