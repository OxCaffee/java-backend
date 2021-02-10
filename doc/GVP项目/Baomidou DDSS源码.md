# Baomidou 动态数据源启动器源码解析

<!-- vscode-markdown-toc -->
* 1. [工程结构](#)
* 2. [注解设计](#-1)
* 3. [配置文件](#-1)
	* 3.1. [外层配置 Model——_DynamicDataSourceProperties_](#Model_DynamicDataSourceProperties_)
	* 3.2. [内层配置 Model——_DataSourceProperty_](#Model_DataSourceProperty_)
* 4. [数据源创建器](#-1)
	* 4.1. [顶级接口 _DataSourceCreator_](#_DataSourceCreator_)
	* 4.2. [默认数据源创建器 _DefaultDataSourceCreator_](#_DefaultDataSourceCreator_)
	* 4.3. [JNDI 数据源创建器 _JndiDataSourceCreator_](#JNDI_JndiDataSourceCreator_)
	* 4.4. [Hikari 数据源创建器 _HikariDataSourceCreator_](#Hikari_HikariDataSourceCreator_)
	* 4.5. [Druid 数据源创建器 _DruidDataSourceCreator_](#Druid_DruidDataSourceCreator_)
	* 4.6. [基础数据源创建器 _BasicDataSourceCreator_](#_BasicDataSourceCreator_)
* 5. [数据源加载器](#-1)
	* 5.1. [顶级接口 _DynamicDataSourceProvider_](#_DynamicDataSourceProvider_)
	* 5.2. [抽象顶级加载器 _AbstractDataSourceProvider_](#_AbstractDataSourceProvider_)
	* 5.3. [JDBC 数据源加载器 _AbstractJdbcDataSourceProvider_](#JDBC_AbstractJdbcDataSourceProvider_)
	* 5.4. [YML 数据源加载器 _YmlDynamicDataSourceProvider_](#YML_YmlDynamicDataSourceProvider_)
* 6. [数据源处理器](#-1)
	* 6.1. [顶级父类处理器 _DsProcessor_](#_DsProcessor_)
	* 6.2. [_DsHeaderProcessor_](#DsHeaderProcessor_)
	* 6.3. [_DsSessionProcessor_](#DsSessionProcessor_)
	* 6.4. [_DsSpelExpressionProcessor_](#DsSpelExpressionProcessor_)
* 7. [辅助工具](#-1)
	
	* 7.1. [数据源解析器 _DataSourceClassResolver_](#_DataSourceClassResolver_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>工程结构

##  2. <a name='-1'></a>注解设计

dynamic-datasource-springboot-starter 基于注解的方式，能够实现数据源的动态配置，提供的注解主要有 4 类:

1. **`@DS`** :数据源注解，该注解可以作用于类和方法上，该注解的`value`字段将指明数据源的名称
2. **`@DSTransactional`** : 多数据源事务管理，可作用于类和方法上，含有该注解的方法将会开启事务管理
3. **`@Master`** :主数据源注解，可作于于类和方法上，添加了该注解之后，将会默认使用名为"master"的数据源
4. **`@Slave`** :从数据源注解，可作用于类和方法上，添加了该注解之后，将会默认使用"salve"的数据源

##  3. <a name='-1'></a>配置文件

dynamic-datasource-springboot-starter 的配置文件大致如下:

```yml
spring:
  datasource:
    dynamic:
      primary: master #设置默认的数据源或者数据源组,默认值即为master
      strict: false #设置严格模式,默认false不启动. 启动后在未匹配到指定数据源时候会抛出异常,不启动则使用默认数据源.
      datasource:
        master:
          url: jdbc:mysql://xx.xx.xx.xx:3306/dynamic
          username: root
          password: 123456
          driver-class-name: com.mysql.jdbc.Driver # 3.2.0开始支持SPI可省略此配置
        slave_1:
          url: jdbc:mysql://xx.xx.xx.xx:3307/dynamic
          username: root
          password: 123456
          driver-class-name: com.mysql.jdbc.Driver
        slave_2:
          url: ENC(xxxxx) # 内置加密,使用请查看详细文档
          username: ENC(xxxxx)
          password: ENC(xxxxx)
          driver-class-name: com.mysql.jdbc.Driver
          schema: db/schema.sql # 配置则生效,自动初始化表结构
          data: db/data.sql # 配置则生效,自动初始化数据
          continue-on-error: true # 默认true,初始化失败是否继续
          separator: ";" # sql默认分号分隔符
```

###  3.1. <a name='Model_DynamicDataSourceProperties_'></a>外层配置 Model——_DynamicDataSourceProperties_

`DynamicDataSourceProperties`对应配置文件中的外层配置，即 prefix 为:**`spring.datasource.dynamic`** ，配置参数对应的实体类定义如下:

```java
@Slf4j
@Getter
@Setter
@ConfigurationProperties(prefix = DynamicDataSourceProperties.PREFIX)
public class DynamicDataSourceProperties {

    public static final String PREFIX = "spring.datasource.dynamic";
    public static final String HEALTH = PREFIX + ".health";
    public static final String DEFAULT_VALID_QUERY = "SELECT 1";

    //必须设置默认的库,默认master
    private String primary = "master";
    //是否启用严格模式,默认不启动. 严格模式下未匹配到数据源直接报错, 非严格模式下则使用默认数据源primary所设置的数据源
    private Boolean strict = false;
    //是否使用p6spy输出，默认不输出
    private Boolean p6spy = false;
    //是否使用开启seata，默认不开启
    private Boolean seata = false;
    //seata使用模式，默认AT
    private SeataMode seataMode = SeataMode.AT;
    //是否使用 spring actuator 监控检查，默认不检查
    private boolean health = false;
    //监控检查SQL
    private String healthValidQuery = DEFAULT_VALID_QUERY;
    //每一个数据源
    private Map<String, DataSourceProperty> datasource = new LinkedHashMap<>();
    //多数据源选择算法clazz，默认负载均衡算法
    private Class<? extends DynamicDataSourceStrategy> strategy = LoadBalanceDynamicDataSourceStrategy.class;
    //aop切面顺序，默认优先级最高
    private Integer order = Ordered.HIGHEST_PRECEDENCE;
    //Druid全局参数配置
    @NestedConfigurationProperty
    private DruidConfig druid = new DruidConfig();
    // HikariCp全局参数配置
    @NestedConfigurationProperty
    private HikariCpConfig hikari = new HikariCpConfig();
    //全局默认publicKey
    private String publicKey = CryptoUtils.DEFAULT_PUBLIC_KEY_STRING;
    //aop 切面是否只允许切 public 方法
    private boolean allowedPublicOnly = true;
}
```

###  3.2. <a name='Model_DataSourceProperty_'></a>内层配置 Model——_DataSourceProperty_

`DataSourceProperty`对应的是配置文件中的内层参数，即分别对应的是每一个数据源的配置参数，例如`master`数据源,`slave1`数据源，`slave2`数据源，定义内容如下:

```java
@Slf4j
@Data
@Accessors(chain = true)
public class DataSourceProperty {
    //加密正则
    private static final Pattern ENC_PATTERN = Pattern.compile("^ENC\\((.*)\\)$");
    //连接池名称(只是一个名称标识)</br> 默认是配置文件上的名称
    private String poolName;
    //连接池类型，如果不设置自动查找 Druid > HikariCp
    private Class<? extends DataSource> type;
    //JDBC driver
    private String driverClassName;
    //JDBC url 地址
    private String url;
    //JDBC 用户名
    private String username;
    //JDBC 密码
    private String password;
    //jndi数据源名称(设置即表示启用)
    private String jndiName;
    //自动运行的建表脚本
    private String schema;
    //自动运行的数据脚本
    private String data;
    //启用seata
    private Boolean seata = true;
    //启用p6spy
    private Boolean p6spy = true;
    //错误是否继续 默认 true
    private boolean continueOnError = true;
    //分隔符 默认 ;
    private String separator = ";";
    //Druid参数配置
    @NestedConfigurationProperty
    private DruidConfig druid = new DruidConfig();
    //HikariCp参数配置
    @NestedConfigurationProperty
    private HikariCpConfig hikari = new HikariCpConfig();
}
```

##  4. <a name='-1'></a>数据源创建器

数据源创建模块位于`com.baomidou.dynamic.datasource.creator`包下。数据源创建器会根据配置参数(来自配置文件或者默认参数)创建不同类型的数据源。

**在创建数据源的时候，需要判别当前配置参数是否支持(support)相应的数据源的创建(后续代码会讲解)**

本项目一共实现了 5 种数据源创建器，分别为:

1. **`DefaultDataSourceCreator`**
2. **`JndiDataSourceCreator`**
3. **`HikariDataSourceCreator`**
4. **`DruidDatSourceCreator`**
5. **`BasicDataSourceCreator`**

5 类数据源创建器的继承关系如下:

<div align=center><img src="/asset/dsc.jpg"></div>

###  4.1. <a name='_DataSourceCreator_'></a>顶级接口 _DataSourceCreator_

`DataSourceCreator`为数据源创建器的顶层接口，定义了**如何创建数据源**，源码如下:

```java
public interface DataSourceCreator {

    /**
     * 通过属性创建数据源
     *
     * @param dataSourceProperty 数据源属性
     * @return 被创建的数据源
     */
    DataSource createDataSource(DataSourceProperty dataSourceProperty);

    /**
     * 通过属性创建数据源
     *
     * @param dataSourceProperty 数据源属性
     * @param publicKey          解密公钥
     * @return 被创建的数据源
     */
    DataSource createDataSource(DataSourceProperty dataSourceProperty, String publicKey);

    /**
     * 当前创建器是否支持根据此属性创建
     *
     * @param dataSourceProperty 数据源属性
     * @return 是否支持创建该数据源
     */
    boolean support(DataSourceProperty dataSourceProperty);
}
```

###  4.2. <a name='_DefaultDataSourceCreator_'></a>默认数据源创建器 _DefaultDataSourceCreator_

`DefaultDataSourceCreator`类成员有两个，分别为`DynamicDataSourceProperties`和`creators`，前者为配置的一些参数封装的 Model，后者为创建器的列表，**在创建数据源之前，需要调用`support`方法判断是否支持创建此数据源**

```java
@Slf4j
@Setter
public class DefaultDataSourceCreator implements DataSourceCreator {

    private DynamicDataSourceProperties properties;
    private List<DataSourceCreator> creators;
```

数据源的创建是根据`DataSourceProperty`，即内置的单个数据源配置参数(配置文件配置或者默认值)，以及解密用的公钥`publicKey`，因为密码等隐私信息不能在配置文件中明文显示出来，需要进行加密解密操作保证数据安全。创建行为发生在`createDataSource`方法上，源码如下:

```java
public DataSource createDataSource(DataSourceProperty dataSourceProperty, String publicKey) {
    DataSourceCreator dataSourceCreator = null;
    //遍历数据源创建器列表，对于每一种创建器，尝试是否能够创建
    for (DataSourceCreator creator : this.creators) {
        if (creator.support(dataSourceProperty)) {
            dataSourceCreator = creator;
            break;
        }
    }
    //创建器获取失败，返回异常
    if (dataSourceCreator == null) {
        throw new IllegalStateException("creator must not be null,please check the DataSourceCreator");
    }
    //开始创建
    DataSource dataSource = dataSourceCreator.createDataSource(dataSourceProperty, publicKey);
    //如果配置有SQL脚本，执行脚本，初始化数据库
    this.runScrip(dataSource, dataSourceProperty);
    //进一步封装数据源，以便支持事务
    return wrapDataSource(dataSource, dataSourceProperty);
}
```

Java 原生支持的`DataSource`需要进一步封装以支持一些高级特性，比如 Seata 事务和 P6spy 等，封装动作由`wrapDataSource`完成，最后返回封装好的`com.baomidou.dynamic.datasource.ds.ItemDataSource`

```java
private DataSource wrapDataSource(DataSource dataSource, DataSourceProperty dataSourceProperty) {
    String name = dataSourceProperty.getPoolName();
    DataSource targetDataSource = dataSource;

    Boolean enabledP6spy = properties.getP6spy() && dataSourceProperty.getP6spy();
    if (enabledP6spy) {//支持p6spy
        targetDataSource = new P6DataSource(dataSource);
        log.debug("dynamic-datasource [{}] wrap p6spy plugin", name);
    }

    Boolean enabledSeata = properties.getSeata() && dataSourceProperty.getSeata();
    SeataMode seataMode = properties.getSeataMode();
    if (enabledSeata) {//支持seata
        if (SeataMode.XA == seataMode) { //seata模式为XA模式
            targetDataSource = new DataSourceProxyXA(dataSource);
        } else { //seata模式为其他模式
            targetDataSource = new DataSourceProxy(dataSource);
        }
        log.debug("dynamic-datasource [{}] wrap seata plugin transaction mode [{}]", name, seataMode);
    }
    return new ItemDataSource(name, dataSource, targetDataSource, enabledP6spy, enabledSeata, seataMode);
}
```

###  4.3. <a name='JNDI_JndiDataSourceCreator_'></a>JNDI 数据源创建器 _JndiDataSourceCreator_

JDNI 数据源由`JndiDataSourceLookup`创建，配置时仅需要`jndi-name`即可

```java
@Override
public DataSource createDataSource(DataSourceProperty dataSourceProperty, String publicKey) {
    return LOOKUP.getDataSource(dataSourceProperty.getJndiName());
}
```

###  4.4. <a name='Hikari_HikariDataSourceCreator_'></a>Hikari 数据源创建器 _HikariDataSourceCreator_

Hikari 数据源在创建之前会首先尝试加载 hikari 驱动，如果加载驱动成功，那么就把`hikariExists`设置为 true

```java
private static Boolean hikariExists = false;

static {
    try {
        Class.forName(HIKARI_DATASOURCE); //尝试加载驱动
        hikariExists = true;
    } catch (ClassNotFoundException ignored) {
        //加载失败，什么也不做，也不会创建
    }
}
```

加载时依旧从配置文件中读取参数，配置`HikariDataSource`

```java
@Override
public DataSource createDataSource(DataSourceProperty dataSourceProperty, String publicKey) {
    //配置文件是否配置了公钥，如果没有，使用默认公钥
    if (StringUtils.isEmpty(dataSourceProperty.getPublicKey())) {
        dataSourceProperty.setPublicKey(publicKey);
    }
    //Hikari配置参数Model
    HikariConfig config = dataSourceProperty.getHikari().toHikariConfig(hikariCpConfig);
    //参数设置
    config.setUsername(dataSourceProperty.getUsername());
    config.setPassword(dataSourceProperty.getPassword());
    config.setJdbcUrl(dataSourceProperty.getUrl());
    config.setPoolName(dataSourceProperty.getPoolName());
    String driverClassName = dataSourceProperty.getDriverClassName();
    if (!StringUtils.isEmpty(driverClassName)) {
        config.setDriverClassName(driverClassName);
    }
    //封装好的数据源
    return new HikariDataSource(config);
}
```

###  4.5. <a name='Druid_DruidDataSourceCreator_'></a>Druid 数据源创建器 _DruidDataSourceCreator_

Druid 数据源的创建过程和 Hikari 基本无区别，但是 Druid 的配置参数相较之下更复杂一些，多了一些过滤器的设置和高级特性的设置，例如:

```java
//stat filter是检查数据库健康状态的过滤器
if (!StringUtils.isEmpty(filters) && filters.contains("stat")) {
    StatFilter statFilter = new StatFilter();
    statFilter.configFromProperties(properties);
    proxyFilters.add(statFilter);
}
```

```java
//wall filter是保护数据库安全的防火墙拦截器
if (!StringUtils.isEmpty(filters) && filters.contains("wall")) {
    WallConfig wallConfig = DruidWallConfigUtil.toWallConfig(dataSourceProperty.getDruid().getWall(), gConfig.getWall());
    WallFilter wallFilter = new WallFilter();
    wallFilter.setConfig(wallConfig);
    proxyFilters.add(wallFilter);
}
```

```java
//日志拦截器
if (!StringUtils.isEmpty(filters) && filters.contains("slf4j")) {
    Slf4jLogFilter slf4jLogFilter = new Slf4jLogFilter();
    // 由于properties上面被用了，LogFilter不能使用configFromProperties方法，这里只能一个个set了。
    DruidSlf4jConfig slf4jConfig = gConfig.getSlf4j();
    slf4jLogFilter.setStatementLogEnabled(slf4jConfig.getEnable());
    slf4jLogFilter.setStatementExecutableSqlLogEnable(slf4jConfig.getStatementExecutableSqlLogEnable());
    proxyFilters.add(slf4jLogFilter);
}
```

###  4.6. <a name='_BasicDataSourceCreator_'></a>基础数据源创建器 _BasicDataSourceCreator_

`BasicDataSourceCreator`是为了适配 spring1.5 和 2.x 创建的。创建数据源的行为基于反射，源码如下:

```java
@Data
@Slf4j
public class BasicDataSourceCreator extends AbstractDataSourceCreator implements DataSourceCreator {

    private static Method createMethod;
    private static Method typeMethod;
    private static Method urlMethod;
    private static Method usernameMethod;
    private static Method passwordMethod;
    private static Method driverClassNameMethod;
    private static Method buildMethod;

    static {
        //to support springboot 1.5 and 2.x
        Class<?> builderClass = null;
        try {
            builderClass = Class.forName("org.springframework.boot.jdbc.DataSourceBuilder");
        } catch (Exception ignored) {
        }
        if (builderClass == null) {
            try {
                builderClass = Class.forName("org.springframework.boot.autoconfigure.jdbc.DataSourceBuilder");
            } catch (Exception e) {
                log.warn("not in springBoot ENV,could not create BasicDataSourceCreator");
            }
        }
        if (builderClass != null) {
            try {
                createMethod = builderClass.getDeclaredMethod("create");
                typeMethod = builderClass.getDeclaredMethod("type", Class.class);
                urlMethod = builderClass.getDeclaredMethod("url", String.class);
                usernameMethod = builderClass.getDeclaredMethod("username", String.class);
                passwordMethod = builderClass.getDeclaredMethod("password", String.class);
                driverClassNameMethod = builderClass.getDeclaredMethod("driverClassName", String.class);
                buildMethod = builderClass.getDeclaredMethod("build");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
```

##  5. <a name='-1'></a>数据源加载器

数据源加载器的作用，是将创建好的`DataSource`加载到`Map`，方便存取和调用，根据加载类型的不同，分为`JdbcDataSourceProvider`和`YmlDataSourceProvider`，分别代表从数据库中加载或者从 yml 文件中加载，**默认实现是从 yml 文件中加载**

加载器位于`com.baomidou.dynamic.datasource.provider`包下，继承关系如下:

<div align=center><img src="/asset/dsp.jpg"></div>

###  5.1. <a name='_DynamicDataSourceProvider_'></a>顶级接口 _DynamicDataSourceProvider_

```java
public interface DynamicDataSourceProvider {

    /**
     * 加载所有数据源
     *
     * @return 所有数据源，key为数据源名称
     */
    Map<String, DataSource> loadDataSources();
}
```

###  5.2. <a name='_AbstractDataSourceProvider_'></a>抽象顶级加载器 _AbstractDataSourceProvider_

```java
@Slf4j
public abstract class AbstractDataSourceProvider implements DynamicDataSourceProvider {

    //数据源创建器
    @Autowired
    private DefaultDataSourceCreator defaultDataSourceCreator;

    protected Map<String, DataSource> createDataSourceMap(
            Map<String, DataSourceProperty> dataSourcePropertiesMap) {
        //返回的数据源集合
        Map<String, DataSource> dataSourceMap = new HashMap<>(dataSourcePropertiesMap.size() * 2);
        for (Map.Entry<String, DataSourceProperty> item : dataSourcePropertiesMap.entrySet()) {
            DataSourceProperty dataSourceProperty = item.getValue();
            //map中存储的key就是poolName
            String poolName = dataSourceProperty.getPoolName();
            if (poolName == null || "".equals(poolName)) {
                poolName = item.getKey();
            }
            dataSourceProperty.setPoolName(poolName);
            dataSourceMap.put(poolName, defaultDataSourceCreator.createDataSource(dataSourceProperty));
        }
        return dataSourceMap;
    }
}
```

###  5.3. <a name='JDBC_AbstractJdbcDataSourceProvider_'></a>JDBC 数据源加载器 _AbstractJdbcDataSourceProvider_

JDBC 数据源加载器是从数据库中读取每一个数据源的 info，然后根据查询结果封装成 Map 并放回，`loadDataSources`的源码如下:

```java
@Override
public Map<String, DataSource> loadDataSources() {
    Connection conn = null;
    Statement stmt = null;
    try {
        // 由于 SPI 的支持，现在已无需显示加载驱动了
        // 但在用户显示配置的情况下，进行主动加载
        if (!StringUtils.isEmpty(driverClassName)) {
            Class.forName(driverClassName);
            log.info("成功加载数据库驱动程序");
        }
        conn = DriverManager.getConnection(url, username, password);
        log.info("成功获取数据库连接");
        //查询数据库中数据源的信息
        stmt = conn.createStatement();
        //封装map
        Map<String, DataSourceProperty> dataSourcePropertiesMap = executeStmt(stmt);
        return createDataSourceMap(dataSourcePropertiesMap);
    } catch (Exception e) {
        e.printStackTrace();
    } finally {
        JdbcUtils.closeConnection(conn);
        JdbcUtils.closeStatement(stmt);
    }
    return null;
}
```

###  5.4. <a name='YML_YmlDynamicDataSourceProvider_'></a>YML 数据源加载器 _YmlDynamicDataSourceProvider_

```java
@Slf4j
@AllArgsConstructor
public class YmlDynamicDataSourceProvider extends AbstractDataSourceProvider {

    /**
     * 所有数据源
     */
    private final Map<String, DataSourceProperty> dataSourcePropertiesMap;

    @Override
    public Map<String, DataSource> loadDataSources() {
        //父类方法，加载yml配置文件，封装成map
        return createDataSourceMap(dataSourcePropertiesMap);
    }
}
```

##  6. <a name='-1'></a>数据源处理器

数据源处理器 Processor 主要功能是**运行时决定使用哪一个数据源** ，位于`com.baomidou.dynamic.datasource.processor`包下，继承关系如下:

<div align=center><img src="/asset/dsproc.jpg"></div>

###  6.1. <a name='_DsProcessor_'></a>顶级父类处理器 _DsProcessor_

`DsProcessor`以链表的形式，存储每一个处理器，链表的下一个节点保存在`nextProcessor`中。该项目供提供了三种处理器类型，`DsHeaderProcessor`，`DsSessionProcessor`和`DsSpelExpressionProcessor`，**根据 http 请求中的参数选择相应的处理器**

```java
public abstract class DsProcessor {

    private DsProcessor nextProcessor;

    public void setNextProcessor(DsProcessor dsProcessor) {
        this.nextProcessor = dsProcessor;
    }

    /**
     * 抽象匹配条件 匹配才会走当前执行器否则走下一级执行器
     *
     * @param key DS注解里的内容
     * @return 是否匹配
     */
    public abstract boolean matches(String key);

    /**
     * 决定数据源
     * <pre>
     *     调用底层doDetermineDatasource，
     *     如果返回的是null则继续执行下一个，否则直接返回
     * </pre>
     *
     * @param invocation 方法执行信息
     * @param key        DS注解里的内容
     * @return 数据源名称
     */
    public String determineDatasource(MethodInvocation invocation, String key) {
        if (matches(key)) {
            String datasource = doDetermineDatasource(invocation, key);
            if (datasource == null && nextProcessor != null) {
                return nextProcessor.determineDatasource(invocation, key);
            }
            return datasource;
        }
        if (nextProcessor != null) {
            //循环匹配
            return nextProcessor.determineDatasource(invocation, key);
        }
        return null;
    }

    /**
     * 抽象最终决定数据源
     *
     * @param invocation 方法执行信息
     * @param key        DS注解里的内容
     * @return 数据源名称
     */
    public abstract String doDetermineDatasource(MethodInvocation invocation, String key);
}
```

###  6.2. <a name='DsHeaderProcessor_'></a>_DsHeaderProcessor_

```java
public class DsHeaderProcessor extends DsProcessor {

    /**
     * header prefix
     */
    private static final String HEADER_PREFIX = "#header";

    @Override
    public boolean matches(String key) {
        return key.startsWith(HEADER_PREFIX);
    }

    @Override
    public String doDetermineDatasource(MethodInvocation invocation, String key) {
        HttpServletRequest request = ((ServletRequestAttributes) RequestContextHolder.getRequestAttributes()).getRequest();
        return request.getHeader(key.substring(8));
    }
}
```

###  6.3. <a name='DsSessionProcessor_'></a>_DsSessionProcessor_

```java
public class DsSessionProcessor extends DsProcessor {

    /**
     * session开头
     */
    private static final String SESSION_PREFIX = "#session";

    @Override
    public boolean matches(String key) {
        return key.startsWith(SESSION_PREFIX);
    }

    @Override
    public String doDetermineDatasource(MethodInvocation invocation, String key) {
        HttpServletRequest request = ((ServletRequestAttributes) RequestContextHolder.getRequestAttributes()).getRequest();
        return request.getSession().getAttribute(key.substring(9)).toString();
    }
}
```

###  6.4. <a name='DsSpelExpressionProcessor_'></a>_DsSpelExpressionProcessor_

```java
public class DsSpelExpressionProcessor extends DsProcessor {

    /**
     * 参数发现器
     */
    private static final ParameterNameDiscoverer NAME_DISCOVERER = new DefaultParameterNameDiscoverer();
    /**
     * Express语法解析器
     */
    private static final ExpressionParser PARSER = new SpelExpressionParser();
    /**
     * 解析上下文的模板
     * 对于默认不设置的情况下,从参数中取值的方式 #param1
     * 设置指定模板 ParserContext.TEMPLATE_EXPRESSION 后的取值方式: #{#param1}
     * issues: https://github.com/baomidou/dynamic-datasource-spring-boot-starter/issues/199
     */
    private ParserContext parserContext = new ParserContext() {

        @Override
        public boolean isTemplate() {
            return false;
        }

        @Override
        public String getExpressionPrefix() {
            return null;
        }

        @Override
        public String getExpressionSuffix() {
            return null;
        }
    };

    @Override
    public boolean matches(String key) {
        return true;
    }

    @Override
    public String doDetermineDatasource(MethodInvocation invocation, String key) {
        Method method = invocation.getMethod();
        Object[] arguments = invocation.getArguments();
        EvaluationContext context = new MethodBasedEvaluationContext(null, method, arguments, NAME_DISCOVERER);
        final Object value = PARSER.parseExpression(key, parserContext).getValue(context);
        return value == null ? null : value.toString();
    }

    public void setParserContext(ParserContext parserContext) {
        this.parserContext = parserContext;
    }
}
```

##  7. <a name='-1'></a>辅助工具

辅助工具组件位于`com.baomidou.dynamic.datasource.support`包下，主要提供一些核心方法支持，包含:

1. **`DataSourceClassResolver`** :数据源解析器，主要通过当前方法获取该方法的`@DS`注解参数配置，即获取该方法使用的数据源的名称
2. **`DbHealthIndicator`** :数据库健康指标
3. **`HealthCheckAdaptor`** :数据库健康检查器
4. **`DbConsts`** : 常量

###  7.1. <a name='_DataSourceClassResolver_'></a>数据源解析器 _DataSourceClassResolver_

该解析器负责解析当前 method 需要使用的数据源，并缓存在 map 中

```java
/**
 * 缓存方法对应的数据源
 */
private final Map<Object, String> dsCache = new ConcurrentHashMap<>();
```

当解析时，应首先尝试从本地缓存中获取，如果本地缓存获取不到，再尝试从解析器中解析

```java
/**
 * 从缓存获取数据
 *
 * @param method       方法
 * @param targetObject 目标对象
 * @return ds
 */
public String findDSKey(Method method, Object targetObject) {
    if (method.getDeclaringClass() == Object.class) {
        return "";
    }
    Object cacheKey = new MethodClassKey(method, targetObject.getClass());
    String ds = this.dsCache.get(cacheKey);
    if (ds == null) {
        ds = computeDatasource(method, targetObject);
        if (ds == null) {
            ds = "";
        }
        this.dsCache.put(cacheKey, ds);
    }
    return ds;
}
```

当本地缓存获取不到数据源信息，解析开始，解析是依据查找`@DS`注解上的`value`字段，解析的顺序为：

1. 当前方法
2. 桥接方法
3. 从当前类一直找到 Object
4. 若支持 mybatis-spring 或者 mybatis-plus，则寻找 mapper 上的注解信息

解析的核心步骤在`computeDataSource`方法中，源码如下:

```java
private String computeDatasource(Method method, Object targetObject) {
        if (allowedPublicOnly && !Modifier.isPublic(method.getModifiers())) {
            return null;
        }
        Class<?> targetClass = targetObject.getClass();
        Class<?> userClass = ClassUtils.getUserClass(targetClass);
        // JDK代理时,  获取实现类的方法声明.  method: 接口的方法, specificMethod: 实现类方法
        Method specificMethod = ClassUtils.getMostSpecificMethod(method, userClass);

        specificMethod = BridgeMethodResolver.findBridgedMethod(specificMethod);
        // 从当前方法查找
        String dsAttr = findDataSourceAttribute(specificMethod);
        if (dsAttr != null) {
            return dsAttr;
        }
        // 从当前方法声明的类查找
        dsAttr = findDataSourceAttribute(specificMethod.getDeclaringClass());
        if (dsAttr != null && ClassUtils.isUserLevelMethod(method)) {
            return dsAttr;
        }
        // 如果存在桥接方法
        if (specificMethod != method) {
            // 从桥接方法查找
            dsAttr = findDataSourceAttribute(method);
            if (dsAttr != null) {
                return dsAttr;
            }
            // 从桥接方法声明的类查找
            dsAttr = findDataSourceAttribute(method.getDeclaringClass());
            if (dsAttr != null && ClassUtils.isUserLevelMethod(method)) {
                return dsAttr;
            }
        }
        //如果获取不到，则执行默认解析
        return getDefaultDataSourceAttr(targetObject);
    }
```

```java
    /**
     * 用于处理嵌套代理
     *
     * @param target JDK 代理类对象
     * @return InvocationHandler 的 Class
     */
    private Class<?> getMapperInterfaceClass(Object target) {
        Object current = target;
        while (Proxy.isProxyClass(current.getClass())) {
            Object currentRefObject = AopProxyUtils.getSingletonTarget(current);
            if (currentRefObject == null) {
                break;
            }
            current = currentRefObject;
        }
        try {
            if (Proxy.isProxyClass(current.getClass())) {
                return (Class<?>) mapperInterfaceField.get(Proxy.getInvocationHandler(current));
            }
        } catch (IllegalAccessException ignore) {
        }
        return null;
    }
```