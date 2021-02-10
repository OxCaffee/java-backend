# MyBatis应用实践

## MyBatis环境搭建及入门案例

* pom.xml

```xml
<dependencies>
        <!-- mysql依赖 -->
        <dependency>
            <groupId>mysql</groupId>
            <artifactId>mysql-connector</artifactId>
            <version>8.0.13</version>
        </dependency>

        <!-- junit测试 -->
        <dependency>
            <groupId>junit</groupId>
            <artifactId>junit</artifactId>
            <version>4.12</version>
        </dependency>

        <!-- mybatis -->
        <dependency>
            <groupId>org.mybatis</groupId>
            <artifactId>mybatis</artifactId>
            <version>3.4.1</version>
        </dependency>

        <!-- log4j -->
        <dependency>
            <groupId>log4j</groupId>
            <artifactId>log4j</artifactId>
            <version>1.2.17</version>
        </dependency>
    </dependencies>
```

* 配置SqlMapConfig.xml

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE configuration
        PUBLIC "-//mybatis.org//DTD Config 3.0//EN"
        "http://mybatis.org/dtd/mybatis-3-config.dtd">

<configuration>
    <!-- 配置环境 -->
    <environments default="mysql">
        <!-- 配置mysql -->
        <environment id="mysql">
            <!-- 配置事务的类型 -->
            <transactionManager type="JDBC"/>
            <!-- 配置数据库使用连接池 -->
            <dataSource type="POOLED">
                <!-- 配置数据库连接的基本信息 -->
                <property name="driver" value="com.mysql.cj.jdbc.Driver"/>
                <property name="url" value="jdbc:mysql://localhost:3306/hibernate?serverTimezone=UTC"/>
                <property name="username" value="root"/>
                <property name="password" value="123456"/>
            </dataSource>
        </environment>
    </environments>

    <!-- 指定映射文件的配置位置 -->
    <mappers>
        <mapper resource="mapper/UserMapper.xml"/>
    </mappers>
</configuration>
```

* 配置第一个Mapper, UserMapper.xml

```xml
<!DOCTYPE mapper
        PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN"
        "http://mybatis.org/dtd/mybatis-3-mapper.dtd">

<mapper namespace="dao">
    <select id="findAllUsers" resultType="dao.User">
        SELECT * FROM t_person
    </select>
</mapper>
```

* UserMapper接口设计

```java
public interface UserMapper {
    List<User> findAllUsers();
}
```

* 封装类型User

```java
public class User implements Serializable {
    private Integer id;
    private String name;
    private Integer age;
    /*getter and setter*/
}
```

* 注解配置运行MyBatis

其本质就是通过注解完成了XXXMapper.xml的功能

```java
public interface UserMapperAnnotation {
    @Select("select * from user")
    List<User> findAllUsers();
}
```

## MyBatis事务配置

MyBatis是通过SqlSession的`commit()`和`rollback()`方法

打开自动事务提交功能(平常开发不推荐使用)

```java
Session sqlSession = factory.openSession(true);
```

## MyBatis ORM问题

场景: 表的列名和需要封装的实体类的属性名称不一致

假定ModifiedUser定义如下:

```java
public class ModifiedUser implements Serializable {
    private Integer userId;
    private String userName;
    private Integer userAge;
    /*getter and setter*/
}
```

```xml
<!-- 设定别名，匹配表的列名和实体类的属性名一致 -->
<select id="findAllModifiedUsers" resultType="dao.ModifiedUser">
        SELECT id as userId, name as userName, age as userAge
        FROM user
</select>

<!-- 或者设置resultMap一次匹配完 -->
<resultMap id="modifiedUserMap" type="dao.ModifiedUser">
        <!-- 主键对应的字段 -->
        <id property="userId" column="id"/>
        <!-- 非主键对应的字段 -->
        <result property="userName" column="name"/>
        <result property="userAge" column="age"/>
</resultMap>
```

### MyBatis标签

* `if`标签

```xml
<select id="findOneUser" parameterType="dao.User" resultType="dao.User">
        SELECT * FROM user  WHERE 1 = 1
        <if test="name != null">
            AND name = #{name}
        </if>
