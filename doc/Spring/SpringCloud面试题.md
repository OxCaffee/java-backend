# Spring Cloud面试题

<!-- vscode-markdown-toc -->
* 1. [什么是微服务？](#)
* 2. [什么是SpringCloud?](#SpringCloud)
* 3. [SpringCloud的优势?](#SpringCloud-1)
* 4. [SpringCloud如何实现微服务的注册与发现?](#SpringCloud-1)
* 5. [Ribbon和Feign的区别?](#RibbonFeign)
* 6. [什么是负载均衡？](#-1)
* 7. [什么是服务容错保护？](#-1)
* 8. [什么是声明式服务调用？](#-1)
* 9. [什么是API服务网关？](#API)
* 10. [什么是SpringCloud Config?](#SpringCloudConfig)
* 11. [什么是SpringCloud Bus?](#SpringCloudBus)
* 12. [什么是SpringCloud Stream?](#SpringCloudStream)
* 13. [同样作为注册中心,Eureka和ZooKeeper的区别?](#EurekaZooKeeper)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>什么是微服务？

微服务是一种架构风格，一个大型的复杂软件应用由一个或者多个微服务组成。系统中的各个微服务可以被独立部署，各个微服务之间是松耦合的。每个微服务仅关注完成一件任务并很好地完成这个任务。在所有的情况下，每个任务代表一个小的业务能力。

可以在自己的程序中运行，并通过轻量级设备与HTTP型API进行沟通。关键在于服务可以在自己的程序中的运行，通过这一点我们可以将服务公开与微服务架构区分开来。在服务公开中，许多服务都可以被内部独立进程所限制。如果其中任何一个微服务需要增加某种功能，那么就必须缩小进程范围。在微服务架构中，只需要在特定的某种服务中增加所需功能，而不影响整体进程。

##  2. <a name='SpringCloud'></a>什么是SpringCloud?

SpringCloud 是⼀个微服务框架，相⽐ Dubbo 等 RPC 框架，SpringCloud 提供的全套的分布式 系统解决⽅案。SpringCloud 对微服务基础框架 Netflix 的多个开源组件进⾏了封装， 同时⼜实 现了和云端平台以及和 SpringBoot 开发框架的集成。SpringCloud 为微服务架构开 发涉及的配 置管理，服务管理，熔断机制，智能路由，微代理，控制总线，⼀次性 token， 全局⼀致性锁， leader 选举，分布式 session，集群状态管理等操作提供了⼀种简单的开发 式。SpringCloud 为开 发者提供了快速构建分布式系统的⼯具，开发者可以快速的启动服务 器或构建应⽤，同时能够 快速和云平台资源进⾏对接。

##  3. <a name='SpringCloud-1'></a>SpringCloud的优势?

* Spring Cloud 来源于 Spring，质量、稳定性、持续性都可以得到保证。 
* Spirng Cloud 天然支持 Spring Boot，更加便于业务落地。 
* Spring Cloud 发展非常的快，从 2016 年开始接触的时候相关组件版本为 1.x，到现在将要发布 2.x 系列。 
* Spring Cloud 是 Java 领域最适合做微服务的框架。 相比于其它框架，Spring Cloud 对微服务周边环境的支持力度最大。 对于中小企业来讲，使用门槛较低。 Spring Cloud 是微服务架构的最佳落地方案。

##  4. <a name='SpringCloud-1'></a>SpringCloud如何实现微服务的注册与发现?

服务在发布时指定对应的服务名(服务名包括了 IP 地址和端⼝)将服务注册到注册中⼼ (eureka 或者 zookeeper) 。 这 ⼀过 程 是 SpringCloud ⾃动 实 现 只 需 要 在 main ⽅法 添 加 EnableDisscoveryClient 同⼀ 服务修改端⼝就可以启动多个实例。 调⽤⽅法：传递服务名称通过注册中⼼获取所有的可⽤实例，通过负载均衡策略调⽤ (ribbon 和 feign)对应的服务。

##  5. <a name='RibbonFeign'></a>Ribbon和Feign的区别?

* Ribbon 添加 Maven 依赖 spring-starter-ribbon 使⽤`@RibbonClient(value=” 服务名称”)` 使 `RestTemplate` 调⽤远程服务对应的⽅法。 
* Feign 添加 Maven 依赖 spring-starter-feign 服务提供⽅提供对外接⼝，调⽤⽅接⼝在接 ⼝上使⽤ `@FeignClient(“指定服务名”)`。 
* Ribbon 和 Feign 都是⽤于调⽤其他服务的，不过⽅法不同。 
  * 启 动 类 使 ⽤ 的 注 解 不 同 ， Ribbon ⽤ 的 是 @RibbonClient ， Feign ⽤ 的 @EnableFeignClients。 
  * 服务的指定位置不同，Ribbon 是在@RibbonClient 注解上声明，Feign 则是在定义 抽象⽅法 的接⼝中使⽤@FeignClient 声明。 
  * 调⽤⽅法不同，Ribbon 需要⾃⼰构建 HTTP 请求，模拟 HTTP 请求然后使⽤ RestTemplate 发 送给其他服务，步骤相当繁琐。Feign 则是在 Ribbon 的基础上进⾏了⼀次改进，采⽤接⼝的⽅式， 将需要调⽤的其他服务的⽅法定义成抽象⽅法即可。不需要⾃⼰构建 HTTP 请求。不过抽象⽅法 的注解、⽅法签名要和提供服务的⽅法完全⼀致。

##  6. <a name='-1'></a>什么是负载均衡？

负载均衡分为服务端负载均衡和客户端负载均衡。 

* **服务端负载均衡** ：当浏览器向后台发出请求的时候，会首先向反向代理服务器发送请求， 反向代理服务器会根据客户端部署的 ip：port 映射表以及负载均衡策略，来决定向哪台服务器发 送请求，一般会使用到 nginx 反向代理技术。 
* **客户端负载均衡** ：当浏览器向后台发出请求的时候，客户端会向服务注册器(例如：Eureka Server)，拉取注册到服务器的可用服务信息，然后根据负载均衡策略，直接命中哪台服务器发送请求。这整个过程都是在客户端完成的，并不需要反向代理服务器的参与。

服务端负载均衡：分为两种，一种是硬件负载均衡，还有一种是软件负载均衡。 我们主要讲客户端负载均衡，Spring cloud Ribbon 是一个基于 Http 和 TCP 的客服端负载均衡工 具，它是基于 Netflix Ribbon 实现。Ribbon 不需要独立部署，但它几乎存在于每个微服务的基础设施中。Ribbon 可以通过在客户端中配置 ribbonServerList 来设置服务端列表去轮询访问以达到 均衡负载的作用。 当 Ribbon 与 Eureka 联合使用时，ribbonServerList 会被 DiscoveryEnabledNIWSServerList 重写， 扩展成从Eureka 注册中心中获取服务实例列表。同时它也会用 NIWSDiscoveryPing 来取代IPing， 它将职责委托给 Eureka 来确定服务端是否已经启动。Spring Cloud Feign 默认集成了 Ribbon，并 和 Eureka 结合，默认实现了负载均衡的效果,也是客户端使用。

##  7. <a name='-1'></a>什么是服务容错保护？

Spring Cloud Hystrix 是服务容错保护，也是服务熔断器。Hystrix 是 Spring Cloud 提供的一种带有 熔断机制的框架，由于在微服务系统中同一个操作会由多个不同的微服务来共同完成，所以微服务 与微服务之间会由很多相互的调用，由于在分布式环境中经常会出现某个微服务节点故障的情况，所 以会由调用失败发生，而熔断器的作用就是当出现远程调用失败的时候提供一种机制来保证程序的正 常运行而不会卡死在某一次调用，类似 Java 程序中的 try-catch 结构，而只有当异常发生的时候才 会进入 catch 的代码块。

##  8. <a name='-1'></a>什么是声明式服务调用？

Spring Cloud Feign 是声明式服务调用。Feign 是一个声明式的 Web Service 客户端，它的目的就 是让 Web Service 调用更加简单。Feign 提供了 HTTP 请求的模板，通过编写简单的接口和插入注 解，就可以定义好 HTTP 请求的参数、格式、地址等信息。而 Feign 则会完全代理 HTTP 请求， 我 们只需要像调用方法一样调用它就可以完成服务请求及相关处理。

##  9. <a name='API'></a>什么是API服务网关？

API 网关是一个服务器，是系统的唯一入口。从面向对象设计的角度看，它与外观模式类似。 API 网关封装了系统内部架构，为每个客户端提供一个定制的 API。它可能还具有其它职责，如 身份验证、监控、负载均衡、缓存、请求分片与管理、静态响应处理。 **API 网关方式的核心要点是，所有的客户端和消费端都通过统一的网关接入微服务，在网关层处理所有的非业务功能** 。通常，网关也是提供 REST/HTTP 的访问 API。服务端通过 API-GW 注册 和管理服务。Spring Cloud Zuul 是 API 网关，Zuul 是 Netflix 开源的微服务网关，他可以和 Eureka,Ribbon,Hystrix 等组件配合使用，Filter 是 Zuul 的核心，用来实现对外服务的控制。Filter 的生命周期有 4 个，分是“PRE”、“ROUTING”、“POST”、“ERROR”。

##  10. <a name='SpringCloudConfig'></a>什么是SpringCloud Config?

配置管理工具包，让你可以把配置放到远程服务器，集中化管理集群配置，目前支持本地存储、 Git 以及 svn。 如果微服务架构中没有使用统一配置中心时，所存在的问题： 配置文件分散在各个项目里，不方便维护 配置内容安全与权限，实际开发中，开发人员是不知道线上环境的配置的更新配置后，项目需要重启 在分布式系统中，由于服务数量巨多，为了方便服务配置文件统一管理，实时更新，所以需要分布 式配置中心组件。市面上开源的配置中心有很多，BAT 每家都出过，360 的 QConf、淘宝的 diamond、百度的 disconf 都是解决这类问题。国外也有很多开源的配置中心 Apache 的 Apache Commons Configuration、owner、cfg4j 等等。在 Spring Cloud 中，有分布式配置中心组件spring cloud config ，它支持配置服务放在配置服务的内存中（即本地），也支持放在远程 Git 仓库中。在 spring cloud config 组件中，分两个角色，一是 config server，二是 config client。一个配置中心提供的 核心功能: 

* 提供服务端和客户端支持 
* 集中管理各环境的配置文件
*  配置文件修改之后，可以快速的生效 
* 可以进行版本管理  支持大的并发查询 
* 支持各种语言

##  11. <a name='SpringCloudBus'></a>什么是SpringCloud Bus?

在微服务架构的系统中，我们通常会使用轻量级的消息代理来构建一个共用的消息主题让系统中所有微服务实例都连接上来，由于该主题中产生的消息会被所有实例监听和消费，所以我们称它为消息总线。Spring Cloud Bus 就像一个分布式执行器，用于扩展的 Spring Boot 应用程序， 但 也可以用作应用程序之间的通信通道。Spring Cloud Bus 支持 RabbitMQ 和 Kafka。

##  12. <a name='SpringCloudStream'></a>什么是SpringCloud Stream?

Spring Cloud Stream 是构建消息驱动的微服务应用程序的框架。Spring Cloud Stream 基于 Spring Boot 建立独立的生产级 Spring 应用程序，并使用 Spring Integration 提供与消息代理的连接。它 提供了来自几家供应商的中间件的意见配置，介绍了持久发布订阅语义，消费者组和分区的概念。

##  13. <a name='EurekaZooKeeper'></a>同样作为注册中心,Eureka和ZooKeeper的区别?

* ZooKeeper 保证的是 CP,Eureka 保证的是 AP ZooKeeper 在选举期间注册服务瘫痪,虽然服务最终会恢复,但是选举期间不可用的 Eureka 各个节点是平等关系,只要有一台 Eureka 就可以保证服务可用,而查询到的数据并不是最新的
  * **自我保护机制会导致Eureka不再从注册列表移除因长时间没收到心跳而应该过期的服务 Eureka 仍然能够接受新服务的注册和查询请求,但是不会被同步到其他节点(高可用) 当网络稳定时,当前实例新的注册信息会被同步到其他节点中(最终一致性) Eureka 可以很好的应对因网络故障导致部分节点失去联系的情况,而不会像 ZooKeeper 一样使得 整个注册系统瘫痪**
* ZooKeeper 有 Leader 和 Follower 角色,Eureka 各个节点平等
* ZooKeeper 采用过半数存活原则,Eureka 采用自我保护机制解决分区问题
* Eureka 本质上是一个工程,而 ZooKeeper 只是一个进程