Hibernate应用实战
===

[TOC]

## 一个Hibernate的基本配置

* jar包的导入(maven管理)

```xml
<dependencies>
        <!-- 添加Hibernate依赖 -->
        <dependency>
            <groupId>org.hibernate</groupId>
            <artifactId>hibernate-core</artifactId>
            <version>3.6.10.Final</version>
        </dependency>

        <dependency>
            <groupId>org.slf4j</groupId>
            <artifactId>slf4j-nop</artifactId>
            <version>1.6.4</version>
        </dependency>
        <!-- 其他配置 -->
    </dependencies>
```

* hibernate.cfg.xml配置

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE hibernate-configuration PUBLIC
        "-//Hibernate/Hibernate Configuration DTD 3.0//EN"
        "http://hibernate.sourceforge.net/hibernate-configuration-3.0.dtd">

<hibernate-configuration>
    <session-factory>
        <property name="hibernate.connection.driver_class">com.mysql.cj.jdbc.Driver</property>
        <property name="hibernate.connection.url">jdbc:mysql://localhost:3306/hibernate?serverTimezone=UTC</property>
        <property name="hibernate.connection.username">root</property>
        <property name="hibernate.connection.password">123456</property>
        <property name="hibernate.dialect">org.hibernate.dialect.MySQLDialect</property>
        <property name="current_session_context_class">thread</property>
        <property name="show_sql">true</property>
        <property name="hbm2ddl.auto">update</property>
        <property name="connection.pool_size">1</property>
        <property name="format_sql">true</property>

        <!-- 实体类的映射 -->
        <mapping resource="Class.hbm.xml"/>
        <mapping resource="Student.hbm.xml"/>
    </session-factory>
</hibernate-configuration>
```

## 一对多关联映射

* 一个t_room表，一个t_person表，表之间的关系如图所示

<div align=center><img src="/assets/otm.png"></div>

* 一个Room可以容纳多个Person

```java
public class Room implements Serializable {
    private Integer roomId;
    private String roomName;
    private Set<Person> persons = new HashSet<Person>();
    /*getter and setter*/
}
```

* 一个Person一次只能在一个Room

```java
public class Person implements Serializable {
    private Integer personId;
    private String personName;
    private Room room;
    /*getter and setter*/
}
```

* Person.hbm.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE hibernate-mapping PUBLIC "-//Hibernate/Hibernate Mapping DTD 3.0//EN"
        "http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd">
<hibernate-mapping>
    <class name="Person" table="t_person">
        <id name="personId" column="person_id">
            <generator class="native"/>
        </id>
        <property name="personName" column="person_name"/>
        <many-to-one name="room" class="Room" column="room_id"/>
    </class>
</hibernate-mapping>
```

* Room.hbm.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE hibernate-mapping PUBLIC "-//Hibernate/Hibernate Mapping DTD 3.0//EN"
        "http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd">

<hibernate-mapping>
    <class name="Room" table="t_room">
        <id name="roomId" column="room_id">
            <generator class="native"/>
        </id>
        <property name="roomName" column="room_name"/>
        <set name="persons">
            <key column="room_id"/>
            <one-to-many class="Person"/>
        </set>
    </class>
</hibernate-mapping>
```

**一对多的级联操作的时候，为了照顾性能，一般将一的一方放弃外键维护，因为hibernate是双向维护外键**

## 多对多关联映射

* 一个t_role表，一个t_user表

<div align=center><img src="/assets/mtm.png"></div>

* 一个User可能会有多个Role

```java
public class User implements Serializable {
    private Integer user_id;
    private String user_name;
    private String user_password;
    private Set<Role> setRole = new HashSet<Role>();
    /*getter and setter*/
}
```

* 一个Role可能会有多个User

```java
public class Role implements Serializable {
    private Integer role_id;
    private String role_name;
    private String role_memo;
    private Set<User> setUser = new HashSet<User>();
    /*getter and setter*/
}
```

* User.hbm.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE hibernate-mapping PUBLIC "-//Hibernate/Hibernate Mapping DTD 3.0//EN"
        "http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd">

<hibernate-mapping>
    <class name="manytomany.User" table="t_user">
        <id name="user_id" column="user_id">
            <generator class="native"/>
        </id>
        <property name="user_name" column="user_name"/>
        <property name="user_password" column="user_password"/>
        <set name="setRole" table="user_role" >
            <key column="user_id"/>
            <many-to-many class="manytomany.Role" column="role_id"/>
        </set>
    </class>
</hibernate-mapping>
```

* Role.hbm.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE hibernate-mapping PUBLIC "-//Hibernate/Hibernate Mapping DTD 3.0//EN"
        "http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd">

<hibernate-mapping>
    <class name="manytomany.Role" table="t_role">
        <id name="role_id" column="role_id">
            <generator class="native"/>
        </id>
        <property name="role_name" column="role_name"/>
        <property name="role_memo" column="role_memo"/>
        <set name="setUser" table="user_role" >
            <key column="role_id"/>
            <many-to-many class="manytomany.User" column="user_id"/>
        </set>
    </class>
</hibernate-mapping>
```

### 多对多的级联删除

上述配置的方法无法处理级联形式的插入和删除，必须配置`<cascade>`，以Role.hbm.xml为例

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE hibernate-mapping PUBLIC "-//Hibernate/Hibernate Mapping DTD 3.0//EN"
        "http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd">

<hibernate-mapping>
    <class name="manytomany.Role" table="t_role">
        <id name="role_id" column="role_id">
            <generator class="native"/>
        </id>
        <property name="role_name" column="role_name"/>
        <property name="role_memo" column="role_memo"/>
        <set name="setUser" table="user_role" cascade="all"> <!-- 配置级联权限 -->
            <key column="role_id"/>
            <many-to-many class="manytomany.User" column="user_id"/>
        </set>
    </class>
</hibernate-mapping>
```

### 多对多的维护是通过第三张表进行的

假定Role和User是多对多的关系，假定有个User叫lucy,有个Role叫老师，为了让lucy具有老师的角色，利用第三张表user_role中来维护，具体hibernate操作为:

```java
User lucy = (User) session.get(User.class, lucy_id);
Role teacher = (Role) session.get(Role.class, teacher_id);

lucy.getRoleSet().add(teacher);
```

## 参考链接

* [JeGe博客](https://www.jianshu.com/u/d54cf1707fd8)
