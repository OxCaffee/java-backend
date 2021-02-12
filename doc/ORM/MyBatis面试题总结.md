# MyBatis面试题总结

<!-- vscode-markdown-toc -->
* 1. [什么是MyBatis](#MyBatis)
* 2. [MyBatis优缺点](#MyBatis-1)
* 3. [MyBatis框架适用场合](#MyBatis-1)
* 4. [MyBatis和Hibernate的不同](#MyBatisHibernate)
* 5. [#{}和${}的区别是什么](#)
* 6. [当实体类的属性名和表中字段的字段名不一样的时候该怎么办](#-1)
* 7. [模糊查询like语句怎么写](#like)
* 8. [通常一个XML映射文件都会写一个Dao接口与之对应，那么Dao接口里的方法，参数不同时，方法能重载吗](#XMLDaoDao)
* 9. [Mapper接口的工作原理是什么](#Mapper)
* 10. [MyBatis如何进行分页?分页插件的原理是什么](#MyBatis-1)
* 11. [MyBatis如何实现ORM](#MyBatisORM)
* 12. [如何执行批量插入](#-1)
* 13. [Mapper中如何传递多个参数](#Mapper-1)
* 14. [MyBatis动态SQL有什么用?执行原理?有哪些动态SQL](#MyBatisSQLSQL)
* 15. [XML映射文件中，除了select|insert|update|delete标签之外，还有哪些动态标签](#XMLselectinsertupdatedelete)
* 16. [Mybatis 动态 SQL 是做什么的？都有哪些动态 SQL ？能简述一下动态 SQL 的执行原理吗？](#MybatisSQLSQLSQL)
* 17. [通常一个 XML 映射文件，都会写一个 Mapper 接口与之对应。请问，这个 Mapper 接口的工作原理是什么？Mapper 接口里的方法，参数不同时，方法能重载吗？](#XMLMapperMapperMapper)
* 18. [MyBatis的XML映射文件中，不同的XML映射文件，id是否可以重复](#MyBatisXMLXMLid)
* 19. [如何获取自动生成的主键值](#-1)
* 20. [MyBatis执行批量插入的时候，可以返回数据库主键列表吗](#MyBatis-1)
* 21. [Mapper中如何传递多个参数](#Mapper-1)
* 22. [MyBatis是否可以映射Enum枚举类](#MyBatisEnum)
* 23. [MyBatis都有哪些Executor执行器？他们之间的区别是什么？](#MyBatisExecutor)
* 24. [MyBatis如何进行批量插入](#MyBatis-1)
* 25. [Mybatis 是否支持延迟加载？如果支持，它的实现原理是什么？](#Mybatis)
* 26. [简述 Mybatis 的插件运行原理？以及如何编写一个插件？](#Mybatis-1)
* 27. [Mybatis 是如何进行分页的？分页插件的原理是什么？](#Mybatis-1)
* 28. [JDBC 编程有哪些不足之处，MyBatis是如何解决这些问题的？](#JDBCMyBatis)
* 29. [为什么说MyBatis是半自动ORM映射工具?它与全自动的区别在哪](#MyBatisORM-1)
* 30. [一对一，一对多关联查询](#-1)
* 31. [MyBatis有哪几种一对一方式?如何实现嵌套查询](#MyBatis-1)
* 32. [MyBatis有哪几种一对多方式?如何操作](#MyBatis-1)
* 33. [MyBatis是否支持延迟加载?](#MyBatis-1)
* 34. [MyBatis一级缓存及二级缓存](#MyBatis-1)
* 35. [参考连接](#-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='MyBatis'></a>什么是MyBatis

* 半ORM持久层框架，内部封装了JDBC，开发时只需要关注SQL语句本身，不需要去可以关注数据库的连接等，让程序员可以编写原生的SQL语句
* MyBatis可以将数据库记录与POJO对象关联起来

##  2. <a name='MyBatis-1'></a>MyBatis优缺点

* 优点:
  * 基于SQL语句，编程灵活，抛弃了冗余的JDBC配置，并且不需要手动进行开关
  * 提供映射标签配置，支持对象和数据库的ORM字段关系映射，提供对象关系映射标签，支持对象关系组件的维护
  * 很好的和各种数据库进行兼容(MyBatis支持对不同的数据库的连接配置)
* 缺点:
  * MyBatis对于SQL语句的编写的工作量比较大，尤其是当字段数多的时候，关联的表多的时候，对开发人员的编写有一定的要求
  * SQL语句依赖于数据库，导致数据库的移植性差，不能随意更换数据库

##  3. <a name='MyBatis-1'></a>MyBatis框架适用场合

* MyBatis专注于SQL本身，是一个足够灵活的DAO层解决方案
* 对于性能有很高的要求，或者需求变化比较多的项目，比如互联网的项目，MyBatis将是一个不错的选择

##  4. <a name='MyBatisHibernate'></a>MyBatis和Hibernate的不同

* MyBatis和Hibernate不同，它不是一个完全的ORM框架，因为MyBatis需要程序员自己编写SQL语言，而Hibernate对于数据库的操作是完全基于对实体对象的操作或者基于对于实体对象的代理对象的操作而进行的
* MyBatis直接编写原生态SQL语言，可以严格控制SQL执行的性能，灵活度非常的高，非常适合对于关系型数据模型要求不高的开发软件
* Hibernate对于对象/关系的映射十分之强，数据库无关性好，如果用Hibernate开发会节省很多的代码，提高很大的效率
* **简而言之，hibernate专注于效率，mybatis专注于性能**

##  5. <a name=''></a>#{}和${}的区别是什么

`#{}`是预编译处理，`${}`是字符串替换

`${}` 是 Properties 文件中的变量占位符，它可以用于 XML 标签属性值和 SQL 内部，属于**字符串替换**。例如将 `${driver}` 会被静态替换为 `com.mysql.jdbc.Driver` ：

```xml
<dataSource type="UNPOOLED">
    <property name="driver" value="${driver}"/>
    <property name="url" value="${url}"/>
    <property name="username" value="${username}"/>
</dataSource>
```

`${}` 也可以对传递进来的参数**原样拼接**在 SQL 中。代码如下：

```xml
<select id="getSubject3" parameterType="Integer" resultType="Subject">
    SELECT * FROM subject
    WHERE id = ${id}
</select>
```

`#{}` 是 SQL 的参数占位符，Mybatis 会将 SQL 中的 `#{}` 替换为 `?` 号，在 SQL 执行前会使用 PreparedStatement 的参数设置方法，按序给 SQL 的 `?` 号占位符设置参数值，比如 `ps.setInt(0, parameterValue)` 。 所以，`#{}` 是**预编译处理**，可以有效防止 SQL 注入，提高系统安全性。

另外，`#{}` 和 `${}` 的取值方式非常方便。例如：`#{item.name}` 的取值方式，为使用反射从参数对象中，获取 `item` 对象的 `name` 属性值，相当于 `param.getItem().getName()` 。

##  6. <a name='-1'></a>当实体类的属性名和表中字段的字段名不一样的时候该怎么办

* **方法一**: 通过定义查询的别名来时SQL语句中的字段名的别名和实体类的属一致

```sql
<select id="selectorder" parameterType="int" resultType="xxx.xxx...order">
    select
        order_id as 'id',
        order_no as 'no',
        order_price as 'price',
    where
        order_id = #{id}
</select>
```

* **方法二**: 通过`<resultMap>`来映射字段名和实体类属性名的一一对应关系

```sql
<select id="getOrder" parameterType="int" resultMap="orderResultMap">
    select * from orders where order_id = #{id}
</select>

<resultMap type="xxx.xxx...Order" id="orderResultMap">
    <id property="id" column="order_id"/>
    <result property="orderno" column="order_no"/>
    <result property="price" column="order_price"/>
</resultMap>
```

* **方法三: ** 是第一种的特殊情况。大多数场景下，数据库字段名和实体类中的属性名差，主要是前者为**下划线风格**，后者为**驼峰风格**。在这种情况下，可以直接配置如下，实现自动的下划线转驼峰的功能。

```xml
<setting name="logImpl" value="LOG4J"/>
    <setting name="mapUnderscoreToCamelCase" value="true" />
</settings>
```

##  7. <a name='like'></a>模糊查询like语句怎么写

* **方法一**: 在Java代码中添加SQL通配符

```java
//Mapper对应的java代码
String wildCardName = "%smi%";
List<String> names = mapper.selectLike(wildCardName);
```

```sql
<!-- 对应的xml代码 -->
<select id="selectLike">
    select * from foo where bar like "%"#{value}"#"
</select>
```

##  8. <a name='XMLDaoDao'></a>通常一个XML映射文件都会写一个Dao接口与之对应，那么Dao接口里的方法，参数不同时，方法能重载吗

**不能**，Mapper接口方法的执行是基于**接口类的全限定名+接口名**来寻找的

##  9. <a name='Mapper'></a>Mapper接口的工作原理是什么

JDK动态代理。MyBatis运行时会使用JDK动态代理为Mapper接口生成代理对象，代理对象会拦截接口方法，转而执行MapperStatement所代表的sql,然后sql执行结果返回

##  10. <a name='MyBatis-1'></a>MyBatis如何进行分页?分页插件的原理是什么

* MyBatis使用RowBounds对象继续分页，针对ResultSet结果集执行的内存分页而非物理分页。可以在SQL内直接书写带有物理分页的参数来完成物理分页的功能，也可使用分页插件来完成物理分页
* 分页插件的基本原理是使用MyBatis提供的接口插件，实现自定义插件，在插件的拦截方法内拦截待执行的SQL，然后重写SQL，根据Dialect方言，田间对应的物理分页语句和物理分页参数

##  11. <a name='MyBatisORM'></a>MyBatis如何实现ORM

* **标签**: 定义列名和对象属性之间的映射关系
* **SQL列的别名**: 反射给对象的属性

##  12. <a name='-1'></a>如何执行批量插入

* 首先在xml中定义插入操作

```sql
<insert id="insertName">
    insert into names (name) values(#{value})
</insert>
```

* 然后在java语句中执行

```java
List<String> names = new ArrayList<>();
names.add(name1);
names.add(name2);
names.add(name3);
names.add(name4);

SqlSession session = sqlSessionFactory.openSession(executortype.batch);
try{
    XXXMapper mapper = session.getMapper(XXXMapper.class);
    for(String name : names){
        mapper.insertName(name);
    }
    session.commit();
}catch(Exception e){
    e.printStackTrace();
}
```

##  13. <a name='Mapper-1'></a>Mapper中如何传递多个参数

* 方法一:

```java
public UserselectUser(String name,String area);
对应的 xml,#{0}代表接收的是 dao 层中的第一个参数，#{1}代表 dao 层中第二参数，更多参数一致往后加即可。
```

```sql
<select id="selectUser"resultMap="BaseResultMap">
select * fromuser_user_t whereuser_name = #{0}
anduser_area=#{1}
</select>
```

* 方法二

```java
//使用@param注解
public interface UserMapper{
    User selectUser(@param("username") String username,
                    @param("hashedpassword") String password);
}
```

```xml
<select id="selectUser" resultType="User">
    select id, username, hashedpassword
    from some_table
    where
        username = #{username}
    and
        hashedpassword = #{hashedpassword}
</select>
```

##  14. <a name='MyBatisSQLSQL'></a>MyBatis动态SQL有什么用?执行原理?有哪些动态SQL

Mybatis 动态 sql 可以在 Xml 映射文件内，以标签的形式编写动态 sql，执行原理
是根据表达式的值 完成逻辑判断并动态拼接 sql 的功能。
Mybatis 提供了 9 种动态 sql 标签：
trim | where | set | foreach | if | choose| when | otherwise | bind

##  15. <a name='XMLselectinsertupdatedelete'></a>XML映射文件中，除了select|insert|update|delete标签之外，还有哪些动态标签

`<resultMap>`, `<parameterMap>`, `<sql>`, `<include>`, `<selectKey>`

- `<cache/>`标签，给定命名空间的缓存配置。
- `<cache-ref />` 标签，其他命名空间缓存配置的引用。
- `<resultMap />` 标签，是最复杂也是最强大的元素，用来描述如何从数据库结果集中来加载对象。
- `<sql/>`标签，可被其他语句引用的可重用语句块。
- `<include />` 标签，引用 `<sql/>` 标签的语句。
- `<selectKey />` 标签，不支持自增的主键生成策略标签。
- `<if />`
- `<choose />`、`<when />`、`<otherwise />`
- `<trim />`、`<where />`、`<set />`
- `<foreach />`
- `<bind />`

[《MyBatis 文档 —— 动态 SQL》](http://www.mybatis.org/mybatis-3/zh/dynamic-sql.html)

##  16. <a name='MybatisSQLSQLSQL'></a>Mybatis 动态 SQL 是做什么的？都有哪些动态 SQL ？能简述一下动态 SQL 的执行原理吗？

- Mybatis 动态 SQL ，可以让我们在 XML 映射文件内，以 XML 标签的形式编写动态 SQL ，完成逻辑判断和动态拼接 SQL 的功能。
- Mybatis 提供了 9 种动态 SQL 标签：`<if />`、`<choose />`、`<when />`、`<otherwise />`、`<trim />`、`<where />`、`<set />`、`<foreach />`、`<bind />` 。
- 其执行原理为，使用 **OGNL** 的表达式，从 SQL 参数对象中计算表达式的值，根据表达式的值动态拼接 SQL ，以此来完成动态 SQL 的功能。

##  17. <a name='XMLMapperMapperMapper'></a>通常一个 XML 映射文件，都会写一个 Mapper 接口与之对应。请问，这个 Mapper 接口的工作原理是什么？Mapper 接口里的方法，参数不同时，方法能重载吗？

Mapper 接口，对应的关系如下：

- 接口的全限名，就是映射文件中的 `"namespace"` 的值。
- 接口的方法名，就是映射文件中 MappedStatement 的 `"id"` 值。
- 接口方法内的参数，就是传递给 SQL 的参数。

Mapper 接口是没有实现类的，当调用接口方法时，接口全限名 + 方法名拼接字符串作为 key 值，可唯一定位一个对应的 MappedStatement 。举例：`com.mybatis3.mappers.StudentDao.findStudentById` ，可以唯一找到 `"namespace"` 为 `com.mybatis3.mappers.StudentDao` 下面 `"id"` 为 `findStudentById` 的 MappedStatement 。

总结来说，在 Mybatis 中，每一个 `<select />`、`<insert />`、`<update />`、`<delete />` 标签，都会被解析为一个 MappedStatement 对象。

另外，Mapper 接口的实现类，通过 MyBatis 使用 **JDK Proxy** 自动生成其代理对象 Proxy ，而代理对象 Proxy 会拦截接口方法，从而“调用”对应的 MappedStatement 方法，最终执行 SQL ，返回执行结果。整体流程如下图：

<div align=center><img src="/assets/mb1.png"/></div>

**Mapper 接口里的方法，是不能重载的，因为是全限名 + 方法名的保存和寻找策略。**

##  18. <a name='MyBatisXMLXMLid'></a>MyBatis的XML映射文件中，不同的XML映射文件，id是否可以重复

如果配置`<namespace>`，那么id可以重复，如果没有配置，id不能重复

##  19. <a name='-1'></a>如何获取自动生成的主键值

不同的数据库，获取自动生成的(主)键值的方式是不同的。

MySQL 有两种方式，但是**自增主键**，代码如下：

```xml
// 方式一，使用 useGeneratedKeys + keyProperty 属性
<insert id="insert" parameterType="Person" useGeneratedKeys="true" keyProperty="id">
    INSERT INTO person(name, pswd)
    VALUE (#{name}, #{pswd})
</insert>
    
// 方式二，使用 `<selectKey />` 标签
<insert id="insert" parameterType="Person">
    <selectKey keyProperty="id" resultType="long" order="AFTER">
        SELECT LAST_INSERT_ID()
    </selectKey>
        
    INSERT INTO person(name, pswd)
    VALUE (#{name}, #{pswd})
</insert>
```

其中，**方式一**较为常用。

##  20. <a name='MyBatis-1'></a>MyBatis执行批量插入的时候，可以返回数据库主键列表吗

能，JDBC 都能做，Mybatis 当然也能做。

##  21. <a name='Mapper-1'></a>Mapper中如何传递多个参数

**第一种，使用 Map 集合，装载多个参数进行传递**。代码如下：

```xml
// 调用方法
Map<String, Object> map = new HashMap();
map.put("start", start);
map.put("end", end);
return studentMapper.selectStudents(map);

// Mapper 接口
List<Student> selectStudents(Map<String, Object> map);

// Mapper XML 代码
<select id="selectStudents" parameterType="Map" resultType="Student">
    SELECT * 
    FROM students 
    LIMIT #{start}, #{end}
</select>
```

**第二种，保持传递多个参数，使用 `@Param` 注解**。代码如下：

```xml
// 调用方法
return studentMapper.selectStudents(0, 10);

// Mapper 接口
List<Student> selectStudents(@Param("start") Integer start, @Param("end") Integer end);

// Mapper XML 代码
<select id="selectStudents" resultType="Student">
    SELECT * 
    FROM students 
    LIMIT #{start}, #{end}
</select>
```

**第三种，保持传递多个参数，不使用 `@Param` 注解** ，代码如下:

```xml
// 调用方法
return studentMapper.selectStudents(0, 10);

// Mapper 接口
List<Student> selectStudents(Integer start, Integer end);

// Mapper XML 代码
<select id="selectStudents" resultType="Student">
    SELECT * 
    FROM students 
    LIMIT #{param1}, #{param2}
</select>
```

##  22. <a name='MyBatisEnum'></a>MyBatis是否可以映射Enum枚举类

Mybatis 可以映射枚举类，对应的实现类为 EnumTypeHandler 或 EnumOrdinalTypeHandler 。

- EnumTypeHandler ，基于 `Enum.name` 属性( String )。**默认**。
- EnumOrdinalTypeHandler ，基于 `Enum.ordinal` 属性( `int` )。可通过 `<setting name="defaultEnumTypeHandler" value="EnumOrdinalTypeHandler" />` 来设置。

当然，实际开发场景，我们很少使用 Enum 类型，更加的方式是，代码如下：

```java
public class Dog {

    public static final int STATUS_GOOD = 1;
    public static final int STATUS_BETTER = 2;
    public static final int STATUS_BEST = 3；
    
    private int status;
    
}
```

并且，不单可以映射枚举类，Mybatis 可以映射任何对象到表的一列上。映射方式为自定义一个 TypeHandler 类，实现 TypeHandler 的`#setParameter(...)` 和 `#getResult(...)` 接口方法。

TypeHandler 有两个作用：

- 一是，完成从 javaType 至 jdbcType 的转换。
- 二是，完成 jdbcType 至 javaType 的转换。

具体体现为 `#setParameter(...)` 和 `#getResult(..)` 两个方法，分别代表设置 SQL 问号占位符参数和获取列查询结果。

##  23. <a name='MyBatisExecutor'></a>MyBatis都有哪些Executor执行器？他们之间的区别是什么？

Mybatis 有四种 Executor 执行器，分别是 SimpleExecutor、ReuseExecutor、BatchExecutor、CachingExecutor 。

- SimpleExecutor ：每执行一次 update 或 select 操作，就创建一个 Statement 对象，用完立刻关闭 Statement 对象。
- ReuseExecutor ：执行 update 或 select 操作，以 SQL 作为key 查找**缓存**的 Statement 对象，存在就使用，不存在就创建；用完后，不关闭 Statement 对象，而是放置于缓存 `Map<String, Statement>` 内，供下一次使用。简言之，就是重复使用 Statement 对象。
- BatchExecutor ：执行 update 操作（没有 select 操作，因为 JDBC 批处理不支持 select 操作），将所有 SQL 都添加到批处理中（通过 addBatch 方法），等待统一执行（使用 executeBatch 方法）。它缓存了多个 Statement 对象，每个 Statement 对象都是调用 addBatch 方法完毕后，等待一次执行 executeBatch 批处理。**实际上，整个过程与 JDBC 批处理是相同**。
- CachingExecutor ：在上述的三个执行器之上，增加**二级缓存**的功能。

**通过设置 `<setting name="defaultExecutorType" value="">` 的 `"value"` 属性，可传入 SIMPLE、REUSE、BATCH 三个值，分别使用 SimpleExecutor、ReuseExecutor、BatchExecutor 执行器。**

**通过设置 `<setting name="cacheEnabled" value=""` 的 `"value"` 属性为 `true` 时，创建 CachingExecutor 执行器。**

##  24. <a name='MyBatis-1'></a>MyBatis如何进行批量插入

首先，在 Mapper XML 编写一个简单的 Insert 语句。代码如下：

```xml
<insert id="insertUser" parameterType="String"> 
    INSERT INTO users(name) 
    VALUES (#{value}) 
</insert>
```

然后，然后在对应的 Mapper 接口中，编写映射的方法。代码如下：

```java
public interface UserMapper {
    
    void insertUser(@Param("name") String name);

}
```

最后，调用该 Mapper 接口方法。代码如下：

```java
private static SqlSessionFactory sqlSessionFactory;

@Test
public void testBatch() {
    // 创建要插入的用户的名字的数组
    List<String> names = new ArrayList<>();
    names.add("占小狼");
    names.add("朱小厮");
    names.add("徐妈");
    names.add("飞哥");

    // 获得执行器类型为 Batch 的 SqlSession 对象，并且 autoCommit = false ，禁止事务自动提交
    try (SqlSession session = sqlSessionFactory.openSession(ExecutorType.BATCH, false)) {
        // 获得 Mapper 对象
        UserMapper mapper = session.getMapper(UserMapper.class);
        // 循环插入
        for (String name : names) {
            mapper.insertUser(name);
        }
        // 提交批量操作
        session.commit();
    }
}
```

代码比较简单，胖友仔细看看。当然，还有另一种方式，代码如下：

```sql
INSERT INTO [表名]([列名],[列名]) 
VALUES
([列值],[列值])),
([列值],[列值])),
([列值],[列值]));
```

- 对于这种方式，需要保证单条 SQL 不超过语句的最大限制 `max_allowed_packet` 大小，默认为 1 M 。

##  25. <a name='Mybatis'></a>Mybatis 是否支持延迟加载？如果支持，它的实现原理是什么？

Mybatis 仅支持 association 关联对象和 collection 关联集合对象的延迟加载。其中，association 指的就是**一对一**，collection 指的就是**一对多查询**。

在 Mybatis 配置文件中，可以配置 `<setting name="lazyLoadingEnabled" value="true" />` 来启用延迟加载的功能。默认情况下，延迟加载的功能是**关闭**的。

它的原理是，使用 CGLIB 或 Javassist( 默认 ) 创建目标对象的代理对象。当调用代理对象的延迟加载属性的 getting 方法时，进入拦截器方法。比如调用 `a.getB().getName()` 方法，进入拦截器的 `invoke(...)` 方法，发现 `a.getB()` 需要延迟加载时，那么就会单独发送事先保存好的查询关联 B 对象的 SQL ，把 B 查询上来，然后调用`a.setB(b)` 方法，于是 `a` 对象 `b` 属性就有值了，接着完成`a.getB().getName()` 方法的调用。这就是延迟加载的基本原理。

当然了，不光是 Mybatis，几乎所有的包括 Hibernate 在内，支持延迟加载的原理都是一样的。

##  26. <a name='Mybatis-1'></a>简述 Mybatis 的插件运行原理？以及如何编写一个插件？

编写一个 MyBatis 插件的步骤如下：

1. 首先，实现 Mybatis 的 Interceptor 接口，并实现 `#intercept(...)` 方法。
2. 然后，在给插件编写注解，指定要拦截哪一个接口的哪些方法即可
3. 最后，在配置文件中配置你编写的插件。

##  27. <a name='Mybatis-1'></a>Mybatis 是如何进行分页的？分页插件的原理是什么？

Mybatis 使用 RowBounds 对象进行分页，它是针对 ResultSet 结果集执行的**内存分页**，而非**数据库分页**。

所以，实际场景下，不适合直接使用 MyBatis 原有的 RowBounds 对象进行分页。而是使用如下两种方案：

- 在 SQL 内直接书写带有数据库分页的参数来完成数据库分页功能
- 也可以使用分页插件来完成数据库分页。

这两者都是基于数据库分页，差别在于前者是工程师**手动**编写分页条件，后者是插件**自动**添加分页条件。

------

分页插件的基本原理是使用 Mybatis 提供的插件接口，实现自定义分页插件。在插件的拦截方法内，拦截待执行的 SQL ，然后重写 SQL ，根据dialect 方言，添加对应的物理分页语句和物理分页参数。

举例：`SELECT * FROM student` ，拦截 SQL 后重写为：`select * FROM student LIMI 0，10` 。

目前市面上目前使用比较广泛的 MyBatis 分页插件有：

- [Mybatis-PageHelper](https://github.com/pagehelper/Mybatis-PageHelper)
- [MyBatis-Plus](https://github.com/baomidou/mybatis-plus)

从现在看来，[MyBatis-Plus](https://github.com/baomidou/mybatis-plus) 逐步使用的更加广泛。

##  28. <a name='JDBCMyBatis'></a>JDBC 编程有哪些不足之处，MyBatis是如何解决这些问题的？

**问题一：SQL 语句写在代码中造成代码不易维护，且代码会比较混乱。**

解决方式：将 SQL 语句配置在 Mapper XML 文件中，与 Java 代码分离。

**问题二：根据参数不同，拼接不同的 SQL 语句非常麻烦。例如 SQL 语句的 WHERE 条件不一定，可能多也可能少，占位符需要和参数一一对应。**

解决方式：MyBatis 提供 `<where />`、`<if />` 等等动态语句所需要的标签，并支持 OGNL 表达式，简化了动态 SQL 拼接的代码，提升了开发效率。

**问题三，对结果集解析麻烦，SQL 变化可能导致解析代码变化，且解析前需要遍历。**

解决方式：Mybatis 自动将 SQL 执行结果映射成 Java 对象。

**问题四，数据库链接创建、释放频繁造成系统资源浪费从而影响系统性能，如果使用数据库链接池可解决此问题。**

解决方式：在 `mybatis-config.xml` 中，配置数据链接池，使用连接池管理数据库链接。

当然，即使不使用 MyBatis ，也可以使用数据库连接池。
另外，MyBatis 默认提供了数据库连接池的实现，只是说，因为其它开源的数据库连接池性能更好，所以一般很少使用 MyBatis 自带的连接池实现。

##  29. <a name='MyBatisORM-1'></a>为什么说MyBatis是半自动ORM映射工具?它与全自动的区别在哪

Hibernate 属于全自动 ORM 映射工具，使用 Hibernate 查询关联对象或者关联集合对象时，可以根据对象关系模型直接获取，所以它是全自动的。而 Mybatis在查询关联对象或关联集合对象时，需要手动编写 sql 来完成，所以，称之为半自动 ORM 映射工具

##  30. <a name='-1'></a>一对一，一对多关联查询

```xml
<mapper namespace="com.lcb.mapping.userMapper">
<!--association 一对一关联查询 -->
<select id="getClass" parameterType="int" resultMap="ClassesResultMap">
    select * from class c,teacher t where c.teacher_id=t.t_id and
    c.c_id=#{id}
</select>
<resultMap type="com.lcb.user.Classes" id="ClassesResultMap">
<!-- 实体类的字段名和数据表的字段名映射 -->
<id property="id" column="c_id"/>
    <result property="name" column="c_name"/>
    <association property="teacher" javaType="com.lcb.user.Teacher">
    <id property="id" column="t_id"/>
    <result property="name" column="t_name"/>
    </association>
    </resultMap>
<!--collection 一对多关联查询 -->
<select id="getClass2" parameterType="int"
resultMap="ClassesResultMap2">
select * from class c,teacher t,student s where c.teacher_id=t.t_id
and c.c_id=s.class_id and c.c_id=#{id}
</select>
<resultMap type="com.lcb.user.Classes" id="ClassesResultMap2">
<id property="id" column="c_id"/>
<result property="name" column="c_name"/>
<association property="teacher"
javaType="com.lcb.user.Teacher">
<id property="id" column="t_id"/>
<result property="name" column="t_name"/>
</association>
<collection property="student"
ofType="com.lcb.user.Student">
<id property="id" column="s_id"/>
<result property="name" column="s_name"/>
</collection>
</resultMap>
</mapper>
```

##  31. <a name='MyBatis-1'></a>MyBatis有哪几种一对一方式?如何实现嵌套查询

* 联合查询: 一次查询
* 嵌套查询: 一次查询结果作为另外一次查询进行查询，通过`<select>`标签来实现嵌套查询

##  32. <a name='MyBatis-1'></a>MyBatis有哪几种一对多方式?如何操作

* 联合查询: 同一对一
* 嵌套查询: `<collection>`标签

##  33. <a name='MyBatis-1'></a>MyBatis是否支持延迟加载?

MyBatis仅仅支持association关联对象和collection关联对象的延迟加载，可以配置是否启用`lazyLoadingEnabled=true|false`

##  34. <a name='MyBatis-1'></a>MyBatis一级缓存及二级缓存

* **一级缓存**: 基于PerpetualCache的HashMap本地缓存，其存储作用域为Session，当Session flush或者close之后，该Session中的所有Cache就将清空，默认打开一级缓存
* **二级缓存**: 默认存储方式同一级缓存，且默认不打开，若要打开，使用二级缓存属性类需要实现Serializable序列化接口，可在它的映射文件中配置`<cahce>`



##  35. <a name='-1'></a>参考连接

* [MyBatis面试集锦](https://www.cnblogs.com/godoforange/p/11623535.html)
