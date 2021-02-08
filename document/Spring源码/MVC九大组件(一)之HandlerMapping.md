# Spring MVC九大组件(一)之HandlerMapping

<!-- vscode-markdown-toc -->
* 1. [处理器顶级接口 _HandlerMapping_](#_HandlerMapping_)
* 2. [_HandlerMapping_ 族谱](#HandlerMapping_)
* 3. [_AbstractHandlerMapping_](#AbstractHandlerMapping_)
	* 3.1. [_#initApplicationContext_](#initApplicationContext_)
	* 3.2. [_#getHandler_](#getHandler_)
* 4. [_MatchableHandlerMapping_](#MatchableHandlerMapping_)
* 5. [_HandlerMapping_ 最后必经之地—— _DispatcherServlet_](#HandlerMapping__DispatcherServlet_)
* 6. [_HandlerInterceptor_](#HandlerInterceptor_)
* 7. [_HandlerExecutorChain_](#HandlerExecutorChain_)
	* 7.1. [_Constructor_](#Constructor_)
	* 7.2. [_#addIntereptor_](#addIntereptor_)
	* 7.3. [_#applyPreHandle_](#applyPreHandle_)
	* 7.4. [_#triggerAfterCompletion_](#triggerAfterCompletion_)
	* 7.5. [_#applyPostHandle_](#applyPostHandle_)
* 8. [_HandlerInterceptor_](#HandlerInterceptor_-1)
	* 8.1. [_MappedInterceptor_](#MappedInterceptor_)
		* 8.1.1. [_#matches_](#matches_)
		* 8.1.2. [_#preHandle_ & _#postHandle_ & _#afterCompletion_](#preHandle__postHandle__afterCompletion_)
* 9. [基于注解的Interceptor配置](#Interceptor)
* 10. [基于注解的灵活配置—— _AbstractHandlerMethodMapping_](#_AbstractHandlerMethodMapping_)
	* 10.1. [_AbstractHandlerMethodMapping_ 族谱关系](#AbstractHandlerMethodMapping_)
	* 10.2. [相关注解](#)
	* 10.3. [_Constructor_](#Constructor_-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='_HandlerMapping_'></a>处理器顶级接口 _HandlerMapping_

org.springframework.web.servlet.HandlerMapping ，处理器匹配接口，根据请求( handler )获得其的处理器( handler )和拦截器们( HandlerInterceptor 数组 )。代码如下:

```java
// HandlerMapping.java

public interface HandlerMapping {

    String PATH_WITHIN_HANDLER_MAPPING_ATTRIBUTE = HandlerMapping.class.getName() + ".pathWithinHandlerMapping";
    String BEST_MATCHING_PATTERN_ATTRIBUTE = HandlerMapping.class.getName() + ".bestMatchingPattern";
    String INTROSPECT_TYPE_LEVEL_MAPPING = HandlerMapping.class.getName() + ".introspectTypeLevelMapping";
    String URI_TEMPLATE_VARIABLES_ATTRIBUTE = HandlerMapping.class.getName() + ".uriTemplateVariables";
    String MATRIX_VARIABLES_ATTRIBUTE = HandlerMapping.class.getName() + ".matrixVariables";
    String PRODUCIBLE_MEDIA_TYPES_ATTRIBUTE = HandlerMapping.class.getName() + ".producibleMediaTypes";

    /**
     * 获得请求对应的处理器和拦截器们
     */
    @Nullable
    HandlerExecutionChain getHandler(HttpServletRequest request) throws Exception;

}
```

- 返回的对象类型是 HandlerExecutionChain ，它包含处理器( `handler` )和拦截器们( HandlerInterceptor 数组 )。

##  2. <a name='HandlerMapping_'></a>_HandlerMapping_ 族谱

<div align=center><img src="/asset/zj1.png"/></div>

- 绿框 **AbstractHandlerMapping** 抽象类，实现了【获得请求对应的处理器和拦截器们】的骨架逻辑，而暴露 `#getHandlerInternal(HttpServletRequest request)` 抽象方法，交由子类实现。这就是我们常说的[“模板方法模式”](http://www.iocoder.cn/DesignPattern/xiaomingge/Template-Method/) 。
- AbstractHandlerMapping 的子类，分成两派，分别是：
  - 红框 **AbstractUrlHandlerMapping** 系，基于 URL 进行匹配。例如 [《基于XML配置的Spring MVC 简单的HelloWorld实例应用》](https://www.cnblogs.com/liuhongfeng/p/4769076.html) 。当然，实际我们开发时，这种方式已经基本不用了，被 `@RequestMapping` 等注解的方式所取代。不过，Spring MVC 内置的一些路径匹配，还是使用这种方式。
  - 黄框 **AbstractHandlerMethodMapping** 系，基于 Method 进行匹配。例如，我们所熟知的 `@RequestMapping` 等注解的方式。
- 白框 MatchableHandlerMapping 接口，定义判断请求和指定 `pattern` 路径是否匹配的接口方法。

##  3. <a name='AbstractHandlerMapping_'></a>_AbstractHandlerMapping_ 

`org.springframework.web.servlet.handler.AbstractHandlerMapping` ，实现 **HandlerMapping**、Ordered、BeanNameAware 接口，继承 WebApplicationObjectSupport 抽象类，HandlerMapping **抽象基类**，实现了【获得请求对应的处理器和拦截器们】的骨架逻辑，而暴露 `#getHandlerInternal(HttpServletRequest request)` 抽象方法，交由子类实现。

```java
// AbstractHandlerMapping.java

//默认处理器。在获得不到处理器时，可使用该属性。
@Nullable
private Object defaultHandler;

//URL 路径工具类，配置文件使用
private UrlPathHelper urlPathHelper = new UrlPathHelper();

//路径匹配器，@RequestMapping使用
private PathMatcher pathMatcher = new AntPathMatcher();

//bean名字
@Nullable
private String beanName;

//顺序
private int order = Ordered.LOWEST_PRECEDENCE;  // default: same as non-Ordered

/**
 * 配置的拦截器数组.
 *
 * 在 {@link #initInterceptors()} 方法中，初始化到 {@link #adaptedInterceptors} 中
 *
 * 添加方式有两种：
 *
 * 1. {@link #setInterceptors(Object...)} 方法
 * 2. {@link #extendInterceptors(List)} 方法
 */
private final List<Object> interceptors = new ArrayList<>();

//初始化后的拦截器 HandlerInterceptor 数组
private final List<HandlerInterceptor> adaptedInterceptors = new ArrayList<>();

private CorsConfigurationSource corsConfigurationSource = new UrlBasedCorsConfigurationSource();
private CorsProcessor corsProcessor = new DefaultCorsProcessor();
```

###  3.1. <a name='initApplicationContext_'></a>_#initApplicationContext_

该方法，是对 WebApplicationObjectSupport 的覆写，而 WebApplicationObjectSupport 的集成关系是 WebApplicationObjectSupport => ApplicationObjectSupport => ApplicationContextAware。

在该方法内部，主要实现初始化拦截器

```java
// AbstractHandlerMapping.java

@Override
protected void initApplicationContext() throws BeansException {
    // <1> 空方法。交给子类实现，用于注册自定义的拦截器到 interceptors 中。目前暂无子类实现。
    extendInterceptors(this.interceptors);
    // <2> 扫描已注册的 MappedInterceptor 的 Bean 们，添加到 mappedInterceptors 中
    detectMappedInterceptors(this.adaptedInterceptors);
    // <3> 将 interceptors 初始化成 HandlerInterceptor 类型，添加到 mappedInterceptors 中
    initInterceptors();
}
```

`<1>` 处，调用 `#extendInterceptors(List<Object> interceptors)` 方法，空方法。交给子类实现，用于注册自定义的拦截器到 `interceptors` 中。目前暂无子类实现。代码如下：

```java
// AbstractHandlerMapping.java
protected void extendInterceptors(List<Object> interceptors) {}
```

`<2>` 处，调用 `#detectMappedInterceptors(List<HandlerInterceptor> mappedInterceptors)` 方法，扫描已注册的 MappedInterceptor 的 Bean 们，添加到 `mappedInterceptors` 中。代码如下：

```java
// AbstractHandlerMapping.java
protected void detectMappedInterceptors(List<HandlerInterceptor> mappedInterceptors) {
	// 扫描已注册的 MappedInterceptor 的 Bean 们，添加到 mappedInterceptors 中
	// MappedInterceptor 会根据请求路径做匹配，是否进行拦截。
	mappedInterceptors.addAll(
			BeanFactoryUtils.beansOfTypeIncludingAncestors(
					obtainApplicationContext(), MappedInterceptor.class, true, false).values());
}
```

`<3>` 处，调用 `#initInterceptors()` 方法，将 `interceptors` 初始化成 HandlerInterceptor 类型，添加到 `mappedInterceptors` 中。代码如下：

```java
// AbstractHandlerMapping.java
protected void initInterceptors() {
    if (!this.interceptors.isEmpty()) {
        // 遍历 interceptors 数组
        for (int i = 0; i < this.interceptors.size(); i++) {
            // 获得 interceptor 对象
            Object interceptor = this.interceptors.get(i);
            if (interceptor == null) { // 若为空，抛出 IllegalArgumentException 异常
                throw new IllegalArgumentException("Entry number " + i + " in interceptors array is null");
            }
            // 将 interceptors 初始化成 HandlerInterceptor 类型，添加到 mappedInterceptors 中
            // 注意，HandlerInterceptor 无需进行路径匹配，直接拦截全部
            this.adaptedInterceptors.add(adaptInterceptor(interceptor)); // <x>
        }
    }
}
```

`<x>` 处，调用 `#adaptInterceptor(Object interceptor)` 方法，将 `interceptors` 初始化成 HandlerInterceptor 类型。代码如下：

```java
// AbstractHandlerMapping.java

protected HandlerInterceptor adaptInterceptor(Object interceptor) {
	// HandlerInterceptor 类型，直接返回
	if (interceptor instanceof HandlerInterceptor) {
		return (HandlerInterceptor) interceptor;
 	// WebRequestInterceptor 类型，适配成 WebRequestHandlerInterceptorAdapter 对象，然后返回
	} else if (interceptor instanceof WebRequestInterceptor) {
		return new WebRequestHandlerInterceptorAdapter((WebRequestInterceptor) interceptor);
	// 错误类型，抛出 IllegalArgumentException 异常
	} else {
		throw new IllegalArgumentException("Interceptor type not supported: " + interceptor.getClass().getName());
	}
}
```

###  3.2. <a name='getHandler_'></a>_#getHandler_

`#getHandler(HttpServletRequest request)` 方法，获得请求对应的 HandlerExecutionChain 对象。代码如下：

```java
// AbstractHandlerMapping.java
@Override
@Nullable
public final HandlerExecutionChain getHandler(HttpServletRequest request) throws Exception {
    // <1> 获得处理器。该方法是抽象方法，由子类实现
    Object handler = getHandlerInternal(request);
    // <2> 获得不到，则使用默认处理器
    if (handler == null) {
        handler = getDefaultHandler();
    }
    // <3> 还是获得不到，则返回 null
    if (handler == null) {
        return null;
    }
    // Bean name or resolved handler?
    // <4> 如果找到的处理器是 String 类型，则从容器中找到 String 对应的 Bean 类型作为处理器。
    if (handler instanceof String) {
        String handlerName = (String) handler;
        handler = obtainApplicationContext().getBean(handlerName);
    }

    // <5> 获得 HandlerExecutionChain 对象
    HandlerExecutionChain executionChain = getHandlerExecutionChain(handler, request);

    // 打印日志
    if (logger.isTraceEnabled()) {
        logger.trace("Mapped to " + handler);
    }
    else if (logger.isDebugEnabled() && !request.getDispatcherType().equals(DispatcherType.ASYNC)) {
        logger.debug("Mapped to " + executionChain.getHandler());
    }

    // <6>
    if (CorsUtils.isCorsRequest(request)) {
        CorsConfiguration globalConfig = this.corsConfigurationSource.getCorsConfiguration(request);
        CorsConfiguration handlerConfig = getCorsConfiguration(handler, request);
        CorsConfiguration config = (globalConfig != null ? globalConfig.combine(handlerConfig) : handlerConfig);
        executionChain = getCorsHandlerExecutionChain(request, executionChain, config);
    }

    // <7> 返回    
    return executionChain;
}
```

`<1>` 处，调用 `#getHandlerInternal(HttpServletRequest request)` **抽象**方法，获得 `handler` 对象。代码如下：

```java
// AbstractHandlerMapping.java 
@Nullable
protected abstract Object getHandlerInternal(HttpServletRequest request) throws Exception;
```

`<2>` 处，获得不到，则调用 `#getDefaultHandler()` 方法，使用默认处理器。代码如下：

```java
// AbstractHandlerMapping.java
public Object getDefaultHandler() {
	return this.defaultHandler;
}
```

`<3>` 处，还是获得不到，则返回 `null` 。

`<5>` 处，调用 `#getHandlerExecutionChain(Object handler, HttpServletRequest request)` 方法，获得 HandlerExecutionChain 对象。代码如下：

```java
// AbstractHandlerMapping.java
protected HandlerExecutionChain getHandlerExecutionChain(Object handler, HttpServletRequest request) {
    // 创建 HandlerExecutionChain 对象
    HandlerExecutionChain chain = (handler instanceof HandlerExecutionChain ?
            (HandlerExecutionChain) handler : new HandlerExecutionChain(handler));

    // 获得请求路径
    String lookupPath = this.urlPathHelper.getLookupPathForRequest(request);
    // 遍历 adaptedInterceptors 数组，获得请求匹配的拦截器
    for (HandlerInterceptor interceptor : this.adaptedInterceptors) {
        // 需要匹配，若路径匹配，则添加到 chain 中
        if (interceptor instanceof MappedInterceptor) {
            MappedInterceptor mappedInterceptor = (MappedInterceptor) interceptor;
            if (mappedInterceptor.matches(lookupPath, this.pathMatcher)) { // 匹配
                chain.addInterceptor(mappedInterceptor.getInterceptor());
            }
        // 无需匹配，直接添加到 chain 中
        } else {
            chain.addInterceptor(interceptor);
        }
    }
    return chain;
}
```

##  4. <a name='MatchableHandlerMapping_'></a>_MatchableHandlerMapping_

`org.springframework.web.servlet.handler.MatchableHandlerMapping` ，定义判断请求和指定 `pattern` 路径是否匹配的接口方法。代码如下：

```java
// MatchableHandlerMapping.java
public interface MatchableHandlerMapping extends HandlerMapping {

	/**
     * 判断请求和指定 `pattern` 路径是否匹配的接口方法
     *
	 * Determine whether the given request matches the request criteria.
	 * @param request the current request
	 * @param pattern the pattern to match
	 * @return the result from request matching, or {@code null} if none
	 */
	@Nullable
	RequestMatchResult match(HttpServletRequest request, String pattern);

}
```

返回的是 `org.springframework.web.servlet.handler.RequestMatchResult` 类，请求匹配结果。代码如下：

```java
// RequestMatchResult.java
public class RequestMatchResult {

    /**
     * 匹配上的路径
     */
	private final String matchingPattern;

    /**
     * 被匹配的路径
     */
	private final String lookupPath;

    /**
     * 路径匹配器
     */
	private final PathMatcher pathMatcher;

	/**
	 * Create an instance with a matching pattern.
	 * @param matchingPattern the matching pattern, possibly not the same as the
	 * input pattern, e.g. inputPattern="/foo" and matchingPattern="/foo/".
	 * @param lookupPath the lookup path extracted from the request
	 * @param pathMatcher the PathMatcher used
	 */
	public RequestMatchResult(String matchingPattern, String lookupPath, PathMatcher pathMatcher) {
		Assert.hasText(matchingPattern, "'matchingPattern' is required");
		Assert.hasText(lookupPath, "'lookupPath' is required");
		Assert.notNull(pathMatcher, "'pathMatcher' is required");
		this.matchingPattern = matchingPattern;
		this.lookupPath = lookupPath;
		this.pathMatcher = pathMatcher;
	}


	/**
	 * Extract URI template variables from the matching pattern as defined in
	 * {@link PathMatcher#extractUriTemplateVariables}.
	 * @return a map with URI template variables
	 */
	public Map<String, String> extractUriTemplateVariables() {
		return this.pathMatcher.extractUriTemplateVariables(this.matchingPattern, this.lookupPath);
	}
}
```

##  5. <a name='HandlerMapping__DispatcherServlet_'></a>_HandlerMapping_ 最后必经之地—— _DispatcherServlet_

在 DispatcherServlet 中，通过调用 `#initHandlerMappings(ApplicationContext context)` 方法，初始化 HandlerMapping 们。酱紫，HandlerMapping 就集成到 DispatcherServlet 中了。代码如下：

```java
// DispatcherServlet.java
public static final String HANDLER_MAPPING_BEAN_NAME = "handlerMapping";

/** Detect all HandlerMappings or just expect "handlerMapping" bean?. */
private boolean detectAllHandlerMappings = true;

/** List of HandlerMappings used by this servlet. */
@Nullable
private List<HandlerMapping> handlerMappings;

private void initHandlerMappings(ApplicationContext context) {
    // 置空 handlerMappings
    this.handlerMappings = null;

    // <1> 如果开启探测功能，则扫描已注册的 HandlerMapping 的 Bean 们，添加到 handlerMappings 中
    if (this.detectAllHandlerMappings) {
        // Find all HandlerMappings in the ApplicationContext, including ancestor contexts.
        // 扫描已注册的 HandlerMapping 的 Bean 们
        Map<String, HandlerMapping> matchingBeans =
                BeanFactoryUtils.beansOfTypeIncludingAncestors(context, HandlerMapping.class, true, false);
        // 添加到 handlerMappings 中，并进行排序
        if (!matchingBeans.isEmpty()) {
            this.handlerMappings = new ArrayList<>(matchingBeans.values());
            // We keep HandlerMappings in sorted order.
            AnnotationAwareOrderComparator.sort(this.handlerMappings);
        }
    // <2> 如果关闭探测功能，则获得 HANDLER_MAPPING_BEAN_NAME 对应的 Bean 对象，并设置为 handlerMappings
    } else {
        try {
            HandlerMapping hm = context.getBean(HANDLER_MAPPING_BEAN_NAME, HandlerMapping.class);
            this.handlerMappings = Collections.singletonList(hm);
        } catch (NoSuchBeanDefinitionException ex) {
            // Ignore, we'll add a default HandlerMapping later.
        }
    }

    // Ensure we have at least one HandlerMapping, by registering
    // a default HandlerMapping if no other mappings are found.
    // <3> 如果未获得到，则获得默认配置的 HandlerMapping 类
    if (this.handlerMappings == null) {
        this.handlerMappings = getDefaultStrategies(context, HandlerMapping.class);
        if (logger.isTraceEnabled()) {
            logger.trace("No HandlerMappings declared for servlet '" + getServletName() +
                    "': using default strategies from DispatcherServlet.properties");
        }
    }
}
```

`<3>` 处，如果未获得到，则调用 `#getDefaultStrategies(ApplicationContext context, Class<T> strategyInterface)` 方法，获得默认配置的 HandlerMapping 类。代码如下：

```java
// DispatcherServlet.java
protected <T> List<T> getDefaultStrategies(ApplicationContext context, Class<T> strategyInterface) {
	// <1> 获得 strategyInterface 对应的 value 值
    String key = strategyInterface.getName();
	String value = defaultStrategies.getProperty(key);
	// <2> 创建 value 对应的对象们，并返回
	if (value != null) {
	    // 基于 "," 分隔，创建 classNames 数组
		String[] classNames = StringUtils.commaDelimitedListToStringArray(value);
		// 创建 strategyInterface 集合
		List<T> strategies = new ArrayList<>(classNames.length);
		// 遍历 classNames 数组，创建对应的类，添加到 strategyInterface 中
		for (String className : classNames) {
			try {
			    // 获得 className 类
				Class<?> clazz = ClassUtils.forName(className, DispatcherServlet.class.getClassLoader());
				// 创建 className 对应的类，并添加到 strategies 中
				Object strategy = createDefaultStrategy(context, clazz);
				strategies.add((T) strategy);
			} catch (ClassNotFoundException ex) {
				throw new BeanInitializationException(
						"Could not find DispatcherServlet's default strategy class [" + className +
						"] for interface [" + key + "]", ex);
			} catch (LinkageError err) {
				throw new BeanInitializationException(
						"Unresolvable class definition for DispatcherServlet's default strategy class [" +
						className + "] for interface [" + key + "]", err);
			}
		}
		// 返回 strategies
		return strategies;
	} else {
		return new LinkedList<>();
	}
}
```

- 实际上，这是个通用的方法，提供给不同的 `strategyInterface` 接口，获得对应的类型的**数组**。
- `<1>` 处，获得 `strategyInterface` 对应的 `value` 值。
- 关于 `defaultStrategies` 属性，涉及的代码如下：

```java
// DispatcherServlet.java
private static final String DEFAULT_STRATEGIES_PATH = "DispatcherServlet.properties";

//默认配置类
private static final Properties defaultStrategies;

static {
	try {
	    // 初始化 defaultStrategies
		ClassPathResource resource = new ClassPathResource(DEFAULT_STRATEGIES_PATH, DispatcherServlet.class);
		defaultStrategies = PropertiesLoaderUtils.loadProperties(resource);
	} catch (IOException ex) {
		throw new IllegalStateException("Could not load '" + DEFAULT_STRATEGIES_PATH + "': " + ex.getMessage());
	}
}
```

其中，`DispatcherServlet.properties` 的配置如下：

```properties
org.springframework.web.servlet.LocaleResolver=org.springframework.web.servlet.i18n.AcceptHeaderLocaleResolver
org.springframework.web.servlet.ThemeResolver=org.springframework.web.servlet.theme.FixedThemeResolver
org.springframework.web.servlet.HandlerMapping=org.springframework.web.servlet.handler.BeanNameUrlHandlerMapping,\
	org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerMapping
org.springframework.web.servlet.HandlerAdapter=org.springframework.web.servlet.mvc.HttpRequestHandlerAdapter,\
	org.springframework.web.servlet.mvc.SimpleControllerHandlerAdapter,\
	org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerAdapter
org.springframework.web.servlet.HandlerExceptionResolver=org.springframework.web.servlet.mvc.method.annotation.ExceptionHandlerExceptionResolver,\
	org.springframework.web.servlet.mvc.annotation.ResponseStatusExceptionResolver,\
	org.springframework.web.servlet.mvc.support.DefaultHandlerExceptionResolver
org.springframework.web.servlet.RequestToViewNameTranslator=org.springframework.web.servlet.view.DefaultRequestToViewNameTranslator
org.springframework.web.servlet.ViewResolver=org.springframework.web.servlet.view.InternalResourceViewResolver
org.springframework.web.servlet.FlashMapManager=org.springframework.web.servlet.support.SessionFlashMapManager
```

##  6. <a name='HandlerInterceptor_'></a>_HandlerInterceptor_

`org.springframework.web.servlet.HandlerInterceptor` ，处理器拦截器接口。代码如下：

```java
// HandlerInterceptor.java
public interface HandlerInterceptor {

    /**
     * 拦截处理器，在 {@link HandlerAdapter#handle(HttpServletRequest, HttpServletResponse, Object)} 执行之前
     */
    default boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler)
            throws Exception {
        return true;
    }

    /**
     * 拦截处理器，在 {@link HandlerAdapter#handle(HttpServletRequest, HttpServletResponse, Object)} 执行成功之后
     */
    default void postHandle(HttpServletRequest request, HttpServletResponse response, Object handler,
            @Nullable ModelAndView modelAndView) throws Exception {
    }

    /**
     * 拦截处理器，在 {@link HandlerAdapter} 执行完之后，无论成功还是失败
     *
     * 并且，只有 {@link #preHandle(HttpServletRequest, HttpServletResponse, Object)} 执行成功之后，才会被执行
     */
    default void afterCompletion(HttpServletRequest request, HttpServletResponse response, Object handler,
            @Nullable Exception ex) throws Exception {
    }

}
```

##  7. <a name='HandlerExecutorChain_'></a>_HandlerExecutorChain_

`org.springframework.web.servlet.HandlerExecutionChain` ，处理器执行链

###  7.1. <a name='Constructor_'></a>_Constructor_

```java
// HandlerExecutionChain.java

/**
 * 处理器
 */
private final Object handler;
/**
 * 拦截器数组
 */
@Nullable
private HandlerInterceptor[] interceptors;
/**
 * 拦截器数组。
 *
 * 在实际使用时，会调用 {@link #getInterceptors()} 方法，初始化到 {@link #interceptors} 中
 */
@Nullable
private List<HandlerInterceptor> interceptorList;

/**
 * 已执行 {@link HandlerInterceptor#preHandle(HttpServletRequest, HttpServletResponse, Object)} 的位置
 *
 * 主要用于实现 {@link #applyPostHandle(HttpServletRequest, HttpServletResponse, ModelAndView)} 的逻辑
 */
private int interceptorIndex = -1;

/**
 * Create a new HandlerExecutionChain.
 * @param handler the handler object to execute
 */
public HandlerExecutionChain(Object handler) {
    this(handler, (HandlerInterceptor[]) null);
}

/**
 * Create a new HandlerExecutionChain.
 * @param handler the handler object to execute
 * @param interceptors the array of interceptors to apply
 * (in the given order) before the handler itself executes
 */
public HandlerExecutionChain(Object handler, @Nullable HandlerInterceptor... interceptors) {
    if (handler instanceof HandlerExecutionChain) {
        HandlerExecutionChain originalChain = (HandlerExecutionChain) handler;
        this.handler = originalChain.getHandler();
        // 初始化到 interceptorList 中
        this.interceptorList = new ArrayList<>();
        CollectionUtils.mergeArrayIntoCollection(originalChain.getInterceptors(), this.interceptorList); // 逻辑比较简单，就是将前者添加到后者中，即添加到 interceptorList 中
        CollectionUtils.mergeArrayIntoCollection(interceptors, this.interceptorList); // 逻辑比较简单，就是将前者添加到后者中，即添加到 interceptorList 中
    } else {
        this.handler = handler;
        this.interceptors = interceptors;
    }
}
```

###  7.2. <a name='addIntereptor_'></a>_#addIntereptor_

`#addInterceptor(HandlerInterceptor interceptor)` 方法，添加拦截器到 `interceptorList` 中。代码如下：

```java
// HandlerExecutionChain.java
public void addInterceptor(HandlerInterceptor interceptor) {
	initInterceptorList().add(interceptor);
}
```

首先，会调用 `#initInterceptorList()` 方法，保证 `interceptorList` 已初始化。代码如下：

```java
// HandlerExecutionChain.java
private List<HandlerInterceptor> initInterceptorList() {
	// 如果 interceptorList 为空，则初始化为 ArrayList
	if (this.interceptorList == null) {
		this.interceptorList = new ArrayList<>();
		// 如果 interceptors 非空，则添加到 interceptorList 中
		if (this.interceptors != null) {
			// An interceptor array specified through the constructor
			CollectionUtils.mergeArrayIntoCollection(this.interceptors, this.interceptorList);
		}
	}
	// 置空 interceptors
	this.interceptors = null;
	// 返回 interceptorList
	return this.interceptorList;
}
```

###  7.3. <a name='applyPreHandle_'></a>_#applyPreHandle_

`#applyPreHandle(HttpServletRequest request, HttpServletResponse response)` 方法，应用拦截器的前置处理。代码如下：

```java
boolean applyPreHandle(HttpServletRequest request, HttpServletResponse response) throws Exception {
    // <1> 获得拦截器数组
    HandlerInterceptor[] interceptors = getInterceptors();
    if (!ObjectUtils.isEmpty(interceptors)) {
        // <2> 遍历拦截器数组
        for (int i = 0; i < interceptors.length; i++) {
            HandlerInterceptor interceptor = interceptors[i];
            // <3> 前置处理
            if (!interceptor.preHandle(request, response, this.handler)) {
                // <3.1> 触发已完成处理
                triggerAfterCompletion(request, response, null);
                // 返回 false ，前置处理失败
                return false;
            }
            // <3.2> 标记 interceptorIndex 位置
            this.interceptorIndex = i;
        }
    }
    // <4> 返回 true ，前置处理成功
    return true;
}
```

###  7.4. <a name='triggerAfterCompletion_'></a>_#triggerAfterCompletion_

`#triggerAfterCompletion(HttpServletRequest request, HttpServletResponse response, Exception ex)` 方法，触发拦截器的已完成处理。代码如下：

```java
// HandlerExecutionChain.java
void triggerAfterCompletion(HttpServletRequest request, HttpServletResponse response, @Nullable Exception ex)
		throws Exception {
	// 获得拦截器数组
	HandlerInterceptor[] interceptors = getInterceptors();
	if (!ObjectUtils.isEmpty(interceptors)) {
		// 遍历拦截器数组
		for (int i = this.interceptorIndex; i >= 0; i--) { // 倒序！！！
			HandlerInterceptor interceptor = interceptors[i];
			try {
				// 已完成处理
				interceptor.afterCompletion(request, response, this.handler, ex);
			} catch (Throwable ex2) { // 注意，如果执行失败，仅仅会打印错误日志，不会结束循环
				logger.error("HandlerInterceptor.afterCompletion threw exception", ex2);
			}
		}
	}
}
```

###  7.5. <a name='applyPostHandle_'></a>_#applyPostHandle_

`#applyPostHandle(HttpServletRequest request, HttpServletResponse response, ModelAndView mv)` 方法，应用拦截器的后置处理。代码如下：

```java
// HandlerExecutionChain.java
void applyPostHandle(HttpServletRequest request, HttpServletResponse response, @Nullable ModelAndView mv)
		throws Exception {
    // 获得拦截器数组
    HandlerInterceptor[] interceptors = getInterceptors();
	if (!ObjectUtils.isEmpty(interceptors)) {
        // 遍历拦截器数组
        for (int i = interceptors.length - 1; i >= 0; i--) { // 倒序
			HandlerInterceptor interceptor = interceptors[i];
			// 后置处理
			interceptor.postHandle(request, response, this.handler, mv);
		}
	}
}
```

##  8. <a name='HandlerInterceptor_-1'></a>_HandlerInterceptor_

<div align=center><img src="/asset/hp1.png"/></div>

###  8.1. <a name='MappedInterceptor_'></a>_MappedInterceptor_

`org.springframework.web.servlet.handler.MappedInterceptor` ，实现 HandlerInterceptor 接口，支持地址匹配的 HandlerInterceptor 实现类。

```xml
<mvc:interceptors>
    <mvc:interceptor>
        <mvc:mapping path="/interceptor/**" />
        <mvc:exclude-mapping path="/interceptor/b/*" />
        <bean class="com.elim.learn.spring.mvc.interceptor.MyInterceptor" />
    </mvc:interceptor>
</mvc:interceptors>
```

- 每一个 `<mvc:interceptor />` 标签，将被解析成一个 MappedInterceptor Bean 对象。
- `includePatterns` + `excludePatterns` + `pathMatcher` 属性，匹配路径。

####  8.1.1. <a name='matches_'></a>_#matches_

`#matches(String lookupPath, PathMatcher pathMatcher)` 方法，判断路径是否匹配。代码如下：

```java
// MappedInterceptor.java
public boolean matches(String lookupPath, PathMatcher pathMatcher) {
	PathMatcher pathMatcherToUse = (this.pathMatcher != null ? this.pathMatcher : pathMatcher);
	// 先排重
	if (!ObjectUtils.isEmpty(this.excludePatterns)) {
		for (String pattern : this.excludePatterns) {
			if (pathMatcherToUse.match(pattern, lookupPath)) { // 匹配
				return false;
			}
		}
	}
	// 特殊，如果包含为空，则默认就是包含
	if (ObjectUtils.isEmpty(this.includePatterns)) {
		return true;
	}
	// 后包含
	for (String pattern : this.includePatterns) {
		if (pathMatcherToUse.match(pattern, lookupPath)) { // 匹配
			return true;
		}
	}
	return false;
}
```

####  8.1.2. <a name='preHandle__postHandle__afterCompletion_'></a>_#preHandle_ & _#postHandle_ & _#afterCompletion_

```java
// MappedInterceptor.java
@Override
public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler)
		throws Exception {
	return this.interceptor.preHandle(request, response, handler);
}

@Override
public void postHandle(HttpServletRequest request, HttpServletResponse response, Object handler,
		@Nullable ModelAndView modelAndView) throws Exception {
	this.interceptor.postHandle(request, response, handler, modelAndView);
}

@Override
public void afterCompletion(HttpServletRequest request, HttpServletResponse response, Object handler,
		@Nullable Exception ex) throws Exception {
	this.interceptor.afterCompletion(request, response, handler, ex);
}
```

当然，基于这样的思路，我们直接配置 MappedInterceptor 的 Bean 对象也是可以的，无论是通过 XML ，还是通过 `@Bean` 注解。

##  9. <a name='Interceptor'></a>基于注解的Interceptor配置

```java
@Component
public class SecurityInterceptor extends HandlerInterceptorAdapter {}
```

```java
// MVCConfiguration.java
@EnableWebMvc
@Configuration
public class MVCConfiguration extends WebMvcConfigurerAdapter {

    @Autowired
    private SecurityInterceptor securityInterceptor;

    @Override
    public void addInterceptors(InterceptorRegistry registry) {
        registry.addInterceptor(securityInterceptor);
    }

}
```

**因为SecurityInterceptor是 HandlerInterceptorAdapter 的子类，而不是 MappedInterceptor 的子类，所以不会被 AbstractHandlerMapping 的 `#detectMappedInterceptors(List<HandlerInterceptor> mappedInterceptors)` 方法扫描到。**

在 MVCConfiguration 的 `#addInterceptors(InterceptorRegistry registry)` 方法中，我们将 `securityInterceptor` 拦截器添加到 InterceptorRegistry 这个拦截器注册表中

##  10. <a name='_AbstractHandlerMethodMapping_'></a>基于注解的灵活配置—— _AbstractHandlerMethodMapping_

###  10.1. <a name='AbstractHandlerMethodMapping_'></a>_AbstractHandlerMethodMapping_ 族谱关系

AbstractHandlerMethodMapping <= RequestMappingInfoHandlerMapping <= RequestMappingHandlerMapping 

###  10.2. <a name=''></a>相关注解

- [`org.springframework.web.bind.annotation.@RequestMapping`](https://github.com/spring-projects/spring-framework/blob/master/spring-web/src/main/java/org/springframework/web/bind/annotation/RequestMapping.java)
- [`org.springframework.web.bind.annotation.@GetMapping`](https://github.com/spring-projects/spring-framework/blob/master/spring-web/src/main/java/org/springframework/web/bind/annotation/GetMapping.java)
- [`org.springframework.web.bind.annotation.@PostMapping`](https://github.com/spring-projects/spring-framework/blob/master/spring-web/src/main/java/org/springframework/web/bind/annotation/PostMapping.java)
- [`org.springframework.web.bind.annotation.@PutMapping`](https://github.com/spring-projects/spring-framework/blob/master/spring-web/src/main/java/org/springframework/web/bind/annotation/PutMapping.java)
- [`org.springframework.web.bind.annotation.@DeleteMapping`](https://github.com/spring-projects/spring-framework/blob/master/spring-web/src/main/java/org/springframework/web/bind/annotation/DeleteMapping.java)
- [`org.springframework.web.bind.annotation.@PatchMapping`](https://github.com/spring-projects/spring-framework/blob/master/spring-web/src/main/java/org/springframework/web/bind/annotation/PatchMapping.java)

###  10.3. <a name='Constructor_-1'></a>_Constructor_

`org.springframework.web.servlet.result.method.AbstractHandlerMethodMapping` ，实现 InitializingBean 接口，继承 AbstractHandlerMapping 抽象类，以 **Method** 作为 **Handler** 的 HandlerMapping 抽象类，提供 Mapping 的初始化、注册等通用的骨架方法。这就是我们常说的[“模板方法模式”](http://www.iocoder.cn/DesignPattern/xiaomingge/Template-Method/) 。

那么具体是什么呢？AbstractHandlerMethodMapping 定义为了 `<T>` 泛型，交给子类做决定。例如，子类 RequestMappingInfoHandlerMapping 使用 RequestMappingInfo 类作为 `<T>` 泛型，也就是`@RequestMapping` 等注解。

```java
// AbstractHandlerMethodMapping.java
public abstract class AbstractHandlerMethodMapping<T> extends AbstractHandlerMapping implements InitializingBean {
    /**
     * Mapping 注册表
     */
    private final MappingRegistry mappingRegistry = new MappingRegistry();

    /**
     * Mapping 命名策略, Handler 的 Method 的 Mapping 的名字生成策略接口
     * 情况一，如果 Mapping 已经配置名字，则直接返回。例如，@RequestMapping(name = "login", value = "user/login") 注解的方法，它对应的 Mapping 的名字就是 "login" 。
	 * 情况二，如果 Mapping 未配置名字，则使用使用类名大写 + "#" + 方法名。例如，@RequestMapping(value = "user/login") 注解的方法，假设它所在的类为 UserController ，对应的方法名为 login ，则它对应的 Mapping 的名字就是 USERCONTROLLER#login 
     */
    @Nullable
    private HandlerMethodMappingNamingStrategy<T> namingStrategy;
}
```