</select>
```

* `where`标签

```xml
<select id="findOneUser" parameterType="dao.User" resultType="dao.User">
        SELECT * FROM user
        <where>
            <if test="name != null">
                AND name = #{name}
            </if>
            <if test="age != null">
                AND age = #{age}
            </if>
        </where>
</select>
```

* `sql`和`include`标签

```xml
<sql id="common-query">
        SELECT * FROM user
</sql>

<select id="findAllUsers" resultType="dao.User">
        <include refid="common-query"></include>
</select>
```

* `foreach`标签

```xml
<select id="findInRange" parameterType="dao.QueryRange" resultType="dao.User">
        <include refid="common-query"></include>
        <where>
            <if test="range != null and range.size() > 0">
                <foreach collection="range" open="and id in (" close=")" item="id" separator=",">
                    #{id}
                </foreach>
            </if>
        </where>
</select>
```

而QueryRange中有一个range集合

```java
public class QueryRange implements Serializable {
    List<Integer> range = new ArrayList<Integer>();
    /*getter and setter*/
}
```

## MyBatis多表查询

* 创建tb_teacher表:

```sql
CREATE TABLE `tb_teacher` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `t_no` varchar(20) DEFAULT NULL,
  `t_name` varchar(20) DEFAULT NULL,
  `position_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;
```

* 创建tb_student表

```sql
CREATE TABLE `tb_student` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `t_stu_name` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8;
```

* 创建tb_position表

```sql
CREATE TABLE `tb_position` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `t_pos_name` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;
```

* 创建教师学生关系表tb_stu_teacher

```sql
CREATE TABLE `tb_stu_teach` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `t_stu_id` int(11) DEFAULT NULL,
  `t_teach_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=utf8;
```

* Position类

```java
@Data
public class Position implements Serializable {
    private int id;
    private String name;
    private Teacher teacher;
}
```

* Teacher类

```java
@Data
public class Student implements Serializable {
    private String id;
    private String name;
    private List<Teacher> list;
}
```

* Student类

```java
@Data
public class Student implements Serializable {
    private String id;
    private String name;
    private List<Teacher> list;
}
```

* **关系表不用实体类表示出来**

### 一对一: 一个老师一个职位

* TeacherMapper.xml

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE mapper PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN" "http://mybatis.org/dtd/mybatis-3-mapper.dtd">
<mapper namespace="com.mapper.TeacherMapper">
	<resultMap type="Teacher" id="teacherPositionResultMap">
		<id property="id" column="id"/>
		<result property="no" column="t_no"/>
		<result property="name" column="t_name"/>
		<!-- association：配置的一对一属性 -->
		<!-- property：名字
			 javaType：类型
		 -->
		<association property="pos" javaType="Position">
			<id property="id" column="id"/>
			<result property="name" column="t_pos_name"/>
		</association>
	</resultMap>
	
	<!-- 一对一关联查询，查询老师及其对应的职位 -->
	<!-- 注意：id不能相同，当多个值传入，比如包装类的时候，我们才能够用SQL片段的形式来做if判断，单个值是不行的 -->
	<select id="queryTeacherPositionResultMapById" resultMap="teacherPositionResultMap" parameterType="Integer">
		SELECT *
		FROM tb_teacher t
		LEFT JOIN tb_position p
		ON t.position_id = p.id
		where t.id = #{id}
	</select>

	<select id="queryTeacherPositionResultMap" resultMap="teacherPositionResultMap">
		SELECT *
		FROM tb_teacher t
		LEFT JOIN tb_position p
		ON t.`position_id` = p.id
	</select> 
</mapper>
```

* TeacherMapper.java

```java
public interface TeacherMapper {
	public List<Teacher> queryTeacherPositionResultMap();
	
	public Teacher queryTeacherPositionResultMapById(Integer id);
}
```

### 一对多: 一个职位对多个老师

