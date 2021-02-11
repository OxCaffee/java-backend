# Spring MVC面试题

<!-- vscode-markdown-toc -->
* 1. [Spring MVC框架有什么用](#SpringMVC)
* 2. [Spring MVC的核心组件](#SpringMVC-1)
* 3. [_DispatcherServlet_ 的工作流程](#DispatcherServlet_)
* 4. [_@Controller_ 的作用](#Controller_)
* 5. [_@RestController_ 和 _@Controller_ 的区别](#RestController__Controller_)
* 6. [_@RequestMapping_ 的作用](#RequestMapping_)
* 7. [_@RequestMapping_ 和 _@GetMapping_ 的作用](#RequestMapping__GetMapping_)
* 8. [返回JSON数据格式需要使用什么注解](#JSON)
* 9. [什么是 _WebApplicationContext_](#_WebApplicationContext_)
* 10. [Spring MVC的异常处理](#SpringMVC-1)
* 11. [Spring MVC优点](#SpringMVC-1)
* 12. [Spring MVC怎么设定重定向和转发](#SpringMVC-1)
* 13. [Spring MVC拦截器可以做的事情](#SpringMVC-1)
* 14. [Spring MVC拦截器和过滤器的区别](#SpringMVC-1)
* 15. [REST是什么](#REST)
* 16. [REST中的资源是什么](#REST-1)
* 17. [什么是安全的REST操作](#REST-1)
* 18. [什么是幂等操作](#)
* 19. [REST使用的HTTP方法](#RESTHTTP)
* 20. [删除的HTTP状态码是什么](#HTTP)
* 21. [REST API是无状态的吗](#RESTAPI)
* 22. [RestTemplate的优势是什么](#RestTemplate)
* 23. [_HttpMessageConverter_ 在Spring REST中代表什么](#HttpMessageConverter_SpringREST)
* 24. [如何创建_HttpMessageConverter_ 的自定义实现来支持一种新的请求/响应](#_HttpMessageConverter_)
* 25. [_@PathVariable_ 注解](#PathVariable_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='SpringMVC'></a>Spring MVC框架有什么用

Spring Web MVC 框架提供”模型-视图-控制器”( Model-View-Controller )架构和随时可用的组件，用于开发灵活且松散耦合的 Web 应用程序。

MVC 模式有助于分离应用程序的不同方面，如输入逻辑，业务逻辑和 UI 逻辑，同时在所有这些元素之间提供松散耦合。

##  2. <a name='SpringMVC-1'></a>Spring MVC的核心组件

Spring MVC 一共有九大核心组件，分别是：

- MultipartResolver
- LocaleResolver
- ThemeResolver
- HandlerMapping
- HandlerAdapter
- HandlerExceptionResolver
- RequestToViewNameTranslator
- ViewResolver
- FlashMapManager

##  3. <a name='DispatcherServlet_'></a>_DispatcherServlet_ 的工作流程

<div align=center><img src="/assets/dispatcher2.png"/></div>

1. 用户向服务器发送请求，请求被MVC的调度控制器DispatcherServlet捕获
2. DispatcherServlet根据请求的URL，调用HandlerMapping获得该Handler配置的相关对象，包括Handler对象以及Handler对应的拦截器，最后以HandlerExecutionChain的形式返回
3. 获得Handler之后，DispatcherServlet会选择一个合适的HandlerAdapter(获得HandlerAdapter之后，会开始执行拦截器的#preHandle方法)，提取Request中的模型数据，填充Handler对象，然后执行Handler(或者Controller)，在Controller执行之前，Spring会做一些额外的工作，比如数据转换/数据格式化/数据验证等
4. Controller执行完成之后，向DispatcherServlet返回一个ModelAndView对象
5. *返回的ModelAndView会送给ViewResolver进行解析，解析完成之后会返回给DispatcherServlet一个View对象(丢弃)*
6. *ViewResolver结合Model和View，来渲染视图，返回给用户浏览器(丢弃)*

**实际上，目前主流的架构，在Controller执行完成之后，如果判断方法有@ResponseBody，则直接返回给用户Json数据结果**

<div align=center><img src="/assets/mvc_arc.png"/></div>

##  4. <a name='Controller_'></a>_@Controller_ 的作用

标记一个类为控制器

##  5. <a name='RestController__Controller_'></a>_@RestController_ 和 _@Controller_ 的区别

`@RestController` 注解，在 `@Controller` 基础上，增加了 `@ResponseBody` 注解，更加适合目前前后端分离的架构下，提供 Restful API ，返回例如 JSON 数据格式。当然，返回什么样的数据格式，根据客户端的 `"ACCEPT"` 请求头来决定。

##  6. <a name='RequestMapping_'></a>_@RequestMapping_ 的作用

`@RequestMapping` 注解，用于将特定 HTTP 请求方法映射到将处理相应请求的控制器中的特定类/方法。此注释可应用于两个级别：

- 类级别：映射请求的 URL。
- 方法级别：映射 URL 以及 HTTP 请求方法。

##  7. <a name='RequestMapping__GetMapping_'></a>_@RequestMapping_ 和 _@GetMapping_ 的作用

- `@RequestMapping` 可注解在类和方法上；`@GetMapping` 仅可注册在方法上。
- `@RequestMapping` 可进行 GET、POST、PUT、DELETE 等请求方法；`@GetMapping` 是 `@RequestMapping` 的 GET 请求方法的特例，目的是为了提高清晰度。

##  8. <a name='JSON'></a>返回JSON数据格式需要使用什么注解

可以使用 **`@ResponseBody`** 注解，或者使用包含 `@ResponseBody` 注解的 **`@RestController`** 注解。

当然，还是需要配合相应的支持 JSON 格式化的 HttpMessageConverter 实现类。例如，Spring MVC 默认使用 MappingJackson2HttpMessageConverter 。

##  9. <a name='_WebApplicationContext_'></a>什么是 _WebApplicationContext_

WebApplicationContext 是实现ApplicationContext接口的子类，专门为 WEB 应用准备的。

- 它允许从相对于 Web 根目录的路径中**加载配置文件**，**完成初始化 Spring MVC 组件的工作**。
- 从 WebApplicationContext 中，可以获取 ServletContext 引用，整个 Web 应用上下文对象将作为属性放置在 ServletContext 中，以便 Web 应用环境可以访问 Spring 上下文。

##  10. <a name='SpringMVC-1'></a>Spring MVC的异常处理

Spring MVC 提供了异常解析器 HandlerExceptionResolver 接口，将处理器( `handler` )执行时发生的异常，解析( 转换 )成对应的 ModelAndView 结果。代码如下：

```java
// HandlerExceptionResolver.java
public interface HandlerExceptionResolver {

    /**
     * 解析异常，转换成对应的 ModelAndView 结果
     */
    @Nullable
    ModelAndView resolveException(
            HttpServletRequest request, HttpServletResponse response, @Nullable Object handler, Exception ex);
}
```

也就是说，如果异常被解析成功，则会返回 ModelAndView 对象。

##  11. <a name='SpringMVC-1'></a>Spring MVC优点

1. 使用真的真的真的非常**方便**，无论是添加 HTTP 请求方法映射的方法，还是不同数据格式的响应。
2. 提供**拦截器机制**，可以方便的对请求进行拦截处理。
3. 提供**异常机制**，可以方便的对异常做统一处理。
4. 可以任意使用各种**视图**技术，而不仅仅局限于 JSP ，例如 Freemarker、Thymeleaf 等等。
5. 不依赖于 Servlet API (目标虽是如此，但是在实现的时候确实是依赖于 Servlet 的，当然仅仅依赖 Servlet ，而不依赖 Filter、Listener )。

##  12. <a name='SpringMVC-1'></a>Spring MVC怎么设定重定向和转发

- 结果转发：在返回值的前面加 `"forward:/"` 。
- 重定向：在返回值的前面加上 `"redirect:/"` 。

##  13. <a name='SpringMVC-1'></a>Spring MVC拦截器可以做的事情

拦截器能做的事情非常非常非常多，例如：

- 记录访问日志。
- 记录异常日志。
- 需要登陆的请求操作，拦截未登陆的用户。

##  14. <a name='SpringMVC-1'></a>Spring MVC拦截器和过滤器的区别

看了文章 [《过滤器( Filter )和拦截器( Interceptor )的区别》](https://blog.csdn.net/xiaodanjava/article/details/32125687) ，感觉对比的怪怪的。艿艿觉得主要几个点吧：

- **功能相同**：拦截器和 Filter都能实现相应的功能，谁也不比谁强。
- **容器不同**：拦截器构建在 Spring MVC 体系中；Filter 构建在 Servlet 容器之上。
- **使用便利性不同**：拦截器提供了三个方法，分别在不同的时机执行；过滤器仅提供一个方法，当然也能实现拦截器的执行时机的效果，就是麻烦一些。

##  15. <a name='REST'></a>REST是什么

REST 代表着抽象状态转移，它是根据 HTTP 协议从客户端发送数据到服务端，例如：服务端的一本书可以以 XML 或 JSON 格式传递到客户端。

然而，假如你不熟悉REST，我建议你先看看 [REST API design and development](http://bit.ly/2zIGzWK) 这篇文章来更好的了解它。不过对于大多数胖友的英语，可能不太好，所以也可以阅读知乎上的 [《怎样用通俗的语言解释 REST，以及 RESTful？》](https://www.zhihu.com/question/28557115) 讨论。

##  16. <a name='REST-1'></a>REST中的资源是什么

资源是指数据在 REST 架构中如何显示的。将实体作为资源公开 ，它允许客户端通过 HTTP 方法如：[GET](http://javarevisited.blogspot.sg/2012/03/get-post-method-in-http-and-https.html), [POST](http://www.java67.com/2014/08/difference-between-post-and-get-request.html),[PUT](http://www.java67.com/2016/09/when-to-use-put-or-post-in-restful-web-services.html), DELETE 等读，写，修改和创建资源。

##  17. <a name='REST-1'></a>什么是安全的REST操作

REST 接口是通过 HTTP 方法完成操作。

- 一些HTTP操作是安全的，如 GET 和 HEAD ，它不能在服务端修改资源
- 换句话说，PUT,POST 和 DELETE 是不安全的，因为他们能修改服务端的资源。

所以，是否安全的界限，在于**是否修改**服务端的资源。

##  18. <a name=''></a>什么是幂等操作

有一些HTTP方法，如：GET，不管你使用多少次它都能产生相同的结果，在没有任何一边影响的情况下，发送多个 GET 请求到相同的[URI](http://www.java67.com/2013/01/difference-between-url-uri-and-urn.html) 将会产生相同的响应结果。因此，这就是所谓**幂等**操作。

换句话说，[POST方法不是幂等操作](http://javarevisited.blogspot.sg/2016/05/what-are-idempotent-and-safe-methods-of-HTTP-and-REST.html) ，因为如果发送多个 POST 请求，它将在服务端创建不同的资源。但是，假如你用PUT更新资源，它将是幂等操作。

甚至多个 PUT 请求被用来更新服务端资源，将得到相同的结果。你可以通过 Pluralsight 学习 [HTTP Fundamentals](http://pluralsight.pxf.io/c/1193463/424552/7490?u=https%3A%2F%2Fwww.pluralsight.com%2Fcourses%2Fxhttp-fund) 课程来了解 HTTP 协议和一般的 HTTP 的更多幂等操作。

##  19. <a name='RESTHTTP'></a>REST使用的HTTP方法

REST 能用任何的 HTTP 方法，但是，最受欢迎的是：

- 用 GET 来检索服务端资源
- 用 POST 来创建服务端资源
- [用 PUT 来更新服务端资源](http://javarevisited.blogspot.sg/2016/04/what-is-purpose-of-http-request-types-in-RESTful-web-service.html#axzz56WGunSwy)
- 用 DELETE 来删除服务端资源。

恰好，这四个操作，对上我们日常逻辑的 CRUD 操作。

##  20. <a name='HTTP'></a>删除的HTTP状态码是什么

在删除成功之后，您的 REST API 应该返回什么状态代码，并没有严格的规则。它可以返回 200 或 204 没有内容。

- 一般来说，如果删除操作成功，响应主体为空，返回 [204](http://www.netingcn.com/http-status-204.html) 。
- 如果删除请求成功且响应体不是空的，则返回 200 。

##  21. <a name='RESTAPI'></a>REST API是无状态的吗

**是的**，REST API 应该是无状态的，因为它是基于 HTTP 的，它也是无状态的。

REST API 中的请求应该包含处理它所需的所有细节。它**不应该**依赖于以前或下一个请求或服务器端维护的一些数据，例如会话。

**REST 规范为使其无状态设置了一个约束，在设计 REST API 时，您应该记住这一点**。

##  22. <a name='RestTemplate'></a>RestTemplate的优势是什么

在 Spring Framework 中，RestTemplate 类是 [模板方法模式](http://www.java67.com/2012/09/top-10-java-design-pattern-interview-question-answer.html) 的实现。跟其他主流的模板类相似，如 JdbcTemplate 或 JmsTempalte ，它将在客户端简化跟 RESTful Web 服务的集成。正如在 RestTemplate 例子中显示的一样，你能非常容易地用它来调用 RESTful Web 服务。

##  23. <a name='HttpMessageConverter_SpringREST'></a>_HttpMessageConverter_ 在Spring REST中代表什么

HttpMessageConverter 是一种[策略接口](http://www.java67.com/2014/12/strategy-pattern-in-java-with-example.html) ，它指定了一个转换器，它可以转换 HTTP 请求和响应。Spring REST 用这个接口转换 HTTP 响应到多种格式，例如：JSON 或 XML 。

每个 HttpMessageConverter 实现都有一种或几种相关联的MIME协议。Spring 使用 `"Accept"` 的标头来确定客户端所期待的内容类型。

然后，它将尝试找到一个注册的 HTTPMessageConverter ，它能够处理特定的内容类型，并使用它将响应转换成这种格式，然后再将其发送给客户端。

如果胖友对 HttpMessageConverter 不了解，可以看看 [《Spring 中 HttpMessageConverter 详解》](https://leokongwq.github.io/2017/06/14/spring-MessageConverter.html) 。

##  24. <a name='_HttpMessageConverter_'></a>如何创建_HttpMessageConverter_ 的自定义实现来支持一种新的请求/响应

我们仅需要创建自定义的 AbstractHttpMessageConverter 的实现，并使用 WebMvcConfigurerAdapter 的 `#extendMessageConverters(List<HttpMessageConverter<?>> converters)` 方法注中册它，该方法可以生成一种新的请求 / 响应类型。

具体的示例，可以学习 [《在 Spring 中集成 Fastjson》](https://github.com/alibaba/fastjson/wiki/在-Spring-中集成-Fastjson) 文章。

##  25. <a name='PathVariable_'></a>_@PathVariable_ 注解

`@PathVariable` 注解，是 Spring MVC 中有用的注解之一，它允许您从 URI 读取值，比如查询参数。它在使用 Spring 创建 RESTful Web 服务时特别有用，因为在 REST 中，资源标识符是 URI 的一部分。



