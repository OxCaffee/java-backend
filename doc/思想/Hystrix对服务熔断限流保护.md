# Hystrix对服务熔断限流保护

## 1. 服务雪崩、常见场景

分布式系统环境下，服务间依赖非常常见，一个业务调用通常依赖多个基础服务。对于同步调用，当某服务不可用时，服务请求线程被阻塞，当有大批量请求调用该不可用服务时，最终可能导致整个系统服务资源耗尽，无法继续对外提供服务。并且这种不可用会沿请求调用链向上传递，这种现象被称为雪崩效应。**产生服务雪崩一般是服务器的线程被不可用服务耗尽**

**产生服务雪崩的场景有:**

* **硬件故障**：如服务器宕机，机房断电，光纤被挖断等。
* **流量激增**：如异常流量，重试加大流量等。
* **缓存穿透**：一般发生在应用重启，所有缓存失效时，以及短时间内大量缓存失效时。大量的缓存不命中，使请求直击后端服务，造
* **服务提供者超负荷运行，引起服务不可用**。
* **程序BUG**：如程序逻辑导致内存泄漏，JVM长时间FullGC等。
* **同步等待**：服务间采用同步调用模式，同步等待造成的资源耗尽。

## 2. 实际业务场景中的限流措施

* **Nginx前端限流** :按照一定的规则如帐号、IP、系统调用逻辑等在Nginx层面做限流
* **业务层限流** :客户端、服务器做限流措施
* **数据库限流** :设置访问警告线，以最大努力保住数据库

## 3. Hystrix的作用、功能及设计原则

**Hystrix可以做到以下事情：**

* 通过控制延迟和故障来保障第三方服务调用的可靠性
* 在复杂的分布式系统中防止级联故障，防止雪崩
* 快速失败、快速恢复
* 回退并优雅降级
* 提供近实时监控、报警和操作控制

**Hystrix遵循的设计原则：**

* 防止任何单独的依赖耗尽资源（线程）
* 过载立即切断并快速失败，防止排队
* 尽可能提供回退以保护用户免受故障
* 使用隔离技术（例如隔板，泳道和断路器模式）来限制任何一个依赖的影响
* 通过近实时的指标，监控和告警，确保故障被及时发现
* 通过动态修改配置属性，确保故障及时恢复
* 防止整个依赖客户端执行失败，而不仅仅是网络通信

## 4. Hystrix保护系统的措施

* 使用命令模式将所有对外部服务（或依赖关系）的调用包装在HystrixCommand或HystrixObservableCommand对象中，并将该对象放在单独的线程中执行；
* 每个依赖都维护着一个线程池（或信号量），线程池被耗尽则拒绝请求（而不是让请求排队）。
* 记录请求成功，失败，超时和线程拒绝。
* 服务错误百分比超过了阈值，熔断器开关自动打开，一段时间内停止对该服务的所有请求。
* 请求失败，被拒绝，超时或熔断时执行降级逻辑。
* 近实时地监控指标和配置的修改。

## 5. Hystrix几个关键的类

* **HystrixThreadPoolProperties**

```java
public abstract class HystrixThreadPoolProperties {
	//核心线程池大小，默认10
    static int default_coreSize = 10;         
    //线程池最大容量，默认10
    static int default_maximumSize = 10;         
    //线程存活时间
    static int default_keepAliveTimeMinutes = 1; 
    //等待队列大小，由于这个不能动态设置，因此用queueSizeRejectionThreshold来认为限制
    //-1代表同步队列，请求的线程数超过maximumSize，请求会被拒绝
    //>0的时候，使用LinkedBlockingQueue，超过的线程会被阻塞等待
    static int default_maxQueueSize = -1;  
    //是否允许最大容量在配置线程池的时候被读写
    static boolean default_allow_maximum_size_to_diverge_from_core_size = false; 
	//被拒绝的阈值
    static int default_queueSizeRejectionThreshold = 5; 
    static int default_threadPoolRollingNumberStatisticalWindow = 10000; 
    static int default_threadPoolRollingNumberStatisticalWindowBuckets = 10;

    ///线程池 corePoolSize
    private final HystrixProperty<Integer> corePoolSize;
    //线程池 maximumPoolSize
    private final HystrixProperty<Integer> maximumPoolSize;
    //线程池 线程空闲存储时长
    private final HystrixProperty<Integer> keepAliveTime;
    //线程池 队列大小
    private final HystrixProperty<Integer> maxQueueSize;
    //队列元素数量超过多少后，拒绝任务
    private final HystrixProperty<Integer> queueSizeRejectionThreshold;
    //是否允许 maximumPoolSize 不等于( 偏离 ) corePoolSize
    private final HystrixProperty<Boolean> allowMaximumSizeToDivergeFromCoreSize;
    private final HystrixProperty<Integer> threadPoolRollingNumberStatisticalWindowInMilliseconds;
    private final HystrixProperty<Integer> threadPoolRollingNumberStatisticalWindowBuckets;
```

* **HystrixThreadPoolKey**

通过相同的 `name` ( 标识 ) 获得同 HystrixThreadPoolKey 对象。通过在内部维持一个 `name` 与 HystrixThreadPoolKey 对象的映射，以达到**枚举**的效果。内置一个InternMap来保存key和value的组合

* **HystrixCommandProperties** :构造Command的时候通过Setter注入

