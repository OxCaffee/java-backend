# SpringCloud OpenFeign源码解析——openfeign代理

<!-- vscode-markdown-toc -->
* 1. [OpenFeign的大致工作流程](#OpenFeign)
* 2. [回忆FeignClientFactoryBean#getTarget](#FeignClientFactoryBeangetTarget)
* 3. [Feign#target](#Feigntarget)
	* 3.1. [SychronousMethodHandler——Rest请求的真正发起点](#SychronousMethodHandlerRest)
	* 3.2. [ReflectiveFeign](#ReflectiveFeign)
	* 3.3. [ParseHandlersByName](#ParseHandlersByName)
* 4. [总结](#)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='OpenFeign'></a>OpenFeign的大致工作流程

<div align=center><img src="/assets/of2.png"/></div>

##  2. <a name='FeignClientFactoryBeangetTarget'></a>回忆FeignClientFactoryBean#getTarget

在前面的源码解析中，我们知道，所有@FeignClient注解配置的类都会被封装为BeanDefinition，装配在容器中，要想获取到相应的bean，需要通过FeignClientFactoryBean中的getTarget方法，然而getTarget获得的bean是什么类型的，下面我们继续深入探索一下`getTarget` 方法，先上源码：

```java
// <T> T getTarget
FeignContext context = beanFactory != null ? beanFactory.getBean(FeignContext.class)
    	: applicationContext.getBean(FeignContext.class);
```

首先获取FeignContext，如果beanFactory获取不到，那么就从applicationContext中获取。在上一节源码的介绍中我们可以知道，FeignContext是Feign的工厂容器，其父容器为Spring ApplicationContext，祖先容器为Bootstrap ApplicationContext。一个FeignContext对应多个FeignClient，FeignClient在FeignContext中存储的Key为`FeignContext-{client-name}` 。

在获取到FeignContext后，需要通过该context创建Feign.Builder

```java
// <T> T getTarget
Feign.Builder builder = feign(context);
```

这里调用了一个`#feign()` 方法，继续深入:

```java
	protected Feign.Builder feign(FeignContext context) {
		//为当前的feign上下文创建一个logger工厂
		FeignLoggerFactory loggerFactory = get(context, FeignLoggerFactory.class);
		Logger logger = loggerFactory.create(type);

        //核心创建方法get
		Feign.Builder builder = get(context, Feign.Builder.class)
				// required values
				.logger(logger)
				.encoder(get(context, Encoder.class))
				.decoder(get(context, Decoder.class))
				.contract(get(context, Contract.class));


		configureFeign(context, builder);
		applyBuildCustomizers(context, builder);

		return builder;
	}
```

继续深入创建Feign.Builder的核心方法`#get` ：

```java
	protected <T> T get(FeignContext context, Class<T> type) {
		T instance = context.getInstance(contextId, type);
		if (instance == null) {
			throw new IllegalStateException("No bean found of type " + type + " for " + contextId);
		}
		return instance;
	}
```

可以看到，**Feign.Builder本质还是由FeignContext创建,而FeignContext本质还是一个NamedContextFactory** 。

在获得Feign.Builder之后，我们继续回到`#getTarget` ，看看继续执行了什么操作。

```java
//<T> T getTarget
		if (!StringUtils.hasText(url)) {

			if (LOG.isInfoEnabled()) {
				LOG.info("For '" + name + "' URL not provided. Will try picking an instance via load-balancing.");
			}
			if (!name.startsWith("http")) {
				url = "http://" + name;
			}
			else {
				url = name;
			}
			url += cleanPath();
			return (T) loadBalance(builder, context, new HardCodedTarget<>(type, name, url));
		}
```

如果相应的FeignClient没有配置url属性(不管是配置文件还是注解配置)，首先给url赋值，**即默认保证url前部有http://字段** ,修正url之后执行`#loadBalance` 方法，这是一个非常重要的方法，我们继续看看这个方法做了哪些工作。

```java
	protected <T> T loadBalance(Feign.Builder builder, FeignContext context, HardCodedTarget<T> target) {
        //创建HTTP客户端
		Client client = getOptional(context, Client.class);
        
        //如果client创建成功
		if (client != null) {
            //设置当前FeignClientFactoryBean的client为已经创建的client
			builder.client(client);
            
            //重点来了，创建代理对象
			Targeter targeter = get(context, Targeter.class);
            //配置代理工厂factory为当前FeignClientFactoryBean
			return targeter.target(this, builder, context, target);
		}

		throw new IllegalStateException(
				"No Feign Client for loadBalancing defined. Did you forget to include spring-cloud-starter-loadbalancer?");
	}
```

**来了来了，终于来了，本节的重点内容，代理对象的创建!!!!!**

不过先别急，我们先继续把`#getTarget` 看完再探究后面的内容，继续回到上面，如果我们没有配置url怎么办？

```java
//<T> T getTarget
		if (StringUtils.hasText(url) && !url.startsWith("http")) {
			url = "http://" + url;
		}
		String url = this.url + cleanPath();
		Client client = getOptional(context, Client.class);
		if (client != null) {
			if (client instanceof FeignBlockingLoadBalancerClient) {
				// not load balancing because we have a url,
				// but Spring Cloud LoadBalancer is on the classpath, so unwrap
				client = ((FeignBlockingLoadBalancerClient) client).getDelegate();
			}
			if (client instanceof RetryableFeignBlockingLoadBalancerClient) {
				// not load balancing because we have a url,
				// but Spring Cloud LoadBalancer is on the classpath, so unwrap
				client = ((RetryableFeignBlockingLoadBalancerClient) client).getDelegate();
			}
			builder.client(client);
		}
```

**由于我们已经配置了url，那么就无法享受到load balancing的好处了，只能乖乖的使用BlockingLoadBalanceClient** ，默认情况下有两种blocking client，一种是FeignBlockingLoadBalancerClient和RetryableFeignBlockingLoadBalancerClient。

client创建好，还是老步骤，继续为当前FeignClientFactoryBean配置代理。

```java
//<T> T getTarget		
Targeter targeter = get(context, Targeter.class);
return (T) targeter.target(this, builder, context, new HardCodedTarget<>(type, name, url));
```

##  3. <a name='Feigntarget'></a>Feign#target

在前面FeignClientFactoryBean#getTarget，无论我们有没有url，都会执行`targeter#target` 方法，我们顺势一找，唉，这个Targeter是个接口啊？这我们怎么分析`#target` 执行了什么操作？我们可以看看Targeter的其中一个实现类DefaultTargeter:

```java
class DefaultTargeter implements Targeter {

	@Override
	public <T> T target(FeignClientFactoryBean factory, Feign.Builder feign, FeignContext context,
			Target.HardCodedTarget<T> target) {
		return feign.target(target);
	}
}
```

铁汁们，大的来了，这里发现调用了`Feign.Builder.#feign` 方法，我们继续看看:

```java
    public <T> T target(Target<T> target) {
      return build().newInstance(target);
    }

	//获取Feign
    public Feign build() {
		...

        //<1>
        SynchronousMethodHandler.Factory synchronousMethodHandlerFactory =
            new SynchronousMethodHandler.Factory(client, retryer, requestInterceptors, logger,
                logLevel, decode404, closeAfterDecode, propagationPolicy, forceDecoding);
        //<2>
        ParseHandlersByName handlersByName =
            new ParseHandlersByName(contract, options, encoder, decoder, queryMapEncoder,
                errorDecoder, synchronousMethodHandlerFactory);
        //<3>
        return new ReflectiveFeign(handlersByName, invocationHandlerFactory, queryMapEncoder);
     }
  }
```

下面我们分三步走，分别看看SynchronousMethodHandler和ParseHandlerByName，以及ReflectiveFeign都干了什么

###  3.1. <a name='SychronousMethodHandlerRest'></a>SychronousMethodHandler——Rest请求的真正发起点

SynchronousMethodHandler继承了MethodHandler接口，注意这不是java中的MethodHandler，是Feign自己封装的MethodHandler，SynchronousMethodHandler重写了`#invoke` 方法，定义如下：

```java
  @Override
  public Object invoke(Object[] argv) throws Throwable {
	//<1>
    RequestTemplate template = buildTemplateFromArgs.create(argv);
    Options options = findOptions(argv);
    Retryer retryer = this.retryer.clone();
    while (true) {
      try {
        //<2>
        return executeAndDecode(template, options);
      } catch (RetryableException e) {
        try {
          retryer.continueOrPropagate(e);
        } catch (RetryableException th) {
          Throwable cause = th.getCause();
          if (propagationPolicy == UNWRAP && cause != null) {
            throw cause;
          } else {
            throw th;
          }
        }
        if (logLevel != Logger.Level.NONE) {
          logger.logRetry(metadata.configKey(), logLevel);
        }
        continue;
      }
    }
  }
```

* `<1>` 处出现了RequestTemplate，这个类是**HTTP代理的请求创建器** 
* `<2>` 处的代码比较重要，首先包裹在一个死循环里，一直尝试`executeAndDecode` 方法，下面深入探究：

```java
  Object executeAndDecode(RequestTemplate template, Options options) throws Throwable {
    //对于所有的RequestInterceptor，执行interceptor.apply(template)方法
    Request request = targetRequest(template);

    if (logLevel != Logger.Level.NONE) {
      logger.logRequest(metadata.configKey(), logLevel, request);
    }

    Response response;
    long start = System.nanoTime();
    try {
      //执行有附加选项的请求，获得response
      response = client.execute(request, options);
      //构建response
      response = response.toBuilder()
          .request(request)
          .requestTemplate(template)
          .build();
    } catch (IOException e) {
      if (logLevel != Logger.Level.NONE) {
        logger.logIOException(metadata.configKey(), logLevel, e, elapsedTime(start));
      }
      throw errorExecuting(request, e);
    }
    long elapsedTime = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - start);

	//解码response
    if (decoder != null)
      return decoder.decode(response, metadata.returnType());
	
    //执行异步处理reponse
    CompletableFuture<Object> resultFuture = new CompletableFuture<>();
    asyncResponseHandler.handleResponse(resultFuture, metadata.configKey(), response,
        metadata.returnType(),
        elapsedTime);

    try {
      if (!resultFuture.isDone())
        throw new IllegalStateException("Response handling not done");

      return resultFuture.join();
    } catch (CompletionException e) {
      Throwable cause = e.getCause();
      if (cause != null)
        throw cause;
      throw e;
    }
  }
```

可以总结一下`executeAndDecode` 完成的操作：

1. 将所有的RequestInterceptor都应用到当前请求创建器当中，也就是当前RequestTemplate创建的请求都能被拦截器所感知
2. 构建Request
3. 发送该Request
4. 获取Response
5. 解码Response
6. 交由CompletableFuture完成异步处理此次请求

**上面的出现的情况是请求能够被正确响应的情况，那么如果请求没有被响应呢或者响应出现错误呢?** 这个时候Retryer的作用来了。
Retryer更加关注的是请求错误的情形，这个源码我们以后再讨论。

###  3.2. <a name='ReflectiveFeign'></a>ReflectiveFeign

ReflectiveFeign继承了Feign，其中比较重要的方法是`#newInstance` 方法：

```java
 /**
  * ReflectiveFeign.newInstance 生成proxy代理实现的主逻辑
  * @param Target<T> 在Feign.target中传入的代理目标Class与host url参数
  */
  public <T> T newInstance(Target<T> target) {
    //核心逻辑入口，反射解析目标Class，得到MethodHandler集合
    Map<String, MethodHandler> nameToHandler = targetToHandlersByName.apply(target);
    Map<Method, MethodHandler> methodToHandler = new LinkedHashMap<Method, MethodHandler>();
    List<DefaultMethodHandler> defaultMethodHandlers = new LinkedList<DefaultMethodHandler>();

    for (Method method : target.type().getMethods()) {
      //Object的方法（如toString之类的）就无视
      if (method.getDeclaringClass() == Object.class) {
        continue;
      } 
      //Default方法处理
      else if(Util.isDefault(method)) {
        DefaultMethodHandler handler = new DefaultMethodHandler(method);
        defaultMethodHandlers.add(handler);
        methodToHandler.put(method, handler);
      } 
      //Http请求代理方法
      else {
        methodToHandler.put(method, nameToHandler.get(Feign.configKey(target.type(), method)));
      }
    }

    //主要逻辑，生成proxy，实现其中的Rest请求方法。
    //InvocationHandler实现类为FeignInvocationHandler
    InvocationHandler handler = factory.create(target, methodToHandler);
    T proxy = (T) Proxy.newProxyInstance(target.type().getClassLoader(), new Class<?>[]{target.type()}, handler);

    //处理其他默认方法,绑定到Proxy上
    for(DefaultMethodHandler defaultMethodHandler : defaultMethodHandlers) {
      defaultMethodHandler.bindTo(proxy);
    }
    return proxy;
  }
```

代理对象的构建主要由三块内容

1、构建Method到MethodHandler的映射关系，后面调用代理的对象的时候将会根据Method找到MethodHandler然后调用MethodHandler的invoke方法，而MethodHandler将包含发起http请求的实现。

2、jdk动态代理需要提供InvocationHandler，这个大家比较熟悉了。而InvocationHandler将由InvocationHandlerFactory的create方法实现

3、通过Proxy.newProxyInstance方法，生成proxy对象。

###  3.3. <a name='ParseHandlersByName'></a>ParseHandlersByName

ParseHandlersByName的作用就是我们传入Target（封装了我们的模拟接口，要访问的域名），返回这个接口下的各个方法，对应的执行HTTP请求需要的一系列信息。
 结果`Map<String, MethodHandler>`的key是这个接口中的方法名字，MethodHandler则是包含此方法执行需要的各种信息。

ParseHandlerByName含有以下组件：

```java
    private final Contract contract;
    private final Options options;
    private final Encoder encoder;
    private final Decoder decoder;
    private final ErrorDecoder errorDecoder;
    private final QueryMapEncoder queryMapEncoder;
    private final SynchronousMethodHandler.Factory factory;
```

**这些组件的作用是处理每个HTTP执行前后的事情** ，比如:

* Contract：作用是将我们传入的接口进行解析验证，看注解的使用是否符合规范，然后返回给我们接口上各种相应的元数据。所以叫合约

* Options：封装Request请求的 连接超时=默认10s ，读取超时=默认60s

* Encoder：怎么把我们的请求编码

* Decoder：怎么把我们执行HTTP请求后得到的结果解码为我们定义的类型

* ErrorDecoder：怎么在我们执行HTTP请求后得到的错误(既不是2xx的状态码)解码为我们定义的类型

##  4. <a name=''></a>总结

最后我们总结一下Feign的代理生成过程:

1. FeignClientFactoryBean在执行getTarget()方法获取bean的时候会调用Targeter.target()方法为该bean配置代理
2. 继续调用Feign.Builder.feign()
3. 继续调用Feign.Builder.build()方法构建Feign
   1. SynchronousMethodHandler为每个MehtodHandler增强，创建HTTP Client，发送HTTP请求，获取Response，异步处理Response
   2. ParseHandlersByName将Method转化为Map类型，实现name和MethodHandler的映射
   3. ReflectiveFeign中的newInstance方法真正创建代理，并返回生成的代理对象