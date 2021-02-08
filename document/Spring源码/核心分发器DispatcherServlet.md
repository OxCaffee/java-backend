# 核心分发器 _DispatcherServlet_

## 前言

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

## _DispatcherServlet_ 的工作流程

Spring MVC框架是请求驱动的：所有设计都围绕着一个中央Servlet来展开，它负责把所有请求分发到控制器；同时提供其他Web应用开发所需要的功能。DispatcherServlet与Spring IoC容器做到了无缝集成，这意味着，Spring提供的任何特性在Spring MVC中你都可以使用。

下图展示了Spring Web MVC的DispatcherServlet处理请求的工作流，DispatcherServlet应用的其实就是一个“前端控制器”的设计模式：

<div align=center><img src="/asset/dispatcher2.png"/></div>

基于Spring Boot可以直接使用Spring MVC，除非有非常个性化的要求，相关配置不需要手动配置。