```java
//使用命令调用隔离方式,默认:采用线程隔离,ExecutionIsolationStrategy.THREAD  
private final HystrixProperty<ExecutionIsolationStrategy> executionIsolationStrategy;   
//使用线程隔离时，调用超时时间，默认:1秒  
private final HystrixProperty<Integer> executionIsolationThreadTimeoutInMilliseconds;   
//线程池的key,用于决定命令在哪个线程池执行  
private final HystrixProperty<String> executionIsolationThreadPoolKeyOverride;   
//使用信号量隔离时，命令调用最大的并发数,默认:10  
private final HystrixProperty<Integer> executionIsolationSemaphoreMaxConcurrentRequests;  
//使用信号量隔离时，命令fallback(降级)调用最大的并发数,默认:10  
private final HystrixProperty<Integer> fallbackIsolationSemaphoreMaxConcurrentRequests;   
//是否开启fallback降级策略 默认:true   
private final HystrixProperty<Boolean> fallbackEnabled;   
// 使用线程隔离时，是否对命令执行超时的线程调用中断（Thread.interrupt()）操作.默认:true  
private final HystrixProperty<Boolean> executionIsolationThreadInterruptOnTimeout;   
// 统计滚动的时间窗口,默认:5000毫秒circuitBreakerSleepWindowInMilliseconds  
private final HystrixProperty<Integer> metricsRollingStatisticalWindowInMilliseconds;  
// 统计窗口的Buckets的数量,默认:10个,每秒一个Buckets统计  
private final HystrixProperty<Integer> metricsRollingStatisticalWindowBuckets; // number of buckets in the statisticalWindow  
//是否开启监控统计功能,默认:true  
private final HystrixProperty<Boolean> metricsRollingPercentileEnabled;   
// 是否开启请求日志,默认:true  
private final HystrixProperty<Boolean> requestLogEnabled;   
//是否开启请求缓存,默认:true  
private final HystrixProperty<Boolean> requestCacheEnabled; // Whether request caching is enabled.

// 熔断器在整个统计时间内是否开启的阀值，默认20秒。也就是10秒钟内至少请求20次，熔断器才发挥起作用  
private final HystrixProperty<Integer> circuitBreakerRequestVolumeThreshold;   
//熔断器默认工作时间,默认:5秒.熔断器中断请求5秒后会进入半打开状态,放部分流量过去重试  
private final HystrixProperty<Integer> circuitBreakerSleepWindowInMilliseconds;   
//是否启用熔断器,默认true. 启动  
private final HystrixProperty<Boolean> circuitBreakerEnabled;   
//默认:50%。当出错率超过50%后熔断器启动.  
private final HystrixProperty<Integer> circuitBreakerErrorThresholdPercentage;  
//是否强制开启熔断器阻断所有请求,默认:false,不开启  
private final HystrixProperty<Boolean> circuitBreakerForceOpen;   
//是否允许熔断器忽略错误,默认false, 不开启  
private final HystrixProperty<Boolean> circuitBreakerForceClosed;
```

* **HystrixCollapserProperties** :构造Collapser的时候用Setter注入

```java
//请求合并是允许的最大请求数,默认: Integer.MAX_VALUE  
private final HystrixProperty<Integer> maxRequestsInBatch;  
//批处理过程中每个命令延迟的时间,默认:10毫秒  
private final HystrixProperty<Integer> timerDelayInMilliseconds;  
//批处理过程中是否开启请求缓存,默认:开启  
private final HystrixProperty<Boolean> requestCacheEnabled;
```

## 6. Hystrix服务保护案例

### 6.1 降级案例

本例展示了利用HystrixCommand实现超时自动降级

```java
//降级代码
public class SoleServiceCommand extends HystrixCommand<String>{
    
    @Autowired
    private SoleService soleService;
    
    //Setter注入配置
    public static Setter setter(){
        //服务分组
        HystrixCommandGroupKey groupKey = HystrixCommandGroupKey.Factory.asKey("sole");
        //命令配置
        HystrixCommandProperties.Setter p = HystrixCommandProperties.Setter()
            .withIsolationStrategy(THREAD)
            .withFallbackEnabled(true)
            .withFallbackIsolationSemaphoreMaxConcurrentRequests(100)
            ...
            .withExecutionTimeoutEnabled(true)
            .withExecutionTimeoutInMilliseconds(1000);
       	return HystrixCommand.Setter
            .withGroupKey(groupKey)
            .withCommandPropertiesDefaults(p);
    }
    
    @Override
    public void run(){		//command正常执行，出现异常或者超时转为执行getFallback
        return soleService.sole();
    }
    
    @Override
    public String getFallback(){
        return "请稍后访问";	//降级措施	
    }
}
```

### 6.2 熔断案例

同样使用HystrixCommand，只需要在HystrixCommandProperties中设置熔断的参数即可

```java
HystrixCommandProperties.Setter p = HystrixCommandPropperties.Setter()
    ...
    .withCircuitBreakerEnabled(true)
    .withCircuitBreakerForceClosed(false)
    .withCircuitBreakerForceOpen(false)
    .withCircuitBreakerErrorThresholdPercentage(50)	//失败50%开启熔断
    .withCircuitBreakerRequestVolumeThreshold(20)	//必须达到的采样的个数
    .withCircuitBreakerSleepWindowInMilliseconds(5000);//熔断之后的重试时间
```

当然上述的操作在Spring中都可以通过注解的方式实现，在这里主要体现思想，就不多做赘述