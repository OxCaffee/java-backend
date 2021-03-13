# Spring面试题

<!-- vscode-markdown-toc -->
* 1. [什么是Spring?](#Spring)
* 2. [Spring有什么好处?](#Spring-1)
* 3. [Spring能帮我们做什么?](#Spring-1)
* 4. [Spring的组成部分有哪些?](#Spring-1)
* 5. [什么是Spring模块的上下文?](#Spring-1)
* 6. [ApplicationContext通常的实现是什么?](#ApplicationContext)
* 7. [什么是Spring中的Bean?](#SpringBean)
* 8. [如何定义Bean的作用域?](#Bean)
* 9. [什么是Bean的自动装配？](#Bean-1)
* 10. [解释一下Bean的各个作用域?](#Bean-1)
* 11. [Spring中的设计模式？](#Spring-1)
* 12. [简单解释一下Spring中的AOP原理？](#SpringAOP)
* 13. [什么是切点?](#)
* 14. [什么是连接点?](#-1)
* 15. [什么是织入?](#-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='Spring'></a>什么是Spring?

Spring 是个 java 企业级应⽤的开源开发框架，Spring 主要⽤来开发 Java 应⽤，但是有些扩展是针 对构建 J2EE 平台的 WEB 应⽤。Spring 框架⽬标是简化 Java 企业级应⽤开发，并通过 POJO 为基 础的编程模型促进良好的编程习惯。Spring 可以是使简单的 JavaBean 实现以前只有 EJB 才能实 现的功能。

Spring 容器的主要核心是：控制反转（IOC），传统的 java 开发模式中，当需要一个对象时，我 们会自己使用 new 或者 getInstance 等直接或者间接调用构造方法创建一个对象。而在 spring 开 发模式中，spring 容器使用了工厂模式为我们创建了所需要的对象，不需要我们自己创建了，直 接调用 spring 提供的对象就可以了，这是控制反转的思想。依赖注入（DI），spring 使用 javaBean 对象的set 方法或者带参数的构造方法为我们在创建所需对象时将其属性自动设置所需要的值的 过程，就是依赖注入的思想。面向切面编程（AOP），在面向对象编程（oop）思想中，我们将事物 纵向抽成一个个的对象。而在面向切面编程中，我们将一个个的对象某些类似的方面横向抽成一个 切面，对这个切面进行一些如权限控制、事物管理，记录日志等公用操作处理的过程就是面向切面编 程的思想。**AOP底层是动态代理，如果是接口采用JDK动态代理，如果是类采用CGLIB方式实现动态代理** 。

##  2. <a name='Spring-1'></a>Spring有什么好处?

* 轻量:Spring 是轻量的,基本的版本⼤约 2MB。 
* 控制反转:Spring 通过控制反转实现了松散耦合,对象们给它们的依赖,⽽不是创建或查找依赖的对象们。 
* ⾯向切⾯编程(AOP):Spring⽀持⾯向切⾯编程,并且把应⽤业务逻辑和系统服务分开。 
* 容器:Spring 包含并管理应⽤中对象的声明周期和配置。 
* MVC 框架:Spring 的 WEB 框架是个精⼼设计的框架,是 Web 框架的⼀个很好的替代品 
* 事务管理:Spring 提供⼀个持续的事务管理接⼝,可以扩展到上⾄本地事务下⾄全局事务 (JTA)。 
* 异常处理:Spring 提供⽅便的 API 把具体技术相关的异常(⽐如由 JDBC,HibernateorJDO 抛出 的)转化为⼀致的 unchecked 异常。

##  3. <a name='Spring-1'></a>Spring能帮我们做什么?

* Spring 能帮我们根据配置文件创建及组装对象之间的依赖关系。Spring 根据配置文件来进行创 建及组装对象间依赖关系，只需要改配置文件即可 
* Spring 面向切面编程能帮助我们无耦合的实现日志记录，性能统计，安全控制。Spring 面向切 面编程能提供一种更好的方式来完成，一般通过配置方式，而且不需要在现有代码中添加任何额 外代码，现有代码专注业务逻辑。 
* Spring 能非常简单的帮我们管理数据库事务。采用 Spring，我们只需获取连接，执行 SQL，其 他事物相关的都交给 Spring 来管理了。 
* Spring 还能与第三方数据库访问框架（如 Hibernate、JPA）无缝集成，而且自己也提供了一套 JDBC 访问模板，来方便数据库访问。 
* Spring 还能与第三方 Web（如 Struts、JSF）框架无缝集成，而且自己也提供了一套 SpringMVC 框架，来方便 web 层搭建
* Spring 能方便的与 JavaEE（如 JavaMail、任务调度）整合，与更多 技术整合（比如缓存框架）。

##  4. <a name='Spring-1'></a>Spring的组成部分有哪些?

* 核心容器：包括 Core、Beans、Context、EL 模块。Core 模块：封装了框架依赖的最底层部 分，包括资源访问、类型转换及一些常用工具类。Beans 模块：提供了框架的基础部分，包括反转 控制和依赖注入。其中 BeanFactory 是容器核心，本质是“工厂设计模式”的实现，而且无需编程 实现“单例设计模式”，单例完全由容器控制，而且提倡面向接口编程，而非面向实现编程； 所有应用 程序对象及对象间关系由框架管理，从而真正把你从程序逻辑中把维护对象之间的依赖关系提取出 来，所有这些依赖关系都由 BeanFactory 来维护。Context 模块：以 Core 和 Beans 为基础，集成 Beans 模块功能并添加资源绑定、数据验证、国际化、JavaEE 支持、容器生命周期、事件传播等； 核心接口是 ApplicationContext。EL 模块：提供强大的表达式语言支持，支持访问和修改属性值， 方法调用，支持访问及修改数组、容器和索引器，命名变量，支持算数和逻辑运算，支持从 Spring 容器获取 Bean，它也支持列表投影、选择和一般的列表聚合等。
* AOP、Aspects 模块：AOP 模块：SpringAOP 模块提供了符合 AOPAlliance 规范的面向方面 的编程（aspect-orientedprogramming）实现，提供比如日志记录、权限控制、性能统计等通用功 能和业务逻辑分离的技术，并且能动态的把这些功能添加到需要的代码中；这样各专其职，降低业务逻辑和通用功能的耦合。Aspects 模块：提供了对 AspectJ 的集成，AspectJ 提供了比SpringASP 更强大的功能。数据访问/集成模块：该模块包括了 JDBC、ORM、OXM、JMS 和事务管理。事 务模块：该模块用于 Spring 管理事务，只要是 Spring 管理对象都能得到 Spring 管理事务的好处， 无需在代码中进行事务控制了，而且支持编程和声明性的事务管理。
* JDBC 模块：提供了一个 JBDC 的样例模板，使用这些模板能消除传统冗长的 JDBC 编码 还有必须的事务控制，而且能享受到 Spring 管理事务的好处。ORM 模块：提供与流行的“对象 -关系”映射框架的无缝集成，包括 Hibernate、JPA、MyBatis 等。而且可以使用 Spring 事务管理， 无需额外控制事务。
* OXM 模块：提供了一个对 Object/XML 映射实现，将 java 对象映射成 XML 数据，或者将 XML 数据映射成 java 对象，Object/XML 映射实现包括 JAXB、Castor、XMLBeans 和 XStream。
* JMS 模块：用于 JMS(JavaMessagingService)，提供一套“消息生产者、消息消费者”模板 用于更加简单的使用 JMS，JMS 用于用于在两个应用程序之间，或分布式系统中发送消息，进 行异步通信。
* Web/Remoting 模块：Web/Remoting 模块包含了 Web、Web-Servlet、Web-Struts、Web-Porlet 模块。
* Web 模块：提供了基础的 web 功能。例如多文件上传、集成 IoC 容器、远程过程访问（RMI、 Hessian、Burlap）以及 WebService 支持，并提供一个 RestTemplate 类来提供方便的 Restfulservices 访问。
* Web-Servlet 模块：提供了一个 SpringMVCWeb 框架实现。SpringMVC 框架提供了基于注 解的请求资源注入、更简单的数据绑定、数据验证等及一套非常易用的 JSP 标签，完全无缝与 Spring 其他技术协作。
* Test 模块：Spring 支持 Junit 和 TestNG 测试框架，而且还额外提供了一些基于 Spring 的测 试功能，比如在测试 Web 框架时，模拟 Http 请求的功能。

##  5. <a name='Spring-1'></a>什么是Spring模块的上下文?

这是基本的 Spring 模块，提供 Spring 框架的基础功能，BeanFactory 是任何以 Spring 为基础 的应⽤的核⼼。Spring 框架建⽴在此模块之上，它使 Spring 成为⼀个容器。

##  6. <a name='ApplicationContext'></a>ApplicationContext通常的实现是什么?

* FileSystemXmlApplicationContext:此容器从⼀个 XML⽂件中加载 beans 的定义 XMLBean 配 置⽂件的全路径名必须提供它的构造函数。
* ClassPathXmlApplicationContext:此容器也从⼀个 XML ⽂件中加载 beans 的定义,这⾥需要正确设置 classpath 因为这个容器将在 classpath⾥找 bean 配置。 WebXmlApplicationContext:此容器加载⼀个 XML⽂件,此⽂件定义了⼀个 WEB 应用的所有 bean。

##  7. <a name='SpringBean'></a>什么是Spring中的Bean?

Springbeans 是那些形成 Spring 应用的主干的 java 对象。它们被 SpringIOC 容器初始化，装 配，和管理。这些 beans 通过容器中配置的元数据创建。比如，以 XML 文件中的形式定 义。Spring 框架定义的 beans 都是单例 beans。在 beantag 中有个属性”singleton”，如果它被赋为 TRUE，bean就是单件，否则就是一个 prototypebean。默认是 TRUE，所以所有在 Spring 框架中的 beans 缺省都是单件。

##  8. <a name='Bean'></a>如何定义Bean的作用域?

当定义一个在 Spring 里，我们还能给这个 bean 声明一个作用域。它可以通过 bean 定义中的 scope 属性来定义。如，当 Spring 要在需要的时候每次生产一个新的 bean 实例，bean 的 scope 属性被指定为 prototype。另一方面，一个 bean 每次使用的时候必须返回同一个实例， 这个 bean 的 scope 属性必须设为 singleton。

##  9. <a name='Bean-1'></a>什么是Bean的自动装配？

无须在 Spring 配置文件中描述 javaBean 之间的依赖关系（ 如配置  ）。IOC 容器会自动建立 javabean 之间的关联关系。

##  10. <a name='Bean-1'></a>解释一下Bean的各个作用域?

Spring 框架支持以下五种 bean 的作用域： 

* singleton: bean 在每个 Springioc 容器中只有一个实例。 

* prototype：一个 bean 的定义可以有多个实例。 

* request ： 每 次 http 请 求 都 会 创 建 一 个 bean ， 该 作 用 域 仅 在基于 web 的 SpringApplicationContext 情形下有效。 

* session：在一个 HTTPSession 中，一个 bean 定义对应一个实例。该作用域仅在基于 web 的 SpringApplicationContext 情形下有效。 

* global-session：在一个全局的 HTTPSession 中，一个 bean 定义对应一个实例。该作用域仅在基于 web 的 SpringApplicationContext 情形下有效。

  **缺省的 Springbean 的作用域是 Singleton.**

##  11. <a name='Spring-1'></a>Spring中的设计模式？

* 单例模式——spring 中两种代理方式，若目标对象实现了若干接口， spring 使用 jdk 的 java.lang.reflect.Proxy-Java 类代理。若目标兑现没有实现任何接口，spring 使用 CGLIB 库生成目 标类的子类。单例模式——在 spring 的配置文件中设置 bean 默认为单例模式。 
* 模板方式模式——用来解决代码重复的问题。比如：RestTemplate、JmsTemplate、JpaTemplate 
* 前端控制器模式——spring 提供了前端控制器 DispatherServlet 来对请求进行分发。 
* 视图帮助（viewhelper）——spring 提供了一系列的 JSP 标签，高效宏来帮助将分散的代码整合在试图中。 
* 依赖注入——贯穿于 BeanFactory/ApplacationContext 接口的核心理念 
* 工厂模式——在工厂模式中，我们在创建对象时不会对客户端暴露创建逻辑，并且是通过使用 同一个接口来指向新创建的对象。Spring 中使用 beanFactory 来创建对象的实例。

##  12. <a name='SpringAOP'></a>简单解释一下Spring中的AOP原理？

AOP （ AspectOrientedProgramming ）即 面 向 切 面 编 程 ， 可 以 说 是 OOP （ObjectOrientedProgramming，面向对象编程）的补充和完善。OOP 引入封装、继承、多态等概 念来建立一种对象层次结构，用于模拟公共行为的一个集合。不过 OOP 允许开发者定义纵向的关 系，但并不适合定义横向的关系，例如日志功能。日志代码往往横向地散布在所有对象层次中， 而与它 对应的对象的核心功能毫无关系对于其他类型的代码，如安全性、异常处理和透明的持续性也都是 如此，这种散布在各处的无关的代码被称为横切（crosscutting），在 OOP 设计中，它导致了大量代码的重复，而不利于各个模块的重用。AOP 技术恰恰相反，它利用一种称为"横切" 的技术， **剖解开封装的对象内部，并将那些影响了多个类的公共行为封装到一个可重用模块** ，并将其命名为 "Aspect"，即切面。所谓"切面"，简单说就是那些与业务无关，却为业务模块所共同调用的逻辑或责任 封装起来，便于减少系统的重复代码，降低模块之间的耦合度，并有利于未来的可操作性和可维护性。 使用"横切"技术，AOP 把软件系统分为两个部分：**核心关注点和横切关注点** 。业务处理的主要流程是核心关注点，与之关系不大的部分是横切关注点。横切关注点的一个特点是，他们经常发生在核心关注点的多处，而各处基本相似，比如权限认证、日志、事物。AOP 的作用在于分离系统中的各种 关注点，将核心关注点和横切关注点分离开来。AOP 核心就是切面，它将多个类的通用行为封 装成可重用的模块，该模块含有一组 API 提供横切功能。比如，一个日志模块可以被称作日志的 AOP 切面。根据需求的不同，一个应用程序可以有若干切面。在 SpringAOP 中，切面通过带有 @Aspect 注解的类实现。

##  13. <a name=''></a>什么是切点?

切入点是一个或一组连接点，通知将在这些位置执行。可以通过表达式或匹配的方式指明切入点。

##  14. <a name='-1'></a>什么是连接点?

被拦截到的点，因为 Spring 只支持方法类型的连接点，所以在 Spring 中连接点指的就是被 拦截到的方法，实际上连接点还可以是字段或者构造器。

##  15. <a name='-1'></a>什么是织入?

织入是将切面和到其他应用类型或对象连接或创建一个被通知对象的过程。织入可以在编译时，加载时，或运行。

