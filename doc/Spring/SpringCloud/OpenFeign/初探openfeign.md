# Spring Cloud OpenFeign源码解析——初探OpenFeign

## 从Feign的使用说起

SpringCloud将Feign集成进自己的体系中，实现服务的声明式HTTP调用，相比于RestTemplate而言简化了代码的编写复杂度，提高了代码的可读性。

Spring Cloud OpenFeign 除了支持 Feign 自带的注解之外，额外提供了对 JAX-RS 注解、SpringMVC 注解的支持。特别是对 **SpringMVC 注解**的支持。

* 在服务的提供方，首先要先在Nacos中注册服务的提供者，即`ServiceProvider` ，其余代码的编写与正常的Controller编写并无差异，简要代码如下:

```java
@RestController
public class ProviderController {

    private Logger logger = LoggerFactory.getLogger(ProviderController.class);

    @Value("${server.port}")
    private Integer serverPort;

    @GetMapping("/echo")
    public String echo(String name) throws InterruptedException {
        // 模拟执行 100ms 时长。方便后续我们测试请求超时
        Thread.sleep(100L);

        // 记录被调用的日志
        logger.info("[echo][被调用啦 name({})]", name);

        return serverPort + "-provider:" + name;
    }
}
```

* 在服务的消费方，仅仅需要声明相应需要调用的方法的接口，并在接口上添加`@FeignClient` 注解

```java
@FeignClient(name = "demo-provider")
public interface DemoProviderFeignClient {

    @GetMapping("/echo")
    String echo(@RequestParam("name") String name);
}
```

之后在消费者的Controller层就可以调用相应的方法了

```java
@FeignClient(name = "demo-provider")
public interface DemoProviderFeignClient {

    @GetMapping("/echo")
    String echo(@RequestParam("name") String name);
}
```

别忘了，启动项还要加上`@EnableFeignClients` 注解来开启Feign服务

```java
@SpringBootApplication
@EnableFeignClients
public class DemoConsumerApplication {

    public static void main(String[] args) {
        SpringApplication.run(DemoConsumerApplication.class, args);
    }
}
```

上面就是SpringCloud Feign一个最基本的使用了，下面我们就开启Feign的探索之旅。

## @EnableFeignClients

上面的示例代码中，@EnableFeignClients是加在启动类上的注解，故它完成了**服务消费者方** 有关的配置，简要代码如下:

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Documented
@Import(FeignClientsRegistrar.class)
public @interface EnableFeignClients {

	//value的值表示的是扫描的基础包路径backPackages
	String[] value() default {};

	//与value互为别名的关系
	String[] basePackages() default {};

	//指定类所在的包将会开启扫描
	Class<?>[] basePackageClasses() default {};

	//所有feignClients的自定义配置
	Class<?>[] defaultConfiguration() default {};

	//所有添加了@FeignClient注解的类
	Class<?>[] clients() default {};

}
```

注意在@EnbaleFeignClient注解上还有一个FeignClientRegistrar类，从字面的角度来看，**大概是对FeignClient进行注册** ，下面我们就来看看这个类。

## FeignClientsRegistrar

FeignClientsRegistrar的类声明如下:

```java
class FeignClientsRegistrar implements ImportBeanDefinitionRegistrar, ResourceLoaderAware, EnvironmentAware {
    ...
}
```

看到ImportBeanDefinitionRegistrar，毫无疑问这是一个BeanDefinition的注册相关类，既然继承了ImportBeanDefinitionRegistrar，那么就一定有`#registerBeanDefinitions方法` ，FeignClientsRegistrar重写了这个方法，如下:

```java
@Override
public void registerBeanDefinitions(AnnotationMetadata metadata, BeanDefinitionRegistry registry) {
	//<1>注册default配置
    registerDefaultConfiguration(metadata, registry);
    //<2>注册所有feign clients
	registerFeignClients(metadata, registry);
}
```

下面具体看看两个方法:

###  #registerDefaultConfiguration

```java
private void registerDefaultConfiguration(AnnotationMetadata metadata, BeanDefinitionRegistry registry) {
	//获取@EnableFeignClients注解上的所有属性，采用 (K-V) 的方式
	Map<String, Object> defaultAttrs = metadata.getAnnotationAttributes(EnableFeignClients.class.getName(), true);

	//如果@EnableFeignClients上有显式指定的defaultConfiguration属性，那么就在前面加一个default前缀
	if (defaultAttrs != null && defaultAttrs.containsKey("defaultConfiguration")) {
		String name;
		if (metadata.hasEnclosingClass()) {
			name = "default." + metadata.getEnclosingClassName();
		}
		else {
			name = "default." + metadata.getClassName();
		}
        //继续执行registerClientConfiguration
		registerClientConfiguration(registry, name, defaultAttrs.get("defaultConfiguration"));
	}
}
```

大概总结一下上面的操作:

1. 获取@EnableFeignClients注解上的所有属性，存储在Mapd的K-V键值对当中
2. 如果@EnableFeignClients注解上有显式指定defaultConfiguration属性，那么注解元数据中将注解元数据中的类名称前面加上`default.`字段
3. 继续执行registerClientConfiguration方法