* PositionMapper.xml

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE mapper PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN" "http://mybatis.org/dtd/mybatis-3-mapper.dtd">
<mapper namespace="com.mapper.PositionMapper"> 
	<resultMap type="Position" id="positionTeacherResultMap">
		<id property="id" column="id"/>
		<result property="name" column="t_pos_name"/> <!-- t_name -->
		<!-- 
		property同association中的一样是属性名称(javaBean中的)；
		javaType也同association中的是类型，
		最后多了一个OfType，因为一对多，不像一对一是单个的！我们这里是List集合，list和List都可以。
		一对多其中是放的一个集合所以这个是集合的泛型的类型，这里我们的list中放的是Teacher：
		所以这里是Teacher。
		 -->
		<collection property="teacherList" javaType="List" ofType="Teacher" >
			<!-- 
				一对多出现的问题：
					当数据库表中，主表的主键id和明细表的 ...
					当表中的字段名相同时怎么办？多表联查？
					
					注意：Mybatis中做多表联查的时候，不管是
					一对一、一对多、一对多对多：多对多：
					都不能有字段重名的情况：不管是主键还是普通字段。
					一旦字段重名的话，就会造成数据少自动赋值，或者覆盖，甚至重复赋值！
					规避和解决此类问题的方法：
						1.尽量不要表间重名，mybatis里处理起来很麻烦！id和普通字段都是。
						但是在表多的时候，很难不会出现字段重名的情况。主键id最容易重名！
						那么就要用以下的办法了！
						
						2.在mybatis中写原生SQL进行查询的时候，查的字段尽可能的少，这
						也影响速率，强烈禁止使用*，用多少查多少！这样也能及时发现字段重
						名的情况！
						
						3.最后如果真的需要查出重名的字段，并且修改数据库字段名造成的更改
						过大，这里推荐的方式是给字段取别名，在写resultMap映射的时候，其
						中的column属性就填写SQL语句中查出字段取的别名，这样就能解决重复
						问题了！
			 -->
			<id property="id" column="t_id"/>
			<result property="no" column="t_no"/>
			<result property="name" column="t_name"/>	
		</collection>
	</resultMap>
	
	<select id="queryPositionTeacherResultMapById" resultMap="positionTeacherResultMap" 
		parameterType="Integer">
		<!-- 
		SELECT *
		FROM tb_position p
		LEFT JOIN tb_teacher t
		ON p.id = t.position_id
		WHERE p.id = #{id}
		-->
		
		SELECT 
		p.*, 
		t.id t_id,
		t.t_name,
		t.t_no
		FROM tb_position p
		LEFT JOIN tb_teacher t
		ON p.id = t.position_id
		WHERE p.id = #{id}
	</select>
	
	<select id="queryPositionTeacherResultMap" resultMap="positionTeacherResultMap" >
		<!-- 
		SELECT *
		FROM tb_position p
		LEFT JOIN tb_teacher t
		ON p.id = t.position_id
		-->
		
		SELECT 
		p.*, 
		t.id t_id,
		t.t_name,
		t.t_no
		FROM tb_position p
		LEFT JOIN tb_teacher t
		ON p.id = t.position_id
		
	</select>
</mapper>
```

* TeacherMapper.java

```java
public interface PositionMapper {
	public Position queryPositionTeacherResultMapById(Integer id);
	
	public List<Position> queryPositionTeacherResultMap();
}
```

### 一对多对多: 职位是教授的所有老师的学生

* PositionMapper.xml

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE mapper PUBLIC "-//mybatis.org//DTD Mapper 3.0//EN" "http://mybatis.org/dtd/mybatis-3-mapper.dtd">
<mapper namespace="com.mapper.PositionMapper"> 
	<resultMap type="Position" id="positionStudentResultMap">
		<!-- <id property="id" column="id"/> -->
		<result property="name" column="t_pos_name"/>
		<collection property="teacherList" javaType="List" ofType="Teacher" >
			<result property="name" column="t_name"/>	
			<collection property="studentList" javaType="List" ofType="Student">
				<result property="name" column="t_stu_name"/>
			</collection>
		</collection>
	</resultMap>
	
	<select id="selectPositionStudentByPosId" resultMap="positionStudentResultMap" parameterType="Integer">
		SELECT p.t_pos_name, t.t_name, s.t_stu_name
		FROM tb_position p
		INNER JOIN tb_teacher t ON p.id = t.position_id
		LEFT JOIN tb_stu_teach st ON st.t_teach_id = t.id
		LEFT JOIN tb_student s ON s.id = st.t_stu_id
		WHERE p.id = #{id}
	</select>
</mapper>
```
