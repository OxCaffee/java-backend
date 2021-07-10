# SpringCloud OpenFeign源码解析——openfeign配置

## 从yaml文件看openfeign配置

在SpringCloud官网，给出了openfeign的yaml文件示例配置，如下:

```yaml
feign:
    client:
    	default-config: my-default-config
        config:
            feignName:
                connectTimeout: 5000
                readTimeout: 5000
                loggerLevel: full
                errorDecoder: com.example.SimpleErrorDecoder
                retryer: com.example.SimpleRetryer
                defaultQueryParameters:
                    query: queryValue
                defaultRequestHeaders:
                    header: headerValue
                requestInterceptors:
                    - com.example.FooRequestInterceptor
                    - com.example.BarRequestInterceptor
                decode404: false
                encoder: com.example.SimpleEncoder
                decoder: com.example.SimpleDecoder
                contract: com.example.SimpleContract
                capabilities:
                    - com.example.FooCapability
                    - com.example.BarCapability
                metrics.enabled: false
```

在@EnableFeignClients主配置注解中，含有一个字段defaultConfiguration，其类型为`Class<?>[]` ，表示对**所有FeignClient的通用配置** ，但是，由于defaultConfiguration的返回类型为`Class<?>[]` ，所以具体的通用配置仍需要编写配置类`XxxxConfig` ，加上`@Configuration` 注解。而FeignClient的具体配置，即详细配置，更多的是在`feign.client.config` 路径里面，在FeignClientProperties类中，我们可以看到与配置文件相对应的配置项。

## FeignClientProperties

```java
@ConfigurationProperties("feign.client")
public class FeignClientProperties {

	private boolean defaultToProperties = true;

	private String defaultConfig = "default";

	private Map<String, FeignClientConfiguration> config = new HashMap<>();

	//Feign 客户端默认不编码斜杠 `/` 字符,如果要更改这个方式，请设置为false
	private boolean decodeSlash = true;
    
    //对应feign.client.config前缀
	public static class FeignClientConfiguration {

		private Logger.Level loggerLevel;

		private Integer connectTimeout;

		private Integer readTimeout;

		private Class<Retryer> retryer;

		private Class<ErrorDecoder> errorDecoder;

		private List<Class<RequestInterceptor>> requestInterceptors;

		private Map<String, Collection<String>> defaultRequestHeaders;

		private Map<String, Collection<String>> defaultQueryParameters;

        //是否编码404错误，如果false，则抛出异常
		private Boolean decode404;

		private Class<Decoder> decoder;

		private Class<Encoder> encoder;

		private Class<Contract> contract;

		private ExceptionPropagationPolicy exceptionPropagationPolicy;

		private List<Class<Capability>> capabilities;

		private MetricsProperties metrics;

		private Boolean followRedirects;
        
        ...
    }
    
    public static class MetricsProperties {

		private Boolean enabled = true;
        
        ...
    }
}
```

## FeignAutoConfiguration

FeignAutoConfiguration是配置类，其作用就是为Feign引入一些配置参数，声明代码如下:

```java
@Configuration(proxyBeanMethods = false)
@ConditionalOnClass(Feign.class)
@EnableConfigurationProperties({ FeignClientProperties.class, FeignHttpClientProperties.class,
		FeignEncoderProperties.class })
@Import(DefaultGzipDecoderConfiguration.class)
public class FeignAutoConfiguration {
    ...
}
```

FeignAutoConfiguration引入了FeignClientProperties，FeignHttpClientProperties，FeignEncoderProperties，DefaultGZipDecoderProperties等4个配置类。

在上一节的源码分析中，我们知道，@EnableFeignClients中的defaultProperties和@FeignClients中的configuration都会被注册为FeignClientSpecification，那么注册后的FeignClientSpecification只是简简单单存放在容器当中，等需要的时候再通过`applicationContext.getBean()` 来获取吗？答案是否定的，在FeignAutoConfiguration中，注册的FeignClientSpecification将会被集中存放在List数组中：

```java
@Autowired(required = false)
private List<FeignClientSpecification> configurations = new ArrayList<>();
```

