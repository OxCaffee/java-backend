# MyBatis插件Plugin源码解析

## 前言

大多数框架，都支持插件，用户可通过编写插件来自行扩展功能，Mybatis也不例外。MyBatis不像Spring，留了很多接口供开发者使用，只留了一个plugin供开发者拓展。国内著名的插件例如PageHelper都是基于MyBatis的Plugin完成的。

## 插件使用示例

```java
@Intercepts({
    @Signature(type= Executor.class, method = "update", args = {MappedStatement.class,Object.class})
})
public class ExamplePlugin implements Interceptor {

    @Override
    public Object intercept(Invocation invocation) throws Throwable {
        return invocation.proceed();
    }

    @Override
    public Object plugin(Object target) {
        return Plugin.wrap(target, this);
    }

    @Override
    public void setProperties(Properties properties) {
    }
}
```

## 插件原理

MyBatis的插件本质还是一个拦截器Interceptor，但是拦截的内容有所限制，默认情况下，MyBatis允许使用插件来拦截的方法调用来自4个类，分别为`Executor`, `ParameterHandler`, `ResultSetHandler`, `StatementHandler`。**MyBatis采用责任链模式，通过动态代理组织多个拦截器(Plugin or Interceptor)** ，通过拦截器可以修改MyBatis的默认行为，最常见的就是对SQL语句进行重写，PageHelper正是基于这个原理实现的。**只有真正了解了Plugin的原理，才能编写出安全且高效的Plugin**

MyBatis的插件配置在`Configuration`的内部，初始化的时候，就会读取这些配件，保存于`Configuration`内部的`InterceptorChain`中

```java
public class Configuration{
    protected final InterceptorChain interceptorChain = new InterceptorChain();
}
```

`InterceptorChain`保存了一个拦截器的列表，利用责任链的方式，将目标对象通过`pluginAll()`注册到责任链中的所有拦截器中

```java
public class InterceptorChain {

  private final List<Interceptor> interceptors = new ArrayList<Interceptor>();

  public Object pluginAll(Object target) {
    for (Interceptor interceptor : interceptors) {
      target = interceptor.plugin(target);
    }
    return target;
  }

  public void addInterceptor(Interceptor interceptor) {
    interceptors.add(interceptor);
  }
  
  public List<Interceptor> getInterceptors() {
    return Collections.unmodifiableList(interceptors);
  }
}
```

大致理解了上面的内容，就可以开始深入理解插件原理了。

上面的插件使用示例章节展示了如何标准的使用MyBatis的插件功能，其中`@Signature`注解和`Interceptor`接口是**必不可少**的两个组件，**`@Signature`主要告诉MyBatis拦截什么样的方法，而`Interceptor`则定义了拦截器需要完成那些功能**

### 拦截器接口 _Interceptor_

```java
public interface Interceptor {
    //执行拦截的内容，由plugin方法触发
    Object intercept(Invocation invocation) throws Throwable;
    //用当前的这个拦截器生成对目标target的代理，决定是否触发intercept方法
    Object plugin(Object target);
    //给自定义的拦截器床底xml配置文件的属性参数
    void setProperties(Properties properties);
}
```

### 拦截定义注解 _@Signature_

```java
package org.apache.ibatis.plugin;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface Signature {
    //拦截的目标类
    Class<?> type();
    //拦截的目标方法
    String method();
    //拦截方法对应的参数
    Class<?>[] args();
}
```