### #registerClientConfiguration

```java
private void registerClientConfiguration(BeanDefinitionRegistry registry, Object name, Object configuration) {
	//FeignClientSpecification含有两个成员变量name和configuration
	BeanDefinitionBuilder builder = BeanDefinitionBuilder.genericBeanDefinition(FeignClientSpecification.class);
    
	builder.addConstructorArgValue(name);
	builder.addConstructorArgValue(configuration);
    
	//将该BeanDefinition注册到BeanDefinitionRegistry当中
    //<1>
	registry.registerBeanDefinition(name + "." + FeignClientSpecification.class.getSimpleName(),
				builder.getBeanDefinition());
}
```

上面代码中出现了一个FeignClientSpecification类，其实就是对feign配置的一个包装，里面含有name和configuration字段:

```java
class FeignClientSpecification implements NamedContextFactory.Specification {
	private String name;
	private Class<?>[] configuration;
    ...
}
```

回归到registerClientConfiguration当中，会发现，真正执行注册的是最后一步，也就是`<1>`标注的地方，其中注册的beanDefinition的名称为`default.(可选)` + @EnableFeignClients中的类名 + FeignClient简单名称

### #registerFeignClients

```java
	public void registerFeignClients(AnnotationMetadata metadata, BeanDefinitionRegistry registry) {
		//链表
		LinkedHashSet<BeanDefinition> candidateComponents = new LinkedHashSet<>();

		//获取@EnableFeignClients注解上的所有属性信息
		Map<String, Object> attrs = metadata.getAnnotationAttributes(EnableFeignClients.class.getName());

		//如果属性不为null, 那么获取@EnableFeignClients上的clients属性
		//clients属性的值是一个数组，包含了所有添加了@FeignClient注解的类，如果clients不为空，取消base package扫描
		final Class<?>[] clients = attrs == null ? null : (Class<?>[]) attrs.get("clients");

		//如果没有clients
		if (clients == null || clients.length == 0) {
			//类路径下的扫描器
			ClassPathScanningCandidateComponentProvider scanner = getScanner();
			//设定资源加载器
			scanner.setResourceLoader(this.resourceLoader);
			//扫描器扫描的对象就是@FeignClient注解标注的类
			scanner.addIncludeFilter(new AnnotationTypeFilter(FeignClient.class));
			//扫描的开始位置，该AnnotationMetadata对应的类包路径作为 basePackage
			Set<String> basePackages = getBasePackages(metadata);

			for (String basePackage : basePackages) {
				candidateComponents.addAll(scanner.findCandidateComponents(basePackage));
			}
		}

		//如果有clients配置，直接依次添加在LinkedList当中
		else {
			for (Class<?> clazz : clients) {
				candidateComponents.add(new AnnotatedGenericBeanDefinition(clazz));
			}
		}

		//依次遍历LinkedList，取得所有标注了@FeignClient的类对应的BeanDefinition
		for (BeanDefinition candidateComponent : candidateComponents) {
			//如果该BeanDefinition被注解标注
			if (candidateComponent instanceof AnnotatedBeanDefinition) {
				// verify annotated class is an interface
				//验证该BeanDefinition是一个接口，因为@FeignClient仅仅只能标注在接口上
				AnnotatedBeanDefinition beanDefinition = (AnnotatedBeanDefinition) candidateComponent;
				AnnotationMetadata annotationMetadata = beanDefinition.getMetadata();
				Assert.isTrue(annotationMetadata.isInterface(), "@FeignClient can only be specified on an interface");

				Map<String, Object> attributes = annotationMetadata
						.getAnnotationAttributes(FeignClient.class.getCanonicalName());

				String name = getClientName(attributes);
				registerClientConfiguration(registry, name, attributes.get("configuration"));

				registerFeignClient(registry, annotationMetadata, attributes);
			}
		}
	}
```

#registerFeignClients主要执行了以下几个步骤：

1. 获取@EnableFeignClients上所有的属性信息
2. 获取@EnableFeignClients上的clients属性配置
   1. 如果没有clients字段，就要自动扫描basePackage包下所有添加了@FeignClient注解的类，组成BeanDefinition
   2. 如果有clients字段，不进行包扫描，按照clients字段中的配置组装BeanDefinition
3. 经过步骤2，会得到一个BeanDefinition的链表，里面是所有添加了@FeignClient的类的BeanDefinition，遍历该链表，检查该BeanDefinition对应的原始类是否是一个interface
   1. 如果是，继续后面的操作
   2. 如果不是，抛出异常，因为@FeignClient只能加在interface上
4. 过滤出符合要求的BeanDefinition后，执行#registerClientConfiguration完成相应client的configuraton注册
5. 执行#registerFeignClient完成真正的FeignClient注册

### #registerFeignClient(重要!!!!!!!注册FeignClient的关键步骤)