上面利用了@Autowired注解的集中装配原理，将所有符合条件的FeignClientSpecification全部注入到List数组中。

出自之外，该类还注册了若干个重要的bean:

### FeignContext

**FeignContext是用于创建和管理FeignClient所依赖的各种类的工厂类** 。

<div align=center><img src="/assets/of1.png"/></div>

每一个FeignClient，都会关联一个AnnotationApplicationContext实例，用于存储FeignClient所依赖的各种类的实例，并以`FeignContext-{feign-client-name}` 作为Key。

@FeignClient中的configurations中保存了每个Feign Client所依赖的配置类，在创建AnnotationConfigApplicationContext的过程中，这些配置类会被注入到Bean工厂中。

FeignClient所依赖的类包括：

1. AnnotationConfigApplicationContext：
2. FeignLoggerFactory：Feign的日志工厂类，每一个Feign Client会关联一个Logger实例；
3. Feign.Builder：用于创建feign.Client实例；
4. feign.Client：
5. feign.codec.Encoder：
6. feign.codec.Decoder：
7. feign.codec.ErrorEncoder：可以通过ErrorEncoder将请求过程中捕获到的异常转换成指定的形式，从而避免调用方无法解析异常。
8. feign.Contract：
9. feign.Retryer：用于处理当Feign Client在请求过程中时捕获到 `RetryableException`时所要采取的动作；
10. feign.Request.Options:可以设置connectTimeoutMillis和readTimeoutMillis；
11. feign.RequestInterceptor：

### FeignJacksonConfiguration

配置前缀为``feign.autoconfiguration.jackson.enabled=true` 

```java
	@Configuration(proxyBeanMethods = false)
	@ConditionalOnClass({ Module.class, Page.class, Sort.class })
	@ConditionalOnProperty(value = "feign.autoconfiguration.jackson.enabled", havingValue = "true")
	protected static class FeignJacksonConfiguration {

		@Bean
		@ConditionalOnMissingBean(PageJacksonModule.class)
		public PageJacksonModule pageJacksonModule() {
			return new PageJacksonModule();
		}

		@Bean
		@ConditionalOnMissingBean(SortJacksonModule.class)
		public SortJacksonModule sortModule() {
			return new SortJacksonModule();
		}

	}
```

### HttpClientFeignConfiguration

```java
	// the following configuration is for alternate feign clients if
	// SC loadbalancer is not on the class path.
	// see corresponding configurations in FeignLoadBalancerAutoConfiguration
	// for load-balanced clients.
```

## FeignClientFactoryBean

在上面FeignAutoConfiguration中，我们通过@Autowired自动装配FeignClientSpecification，之后可以通过FeignClientFactoryBean#getObject来获取到对应的实例。

###  #configureFeign

```java
	protected void configureFeign(FeignContext context, Feign.Builder builder) {
		//首先尝试由BeanFactory获取FeignClientProperties Bean，如果获取不到由当前上下文applicationContext容器中去获取
		FeignClientProperties properties = beanFactory != null ? beanFactory.getBean(FeignClientProperties.class)
				: applicationContext.getBean(FeignClientProperties.class);

		//获取FeignClientConfigurer
		FeignClientConfigurer feignClientConfigurer = getOptional(context, FeignClientConfigurer.class);
		//配置是否继承父类上下文
		setInheritParentContext(feignClientConfigurer.inheritParentConfiguration());


		//如果FeignClient配置参数不为空且允许继承父类上下文
		if (properties != null && inheritParentContext) {
			if (properties.isDefaultToProperties()) {
				configureUsingConfiguration(context, builder);
				configureUsingProperties(properties.getConfig().get(properties.getDefaultConfig()), builder);
				configureUsingProperties(properties.getConfig().get(contextId), builder);
			}
			else {
				configureUsingProperties(properties.getConfig().get(properties.getDefaultConfig()), builder);
				configureUsingProperties(properties.getConfig().get(contextId), builder);
				configureUsingConfiguration(context, builder);
			}
		}
		else {
			configureUsingConfiguration(context, builder);
		}
	}
