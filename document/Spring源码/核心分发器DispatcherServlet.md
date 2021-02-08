# 核心分发器 _DispatcherServlet_

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_DispatcherServlet_ 的工作流程](#DispatcherServlet_)
* 3. [_DispatcherServlet_ 族谱](#DispatcherServlet_-1)
	* 3.1. [_HttpServletBean_](#HttpServletBean_)
	* 3.2. [_FrameworkServlet_](#FrameworkServlet_)
		* 3.2.1. [_FrameworkServlet_ 关键属性](#FrameworkServlet_-1)
		* 3.2.2. [_#initServletBean_ ——真正初始化 _WebApplicationContext_ 的入口](#initServletBean__WebApplicationContext_)
		* 3.2.3. [_#initWebApplicationContext_ ——初始化 _WebApplicationContext_ 入口](#initWebApplicationContext__WebApplicationContext_)
		* 3.2.4. [_#configureAndRefreshWebApplicationContext_ ——初始化后处理，刷新_WebApplicationContext_](#configureAndRefreshWebApplicationContext__WebApplicationContext_)
		* 3.2.5. [_#onRefresh_——刷新完成后操作，开始初始化MVC组件](#onRefresh_MVC)
* 4. [_DispatcherServlet_ 请求处理工作揭秘](#DispatcherServlet_-1)
	* 4.1. [一切都要从 _FrameworkServlet_ 开始说起](#_FrameworkServlet_)
		* 4.1.1. [HTTP请求处理的开始—— _#service_](#HTTP_service_)
		* 4.1.2. [_#doGet_ & _#doPost_ & _#doPut_ & _#doDelete_](#doGet__doPost__doPut__doDelete_)
		* 4.1.3. [_#doOptions_ & _#doTrace_ (不常用)](#doOptions__doTrace_)
		* 4.1.4. [_#processRequest_ —— 处理请求的核心方法](#processRequest_)
	* 4.2. [_DispatcherServlet_ 核心处理方法 _#doService_](#DispatcherServlet__doService_)
	* 4.3. [_#doDispatch_ 请求属性处理完毕，执行请求分发](#doDispatch_)
	* 4.4. [_processDispatchResult_ ——转发后处理](#processDispatchResult_)
	* 4.5. [_#render_ ——页面渲染](#render_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

Spring MVC框架是围绕DispatcherServlet来设计的，这个Servlet会把请求分发给各个处理器，并支持可配置的处理器映射、视图渲染、本地化、时区与主题渲染和文件上传等功能。

处理器是应用中注解了@Controller和@RequestMapping的类和方法，Spring为处理器方法提供了非常多样灵活的配置。

Spring 3.0以后提供了@Controller注解机制、@PathVariable注解以及一些其他的特性，你可以使用它们来进行RESTful web站点和应用的开发。

Spring MVC具有以下特性：

- 清晰的职责分离。每个角色——控制器，验证器，命令对象，表单对象，模型对象，DispatcherServlet，处理器映射，视图解析器，等等许多——的工作，都可以由相应的对象来完成。
- 强大、直观的框架和应用bean的配置。这种配置能力包括能够从不同的上下文中进行简单的引用，比如在web控制器中引用业务对象、验证器等。
- 强大的适配能力、非侵入性和灵活性。Spring MVC支持你定义任意的控制器方法签名，在特定的场景下你还可以添加适合的注解（比如@RequestParam、@RequestHeader、@PathVariable等）
- 可复用的业务代码，使你远离重复代码。你可以使用已有的业务对象作为命令对象或表单对象，而不需让它们去继承一个框架提供的基类。
- 可定制的数据绑定和验证。类型不匹配仅被认为是应用级别的验证错误，错误值、本地化日期、数字绑定等会被保存。你不需要再在表单对象使用全String字段，然后再手动将它们转换成业务对象。
- 可定制的处理器映射和视图解析。处理器映射和视图解析策略从简单的基于URL配置，到精细专用的解析策略，Spring全都支持。在这一点上，Spring比一些依赖于特定技术的web框架要更加灵活。
- 灵活的模型传递。Spring使用一个名称/值对的Map来做模型，这使得模型很容易集成、传递给任何类型的视图技术。
- 可定制的本地化信息、时区和主题解析。支持用/不用Spring标签库的JSP技术，支持JSTL，支持无需额外配置的Velocity模板，等等。;
- 一个简单但功能强大的JSP标签库，通常称为Spring标签库，它提供了诸如数据绑定、主题支持等一些特性的支持。这些定制的标签为标记（markup）你的代码提供了最大程度的灵活性。
- 一个Spring 2.0开始引入的JSP表单标签库。它让你在JSP页面中编写表单简单许多。
- 新增生命周期仅绑定到当前HTTP请求或HTTP会话的Bean类型。

##  2. <a name='DispatcherServlet_'></a>_DispatcherServlet_ 的工作流程

Spring MVC框架是请求驱动的：所有设计都围绕着一个中央Servlet来展开，它负责把所有请求分发到控制器；同时提供其他Web应用开发所需要的功能。DispatcherServlet与Spring IoC容器做到了无缝集成，这意味着，Spring提供的任何特性在Spring MVC中你都可以使用。

下图展示了Spring Web MVC的DispatcherServlet处理请求的工作流，DispatcherServlet应用的其实就是一个“前端控制器”的设计模式：

<div align=center><img src="/asset/dispatcher2.png"/></div>

基于Spring Boot可以直接使用Spring MVC，除非有非常个性化的要求，相关配置不需要手动配置。

##  3. <a name='DispatcherServlet_-1'></a>_DispatcherServlet_ 族谱

<div align=center><img src="/asset/dispatcher1.png" width=100%/></div>

* **HttpServletBean** : 负责将ServletConfig设置到当前Servlet对象中，即初始化web.xml中init-param参数
* **FrameworkServlet** : 负责初始化Spring Servlet WebApplicationContext容器
* **DispatcherServlet** : 负责初始化Spring MVC的各个组件

###  3.1. <a name='HttpServletBean_'></a>_HttpServletBean_

HttpServletBean继承自HttpServlet，本质还是一个Servlet，重写了init方法，对初始化过程进行了一些处理

```java
@Override
	public final void init() throws ServletException {

		// Set bean properties from init parameters. 获取配置文件的bean的初始化配置参数
		PropertyValues pvs = new ServletConfigPropertyValues(getServletConfig(), this.requiredProperties);
		if (!pvs.isEmpty()) {
			try {
				BeanWrapper bw = PropertyAccessorFactory.forBeanPropertyAccess(this);
				ResourceLoader resourceLoader = new ServletContextResourceLoader(getServletContext());
				bw.registerCustomEditor(Resource.class, new ResourceEditor(resourceLoader, getEnvironment()));
				initBeanWrapper(bw);
                //设置DispatcherServlet属性
				bw.setPropertyValues(pvs, true);
			}
			catch (BeansException ex) {
				if (logger.isErrorEnabled()) {
					logger.error("Failed to set bean properties on servlet '" + getServletName() + "'", ex);
				}
				throw ex;
			}
		}

		// Let subclasses do whatever initialization they like.
        //默认为空实现，子类可以重写这个方法来进行更多的处理
		initServletBean();
	}
```

###  3.2. <a name='FrameworkServlet_'></a>_FrameworkServlet_

FrameServletBean继承自HttpServletBean，重写了initServletBean方法，也就是HttpServletBean中的空实现，**负责初始化Spring WebApplicationContext容器**

####  3.2.1. <a name='FrameworkServlet_-1'></a>_FrameworkServlet_ 关键属性

```java
//配置文件的地址,例如：/WEB-INF/spring-servlet.xml 。
@Nullable
private String contextConfigLocation;

//WebApplicationContext 对象, 即本文的关键，Servlet WebApplicationContext 容器
@Nullable
private WebApplicationContext webApplicationContext;

//创建的 WebApplicationContext 类型
private Class<?> contextClass = DEFAULT_CONTEXT_CLASS;
```

####  3.2.2. <a name='initServletBean__WebApplicationContext_'></a>_#initServletBean_ ——真正初始化 _WebApplicationContext_ 的入口

```java
// FrameworkServlet.java

@Override
protected final void initServletBean() throws ServletException {
	// 打日志
    getServletContext().log("Initializing Spring " + getClass().getSimpleName() + " '" + getServletName() + "'");
	if (logger.isInfoEnabled()) {
		logger.info("Initializing Servlet '" + getServletName() + "'");
	}

	// 记录开始时间
	long startTime = System.currentTimeMillis();

	try {
	    // 初始化 WebApplicationContext 对象，具体见下面的代码
		this.webApplicationContext = initWebApplicationContext();
		// 空实现。子类有需要，可以实现该方法，实现自定义逻辑
		initFrameworkServlet();
	} catch (ServletException | RuntimeException ex) {
		logger.error("Context initialization failed", ex);
		throw ex;
	}

	// 打日志
	if (logger.isDebugEnabled()) {
		String value = this.enableLoggingRequestDetails ?
				"shown which may lead to unsafe logging of potentially sensitive data" :
				"masked to prevent unsafe logging of potentially sensitive data";
		logger.debug("enableLoggingRequestDetails='" + this.enableLoggingRequestDetails +
				"': request parameters and headers will be " + value);
	}

	// 打日志
	if (logger.isInfoEnabled()) {
		logger.info("Completed initialization in " + (System.currentTimeMillis() - startTime) + " ms");
	}
}
```

####  3.2.3. <a name='initWebApplicationContext__WebApplicationContext_'></a>_#initWebApplicationContext_ ——初始化 _WebApplicationContext_ 入口

```java
// FrameworkServlet.java

protected WebApplicationContext initWebApplicationContext() {
    // <1> 获得根 WebApplicationContext 对象
	WebApplicationContext rootContext = WebApplicationContextUtils.getWebApplicationContext(getServletContext());

	// <2> 获得 WebApplicationContext wac 变量
	WebApplicationContext wac = null;
	// 第一种情况，如果构造方法已经传入 webApplicationContext 属性，则直接使用
	if (this.webApplicationContext != null) {
		// A context instance was injected at construction time -> use it
        // 赋值给 wac 变量
		wac = this.webApplicationContext;
		// 如果是 ConfigurableWebApplicationContext 类型，并且未激活，则进行初始化
		if (wac instanceof ConfigurableWebApplicationContext) {
			ConfigurableWebApplicationContext cwac = (ConfigurableWebApplicationContext) wac;
			if (!cwac.isActive()) { // 未激活
				// The context has not yet been refreshed -> provide services such as
				// setting the parent context, setting the application context id, etc
                // 设置 wac 的父 context 为 rootContext 对象
				if (cwac.getParent() == null) {
					// The context instance was injected without an explicit parent -> set
					// the root application context (if any; may be null) as the parent
					cwac.setParent(rootContext);
				}
				// 配置和初始化 wac
				configureAndRefreshWebApplicationContext(cwac);
			}
		}
	}
	// 第二种情况，从 ServletContext 获取对应的 WebApplicationContext 对象
	if (wac == null) {
		// No context instance was injected at construction time -> see if one
		// has been registered in the servlet context. If one exists, it is assumed
		// that the parent context (if any) has already been set and that the
		// user has performed any initialization such as setting the context id
		wac = findWebApplicationContext();
	}
	// 第三种，创建一个 WebApplicationContext 对象
	if (wac == null) {
		// No context instance is defined for this servlet -> create a local one
		wac = createWebApplicationContext(rootContext);
	}

	// <3> 如果未触发刷新事件，则主动触发刷新事件
	if (!this.refreshEventReceived) {
		// Either the context is not a ConfigurableApplicationContext with refresh
		// support or the context injected at construction time had already been
		// refreshed -> trigger initial onRefresh manually here.
		onRefresh(wac);
	}

	// <4> 将 context 设置到 ServletContext 中
	if (this.publishContext) {
		// Publish the context as a servlet context attribute.
		String attrName = getServletContextAttributeName();
		getServletContext().setAttribute(attrName, wac);
	}

	return wac;
}
```

####  3.2.4. <a name='configureAndRefreshWebApplicationContext__WebApplicationContext_'></a>_#configureAndRefreshWebApplicationContext_ ——初始化后处理，刷新_WebApplicationContext_

```java
// FrameworkServlet.java

protected void configureAndRefreshWebApplicationContext(ConfigurableWebApplicationContext wac) {
	// <1> 如果 wac 使用了默认编号，则重新设置 id 属性
    if (ObjectUtils.identityToString(wac).equals(wac.getId())) {
        // 情况一，使用 contextId 属性
		if (this.contextId != null) {
			wac.setId(this.contextId);
        // 情况二，自动生成
		} else {
			// Generate default id...
			wac.setId(ConfigurableWebApplicationContext.APPLICATION_CONTEXT_ID_PREFIX +
					ObjectUtils.getDisplayString(getServletContext().getContextPath()) + '/' + getServletName());
		}
	}

	// <2> 设置 wac 的 servletContext、servletConfig、namespace 属性
	wac.setServletContext(getServletContext());
	wac.setServletConfig(getServletConfig());
	wac.setNamespace(getNamespace());

	// <3> 添加监听器 SourceFilteringListener 到 wac 中
	wac.addApplicationListener(new SourceFilteringListener(wac, new ContextRefreshListener()));

	// <4> 暂时忽略
	ConfigurableEnvironment env = wac.getEnvironment();
	if (env instanceof ConfigurableWebEnvironment) {
		((ConfigurableWebEnvironment) env).initPropertySources(getServletContext(), getServletConfig());
	}

	// <5> 执行处理完 WebApplicationContext 后的逻辑。目前是个空方法，暂无任何实现
	postProcessWebApplicationContext(wac);

	// <6> 执行自定义初始化 context TODO 芋艿，暂时忽略
	applyInitializers(wac);

	// <7> 刷新 wac ，从而初始化 wac
	wac.refresh();
}
```

####  3.2.5. <a name='onRefresh_MVC'></a>_#onRefresh_——刷新完成后操作，开始初始化MVC组件

当 Servlet WebApplicationContext 刷新完成后，触发 Spring MVC 组件的初始化

```java
//FrameworkServlet.java
@Override
protected void onRefresh(ApplicationContext context) {
	initStrategies(context);
}
    
protected void initStrategies(ApplicationContext context) {
    // 初始化 MultipartResolver
	initMultipartResolver(context);
	// 初始化 LocaleResolver
	initLocaleResolver(context);
	// 初始化 ThemeResolver
	initThemeResolver(context);
	// 初始化 HandlerMappings
	initHandlerMappings(context);
	// 初始化 HandlerAdapters
	initHandlerAdapters(context);
	// 初始化 HandlerExceptionResolvers 
	initHandlerExceptionResolvers(context);
	// 初始化 RequestToViewNameTranslator
	initRequestToViewNameTranslator(context);
	// 初始化 ViewResolvers
	initViewResolvers(context);
	// 初始化 FlashMapManager
	initFlashMapManager(context);
}
```

大致了解DispatcherServlet的UML类图关系，下面就可以正式开始开启DispatcherServlet的探秘之旅了

##  4. <a name='DispatcherServlet_-1'></a>_DispatcherServlet_ 请求处理工作揭秘

###  4.1. <a name='_FrameworkServlet_'></a>一切都要从 _FrameworkServlet_ 开始说起

虽然请求首先是被 DispatcherServlet 所处理，但是实际上，FrameworkServlet 才是真正的入门,，其定义了关于请求的一系列操作。FrameworkServlet 会实现下列方法:

- `#doGet(HttpServletRequest request, HttpServletResponse response)`
- `#doPost(HttpServletRequest request, HttpServletResponse response)`
- `#doPut(HttpServletRequest request, HttpServletResponse response)`
- `#doDelete(HttpServletRequest request, HttpServletResponse response)`
- `#doOptions(HttpServletRequest request, HttpServletResponse response)`
- `#doTrace(HttpServletRequest request, HttpServletResponse response)`
- `#service(HttpServletRequest request, HttpServletResponse response)`

等方法。而这些实现，最终会调用 `#processRequest(HttpServletRequest request, HttpServletResponse response)` 方法，处理请求。

####  4.1.1. <a name='HTTP_service_'></a>HTTP请求处理的开始—— _#service_

```java
// FrameworkServlet.java

@Override
protected void service(HttpServletRequest request, HttpServletResponse response)
		throws ServletException, IOException {
	// <1> 获得请求方法
	HttpMethod httpMethod = HttpMethod.resolve(request.getMethod());
	// <2.1> 处理 PATCH 请求，因为HttpServlet并没有定义关于PATCH的操作，这是一个个例，需要特殊处理
	if (httpMethod == HttpMethod.PATCH || httpMethod == null) {
		processRequest(request, response);
	// <2.2> 调用父类，处理其它请求，FrameworkServlet的父类是HttpServletBean
    //然后继续调用HttpServlet，即HttpServletBean的父类方法service
	} else {
		super.service(request, response);
	}
}
```

在<2>中，调用链会延伸至HttpServlet#service来完成

```java
// HttpServlet.java

protected void service(HttpServletRequest req, HttpServletResponse resp)
    throws ServletException, IOException {
    String method = req.getMethod();

    if (method.equals(METHOD_GET)) {
        long lastModified = getLastModified(req);
        if (lastModified == -1) {
            // servlet doesn't support if-modified-since, no reason
            // to go through further expensive logic
            doGet(req, resp);
        } else {
            long ifModifiedSince = req.getDateHeader(HEADER_IFMODSINCE);
            if (ifModifiedSince < lastModified) {
                // If the servlet mod time is later, call doGet()
                // Round down to the nearest second for a proper compare
                // A ifModifiedSince of -1 will always be less
                maybeSetLastModified(resp, lastModified);
                doGet(req, resp);
            } else {
                resp.setStatus(HttpServletResponse.SC_NOT_MODIFIED);
            }
        }
    } else if (method.equals(METHOD_HEAD)) {
        long lastModified = getLastModified(req);
        maybeSetLastModified(resp, lastModified);
        doHead(req, resp);
    } else if (method.equals(METHOD_POST)) {
        doPost(req, resp);
    } else if (method.equals(METHOD_PUT)) {
        doPut(req, resp);
    } else if (method.equals(METHOD_DELETE)) {
        doDelete(req, resp);
    } else if (method.equals(METHOD_OPTIONS)) {
        doOptions(req,resp);
    } else if (method.equals(METHOD_TRACE)) {
        doTrace(req,resp);
    } else {
        //
        // Note that this means NO servlet supports whatever
        // method was requested, anywhere on this server.
        //

        String errMsg = lStrings.getString("http.method_not_implemented");
        Object[] errArgs = new Object[1];
        errArgs[0] = method;
        errMsg = MessageFormat.format(errMsg, errArgs);
        
        resp.sendError(HttpServletResponse.SC_NOT_IMPLEMENTED, errMsg);
    }
}
```

####  4.1.2. <a name='doGet__doPost__doPut__doDelete_'></a>_#doGet_ & _#doPost_ & _#doPut_ & _#doDelete_

这四个方法，都是直接调用 `#processRequest(HttpServletRequest request, HttpServletResponse response)` 方法，处理请求。代码如下：

```java
// FrameworkServlet.java
@Override
protected final void doGet(HttpServletRequest request, HttpServletResponse response)
		throws ServletException, IOException {
	processRequest(request, response);
}

@Override
protected final void doPost(HttpServletRequest request, HttpServletResponse response)
		throws ServletException, IOException {
	processRequest(request, response);
}

@Override
protected final void doPut(HttpServletRequest request, HttpServletResponse response)
		throws ServletException, IOException {
	processRequest(request, response);
}

@Override
protected final void doDelete(HttpServletRequest request, HttpServletResponse response)
		throws ServletException, IOException {
	processRequest(request, response);
}
```

####  4.1.3. <a name='doOptions__doTrace_'></a>_#doOptions_ & _#doTrace_ (不常用)

OPTIONS 请求方法，实际场景下用的少。

####  4.1.4. <a name='processRequest_'></a>_#processRequest_ —— 处理请求的核心方法

```java
// FrameworkServlet.java

protected final void processRequest(HttpServletRequest request, HttpServletResponse response)
		throws ServletException, IOException {
	// <1> 记录当前时间，用于计算 web 请求的处理时间
	long startTime = System.currentTimeMillis();
	// <2> 记录异常
	Throwable failureCause = null;

	// <3> 
	LocaleContext previousLocaleContext = LocaleContextHolder.getLocaleContext();
	LocaleContext localeContext = buildLocaleContext(request);

	// <4>
	RequestAttributes previousAttributes = RequestContextHolder.getRequestAttributes();
	ServletRequestAttributes requestAttributes = buildRequestAttributes(request, response, previousAttributes);

	// <5>
	WebAsyncManager asyncManager = WebAsyncUtils.getAsyncManager(request);
	asyncManager.registerCallableInterceptor(FrameworkServlet.class.getName(), new RequestBindingInterceptor());

	// <6> 
	initContextHolders(request, localeContext, requestAttributes);

	try {
		// <7> 执行真正的逻辑!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // 事实上由子类DispatcherServlet来完成
		doService(request, response);
	} catch (ServletException | IOException ex) {
		failureCause = ex; // <8>
		throw ex;
	} catch (Throwable ex) {
		failureCause = ex; // <8>
		throw new NestedServletException("Request processing failed", ex);
	} finally {
		// <9> 
		resetContextHolders(request, previousLocaleContext, previousAttributes);
		// <10> 
		if (requestAttributes != null) {
			requestAttributes.requestCompleted();
		}
		// <11> 打印请求日志，并且日志级别为 DEBUG 
		logResult(request, response, failureCause, asyncManager);
		// <12> 发布 ServletRequestHandledEvent 事件，可用于容器的监听
		publishRequestHandledEvent(request, response, startTime, failureCause);
	}
}
```

###  4.2. <a name='DispatcherServlet__doService_'></a>_DispatcherServlet_ 核心处理方法 _#doService_

上面FrameworkServlet#processRequest中的#doService只是一个空实现方法，具体的逻辑操作需要DispatcherServlet#doService来实现，下面看看源码:

```java
// DispatcherServlet.java
@Override
protected void doService(HttpServletRequest request, HttpServletResponse response) throws Exception {
	// <1> 打印请求日志，并且日志级别为 DEBUG
    logRequest(request);

	// <2> 请求属性的处理
	Map<String, Object> attributesSnapshot = null;
	if (WebUtils.isIncludeRequest(request)) {
		attributesSnapshot = new HashMap<>();
		Enumeration<?> attrNames = request.getAttributeNames();
		while (attrNames.hasMoreElements()) {
			String attrName = (String) attrNames.nextElement();
			if (this.cleanupAfterInclude || attrName.startsWith(DEFAULT_STRATEGIES_PREFIX)) {
				attributesSnapshot.put(attrName, request.getAttribute(attrName));
			}
		}
	}

	// <3> 设置 Spring 框架中的常用对象到 request 属性中
	request.setAttribute(WEB_APPLICATION_CONTEXT_ATTRIBUTE, getWebApplicationContext());
	request.setAttribute(LOCALE_RESOLVER_ATTRIBUTE, this.localeResolver);
	request.setAttribute(THEME_RESOLVER_ATTRIBUTE, this.themeResolver);
	request.setAttribute(THEME_SOURCE_ATTRIBUTE, getThemeSource());

	// <4> 
	if (this.flashMapManager != null) {
		FlashMap inputFlashMap = this.flashMapManager.retrieveAndUpdate(request, response);
		if (inputFlashMap != null) {
			request.setAttribute(INPUT_FLASH_MAP_ATTRIBUTE, Collections.unmodifiableMap(inputFlashMap));
		}
		request.setAttribute(OUTPUT_FLASH_MAP_ATTRIBUTE, new FlashMap());
		request.setAttribute(FLASH_MAP_MANAGER_ATTRIBUTE, this.flashMapManager);
	}

	try {
		// <5> 执行请求的分发，重要！！！！！！！！！
		doDispatch(request, response);
	} finally {
		// <6> 
		if (!WebAsyncUtils.getAsyncManager(request).isConcurrentHandlingStarted()) {
			// Restore the original attribute snapshot, in case of an include.
			if (attributesSnapshot != null) {
				restoreAttributesAfterInclude(request, attributesSnapshot);
			}
		}
	}
}
```

###  4.3. <a name='doDispatch_'></a>_#doDispatch_ 请求属性处理完毕，执行请求分发

```java
// DispatcherServlet.java
protected void doDispatch(HttpServletRequest request, HttpServletResponse response) throws Exception {
	HttpServletRequest processedRequest = request;
	HandlerExecutionChain mappedHandler = null;
	boolean multipartRequestParsed = false;
	WebAsyncManager asyncManager = WebAsyncUtils.getAsyncManager(request);

	try {
		ModelAndView mv = null;
		Exception dispatchException = null;

		try {
			processedRequest = checkMultipart(request);
			multipartRequestParsed = (processedRequest != request);

			// Determine handler for the current request.
			// <3> 获得请求对应的 HandlerExecutionChain 对象
			mappedHandler = getHandler(processedRequest);
			if (mappedHandler == null) { // <3.1> 如果获取不到，则根据配置抛出异常或返回 404 错误
				noHandlerFound(processedRequest, response);
				return;
			}

			// Determine handler adapter for the current request.
			// <4> 获得当前 handler 对应的 HandlerAdapter 对象
			HandlerAdapter ha = getHandlerAdapter(mappedHandler.getHandler());

			// Process last-modified header, if supported by the handler.
			String method = request.getMethod();
			boolean isGet = "GET".equals(method);
			if (isGet || "HEAD".equals(method)) {
				long lastModified = ha.getLastModified(request, mappedHandler.getHandler());
				if (new ServletWebRequest(request, response).checkNotModified(lastModified) && isGet) {
					return;
				}
			}

			// <5> 前置处理 拦截器
			if (!mappedHandler.applyPreHandle(processedRequest, response)) {
				return;
			}

			// Actually invoke the handler.
			// <6> 真正的调用 handler 方法，并返回视图
			mv = ha.handle(processedRequest, response, mappedHandler.getHandler());

			// <7>
			if (asyncManager.isConcurrentHandlingStarted()) {
				return;
			}

			// <8> 视图
			applyDefaultViewName(processedRequest, mv);
			// <9> 后置处理 拦截器
			mappedHandler.applyPostHandle(processedRequest, response, mv);
		} catch (Exception ex) {
			dispatchException = ex; // <10> 记录异常
		} catch (Throwable err) {
			// As of 4.3, we're processing Errors thrown from handler methods as well,
			// making them available for @ExceptionHandler methods and other scenarios.
			dispatchException = new NestedServletException("Handler dispatch failed", err); // <10> 记录异常
		}

		// <11> 处理正常和异常的请求调用结果。
		processDispatchResult(processedRequest, response, mappedHandler, mv, dispatchException);
	} catch (Exception ex) {
		// <12> 已完成 拦截器
		triggerAfterCompletion(processedRequest, response, mappedHandler, ex);
	} catch (Throwable err) {
		// <12> 已完成 拦截器
		triggerAfterCompletion(processedRequest, response, mappedHandler,
				new NestedServletException("Handler processing failed", err));
	} finally {
		// <13.1> 
		if (asyncManager.isConcurrentHandlingStarted()) {
			// Instead of postHandle and afterCompletion
			if (mappedHandler != null) {
				mappedHandler.applyAfterConcurrentHandlingStarted(processedRequest, response);
			}
		} else {
			// <13.2> Clean up any resources used by a multipart request.
			if (multipartRequestParsed) {
				cleanupMultipart(processedRequest);
			}
		}
	}
}
```

###  4.4. <a name='processDispatchResult_'></a>_processDispatchResult_ ——转发后处理

该方法是doDispatch后处理，分为下面的情况:

1. 发生异常：从ModelAndViewDefinitionException中获得ModelAndView对象，或者处理异常，生成ModelAndView对象
2. 没有发生异常，生成ModelAndView，并由#render方法渲染页面

```java
// DispatcherServlet.java

private void processDispatchResult(HttpServletRequest request, HttpServletResponse response,
		@Nullable HandlerExecutionChain mappedHandler, @Nullable ModelAndView mv,
		@Nullable Exception exception) throws Exception {
	// <1> 标记，是否是生成的 ModelAndView 对象
	boolean errorView = false;

	// <2> 如果是否异常的结果
	if (exception != null) {
	    // 情况一，从 ModelAndViewDefiningException 中获得 ModelAndView 对象
		if (exception instanceof ModelAndViewDefiningException) {
			logger.debug("ModelAndViewDefiningException encountered", exception);
			mv = ((ModelAndViewDefiningException) exception).getModelAndView();
		// 情况二，处理异常，生成 ModelAndView 对象
		} else {
			Object handler = (mappedHandler != null ? mappedHandler.getHandler() : null);
			mv = processHandlerException(request, response, handler, exception);
			// 标记 errorView
			errorView = (mv != null);
		}
	}

	// Did the handler return a view to render?
	if (mv != null && !mv.wasCleared()) {
		// <3.1> 渲染页面
		render(mv, request, response);
		// <3.2> 清理请求中的错误消息属性
		if (errorView) {
			WebUtils.clearErrorRequestAttributes(request);
		}
	} else {
		if (logger.isTraceEnabled()) {
			logger.trace("No view rendering, null ModelAndView returned.");
		}
	}

	// <4> TODO 芋艿
	if (WebAsyncUtils.getAsyncManager(request).isConcurrentHandlingStarted()) {
		// Concurrent handling started during a forward
		return;
	}

	// <5> 已完成处理 拦截器
	if (mappedHandler != null) {
		mappedHandler.triggerAfterCompletion(request, response, null);
	}
}
```

###  4.5. <a name='render_'></a>_#render_ ——页面渲染

`#render(ModelAndView mv, HttpServletRequest request, HttpServletResponse response)` 方法，渲染 ModelAndView 。代码如下：

```java
// DispatcherServlet.java

protected void render(ModelAndView mv, HttpServletRequest request, HttpServletResponse response) throws Exception {
	// Determine locale for request and apply it to the response.
	// <1> TODO 芋艿 从 request 中获得 Locale 对象，并设置到 response 中
	Locale locale = (this.localeResolver != null ? this.localeResolver.resolveLocale(request) : request.getLocale());
	response.setLocale(locale);

	// 获得 View 对象
	View view;
	String viewName = mv.getViewName();
	// 情况一，使用 viewName 获得 View 对象
	if (viewName != null) {
		// We need to resolve the view name.
		// <2.1> 使用 viewName 获得 View 对象
		view = resolveViewName(viewName, mv.getModelInternal(), locale, request);
		if (view == null) { // 获取不到，抛出 ServletException 异常
			throw new ServletException("Could not resolve view with name '" + mv.getViewName() +
					"' in servlet with name '" + getServletName() + "'");
		}
	// 情况二，直接使用 ModelAndView 对象的 View 对象
	} else {
		// No need to lookup: the ModelAndView object contains the actual View object.
		// 直接使用 ModelAndView 对象的 View 对象
		view = mv.getView();
		if (view == null) { // 获取不到，抛出 ServletException 异常
			throw new ServletException("ModelAndView [" + mv + "] neither contains a view name nor a " +
					"View object in servlet with name '" + getServletName() + "'");
		}
	}

	// Delegate to the View object for rendering.
	// 打印日志
	if (logger.isTraceEnabled()) {
		logger.trace("Rendering view [" + view + "] ");
	}
	try {
		// <3> 设置响应的状态码
		if (mv.getStatus() != null) {
			response.setStatus(mv.getStatus().value());
		}
		// <4> 渲染页面
		view.render(mv.getModelInternal(), request, response);
	} catch (Exception ex) {
		if (logger.isDebugEnabled()) {
			logger.debug("Error rendering view [" + view + "]", ex);
		}
		throw ex;
	}
}
```

## 总结

### Spring MVC运行流程图

<div align=center><img src="/asset/mvc1.png"/></div>

### Spring MVC代码时序图

<div align=center><img src="/asset/mvc2.png"></div>

### Spring MVC流程示意图

<div align=center><img src="/asset/mvc3.png"></div>