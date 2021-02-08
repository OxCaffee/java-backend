# 核心分发器 _DispatcherServlet_

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_DispatcherServlet_ 的工作流程](#DispatcherServlet_)
* 3. [_DispatcherServlet_ 族谱](#DispatcherServlet_-1)
	* 3.1. [_HttpServletBean_](#HttpServletBean_)
	* 3.2. [_FrameworkServlet_](#FrameworkServlet_)
		* 3.2.1. [_FrameworkServlet_ 关键属性](#FrameworkServlet_-1)
		* 3.2.2. [_#initServletBean_ ——真正初始化_WebApplicationContext_ 的入口](#initServletBean__WebApplicationContext_)
		* 3.2.3. [_#initWebApplicationContext_ ——初始化 _WebApplicationContext_ 入口](#initWebApplicationContext__WebApplicationContext_)
		* 3.2.4. [_#configureAndRefreshWebApplicationContext_ ——初始化后处理，刷新_WebApplicationContext_](#configureAndRefreshWebApplicationContext__WebApplicationContext_)
		* 3.2.5. [_#onRefresh_——刷新完成后操作，开始初始化MVC组件](#onRefresh_MVC)

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