```

### #configureUsingConfiguration

```java
	protected void configureUsingConfiguration(FeignContext context, Feign.Builder builder) {
		Logger.Level level = getInheritedAwareOptional(context, Logger.Level.class);
		if (level != null) {
			builder.logLevel(level);
		}
		//最终目的都是为了根据当前的context，更具体的说是当前的contextId来获取bean
		Retryer retryer = getInheritedAwareOptional(context, Retryer.class);
		if (retryer != null) {
			builder.retryer(retryer);
		}
		ErrorDecoder errorDecoder = getInheritedAwareOptional(context, ErrorDecoder.class);
		if (errorDecoder != null) {
			builder.errorDecoder(errorDecoder);
		}
		else {
			FeignErrorDecoderFactory errorDecoderFactory = getOptional(context, FeignErrorDecoderFactory.class);
			if (errorDecoderFactory != null) {
				ErrorDecoder factoryErrorDecoder = errorDecoderFactory.create(type);
				builder.errorDecoder(factoryErrorDecoder);
			}
		}
		Request.Options options = getInheritedAwareOptional(context, Request.Options.class);
		if (options == null) {
			options = getOptionsByName(context, contextId);
		}

		if (options != null) {
			builder.options(options);
			readTimeoutMillis = options.readTimeoutMillis();
			connectTimeoutMillis = options.connectTimeoutMillis();
			followRedirects = options.isFollowRedirects();
		}
		Map<String, RequestInterceptor> requestInterceptors = getInheritedAwareInstances(context,
				RequestInterceptor.class);
		if (requestInterceptors != null) {
			List<RequestInterceptor> interceptors = new ArrayList<>(requestInterceptors.values());
			AnnotationAwareOrderComparator.sort(interceptors);
			builder.requestInterceptors(interceptors);
		}
		QueryMapEncoder queryMapEncoder = getInheritedAwareOptional(context, QueryMapEncoder.class);
		if (queryMapEncoder != null) {
			builder.queryMapEncoder(queryMapEncoder);
		}
		if (decode404) {
			builder.decode404();
		}
		ExceptionPropagationPolicy exceptionPropagationPolicy = getInheritedAwareOptional(context,
				ExceptionPropagationPolicy.class);
		if (exceptionPropagationPolicy != null) {
			builder.exceptionPropagationPolicy(exceptionPropagationPolicy);
		}

		Map<String, Capability> capabilities = getInheritedAwareInstances(context, Capability.class);
		if (capabilities != null) {
			capabilities.values().stream().sorted(AnnotationAwareOrderComparator.INSTANCE)
					.forEach(builder::addCapability);
		}
	}
```

### #getTarget配置FeignClient

```java
	<T> T getTarget() {
        //首先尝试获取FeignContext，如果当前容器中没有则从applicationContext中获取
		FeignContext context = beanFactory != null ? beanFactory.getBean(FeignContext.class)
				: applicationContext.getBean(FeignContext.class);
        //feign()主要为当前context设置logger
		Feign.Builder builder = feign(context);

        //修正url
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
		if (StringUtils.hasText(url) && !url.startsWith("http")) {
			url = "http://" + url;
		}
		String url = this.url + cleanPath();
        
        //根据context创建Client
		Client client = getOptional(context, Client.class);
		if (client != null) {
            //默认的client
			if (client instanceof FeignBlockingLoadBalancerClient) {
				// not load balancing because we have a url,
				// but Spring Cloud LoadBalancer is on the classpath, so unwrap
				client = ((FeignBlockingLoadBalancerClient) client).getDelegate();
			}
            //优先级仅次于FeignBlockingLoadBalancerClient
			if (client instanceof RetryableFeignBlockingLoadBalancerClient) {
				// not load balancing because we have a url,
				// but Spring Cloud LoadBalancer is on the classpath, so unwrap
				client = ((RetryableFeignBlockingLoadBalancerClient) client).getDelegate();
			}
			builder.client(client);
		}
		Targeter targeter = get(context, Targeter.class);
		return (T) targeter.target(this, builder, context, new HardCodedTarget<>(type, name, url));
	}
```

可以大致总结一下上面创建FeignClient的步骤：

1. 获取FeignContext
2. 根据FeignContext获取Client构造器FeignBuilder
3. 根据是否配置url创建相应的client