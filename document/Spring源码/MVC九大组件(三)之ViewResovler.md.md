# Spring MVC源码解析——MVC九大组件(三)之ViewResolver

## 前言

ViewResolver为实体解析器，根据视图名viewName和国际化locale，获得最终的视图View对象

```java
//ViewResolver.java
public interface ViewResolver{
    //根据视图名和国际化，获得最终的View对象
    @Nullable
    View resolveViewName(String viewName, Locale locale) throws Exception;
}
```

## _ViewResolver_ 族谱

<div align=center><img src="/asset/vr1.png"></div>

虽然实现类比较多，ViewResolver基本可以分为五个实现类，就是ViewResolver的五个直接实现类

## _ViewResolver_ 初始化

我们以**默认配置的 Spring Boot 场景**下为例，来一起看看 DispatcherServlet 的 `#initViewResolvers(ApplicationContext context)` 方法，初始化 `viewResolvers` 变量。代码如下：

```java
// DispatcherServlet.java

/** List of ViewResolvers used by this servlet. */
@Nullable
private List<ViewResolver> viewResolvers;

/** Detect all ViewResolvers or just expect "viewResolver" bean?. */
private boolean detectAllViewResolvers = true;

private void initViewResolvers(ApplicationContext context) {
    // 置空 viewResolvers 处理
    this.viewResolvers = null;

    // 情况一，自动扫描 ViewResolver 类型的 Bean 们
    if (this.detectAllViewResolvers) {
        // Find all ViewResolvers in the ApplicationContext, including ancestor contexts.
        Map<String, ViewResolver> matchingBeans =
                BeanFactoryUtils.beansOfTypeIncludingAncestors(context, ViewResolver.class, true, false);
        if (!matchingBeans.isEmpty()) {
            this.viewResolvers = new ArrayList<>(matchingBeans.values());
            // We keep ViewResolvers in sorted order.
            AnnotationAwareOrderComparator.sort(this.viewResolvers);
        }
    // 情况二，获得名字为 VIEW_RESOLVER_BEAN_NAME 的 Bean 们
    } else {
        try {
            ViewResolver vr = context.getBean(VIEW_RESOLVER_BEAN_NAME, ViewResolver.class);
            this.viewResolvers = Collections.singletonList(vr);
        } catch (NoSuchBeanDefinitionException ex) {
            // Ignore, we'll add a default ViewResolver later.
        }
    }

    // Ensure we have at least one ViewResolver, by registering
    // a default ViewResolver if no other resolvers are found.
    // 情况三，如果未获得到，则获得默认配置的 ViewResolver 类
    if (this.viewResolvers == null) {
        this.viewResolvers = getDefaultStrategies(context, ViewResolver.class);
        if (logger.isTraceEnabled()) {
            logger.trace("No ViewResolvers declared for servlet '" + getServletName() +
                    "': using default strategies from DispatcherServlet.properties");
        }
    }
}
```

- 一共有三种情况，初始化 `viewResolvers` 属性。
- 默认情况下，detectAllViewResolvers为true，所以走情况一的逻辑，自动扫描 ViewResolver 类型的 Bean 们。在默认配置的 Spring Boot 场景下，viewResolvers的结果是：
  - ContentNegotiatingViewResolver
  - BeanNameViewResolver
  - ThymeleafViewResolver
  - ViewResolverComposite
  - InternalResourceViewResolver

从实现上来说，ContentNegotiatingViewResolver 是最最最重要的 ViewResolver 实现类，所以我们先开始瞅瞅它。

## _ContentNegotiatingViewResolver_

`org.springframework.web.servlet.view.ContentNegotiatingViewResolver` ，实现 ViewResolver、Ordered、InitializingBean 接口，继承 WebApplicationObjectSupport 抽象类，基于**内容类型**来获取对应 View 的 ViewResolver 实现类。

其中，**内容类型**指的是 `"Content-Type"` 和拓展后缀。

