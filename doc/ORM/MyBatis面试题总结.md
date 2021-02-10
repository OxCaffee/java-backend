# MyBatis面试题总结

## 什么是MyBatis

* 半ORM持久层框架，内部封装了JDBC，开发时只需要关注SQL语句本身，不需要去可以关注数据库的连接等，让程序员可以编写原生的SQL语句
* MyBatis可以将数据库记录与POJO对象关联起来

## MyBatis优缺点

* 优点:
  * 基于SQL语句，编程灵活，抛弃了冗余的JDBC配置，并且不需要手动进行开关
  * 提供映射标签配置，支持对象和数据库的ORM字段关系映射，提供对象关系映射标签，支持对象关系组件的维护
  * 很好的和各种数据库进行兼容(MyBatis支持对不同的数据库的连接配置)
* 缺点:
  * MyBatis对于SQL语句的编写的工作量比较大，尤其是当字段数多的时候，关联的表多的时候，对开发人员的编写有一定的要求
  * SQL语句依赖于数据库，导致数据库的移植性差，不能随意更换数据库

## MyBatis框架适用场合

* MyBatis专注于SQL本身，是一个足够灵活的DAO层解决方案
* 对于性能有很高的要求，或者需求变化比较多的项目，比如互联网的项目，MyBatis将是一个不错的选择

## MyBatis和Hibernate的区别

* MyBatis和Hibernate不同，它不是一个完全的ORM框架，因为MyBatis需要程序员自己编写SQL语言，而Hibernate对于数据库的操作是完全基于对实体对象的操作或者基于对于实体对象的代理对象的操作而进行的
* MyBatis直接编写原生态SQL语言，可以严格控制SQL执行的性能，灵活度非常的高，非常适合对于关系型数据模型要求不高的开发软件
* Hibernate对于对象/关系的映射十分之强，数据库无关性好，如果用Hibernate开发会节省很多的代码，提高很大的效率
* **简而言之，hibernate专注于效率，mybatis专注于性能**

## #{}和${}的区别是什么

`#{}`是预编译处理，`${}`是字符串替换

## 当实体类的属性名和表中字段的字段名不一样的时候该怎么办

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

## 模糊查询like语句怎么写

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

## 通常一个XML映射文件都会写一个Dao接口与之对应，那么Dao接口里的方法，参数不同时，方法能重载吗

**不能**，Mapper接口方法的执行是基于**接口类的全限定名+接口名**来寻找的

## Mapper接口的工作原理是什么

JDK动态代理。MyBatis运行时会使用JDK动态代理为Mapper接口生成代理对象，代理对象会拦截接口方法，转而执行MapperStatement所代表的sql,然后sql执行结果返回

## MyBatis如何进行分页?分页插件的原理是什么

* MyBatis使用RowBounds对象继续分页，针对ResultSet结果集执行的内存分页而非物理分页。可以在SQL内直接书写带有物理分页的参数来完成物理分页的功能，也可使用分页插件来完成物理分页
* 分页插件的基本原理是使用MyBatis提供的接口插件，实现自定义插件，在插件的拦截方法内拦截待执行的SQL，然后重写SQL，根据Dialect方言，田间对应的物理分页语句和物理分页参数

## MyBatis如何实现ORM

* **标签**: 定义列名和对象属性之间的映射关系
* **SQL列的别名**: 反射给对象的属性

## 如何执行批量插入

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

## Mapper中如何传递多个参数

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

```sql
<select id="selectUser" resultType="User">
    select id, username, hashedpassword
    from some_table
    where
        username = #{username}
    and
        hashedpassword = #{hashedpassword}
</select>
```

* 方法三

多个参数封装成Map或者List，一个Map或者List作为一个参数来传递

## MyBatis动态SQL有什么用?执行原理?有哪些动态SQL

Mybatis 动态 sql 可以在 Xml 映射文件内，以标签的形式编写动态 sql，执行原理
是根据表达式的值 完成逻辑判断并动态拼接 sql 的功能。
Mybatis 提供了 9 种动态 sql 标签：
trim | where | set | foreach | if | choose| when | otherwise | bind

## XML映射文件中，除了select|insert|update|delete标签之外，还有哪些动态标签

`<resultMap>`, `<parameterMap>`, `<sql>`, `<include>`, `<selectKey>`

## MyBatis的XML映射文件中，不同的XML映射文件，id是否可以重复

如果配置`<namespace>`，那么id可以重复，如果没有配置，id不能重复

## 为什么说MyBatis是半自动ORM映射工具?它与全自动的区别在哪

Hibernate 属于全自动 ORM 映射工具，使用 Hibernate 查询关联对象或者关联集合对象时，可以根据对象关系模型直接获取，所以它是全自动的。而 Mybatis在查询关联对象或关联集合对象时，需要手动编写 sql 来完成，所以，称之为半自动 ORM 映射工具

## 一对一，一对多关联查询

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

## MyBatis有哪几种一对一方式?如何实现嵌套查询

* 联合查询: 一次查询
* 嵌套查询: 一次查询结果作为另外一次查询进行查询，通过`<select>`标签来实现嵌套查询

## MyBatis有哪几种一对多方式?如何操作

* 联合查询: 同一对一
* 嵌套查询: `<collection>`标签

## MyBatis是否支持延迟加载?

MyBatis仅仅支持association关联对象和collection关联对象的延迟加载，可以配置是否启用`lazyLoadingEnabled=true|false`

## MyBatis一级缓存及二级缓存

* **一级缓存**: 基于PerpetualCache的HashMap本地缓存，其存储作用域为Session，当Session flush或者close之后，该Session中的所有Cache就将清空，默认打开一级缓存
* **二级缓存**: 默认存储方式同一级缓存，且默认不打开，若要打开，使用二级缓存属性类需要实现Serializable序列化接口，可在它的映射文件中配置`<cahce>`

## 参考连接

* [MyBatis面试集锦](https://www.cnblogs.com/godoforange/p/11623535.html)
