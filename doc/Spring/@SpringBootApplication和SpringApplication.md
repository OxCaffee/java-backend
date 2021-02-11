# SpringBoot源码解析—— _@SpringBootApplication_ 和 _SpringApplication_

<!-- vscode-markdown-toc -->
* 1. [前言](#)
	* 1.1. [什么是自动装配](#-1)
* 2. [_@SpringBootApplication_](#SpringBootApplication_)
	* 2.1. [_@Inherited_](#Inherited_)
	* 2.2. [_@SpringBootConfiguration_](#SpringBootConfiguration_)
	* 2.3. [_@ComponentScan_](#ComponentScan_)
	* 2.4. [_@EnableAutoConfiguration_](#EnableAutoConfiguration_)
		* 2.4.1. [_AutoConfigurationImportSelector_](#AutoConfigurationImportSelector_)
* 3. [_SpringApplication_](#SpringApplication_)
	* 3.1. [构造方法](#-1)
	* 3.2. [_#run_](#run_)
	* 3.3. [_#prepareEnvironment_](#prepareEnvironment_)
	* 3.4. [_#createApplicationContext_](#createApplicationContext_)
	* 3.5. [_#prepareContext_](#prepareContext_)
	* 3.6. [_#refreshContext_](#refreshContext_)
	* 3.7. [_#callRunners_](#callRunners_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

先来看一段再常见不过的启动代码:

```java
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication //<1>
public class MyApplication{
    public static void main(String[] args){
        SpringApplication.run(MyApplication.class, args); //<2>
    }
}
```

* `<1>`处，使用@SpringBootApplication注解，标明是Spring应用，通过它，可以开启**自动配置**的功能
* `<2>`处，调用SpringApplication#run(Class<?>... primarySources)方法，**启动SpringBoot应用**

###  1.1. <a name='-1'></a>什么是自动装配

这个网上有很多优秀的博文，这里就不多做赘述，SpringBoot的自动配置主要包含一下的部分：

1. **@ConfigurationProperties** (非必须，主要是读取配置文件中的内容)
2. **@Configuration** : 配置类，主要在内部声明一些@Bean，若之前使用了@ConfigurationProperties，则需要在配置类上添加@EnableConfigurationProperties
3. **/META-INF/spring.factories** 中配置EnableAutoConfiguration

##  2. <a name='SpringBootApplication_'></a>_@SpringBootApplication_

`org.springframework.boot.autoconfigure.@SpringBootApplication` 注解，基本我们的 Spring Boot 应用，一定会去有这样一个注解。并且，通过使用它，不仅仅能标记这是一个 Spring Boot 应用，而且能够开启自动配置的功能。**但是如果项目不需要开启自动配置，那么就不需要这个注解**  ，下面先来看看注解的源码:

```java
// SpringBootApplication.java
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@SpringBootConfiguration
@EnableAutoConfiguration
@ComponentScan(excludeFilters = {
		@Filter(type = FilterType.CUSTOM, classes = TypeExcludeFilter.class),
		@Filter(type = FilterType.CUSTOM, classes = AutoConfigurationExcludeFilter.class) })
public @interface SpringBootApplication {

	/**
	 * Exclude specific auto-configuration classes such that they will never be applied.
	 * @return the classes to exclude
	 */
	@AliasFor(annotation = EnableAutoConfiguration.class)
	Class<?>[] exclude() default {};

	/**
	 * Exclude specific auto-configuration class names such that they will never be
	 * applied.
	 * @return the class names to exclude
	 * @since 1.3.0
	 */
	@AliasFor(annotation = EnableAutoConfiguration.class)
	String[] excludeName() default {};

	/**
	 * Base packages to scan for annotated components. Use {@link #scanBasePackageClasses}
	 * for a type-safe alternative to String-based package names.
	 * @return base packages to scan
	 * @since 1.3.0
	 */
	@AliasFor(annotation = ComponentScan.class, attribute = "basePackages")
	String[] scanBasePackages() default {};

	/**
	 * Type-safe alternative to {@link #scanBasePackages} for specifying the packages to
	 * scan for annotated components. The package of each class specified will be scanned.
	 * <p>
	 * Consider creating a special no-op marker class or interface in each package that
	 * serves no purpose other than being referenced by this attribute.
	 * @return base packages to scan
	 * @since 1.3.0
	 */
	@AliasFor(annotation = ComponentScan.class, attribute = "basePackageClasses")
	Class<?>[] scanBasePackageClasses() default {};

}
```

上面可以看到@SpringBootApplication是个组合注解，下面看看每一个注解的作用。

###  2.1. <a name='Inherited_'></a>_@Inherited_

`java.lang.annotation.@Inherited` 注解，使用此注解声明出来的自定义注解，在使用**此**自定义注解时，如果注解在类上面时，子类会自动继承此注解，否则的话，子类不会继承此注解。

**这里一定要记住，使用 `@Inherited` 声明出来的注解，只有在类上使用时才会有效，对方法，属性等其他无效。**

###  2.2. <a name='SpringBootConfiguration_'></a>_@SpringBootConfiguration_

`org.springframework.boot.@SpringBootConfiguration` 注解，标记这是一个 Spring Boot 配置类。代码如下：

```java
// SpringBootConfiguration.java
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Configuration
public @interface SpringBootConfiguration {}
```

可以看到，它上面继承自 `@Configuration` 注解，所以两者功能也一致，可以将当前类内声明的一个或多个以 `@Bean` 注解标记的方法的实例纳入到 Srping 容器中，并且实例名就是方法名。**换而言之，在SpringBoot启动类中定义Bean也是可以加入到Spring容器中去的**

###  2.3. <a name='ComponentScan_'></a>_@ComponentScan_

`org.springframework.context.annotation.@ComponentScan` 注解，扫描指定路径下的 Component（`@Componment`、`@Configuration`、`@Service` 等等）。

###  2.4. <a name='EnableAutoConfiguration_'></a>_@EnableAutoConfiguration_

`org.springframework.boot.autoconfigure.@EnableAutoConfiguration` 注解，用于开启自动配置功能，是 `spring-boot-autoconfigure` 项目最核心的注解。代码如下：

```java
// EnableAutoConfiguration.java

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@AutoConfigurationPackage //自动获取主程序所在的包路径，并将该路径下所有的子包的配置项添加进去
@Import(AutoConfigurationImportSelector.class)
public @interface EnableAutoConfiguration {

	String ENABLED_OVERRIDE_PROPERTY = "spring.boot.enableautoconfiguration";

	/**
	 * Exclude specific auto-configuration classes such that they will never be applied.
	 * @return the classes to exclude
	 */
	Class<?>[] exclude() default {};

	/**
	 * Exclude specific auto-configuration class names such that they will never be
	 * applied.
	 * @return the class names to exclude
	 * @since 1.3.0
	 */
	String[] excludeName() default {};

}
```

`org.springframework.boot.autoconfigure.@AutoConfigurationPackage` 注解，主要功能自动配置包，它会获取主程序类所在的包路径，并将包路径（包括子包）下的所有组件注册到 Spring IOC 容器中。代码如下：

```java
// AutoConfigurationPackage.java

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Inherited
@Import(AutoConfigurationPackages.Registrar.class)
public @interface AutoConfigurationPackage {}
```

####  2.4.1. <a name='AutoConfigurationImportSelector_'></a>_AutoConfigurationImportSelector_

`org.springframework.boot.autoconfigure.AutoConfigurationImportSelector` ，实现 DeferredImportSelector、BeanClassLoaderAware、ResourceLoaderAware、BeanFactoryAware、EnvironmentAware、Ordered 接口，处理 `@EnableAutoConfiguration` 注解的资源导入。

##### _#getCandidateConfigurations_

`#getCandidateConfigurations(AnnotationMetadata metadata, AnnotationAttributes attributes)` 方法，获得符合条件的配置类的数组，**其加载的就是/META-INF/spring.factories对应的配置项** ,代码如下：

```java
// AutoConfigurationImportSelector.java

protected List<String> getCandidateConfigurations(AnnotationMetadata metadata, AnnotationAttributes attributes) {
    // <1> 加载指定类型 EnableAutoConfiguration 对应的，在 `META-INF/spring.factories` 里的类名的数组
	List<String> configurations = SpringFactoriesLoader.loadFactoryNames(getSpringFactoriesLoaderFactoryClass(), getBeanClassLoader());
	// 断言，非空
	Assert.notEmpty(configurations, "No auto configuration classes found in META-INF/spring.factories. If you " + "are using a custom packaging, make sure that file is correct.");
	return configurations;
}
```

##### _#getImportGroup_

`#getImportGroup()` 方法，获得对应的 Group 实现类。代码如下：

```java
// AutoConfigurationImportSelector.java

@Override // 实现自 DeferredImportSelector 接口
public Class<? extends Group> getImportGroup() {
	return AutoConfigurationGroup.class;
}
```

AutoConfigurationGroup ，是 AutoConfigurationImportSelector 的内部类，实现 DeferredImportSelector.Group、BeanClassLoaderAware、BeanFactoryAware、ResourceLoaderAware 接口，自动配置的 Group 实现类。

##  3. <a name='SpringApplication_'></a>_SpringApplication_

`org.springframework.boot.SpringApplication` ，Spring 应用启动器。正如其代码上所添加的注释，它来提供启动 Spring 应用的功能。大多数情况下，我们都是使用它提供的**静态**方法：

```java
// SpringApplication.java

public static void main(String[] args) throws Exception {
	SpringApplication.run(new Class<?>[0], args);
}

public static ConfigurableApplicationContext run(Class<?> primarySource, String... args) {
	return run(new Class<?>[] { primarySource }, args);
}

public static ConfigurableApplicationContext run(Class<?>[] primarySources, String[] args) {
	// 创建 SpringApplication 对象，并执行运行。
	return new SpringApplication(primarySources).run(args);
}
```

###  3.1. <a name='-1'></a>构造方法

```java
// SpringApplication.java

/**
 * 资源加载器
 */
private ResourceLoader resourceLoader;
/**
 * 主要的 Java Config 类的数组
 */
private Set<Class<?>> primarySources;
/**
 * Web 应用类型，通过classpath，判断其web类型
 */
private WebApplicationType webApplicationType;

/**
 * ApplicationContextInitializer 数组
 */
private List<ApplicationContextInitializer<?>> initializers;
/**
 * ApplicationListener 数组
 */
private List<ApplicationListener<?>> listeners;

public SpringApplication(Class<?>... primarySources) {
    this(null, primarySources);
}

public SpringApplication(ResourceLoader resourceLoader, Class<?>... primarySources) {
    this.resourceLoader = resourceLoader;
    Assert.notNull(primarySources, "PrimarySources must not be null");
    this.primarySources = new LinkedHashSet<>(Arrays.asList(primarySources));
    this.webApplicationType = WebApplicationType.deduceFromClasspath();
    // 初始化 initializers 属性
    setInitializers((Collection) getSpringFactoriesInstances(ApplicationContextInitializer.class));
    // 初始化 listeners 属性
    setListeners((Collection) getSpringFactoriesInstances(ApplicationListener.class));
    this.mainApplicationClass = deduceMainApplicationClass();
}
```

###  3.2. <a name='run_'></a>_#run_

`#run(String... args)` 方法，运行 Spring 应用。代码如下：

```java
// SpringApplication.java

public ConfigurableApplicationContext run(String... args) {
    // <1> 创建 StopWatch 对象，并启动。StopWatch 主要用于简单统计 run 启动过程的时长。
    StopWatch stopWatch = new StopWatch();
    stopWatch.start();
    //
    ConfigurableApplicationContext context = null;
    Collection<SpringBootExceptionReporter> exceptionReporters = new ArrayList<>();
    // <2> 配置 headless 属性
    configureHeadlessProperty();
    // 获得 SpringApplicationRunListener 的数组，并启动监听
    SpringApplicationRunListeners listeners = getRunListeners(args);
    listeners.starting();
    try {
        // <3> 创建  ApplicationArguments 对象
        ApplicationArguments applicationArguments = new DefaultApplicationArguments(args);
        // <4> 加载属性配置。执行完成后，所有的 environment 的属性都会加载进来，包括 application.properties 和外部的属性配置。
        ConfigurableEnvironment environment = prepareEnvironment(listeners, applicationArguments);
        configureIgnoreBeanInfo(environment);
        // <5> 打印 Spring Banner
        Banner printedBanner = printBanner(environment);
        // <6> 创建 Spring 容器。
        context = createApplicationContext();
        // <7> 异常报告器
        exceptionReporters = getSpringFactoriesInstances(
                SpringBootExceptionReporter.class,
                new Class[] { ConfigurableApplicationContext.class }, context);
        // <8> 主要是调用所有初始化类的 initialize 方法
        prepareContext(context, environment, listeners, applicationArguments,
                printedBanner);
        // <9> 初始化 Spring 容器。
        refreshContext(context);
        // <10> 执行 Spring 容器的初始化的后置逻辑。默认实现为空。
        afterRefresh(context, applicationArguments);
        // <11> 停止 StopWatch 统计时长
        stopWatch.stop();
        // <12> 打印 Spring Boot 启动的时长日志。
        if (this.logStartupInfo) {
            new StartupInfoLogger(this.mainApplicationClass).logStarted(getApplicationLog(), stopWatch);
        }
        // <13> 通知 SpringApplicationRunListener 的数组，Spring 容器启动完成。
        listeners.started(context);
        // <14> 调用 ApplicationRunner 或者 CommandLineRunner 的运行方法。
        callRunners(context, applicationArguments);
    } catch (Throwable ex) {
        // <14.1> 如果发生异常，则进行处理，并抛出 IllegalStateException 异常
        handleRunFailure(context, ex, exceptionReporters, listeners);
        throw new IllegalStateException(ex);
    }

    // <15> 通知 SpringApplicationRunListener 的数组，Spring 容器运行中。
    try {
        listeners.running(context);
    } catch (Throwable ex) {
        // <15.1> 如果发生异常，则进行处理，并抛出 IllegalStateException 异常
        handleRunFailure(context, ex, exceptionReporters, null);
        throw new IllegalStateException(ex);
    }
    return context;
}
```

下面来看看几个关键的步骤:

###  3.3. <a name='prepareEnvironment_'></a>_#prepareEnvironment_

`#prepareEnvironment(SpringApplicationRunListeners listeners, ApplicationArguments applicationArguments)` 方法，加载属性配置。代码如下：

```java
// SpringApplication.java
private ConfigurableEnvironment prepareEnvironment(SpringApplicationRunListeners listeners, ApplicationArguments applicationArguments) {
	// Create and configure the environment
	// <1> 创建 ConfigurableEnvironment 对象，并进行配置
	ConfigurableEnvironment environment = getOrCreateEnvironment();
	configureEnvironment(environment, applicationArguments.getSourceArgs());
	// <2> 通知 SpringApplicationRunListener 的数组，环境变量已经准备完成。
	listeners.environmentPrepared(environment);
	// <3> 绑定 environment 到 SpringApplication 上
	bindToSpringApplication(environment);
	// <4> 如果非自定义 environment ，则根据条件转换
	if (!this.isCustomEnvironment) {
		environment = new EnvironmentConverter(getClassLoader()).convertEnvironmentIfNecessary(environment, deduceEnvironmentClass());
	}
	// <5> 如果有 attach 到 environment 上的 MutablePropertySources ，则添加到 environment 的 PropertySource 中。
	ConfigurationPropertySources.attach(environment);
	return environment;
}
```

###  3.4. <a name='createApplicationContext_'></a>_#createApplicationContext_

`#createApplicationContext()` 方法，创建 Spring 容器。代码如下：

```java
// SpringApplication.java
public static final String DEFAULT_CONTEXT_CLASS = "org.springframework.context."
		+ "annotation.AnnotationConfigApplicationContext";

/**
 * The class name of application context that will be used by default for web
 * environments.
 */
public static final String DEFAULT_SERVLET_WEB_CONTEXT_CLASS = "org.springframework.boot."
		+ "web.servlet.context.AnnotationConfigServletWebServerApplicationContext";

/**
 * The class name of application context that will be used by default for reactive web
 * environments.
 */
public static final String DEFAULT_REACTIVE_WEB_CONTEXT_CLASS = "org.springframework."
		+ "boot.web.reactive.context.AnnotationConfigReactiveWebServerApplicationContext";

protected ConfigurableApplicationContext createApplicationContext() {
    // 根据 webApplicationType 类型，获得 ApplicationContext 类型
	Class<?> contextClass = this.applicationContextClass;
	if (contextClass == null) {
		try {
			switch (this.webApplicationType) {
			case SERVLET:
				contextClass = Class.forName(DEFAULT_SERVLET_WEB_CONTEXT_CLASS);
				break;
			case REACTIVE:
				contextClass = Class.forName(DEFAULT_REACTIVE_WEB_CONTEXT_CLASS);
				break;
			default:
				contextClass = Class.forName(DEFAULT_CONTEXT_CLASS);
			}
		} catch (ClassNotFoundException ex) {
			throw new IllegalStateException("Unable create a default ApplicationContext, " + "please specify an ApplicationContextClass", ex);
		}
	}
	// 创建 ApplicationContext 对象
	return (ConfigurableApplicationContext) BeanUtils.instantiateClass(contextClass);
}
```

- 根据 `webApplicationType` 类型，获得对应的 ApplicationContext 对象。

###  3.5. <a name='prepareContext_'></a>_#prepareContext_

`#prepareContext(ConfigurableApplicationContext context, ConfigurableEnvironment environment, SpringApplicationRunListeners listeners, ApplicationArguments applicationArguments, Banner printedBanner)` 方法，准备 ApplicationContext 对象，主要是初始化它的一些属性。代码如下：

```java
// SpringApplication.java

private void prepareContext(ConfigurableApplicationContext context,
        ConfigurableEnvironment environment, SpringApplicationRunListeners listeners,
        ApplicationArguments applicationArguments, Banner printedBanner) {
    // <1> 设置 context 的 environment 属性
    context.setEnvironment(environment);
    // <2> 设置 context 的一些属性
    postProcessApplicationContext(context);
    // <3> 初始化 ApplicationContextInitializer
    applyInitializers(context);
    // <4> 通知 SpringApplicationRunListener 的数组，Spring 容器准备完成。
    listeners.contextPrepared(context);
    // <5> 打印日志
    if (this.logStartupInfo) {
        logStartupInfo(context.getParent() == null);
        logStartupProfileInfo(context);
    }
    // Add boot specific singleton beans
    // <6> 设置 beanFactory 的属性
    ConfigurableListableBeanFactory beanFactory = context.getBeanFactory();
    beanFactory.registerSingleton("springApplicationArguments", applicationArguments);
    if (printedBanner != null) {
        beanFactory.registerSingleton("springBootBanner", printedBanner);
    }
    if (beanFactory instanceof DefaultListableBeanFactory) {
        ((DefaultListableBeanFactory) beanFactory).setAllowBeanDefinitionOverriding(this.allowBeanDefinitionOverriding);
    }
    // Load the sources
    // <7> 加载 BeanDefinition 们
    Set<Object> sources = getAllSources();
    Assert.notEmpty(sources, "Sources must not be empty");
    load(context, sources.toArray(new Object[0]));
    // <8> 通知 SpringApplicationRunListener 的数组，Spring 容器加载完成。
    listeners.contextLoaded(context);
}
```

###  3.6. <a name='refreshContext_'></a>_#refreshContext_

`#refreshContext(ConfigurableApplicationContext context)` 方法，启动（刷新） Spring 容器。代码如下：

```java
// SpringApplication.java

/**
 * 是否注册 ShutdownHook 钩子
 */
private boolean registerShutdownHook = true;

private void refreshContext(ConfigurableApplicationContext context) {
	// <1> 开启（刷新）Spring 容器
	refresh(context);
	// <2> 注册 ShutdownHook 钩子
	if (this.registerShutdownHook) {
		try {
			context.registerShutdownHook();
		} catch (AccessControlException ex) {
			// Not allowed in some environments.
		}
	}
}
```

###  3.7. <a name='callRunners_'></a>_#callRunners_

`#callRunners(ApplicationContext context, ApplicationArguments args)` 方法，调用 ApplicationRunner 或者 CommandLineRunner 的运行方法。代码如下：

```java
// SpringApplication.java

private void callRunners(ApplicationContext context, ApplicationArguments args) {
    // <1> 获得所有 Runner 们
    List<Object> runners = new ArrayList<>();
    // <1.1> 获得所有 ApplicationRunner Bean 们
    runners.addAll(context.getBeansOfType(ApplicationRunner.class).values());
    // <1.2> 获得所有 CommandLineRunner Bean 们
    runners.addAll(context.getBeansOfType(CommandLineRunner.class).values());
    // <1.3> 排序 runners
    AnnotationAwareOrderComparator.sort(runners);
    // <2> 遍历 Runner 数组，执行逻辑
    for (Object runner : new LinkedHashSet<>(runners)) {
        if (runner instanceof ApplicationRunner) {
            callRunner((ApplicationRunner) runner, args);
        }
        if (runner instanceof CommandLineRunner) {
            callRunner((CommandLineRunner) runner, args);
        }
    }
}
```