```java
	/**
	 * 重要!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 * 所有注册的核心步骤
	 *
	 * @param registry BeanDefinition注册中心
	 * @param annotationMetadata 注解元数据
	 * @param attributes K-V键值对表示的注解属性
	 */
	private void registerFeignClient(BeanDefinitionRegistry registry, AnnotationMetadata annotationMetadata,
			Map<String, Object> attributes) {
		//该注解的类全名称
		String className = annotationMetadata.getClassName();
		//获取该类
		Class clazz = ClassUtils.resolveClassName(className, null);
		//只有ConfigurableBeanFactory才能执行注册
		ConfigurableBeanFactory beanFactory = registry instanceof ConfigurableBeanFactory
				? (ConfigurableBeanFactory) registry : null;
		//根据该beanFactory获取上下文id
		String contextId = getContextId(beanFactory, attributes);
		//获取属性名称
		String name = getName(attributes);

		//实例化FeignClient工厂bean
		FeignClientFactoryBean factoryBean = new FeignClientFactoryBean();

		//参数配置
		factoryBean.setBeanFactory(beanFactory);
		factoryBean.setName(name);
		factoryBean.setContextId(contextId);
		factoryBean.setType(clazz);
		factoryBean.setRefreshableClient(isClientRefreshEnabled());

		BeanDefinitionBuilder definition = BeanDefinitionBuilder.genericBeanDefinition(clazz, () -> {
			factoryBean.setUrl(getUrl(beanFactory, attributes));
			factoryBean.setPath(getPath(beanFactory, attributes));
			factoryBean.setDecode404(Boolean.parseBoolean(String.valueOf(attributes.get("decode404"))));
			Object fallback = attributes.get("fallback");
			if (fallback != null) {
				factoryBean.setFallback(fallback instanceof Class ? (Class<?>) fallback
						: ClassUtils.resolveClassName(fallback.toString(), null));
			}
			Object fallbackFactory = attributes.get("fallbackFactory");
			if (fallbackFactory != null) {
				factoryBean.setFallbackFactory(fallbackFactory instanceof Class ? (Class<?>) fallbackFactory
						: ClassUtils.resolveClassName(fallbackFactory.toString(), null));
			}
			return factoryBean.getObject();
		});
		definition.setAutowireMode(AbstractBeanDefinition.AUTOWIRE_BY_TYPE);
		definition.setLazyInit(true);
		validate(attributes);

		AbstractBeanDefinition beanDefinition = definition.getBeanDefinition();
		beanDefinition.setAttribute(FactoryBean.OBJECT_TYPE_ATTRIBUTE, className);
		beanDefinition.setAttribute("feignClientsRegistrarFactoryBean", factoryBean);

		// has a default, won't be null
		boolean primary = (Boolean) attributes.get("primary");

		beanDefinition.setPrimary(primary);

		String[] qualifiers = getQualifiers(attributes);
		if (ObjectUtils.isEmpty(qualifiers)) {
			qualifiers = new String[] { contextId + "FeignClient" };
		}

		BeanDefinitionHolder holder = new BeanDefinitionHolder(beanDefinition, className, qualifiers);
		BeanDefinitionReaderUtils.registerBeanDefinition(holder, registry);

		registerOptionsBeanDefinition(registry, contextId);
	}
```

#registerFeignClient的关键步骤:

1. 获得添加了@FeignClient注解的类名称
2. 根据该名称获得相应的类
3. 获取BeanDefiniton的注册工厂beanFactory
4. 获得该beanFactory所在的上下文id, （一般一个@FeignClient会有一个上下文id)
5. 实例化FeignClient工厂bean, 即FeignClientFactoryBean
6. 完成factoryBean属性配置，path/url/fallback
7. 创建FeignClient的bean，注册到BeanDefinitonHolder中
8. 执行#registerOptionsBeanDefinition

### #registerOptionsBeanDefinition

```java
	private void registerOptionsBeanDefinition(BeanDefinitionRegistry registry, String contextId) {
		if (isClientRefreshEnabled()) {
            //beanName获取，为了更有区分度，加上contextid字段
			String beanName = Request.Options.class.getCanonicalName() + "-" + contextId;
			BeanDefinitionBuilder definitionBuilder = BeanDefinitionBuilder
					.genericBeanDefinition(OptionsFactoryBean.class);
			definitionBuilder.setScope("refresh");
			definitionBuilder.addPropertyValue("contextId", contextId);
			BeanDefinitionHolder definitionHolder = new BeanDefinitionHolder(definitionBuilder.getBeanDefinition(),
					beanName);
			definitionHolder = ScopedProxyUtils.createScopedProxy(definitionHolder, registry, true);
			//注册该bean
            BeanDefinitionReaderUtils.registerBeanDefinition(definitionHolder, registry);
		}
	}
```

## 总结

至此，我们Feign的源码初探结束了，本次源码主要探索了Feign的主配置注解@EnableFeignClients和@FeignClient是如何工作，被发现，注册到Spring容器中去的，接下来的文章我们会继续探索Feign是如何完成服务的调用过程。

