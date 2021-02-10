# Spring源码解析——MVC九大组件(二)之HandlerAdapter

<!-- vscode-markdown-toc -->
* 1. [顶级接口 _HandlerAdapter_](#_HandlerAdapter_)
* 2. [_HandlerAdapter_ 族谱关系](#HandlerAdapter_)
* 3. [_SimpleControllerHandlerAdapter_](#SimpleControllerHandlerAdapter_)
* 4. [_HttpRequestHandlerAdapter_](#HttpRequestHandlerAdapter_)
* 5. [_SimpleServletHandlerAdapter_](#SimpleServletHandlerAdapter_)
* 6. [_AbstractHandlerMethodAdapter_](#AbstractHandlerMethodAdapter_)
* 7. [_RequestMappingHandlerAdapter_](#RequestMappingHandlerAdapter_)
	* 7.1. [_#afterPropertiesSet_](#afterPropertiesSet_)
	* 7.2. [_#initControllerAdviceCache_](#initControllerAdviceCache_)
	* 7.3. [_#handleInternal_ (重要)](#handleInternal_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='_HandlerAdapter_'></a>顶级接口 _HandlerAdapter_

`org.springframework.web.servlet.HandlerAdapter` ，处理器适配器接口。代码如下：

```java
// HandlerAdapter.java
public interface HandlerAdapter {

    /**
     * 是否支持该处理器
     */
    boolean supports(Object handler);

    /**
     * 执行处理器，返回 ModelAndView 结果
     */
    @Nullable
    ModelAndView handle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception;

    /**
     * 返回请求的最新更新时间。
     *
     * 如果不支持该操作，则返回 -1 即可
     */
    long getLastModified(HttpServletRequest request, Object handler);

}
```

- 因为，处理器 `handler` 的类型是 Object 类型，需要有一个调用者来实现 `handler` 是怎么被使用，怎么被执行。而 HandlerAdapter 的用途就在于此。可能如果接口名改成 HandlerInvoker ，笔者觉得会更好理解。

##  2. <a name='HandlerAdapter_'></a>_HandlerAdapter_ 族谱关系

<div align=center><img src="/assets/hd1.png"/></div>

- 左边的 AbstractHandlerMethodAdapter 和 RequestMappingHandlerAdapter 相对复杂
- 右边的 SimpleServletHandlerAdapter、HttpRequestHandlerAdapter、SimpleControllerHandlerAdapter 相对简单

##  3. <a name='SimpleControllerHandlerAdapter_'></a>_SimpleControllerHandlerAdapter_

`org.springframework.web.servlet.mvc.SimpleControllerHandlerAdapter` ，实现 HandlerAdapter 接口，基于 `org.springframework.web.servlet.mvc.Controller` 的 HandlerAdapter 实现类。代码如下：

```java
// SimpleControllerHandlerAdapter.java
public class SimpleControllerHandlerAdapter implements HandlerAdapter {

    @Override
    public boolean supports(Object handler) {
        // <1> 判断是 Controller 类型
        return (handler instanceof Controller);
    }

    @Override
    @Nullable
    public ModelAndView handle(HttpServletRequest request, HttpServletResponse response, Object handler)
            throws Exception {
        // <2> Controller 类型的调用
        return ((Controller) handler).handleRequest(request, response);
    }

    @Override
    public long getLastModified(HttpServletRequest request, Object handler) {
        // 处理器实现了 LastModified 接口的情况下
        if (handler instanceof LastModified) {
            return ((LastModified) handler).getLastModified(request);
        }
        return -1L;
    }

}
```

- `<1>` 处，判断处理器 `handler` 是 Controller 类型。注意，不是 `@Controller` 注解。
- `<2>` 处，调用 `Controller#handleRequest(HttpServletRequest request, HttpServletResponse response)` 方法，Controller 类型的调用。

##  4. <a name='HttpRequestHandlerAdapter_'></a>_HttpRequestHandlerAdapter_

`org.springframework.web.servlet.mvc.HttpRequestHandlerAdapter` ，实现 HandlerAdapter 接口，基于 `org.springframework.web.HttpRequestHandler` 的 HandlerAdapter 实现类。代码如下：

```java
// HttpRequestHandlerAdapter.java
public class HttpRequestHandlerAdapter implements HandlerAdapter {

    @Override
    public boolean supports(Object handler) {
        // 判断是 HttpRequestHandler 类型
        return (handler instanceof HttpRequestHandler);
    }

    @Override
    @Nullable
    public ModelAndView handle(HttpServletRequest request, HttpServletResponse response, Object handler)
            throws Exception {
        // HttpRequestHandler 类型的调用
        ((HttpRequestHandler) handler).handleRequest(request, response);
        return null;
    }

    @Override
    public long getLastModified(HttpServletRequest request, Object handler) {
        // 处理器实现了 LastModified 接口的情况下
        if (handler instanceof LastModified) {
            return ((LastModified) handler).getLastModified(request);
        }
        return -1L;
    }

}
```

##  5. <a name='SimpleServletHandlerAdapter_'></a>_SimpleServletHandlerAdapter_

`org.springframework.web.servlet.handler.SimpleServletHandlerAdapter` ，实现 HandlerAdapter 接口，基于 `javax.servlet.Servlet` 的 HandlerAdapter 实现类。代码如下：

```java
// SimpleServletHandlerAdapter.java
public class SimpleServletHandlerAdapter implements HandlerAdapter {

    @Override
    public boolean supports(Object handler) {
        // 判断是 Servlet 类型
        return (handler instanceof Servlet);
    }

    @Override
    @Nullable
    public ModelAndView handle(HttpServletRequest request, HttpServletResponse response, Object handler)
            throws Exception {
        // Servlet 类型的调用
        ((Servlet) handler).service(request, response);
        return null;
    }

    @Override
    public long getLastModified(HttpServletRequest request, Object handler) {
        return -1;
    }
}
```

##  6. <a name='AbstractHandlerMethodAdapter_'></a>_AbstractHandlerMethodAdapter_

`org.springframework.web.servlet.mvc.method.AbstractHandlerMethodAdapter` ，实现 HandlerAdapter、Ordered 接口，继承 WebContentGenerator 抽象类，基于 `org.springframework.web.method.HandlerMethod` 的 HandlerMethodAdapter 抽象类。

对于HandlerAdapter中的方法，AbstractHandlerMethodAdapter均使用了xxxInternal的方式重写

##  7. <a name='RequestMappingHandlerAdapter_'></a>_RequestMappingHandlerAdapter_

`org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerAdapter` ，实现 BeanFactoryAware、InitializingBean 接口，继承 AbstractHandlerMethodAdapter 抽象类，基于 `@RequestMapping` 注解的 HandlerMethod 的 HandlerMethodAdapter 实现类。

###  7.1. <a name='afterPropertiesSet_'></a>_#afterPropertiesSet_

`#afterPropertiesSet()` 方法，进一步初始化 RequestMappingHandlerAdapter 。代码如下：

```java
// RequestMappingHandlerAdapter.java

@Override
public void afterPropertiesSet() {
	// Do this first, it may add ResponseBody advice beans
	// <1> 初始化 ControllerAdvice 相关
	initControllerAdviceCache();

	// <2> 初始化 argumentResolvers 属性
	if (this.argumentResolvers == null) {
		List<HandlerMethodArgumentResolver> resolvers = getDefaultArgumentResolvers();
		this.argumentResolvers = new HandlerMethodArgumentResolverComposite().addResolvers(resolvers);
	}
	// <3> 初始化 initBinderArgumentResolvers 属性
	if (this.initBinderArgumentResolvers == null) {
		List<HandlerMethodArgumentResolver> resolvers = getDefaultInitBinderArgumentResolvers();
		this.initBinderArgumentResolvers = new HandlerMethodArgumentResolverComposite().addResolvers(resolvers);
	}
	// <4> 初始化 returnValueHandlers 属性
	if (this.returnValueHandlers == null) {
		List<HandlerMethodReturnValueHandler> handlers = getDefaultReturnValueHandlers();
		this.returnValueHandlers = new HandlerMethodReturnValueHandlerComposite().addHandlers(handlers);
	}
}
```

###  7.2. <a name='initControllerAdviceCache_'></a>_#initControllerAdviceCache_

`#initControllerAdviceCache()` 方法，初始化 ControllerAdvice 相关。代码如下：

```java
// RequestMappingHandlerAdapter.java

private void initControllerAdviceCache() {
	if (getApplicationContext() == null) {
		return;
	}

	// <1> 扫描 @ControllerAdvice 注解的 Bean 们，并将进行排序
	List<ControllerAdviceBean> adviceBeans = ControllerAdviceBean.findAnnotatedBeans(getApplicationContext());
	AnnotationAwareOrderComparator.sort(adviceBeans);

	List<Object> requestResponseBodyAdviceBeans = new ArrayList<>();

	// <2> 遍历 ControllerAdviceBean 数组
	for (ControllerAdviceBean adviceBean : adviceBeans) {
		Class<?> beanType = adviceBean.getBeanType();
		if (beanType == null) {
			throw new IllegalStateException("Unresolvable type for ControllerAdviceBean: " + adviceBean);
		}
		// <2.1> 扫描有 @ModelAttribute ，无 @RequestMapping 注解的方法，添加到 modelAttributeAdviceCache 中
		Set<Method> attrMethods = MethodIntrospector.selectMethods(beanType, MODEL_ATTRIBUTE_METHODS);
		if (!attrMethods.isEmpty()) {
			this.modelAttributeAdviceCache.put(adviceBean, attrMethods);
		}
		// <2.2> 扫描有 @InitBinder 注解的方法，添加到 initBinderAdviceCache 中
		Set<Method> binderMethods = MethodIntrospector.selectMethods(beanType, INIT_BINDER_METHODS);
		if (!binderMethods.isEmpty()) {
			this.initBinderAdviceCache.put(adviceBean, binderMethods);
		}
		// <2.3> 如果是 RequestBodyAdvice 或 ResponseBodyAdvice 的子类，添加到 requestResponseBodyAdviceBeans 中
		if (RequestBodyAdvice.class.isAssignableFrom(beanType)) {
			requestResponseBodyAdviceBeans.add(adviceBean);
		}
		if (ResponseBodyAdvice.class.isAssignableFrom(beanType)) {
			requestResponseBodyAdviceBeans.add(adviceBean);
		}
	}

	// <2.3> 将 requestResponseBodyAdviceBeans 添加到 this.requestResponseBodyAdvice 属性种
	if (!requestResponseBodyAdviceBeans.isEmpty()) {
		this.requestResponseBodyAdvice.addAll(0, requestResponseBodyAdviceBeans);
	}

	// 打印日志
	if (logger.isDebugEnabled()) {
		int modelSize = this.modelAttributeAdviceCache.size();
		int binderSize = this.initBinderAdviceCache.size();
		int reqCount = getBodyAdviceCount(RequestBodyAdvice.class);
		int resCount = getBodyAdviceCount(ResponseBodyAdvice.class);
		if (modelSize == 0 && binderSize == 0 && reqCount == 0 && resCount == 0) {
			logger.debug("ControllerAdvice beans: none");
		} else {
			logger.debug("ControllerAdvice beans: " + modelSize + " @ModelAttribute, " + binderSize +
					" @InitBinder, " + reqCount + " RequestBodyAdvice, " + resCount + ", ResponseBodyAdvice");
		}
	}
}
```

###  7.3. <a name='handleInternal_'></a>_#handleInternal_ (重要)

实现 `#handleInternal(HttpServletRequest request, HttpServletResponse response, HandlerMethod handlerMethod)` 方法，处理请求。代码如下：

```java
// RequestMappingHandlerAdapter.java
@Override
protected ModelAndView handleInternal(HttpServletRequest request, HttpServletResponse response, HandlerMethod handlerMethod) throws Exception {
    // 处理结果 ModelAndView 对象
    ModelAndView mav;

    // <1> 校验请求
    checkRequest(request);

	// Execute invokeHandlerMethod in synchronized block if required.
	// <2> 调用 HandlerMethod 方法
	if (this.synchronizeOnSession) { // 同步相同 Session 的逻辑
		HttpSession session = request.getSession(false);
		if (session != null) {
			Object mutex = WebUtils.getSessionMutex(session);
			synchronized (mutex) {
				mav = invokeHandlerMethod(request, response, handlerMethod);
			}
		} else {
			// No HttpSession available -> no mutex necessary
			mav = invokeHandlerMethod(request, response, handlerMethod);
		}
	} else {
		// No synchronization on session demanded at all...
		mav = invokeHandlerMethod(request, response, handlerMethod);
	}

	// <3> TODO WebContentGenerator
	if (!response.containsHeader(HEADER_CACHE_CONTROL)) {
		if (getSessionAttributesHandler(handlerMethod).hasSessionAttributes()) {
			applyCacheSeconds(response, this.cacheSecondsForSessionAttributeHandlers);
		} else {
			prepareResponse(response);
		}
	}

	return mav;
}
```

`<1>` 处，调用父类 WebContentGenerator 的 `#checkRequest(ttpServletRequest request)` 方法，校验请求是否合法。代码如下：

```java
// WebContentGenerator.java
protected final void checkRequest(HttpServletRequest request) throws ServletException {
	// Check whether we should support the request method.
	String method = request.getMethod();
	if (this.supportedMethods != null && !this.supportedMethods.contains(method)) {
		throw new HttpRequestMethodNotSupportedException(method, this.supportedMethods);
	}

	// Check whether a session is required.
	if (this.requireSession && request.getSession(false) == null) {
		throw new HttpSessionRequiredException("Pre-existing session required but none found");
	}
}
```

- 主要是 **HttpMethod 的类型**和**是否有 Session** 的校验。