# Spring MVC源码解析——容器的初始化(一)之Root ApplicationContext容器

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_web.xml_ 中配置MVC](#web.xml_MVC)
* 3. [Root _WebApplicationContext_ 容器](#Root_WebApplicationContext_)
* 4. [_ContextLoader_](#ContextLoader_)
	* 4.1. [_#createWebApplicationContext_](#createWebApplicationContext_)
	* 4.2. [_#configureAndRefreshWebApplicationContext_](#configureAndRefreshWebApplicationContext_)
	* 4.3. [_#closeWebApplicationContext_](#closeWebApplicationContext_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

随着Spring Boot逐步全面覆盖到我们的项目之中，我们已经基本忘记了当前经典的Servlet+Spring MVC的组合，包括熟悉的`web.xml`配置。但是回到最初的起点去理解之前的配置，有利于我们更好的理解当前Spring Boot是如何更加精简的集成MVC的。下面就让我们看看Spring容器是如何初始化的。

##  2. <a name='web.xml_MVC'></a>_web.xml_ 中配置MVC

```xml
<!-- web.xml使用的文件模式 -->
<?xml version="1.0" encoding="UTF-8"?>
<web-app version="2.5"
 xmlns="http://java.sun.com/xml/ns/javaee"
 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
 xsi:schemaLocation="http://java.sun.com/xml/ns/javaee
 http://java.sun.com/xml/ns/javaee/web-app_2_5.xsd">
</web-app>

<!-- [1] Spring配置 -->
<listener>
    <listener-class>org.springframework.web.context.ContextLoaderListener</listener-class>
</listener>
<!-- 指定Spring Bean的配置文件所在目录。默认配置在WEB-INF目录下 -->
<context-param>
    <param-name>contextConfigLocation</param-name>
    <param-value>classpath:config/applicationContext.xml</param-value>
</context-param>

<!-- [2] Spring MVC配置 -->
<servlet>
    <servlet-name>spring</servlet-name>
    <servlet-class>org.springframework.web.servlet.DispatcherServlet</servlet-class>
    <!-- 可以自定义servlet.xml配置文件的位置和名称，默认为WEB-INF目录下，名称为[<servlet-name>]-servlet.xml，如spring-servlet.xml
    <init-param>
        <param-name>contextConfigLocation</param-name>
        <param-value>/WEB-INF/spring-servlet.xml</param-value> // 默认
    </init-param>
    -->
    <load-on-startup>1</load-on-startup>
</servlet>
<servlet-mapping>
    <servlet-name>spring</servlet-name>
    <url-pattern>*.do</url-pattern>
</servlet-mapping>
```

* `[1]` 处，配置了 `org.springframework.web.context.ContextLoaderListener` 对象。这是一个 `javax.servlet.ServletContextListener` 对象，会初始化一个**Root** Spring WebApplicationContext 容器。
* `[2]` 处，配置了 `org.springframework.web.servlet.DispatcherServlet` 对象。这是一个 `javax.servlet.http.HttpServlet` 对象，它除了拦截我们制定的 `*.do` 请求外，也会初始化一个**属于它**的 Spring WebApplicationContext 容器。并且，这个容器是以 `[1]` 处的 Root 容器作为父容器。

##  3. <a name='Root_WebApplicationContext_'></a>Root _WebApplicationContext_ 容器

Root WebApplicationContext 容器的初始化，通过 ContextLoaderListener 来实现。在 Servlet 容器启动时，例如 Tomcat、Jetty 启动，则会被 ContextLoaderListener 监听到，从而调用 `#contextInitialized(ServletContextEvent event)` 方法，初始化 Root WebApplicationContext 容器。

```java
//ContextLoaderListener.java
@Override
public void contextInitialized(ServletContextEvent event) {
    //初始化WebApplicationContext
	initWebApplicationContext(event.getServletContext());
}
```

##  4. <a name='ContextLoader_'></a>_ContextLoader_

`org.springframework.web.context.ContextLoader`是真正实现初始化和销毁`WebApplicationContext`容器的逻辑类

 ```java
// ContextLoader.java

public WebApplicationContext initWebApplicationContext(ServletContext servletContext) {
    // <1> 若已经存在 ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE 对应的 WebApplicationContext 对象，则抛出 IllegalStateException 异常。
    // 例如，在 web.xml 中存在多个 ContextLoader 。
	if (servletContext.getAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE) != null) {
		throw new IllegalStateException(
				"Cannot initialize context because there is already a root application context present - " +
				"check whether you have multiple ContextLoader* definitions in your web.xml!");
	}

	// <2> 打印日志
	servletContext.log("Initializing Spring root WebApplicationContext");
	Log logger = LogFactory.getLog(ContextLoader.class);
	if (logger.isInfoEnabled()) {
		logger.info("Root WebApplicationContext: initialization started");
	}

	// 记录开始时间
	long startTime = System.currentTimeMillis();

	try {
		// Store context in local instance variable, to guarantee that
		// it is available on ServletContext shutdown.
		if (this.context == null) {
		    // <3> 初始化 context ，即创建 context 对象
			this.context = createWebApplicationContext(servletContext);
		}
		// <4> 如果是 ConfigurableWebApplicationContext 的子类，如果未刷新，则进行配置和刷新
		if (this.context instanceof ConfigurableWebApplicationContext) {
			ConfigurableWebApplicationContext cwac = (ConfigurableWebApplicationContext) this.context;
			if (!cwac.isActive()) { // <4.1> 未刷新( 激活 )
				// The context has not yet been refreshed -> provide services such as
				// setting the parent context, setting the application context id, etc
				if (cwac.getParent() == null) { // <4.2> 无父容器，则进行加载和设置。
					// The context instance was injected without an explicit parent ->
					// determine parent for root web application context, if any.
					ApplicationContext parent = loadParentContext(servletContext);
					cwac.setParent(parent);
				}
				// <4.3> 配置 context 对象，并进行刷新
				configureAndRefreshWebApplicationContext(cwac, servletContext);
			}
		}
		// <5> 记录在 servletContext 中
		servletContext.setAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE, this.context);

		// <6> 记录到 currentContext 或 currentContextPerThread 中
		ClassLoader ccl = Thread.currentThread().getContextClassLoader();
		if (ccl == ContextLoader.class.getClassLoader()) {
			currentContext = this.context;
		} else if (ccl != null) {
			currentContextPerThread.put(ccl, this.context);
		}

		// <7> 打印日志
		if (logger.isInfoEnabled()) {
			long elapsedTime = System.currentTimeMillis() - startTime;
			logger.info("Root WebApplicationContext initialized in " + elapsedTime + " ms");
		}

		// <8> 返回 context
		return this.context;
	} catch (RuntimeException | Error ex) {
	    // <9> 当发生异常，记录异常到 WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE 中，不再重新初始化。
		logger.error("Context initialization failed", ex);
		servletContext.setAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE, ex);
		throw ex;
	}
}
 ```

###  4.1. <a name='createWebApplicationContext_'></a>_#createWebApplicationContext_

 `#createWebApplicationContext(ServletContext sc)` 方法，初始化 `context` ，即创建 WebApplicationContext 对象。代码如下：

```java
// ContextLoader.java

protected WebApplicationContext createWebApplicationContext(ServletContext sc) {
    // <1> 获得 context 的类
	Class<?> contextClass = determineContextClass(sc);
	// <2> 判断 context 的类，是否符合 ConfigurableWebApplicationContext 的类型
	if (!ConfigurableWebApplicationContext.class.isAssignableFrom(contextClass)) {
		throw new ApplicationContextException("Custom context class [" + contextClass.getName() +
				"] is not of type [" + ConfigurableWebApplicationContext.class.getName() + "]");
	}
	// <3> 创建 context 的类的对象
	return (ConfigurableWebApplicationContext) BeanUtils.instantiateClass(contextClass);
}
```

* `<1>` 处，调用 `#determineContextClass(ServletContext servletContext)` 方法，获得 `context` 的类。代码如下：
* `<2>` 处，判断 `context` 的类，是否符合 ConfigurableWebApplicationContext 的类型。显然，XmlWebApplicationContext 是符合条件的，所以不会抛出 ApplicationContextException 异常。
* `<3>` 处，调用 `BeanUtils#instantiateClass(Class<T> clazz)` 方法，创建 `context` 的类的对象

```java
// ContextLoader.java

/**
 * Config param for the root WebApplicationContext implementation class to use: {@value}.
 * @see #determineContextClass(ServletContext)
 */
public static final String CONTEXT_CLASS_PARAM = "contextClass";

protected Class<?> determineContextClass(ServletContext servletContext) {
    // 获得参数 contextClass 的值
	String contextClassName = servletContext.getInitParameter(CONTEXT_CLASS_PARAM);
	// 情况一，如果值非空，则获得该类
	if (contextClassName != null) {
		try {
			return ClassUtils.forName(contextClassName, ClassUtils.getDefaultClassLoader());
		} catch (ClassNotFoundException ex) {
			throw new ApplicationContextException(
					"Failed to load custom context class [" + contextClassName + "]", ex);
		}
    // 情况二，从 defaultStrategies 获得该类
	} else {
		contextClassName = defaultStrategies.getProperty(WebApplicationContext.class.getName());
		try {
			return ClassUtils.forName(contextClassName, ContextLoader.class.getClassLoader());
		} catch (ClassNotFoundException ex) {
			throw new ApplicationContextException(
					"Failed to load default context class [" + contextClassName + "]", ex);
		}
	}
}
```

###  4.2. <a name='configureAndRefreshWebApplicationContext_'></a>_#configureAndRefreshWebApplicationContext_

`#configureAndRefreshWebApplicationContext(ConfigurableWebApplicationContext wac, ServletContext sc)` 方法，配置 ConfigurableWebApplicationContext 对象，并进行刷新。代码如下：

```java
// ContextLoader.java

/**
 * Config param for the root WebApplicationContext id,
 * to be used as serialization id for the underlying BeanFactory: {@value}.
 */
public static final String CONTEXT_ID_PARAM = "contextId";

/**
 * Name of servlet context parameter (i.e., {@value}) that can specify the
 * config location for the root context, falling back to the implementation's
 * default otherwise.
 * @see org.springframework.web.context.support.XmlWebApplicationContext#DEFAULT_CONFIG_LOCATION
 */
public static final String CONFIG_LOCATION_PARAM = "contextConfigLocation";

protected void configureAndRefreshWebApplicationContext(ConfigurableWebApplicationContext wac, ServletContext sc) {
    // <1> 如果 wac 使用了默认编号，则重新设置 id 属性
    if (ObjectUtils.identityToString(wac).equals(wac.getId())) {
		// The application context id is still set to its original default value
		// -> assign a more useful id based on available information
        // 情况一，使用 contextId 属性
        String idParam = sc.getInitParameter(CONTEXT_ID_PARAM);
		if (idParam != null) {
			wac.setId(idParam);
        // 情况二，自动生成
        } else {
			// Generate default id...
			wac.setId(ConfigurableWebApplicationContext.APPLICATION_CONTEXT_ID_PREFIX +
					ObjectUtils.getDisplayString(sc.getContextPath()));
		}
	}

	// <2>设置 context 的 ServletContext 属性
	wac.setServletContext(sc);
    // <3> 设置 context 的配置文件地址
	String configLocationParam = sc.getInitParameter(CONFIG_LOCATION_PARAM);
	if (configLocationParam != null) {
		wac.setConfigLocation(configLocationParam);
	}

	// The wac environment's #initPropertySources will be called in any case when the context
	// is refreshed; do it eagerly here to ensure servlet property sources are in place for
	// use in any post-processing or initialization that occurs below prior to #refresh
	// <4> TODO 芋艿，暂时忽略
    ConfigurableEnvironment env = wac.getEnvironment();
	if (env instanceof ConfigurableWebEnvironment) {
		((ConfigurableWebEnvironment) env).initPropertySources(sc, null);
	}

	// <5> 执行自定义初始化 context
	customizeContext(sc, wac);

	// 刷新 context ，执行初始化
	wac.refresh();
}
```

###  4.3. <a name='closeWebApplicationContext_'></a>_#closeWebApplicationContext_

`#closeWebApplicationContext(ServletContext servletContext)` 方法，关闭 WebApplicationContext 容器对象。代码如下：

```java
// ContextLoader.java

public void closeWebApplicationContext(ServletContext servletContext) {
	servletContext.log("Closing Spring root WebApplicationContext");
	try {
	    // 关闭 context
		if (this.context instanceof ConfigurableWebApplicationContext) {
			((ConfigurableWebApplicationContext) this.context).close();
		}
	} finally {
	    // 移除 currentContext 或 currentContextPerThread
		ClassLoader ccl = Thread.currentThread().getContextClassLoader();
		if (ccl == ContextLoader.class.getClassLoader()) {
			currentContext = null;
		} else if (ccl != null) {
			currentContextPerThread.remove(ccl);
		}
		// 从 ServletContext 中移除
		servletContext.removeAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE);
	}
}
```

