# Spring Boot面试题

<!-- vscode-markdown-toc -->
* 1. [简要阐述Spring Boot](#SpringBoot)
* 2. [Spring Boot提供的核心功能](#SpringBoot-1)
* 3. [Spring Boot优缺点](#SpringBoot-1)
	* 3.1. [优点](#)
	* 3.2. [缺点](#-1)
* 4. [Spring Boot/ Spring MVC/ Spring的区别](#SpringBootSpringMVCSpring)
* 5. [Spring Boot的Starter是什么](#SpringBootStarter)
	* 5.1. [Spring Boot常用的Starter有哪些](#SpringBootStarter-1)
* 6. [如何统一引入Spring Boot版本](#SpringBoot-1)
* 7. [运行Spring Boot项目有哪几种方式](#SpringBoot-1)
* 8. [如何打包Spring Boot项目](#SpringBoot-1)
* 9. [如何重新加载Spring Boot上面的更改，而无需重新启动服务器](#SpringBoot-1)
* 10. [Spring Boot的默认配置文件是什么](#SpringBoot-1)
* 11. [Spring Boot如何定义多套不同的环境配置](#SpringBoot-1)
* 12. [Spring Boot配置加载的顺序](#SpringBoot-1)
* 13. [Spring有哪些配置方式](#Spring)
* 14. [Spring的核心注解是哪一个](#Spring-1)
* 15. [Spring有哪几种读取配置的方式](#Spring-1)
* 16. [如何在Spring Boot启动的时候运行一些特殊的代码](#SpringBoot-1)
* 17. [SpringBoot如何将内嵌服务器换成Jetty](#SpringBootJetty)
* 18. [Spring Boot中的监视器Actuator是什么](#SpringBootActuator)
* 19. [Spring Boot怎么集成MVC](#SpringBootMVC)
* 20. [Spring Boot如何集成Spring Security](#SpringBootSpringSecurity)
* 21. [Spring Boot 支持哪些日志框架](#SpringBoot-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='SpringBoot'></a>简要阐述Spring Boot

Spring Boot是Spring的子项目，提供Spring的引导功能。

通过Spring Boot，开发者可以快速配置Spring项目，引入Spring MVC，Spring Transaction，Spring AOP，MyBatis等，无需不断重复编写繁重的Spring配置，降低了Spring的使用成本。

Spring Boot提供了各种start启动器，提供标准化的默认配置，例如：

* spring-boot-starter-web: 快速配置MVC
* mybatis-spring-boot-starter: 快速配置MyBatis
* incubator-dubbo-spring-boot-project: 快速配置Dubbo
* rocketmq-spring-boot-starter: 快速配置RocketMQ

##  2. <a name='SpringBoot-1'></a>Spring Boot提供的核心功能

1. 可以用`java -jar xxxxx.jar`方式让Spring Boot项目以jar包的方式运行
2. 通过**内嵌Servlet容器**可以让Spring Boot不必以war包的方式运行
3. 提供pom.xml简化maven配置
4. 自动配置bean
5. 准生产的应用监控
6. 无代码生成和xml配置: 使用的是@Condition注解以实现根据条件进行配置，同时使用了Maven/Gradle的依赖传递解析机制来实现Spring应用里面的自动配置

##  3. <a name='SpringBoot-1'></a>Spring Boot优缺点

###  3.1. <a name=''></a>优点

1. 使【编码】变简单。

2. 使【配置】变简单。

3. 使【部署】变简单。

4. 使【监控】变简单。

###  3.2. <a name='-1'></a>缺点

1. 没有提供相应的【服务发现与注册】
2. 自身的acturator所提供的【监控功能】，也需要现有的监控对接
3. 没有配套的【安全管控方案】
4. 对于REST的落地，还要自身结合实际进行URI的规范化工作

##  4. <a name='SpringBootSpringMVCSpring'></a>Spring Boot/ Spring MVC/ Spring的区别

Spring 的完整名字，应该是 Spring Framework 。它提供了多个模块，Spring IoC、Spring AOP、Spring MVC 等等。所以，Spring MVC 是 Spring Framework 众多模块中的一个。

而 Spring Boot 是构造在 Spring Framework 之上的 Boot 启动器，旨在更容易的配置一个 Spring 项目。

##  5. <a name='SpringBootStarter'></a>Spring Boot的Starter是什么

比如我们要在 Spring Boot 中引入 Web MVC 的支持时，我们通常会引入这个模块 `spring-boot-starter-web` ，而这个模块如果解压包出来会发现里面什么都没有，只定义了一些 **POM** 依赖。Starter 主要用来简化依赖用的。比如我们之前做MVC时要引入日志组件，那么需要去找到log4j的版本，然后引入，现在有了Starter之后，直接用这个之后，log4j就自动引入了，也不用关心版本这些问题。

###  5.1. <a name='SpringBootStarter-1'></a>Spring Boot常用的Starter有哪些

- `spring-boot-starter-web` ：提供 Spring MVC + 内嵌的 Tomcat 。
- `spring-boot-starter-data-jpa` ：提供 Spring JPA + Hibernate 。
- `spring-boot-starter-data-redis` ：提供 Redis 。
- `mybatis-spring-boot-starter` ：提供 MyBatis 。

##  6. <a name='SpringBoot-1'></a>如何统一引入Spring Boot版本

**目前有两种方式**。

* 方式一：继承 spring-boot-starter-parent项目。配置代码如下：

```xml
<parent>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-parent</artifactId>
    <version>1.5.1.RELEASE</version>
</parent>
```

* 方式二：导入 spring-boot-dependencies 项目依赖。配置代码如下：

```xml
<dependencyManagement>
    <dependencies>
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-dependencies</artifactId>
            <version>1.5.1.RELEASE</version>
            <type>pom</type>
            <scope>import</scope>
        </dependency>
    </dependencies>
</dependencyManagement>
```

**因为一般我们的项目中，都有项目自己的 Maven parent 项目，所以【方式一】显然会存在冲突。所以实际场景下，推荐使用【方式二】。**

##  7. <a name='SpringBoot-1'></a>运行Spring Boot项目有哪几种方式

1. 打包成 Fat Jar ，直接使用 `java -jar` 运行。目前主流的做法，推荐。

2. 在 IDEA 或 Eclipse 中，直接运行应用的 Spring Boot 启动类的 `#main(String[] args)` 启动。适用于开发调试场景。

3. 如果是 Web 项目，可以打包成 War 包，使用外部 Tomcat 或 Jetty 等容器。

##  8. <a name='SpringBoot-1'></a>如何打包Spring Boot项目

通过引入 `spring-boot-maven-plugin` 插件，执行 `mvn clean package` 命令，将 Spring Boot 项目打成一个 Fat Jar 。后续，我们就可以直接使用 `java -jar` 运行。

##  9. <a name='SpringBoot-1'></a>如何重新加载Spring Boot上面的更改，而无需重新启动服务器

一共有三种方式：

1. 【推荐】spring-boot-devtools插件，但是这个工具需要配置IDEA的自动编译
2. Spring Loaded插件(不再更新)
3. JRebel插件，需要付费

##  10. <a name='SpringBoot-1'></a>Spring Boot的默认配置文件是什么

对于 Spring Boot 应用，默认的配置文件根目录下的 **application** 配置文件，当然可以是 Properties 格式，也可以是 YAML 格式。

还有一个根目录下的 **bootstrap** 配置文件。这个是 Spring Cloud 新增的启动配置文件，[需要引入 `spring-cloud-context` 依赖后，才会进行加载](https://my.oschina.net/freeskyjs/blog/1843048)。

- 【特点】因为 bootstrap 由父 ApplicationContext 加载，比 application 优先加载。
- 【特点】因为 bootstrap 优先于 application 加载，所以不会被它覆盖。
- 【用途】使用配置中心 Spring Cloud Config 时，需要在 bootstrap 中配置配置中心的地址，从而实现父 ApplicationContext 加载时，从配置中心拉取相应的配置到应用中。

##  11. <a name='SpringBoot-1'></a>Spring Boot如何定义多套不同的环境配置

可以参考 [《Spring Boot 教程 - Spring Boot Profiles 实现多环境下配置切换》](https://blog.csdn.net/top_code/article/details/78570047) 一文

但是，需要考虑一个问题，**生产环境的配置文件的安全性，显然我们不能且不应该把生产的配置放到项目的 Git 仓库中进行管理。那么应该怎么办呢？**

- 方案一，生产环境的配置文件放在生产环境的服务器中，以 `java -jar myproject.jar --spring.config.location=/xxx/yyy/application-prod.properties` 命令，设置 参数 `spring.config.location` 指向配置文件。
- 方案二，使用 Jenkins 在执行打包，配置上 Maven Profile 功能，使用服务器上的配置文件。整体来说，和【方案一】的差异是，将配置文件打包进了 Jar 包中。
- 方案三，使用配置中心。

##  12. <a name='SpringBoot-1'></a>Spring Boot配置加载的顺序

1. `spring-boot-devtools`依赖的`spring-boot-devtools.properties`配置文件
2. 单元测试上的`@TestPropertySource`和`@SpringBootTest`
3. 命令行指定的参数。例如`java -jar springboot.jar --server.port=8090`
4. 命令行中的`spring.application.json`指定参数。例如`java -Dspring.application.json='{"name": "Java"}' -jar springboot.jar`
5. ServletConfig初始化参数
6. ServletContext初始化参数
7. JNDI参数。例如`java:comp/env`
8. Java系统变量。即`System#getProperties()`方法对应的
9. 操作系统环境变量
10. RandomValuePropertySource配置的`random.*`属性对应的值
11. Jar **外部**的带指定 profile 的 application 配置文件。例如 `application-{profile}.yaml` 。
12. Jar **内部**的带指定 profile 的 application 配置文件。例如 `application-{profile}.yaml` 。
13. Jar **外部** application 配置文件。例如 `application.yaml` 。
14. Jar **内部** application 配置文件。例如 `application.yaml` 。
15. 在自定义的 `@Configuration` 类中定于的 `@PropertySource` 。
16. 启动的 main 方法中，定义的默认配置。即通过 `SpringApplication#setDefaultProperties(Map<String, Object> defaultProperties)` 方法进行设置。

##  13. <a name='Spring'></a>Spring有哪些配置方式

和 Spring 一样，一共提供了三种方式。

- 1、XML 配置文件。

  Bean 所需的依赖项和服务在 XML 格式的配置文件中指定。这些配置文件通常包含许多 bean 定义和特定于应用程序的配置选项。它们通常以 bean 标签开头。例如：

  ```xml
  <bean id="studentBean" class="org.edureka.firstSpring.StudentBean">
      <property name="name" value="Edureka"></property>
  </bean>
  ```

- 2、注解配置。

  您可以通过在相关的类，方法或字段声明上使用注解，将 Bean 配置为组件类本身，而不是使用 XML 来描述 Bean 装配。默认情况下，Spring 容器中未打开注解装配。因此，您需要在使用它之前在 Spring 配置文件中启用它。例如：

  ```xml
  <beans>
  <context:annotation-config/>
  <!-- bean definitions go here -->
  </beans>
  ```

- 3、Java Config 配置。

  Spring 的 Java 配置是通过使用 @Bean 和 @Configuration 来实现。

  - `@Bean` 注解扮演与 `<bean />` 元素相同的角色。

  - `@Configuration` 类允许通过简单地调用同一个类中的其他 `@Bean` 方法来定义 Bean 间依赖关系。

  - 例如：

    ```java
    @Configuration
    public class StudentConfig {
        
        @Bean
        public StudentBean myStudent() {
            return new StudentBean();
        }
        
    }
    ```

    - 是不是很熟悉 😈

目前主要使用 **Java Config** 配置为主。当然，三种配置方式是可以混合使用的。例如说：

- Dubbo 服务的配置，艿艿喜欢使用 XML 。
- Spring MVC 请求的配置，艿艿喜欢使用 `@RequestMapping` 注解。
- Spring MVC 拦截器的配置，艿艿喜欢 Java Config 配置

另外，现在已经是 Spring Boot 的天下，所以更加是 **Java Config** 配置为主。

##  14. <a name='Spring-1'></a>Spring的核心注解是哪一个

@SpringBootApplication开启自动配置

##  15. <a name='Spring-1'></a>Spring有哪几种读取配置的方式

目前有两种:

1. `@Value`注解，最常用，搭配`@PropertySource` 指定使用的配置文件
2. `@ConfigurationProperties` 注解，读取配置到类上，可以搭配`@PropertySource` 指定使用的配置文件

##  16. <a name='SpringBoot-1'></a>如何在Spring Boot启动的时候运行一些特殊的代码

如果需要在 SpringApplication 启动后执行一些特殊的代码，你可以实现 ApplicationRunner 或 CommandLineRunner 接口，这两个接口工作方式相同，都只提供单一的 run 方法，该方法仅在 `SpringApplication#run(...)` 方法**完成之前调用**。

##  17. <a name='SpringBootJetty'></a>SpringBoot如何将内嵌服务器换成Jetty

默认情况下，`spring-boot-starter-web` 模块使用 Tomcat 作为内嵌的服务器。所以需要去除对 `spring-boot-starter-tomcat` 模块的引用，添加 `spring-boot-starter-jetty` 模块的引用。代码如下：

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-web</artifactId>
    <exclusions>
        <exclusion> <!-- 去除 Tomcat -->
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-tomcat</artifactId>
        </exclusion>
    </exclusions>
</dependency>
<dependency> <!-- 引入 Jetty -->
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-jetty</artifactId>
</dependency>
```

##  18. <a name='SpringBootActuator'></a>Spring Boot中的监视器Actuator是什么

`spring-boot-actuator` 提供 Spring Boot 的监视器功能，可帮助我们访问生产环境中正在运行的应用程序的**当前状态**。

- 关于 Spring Boot Actuator 的教程，可以看看 [《Spring Boot Actuator 使用》](https://www.jianshu.com/p/af9738634a21) 。
- 上述教程是基于 Spring Boot 1.X 的版本，如果胖友使用 Spring Boot 2.X 的版本，你将会发现 `/beans` 等 Endpoint 是不存在的，参考 [《Spring boot 2 - Actuator endpoint, where is /beans endpoint》](https://stackoverflow.com/questions/49174700/spring-boot-2-actuator-endpoint-where-is-beans-endpoint) 问题来解决。

**安全性**

Spring Boot 2.X 默认情况下，`spring-boot-actuator` 产生的 Endpoint 是没有安全保护的，但是 Actuator 可能暴露敏感信息。

所以一般的做法是，引入 `spring-boot-start-security` 依赖，使用 Spring Security 对它们进行安全保护。

##  19. <a name='SpringBootMVC'></a>Spring Boot怎么集成MVC

1. 引入 `spring-boot-starter-web` 的依赖。
2. 实现 WebMvcConfigurer 接口，可添加自定义的 Spring MVC 配置：**因为 Spring Boot 2 基于 JDK 8 的版本，而 JDK 8 提供 `default` 方法，所以 Spring Boot 2 废弃了 WebMvcConfigurerAdapter 适配类，直接使用 WebMvcConfigurer 即可。**

```java
// WebMvcConfigurer.java
public interface WebMvcConfigurer {

    /** 配置路径匹配器 **/
    default void configurePathMatch(PathMatchConfigurer configurer) {}
    
    /** 配置内容裁决的一些选项 **/
    default void configureContentNegotiation(ContentNegotiationConfigurer configurer) { }

    /** 异步相关的配置 **/
    default void configureAsyncSupport(AsyncSupportConfigurer configurer) { }

    default void configureDefaultServletHandling(DefaultServletHandlerConfigurer configurer) { }

    default void addFormatters(FormatterRegistry registry) {
    }

    /** 添加拦截器 **/
    default void addInterceptors(InterceptorRegistry registry) { }

    /** 静态资源处理 **/
    default void addResourceHandlers(ResourceHandlerRegistry registry) { }

    /** 解决跨域问题 **/
    default void addCorsMappings(CorsRegistry registry) { }

    default void addViewControllers(ViewControllerRegistry registry) { }

    /** 配置视图解析器 **/
    default void configureViewResolvers(ViewResolverRegistry registry) { }

    /** 添加参数解析器 **/
    default void addArgumentResolvers(List<HandlerMethodArgumentResolver> resolvers) {
    }

    /** 添加返回值处理器 **/
    default void addReturnValueHandlers(List<HandlerMethodReturnValueHandler> handlers) { }

    /** 这里配置视图解析器 **/
    default void configureMessageConverters(List<HttpMessageConverter<?>> converters) { }

    /** 配置消息转换器 **/
    default void extendMessageConverters(List<HttpMessageConverter<?>> converters) { }

   /** 配置异常处理器 **/
    default void configureHandlerExceptionResolvers(List<HandlerExceptionResolver> resolvers) { }

    default void extendHandlerExceptionResolvers(List<HandlerExceptionResolver> resolvers) { }

    @Nullable
    default Validator getValidator() { return null; }

    @Nullable
    default MessageCodesResolver getMessageCodesResolver() {  return null; }

}
```

在使用 Spring MVC 时，我们一般会做如下几件事情：

1. 实现自己项目需要的拦截器，并在 WebMvcConfigurer 实现类中配置。

   ```java
   @EnableWebMvc
   @Configuration
   public class MVCConfiguration extends WebMvcConfigurerAdapter {
       
       @Autowired
       private SecurityInterceptor securityInterceptor;
   
       @Override
       public void configureMessageConverters(List<HttpMessageConverter<?>> converters) {
           converters.add(new MappingJackson2HttpMessageConverter());
           super.configureMessageConverters(converters);
       }
   
       @Override
       public void addInterceptors(InterceptorRegistry registry) {
           registry.addInterceptor(securityInterceptor);
       }
   
   }
   ```

1. 配置 `@ControllerAdvice` + `@ExceptionHandler` 注解，实现全局异常处理。
2. 配置 `@ControllerAdvice` ，实现 ResponseBodyAdvice 接口，实现全局统一返回。

当然，有一点需要注意，WebMvcConfigurer、ResponseBodyAdvice、`@ControllerAdvice`、`@ExceptionHandler` 接口，都是 Spring MVC 框架自身已经有的东西。

- `spring-boot-starter-web` 的依赖，帮我们解决的是 Spring MVC 的依赖以及相关的 Tomcat 等组件。

##  20. <a name='SpringBootSpringSecurity'></a>Spring Boot如何集成Spring Security

目前比较主流的安全框架有两个：

1. Spring Security
2. Apache Shiro

对于任何项目来说，安全认证总是少不了，同样适用于使用 Spring Boot 的项目。相对来说，Spring Security 现在会比 Apache Shiro 更流行。

Spring Boot 和 Spring Security 的配置方式比较简单：

1. 引入 `spring-boot-starter-security` 的依赖。
2. 继承 WebSecurityConfigurerAdapter ，添加**自定义**的安全配置。

当然，每个项目的安全配置是不同的，需要胖友自己选择。更多详细的使用，建议认真阅读如下文章：

- [《Spring Boot中 使用 Spring Security 进行安全控制》](http://blog.didispace.com/springbootsecurity/) ，快速上手。
- [《Spring Security 实现原理与源码解析系统 —— 精品合集》](http://www.iocoder.cn/Spring-Security/good-collection/) ，深入源码。

另外，安全是一个很大的话题，感兴趣的胖友，可以看看 [《Spring Boot 十种安全措施》](https://www.jdon.com/49653) 一文。

##  21. <a name='SpringBoot-1'></a>Spring Boot 支持哪些日志框架

Spring Boot 支持的日志框架有：

- Logback
- Log4j2
- Log4j
- Java Util Logging