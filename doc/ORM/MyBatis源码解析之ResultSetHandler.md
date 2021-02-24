# MyBatis源码解析——ResultSetHandler

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_ResultSetWrapper_](#ResultSetWrapper_)
	* 2.1. [_#getTypeHandler_](#getTypeHandler_)
	* 2.2. [_#getJdbcType_](#getJdbcType_)
	* 2.3. [_loadMappedAndUnmappedColumnNames_](#loadMappedAndUnmappedColumnNames_)
* 3. [剩下的写不动了。。。。下次一定](#-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

SQL在执行完成之后，即`java.sql.Statement`返回ResultSet时，需要由ResultSetHandler来完成对响应的结果集ResultSet的处理。ResultSetHandler的UML类图如下图所示:

<div align=center><img src="/assets/mb6.png"/></div>

其中核心类是 ResultSetHandler 接口及其实现类 DefaultResultSetHandler 。在它的代码逻辑中，会调用类图中的其它类，实现将查询结果的 ResultSet ，转换成映射的对应结果。

##  2. <a name='ResultSetWrapper_'></a>_ResultSetWrapper_

`org.apache.ibatis.executor.resultset.ResultSetWrapper` ，`java.sql.ResultSet` 的 包装器，可以理解成 ResultSet 的工具类，提供给 DefaultResultSetHandler 使用。

```java
/**
 * ResultSet 对象
 */
private final ResultSet resultSet;
private final TypeHandlerRegistry typeHandlerRegistry;
/**
 * 字段的名字的数组
 */
private final List<String> columnNames = new ArrayList<>();
/**
 * 字段的 Java Type 的数组
 */
private final List<String> classNames = new ArrayList<>();
/**
 * 字段的 JdbcType 的数组
 */
private final List<JdbcType> jdbcTypes = new ArrayList<>();
private final Map<String, Map<Class<?>, TypeHandler<?>>> typeHandlerMap = new HashMap<>();
private final Map<String, List<String>> mappedColumnNamesMap = new HashMap<>();
private final Map<String, List<String>> unMappedColumnNamesMap = new HashMap<>();

public ResultSetWrapper(ResultSet rs, Configuration configuration) throws SQLException {
    this.typeHandlerRegistry = configuration.getTypeHandlerRegistry();
    this.resultSet = rs;
    // <1> 遍历 ResultSetMetaData 的字段们，解析出 columnNames、jdbcTypes、classNames 属性
    final ResultSetMetaData metaData = rs.getMetaData();
    final int columnCount = metaData.getColumnCount();
    for (int i = 1; i <= columnCount; i++) {
        columnNames.add(configuration.isUseColumnLabel() ? metaData.getColumnLabel(i) : metaData.getColumnName(i));
        jdbcTypes.add(JdbcType.forCode(metaData.getColumnType(i)));
        classNames.add(metaData.getColumnClassName(i));
    }
}
```

###  2.1. <a name='getTypeHandler_'></a>_#getTypeHandler_

```java
/**
 * TypeHandler 的映射
 *
 * KEY1：字段的名字
 * KEY2：Java 属性类型
 */
private final Map<String, Map<Class<?>, TypeHandler<?>>> typeHandlerMap = new HashMap<>();

/**
 *
 * 获得指定字段名的指定 JavaType 类型的 TypeHandler 对象
 *
 * @param propertyType JavaType
 * @param columnName 执行字段
 * @return TypeHandler 对象
 */
public TypeHandler<?> getTypeHandler(Class<?> propertyType, String columnName) {
    TypeHandler<?> handler = null;
    // <1> 先从缓存的 typeHandlerMap 中，获得指定字段名的指定 JavaType 类型的 TypeHandler 对象
    Map<Class<?>, TypeHandler<?>> columnHandlers = typeHandlerMap.get(columnName);
    if (columnHandlers == null) {
        columnHandlers = new HashMap<>();
        typeHandlerMap.put(columnName, columnHandlers);
    } else {
        handler = columnHandlers.get(propertyType);
    }
    // <2> 如果获取不到，则进行查找
    if (handler == null) {
        // <2> 获得 JdbcType 类型
        JdbcType jdbcType = getJdbcType(columnName);
        // <2> 获得 TypeHandler 对象
        handler = typeHandlerRegistry.getTypeHandler(propertyType, jdbcType);
        // <3> 如果获取不到，则再次进行查找
        if (handler == null || handler instanceof UnknownTypeHandler) {
            // <3.1> 使用 classNames 中的类型，进行继续查找 TypeHandler 对象
            final int index = columnNames.indexOf(columnName);
            final Class<?> javaType = resolveClass(classNames.get(index));
            if (javaType != null && jdbcType != null) {
                handler = typeHandlerRegistry.getTypeHandler(javaType, jdbcType);
            } else if (javaType != null) {
                handler = typeHandlerRegistry.getTypeHandler(javaType);
            } else if (jdbcType != null) {
                handler = typeHandlerRegistry.getTypeHandler(jdbcType);
            }
        }
        // <4> 如果获取不到，则使用 ObjectTypeHandler 对象
        if (handler == null || handler instanceof UnknownTypeHandler) {
            handler = new ObjectTypeHandler();
        }
        // <5> 缓存到 typeHandlerMap 中
        columnHandlers.put(propertyType, handler);
    }
    return handler;
}
```

###  2.2. <a name='getJdbcType_'></a>_#getJdbcType_

```java
public JdbcType getJdbcType(String columnName) {
    for (int i = 0; i < columnNames.size(); i++) {
        if (columnNames.get(i).equalsIgnoreCase(columnName)) {
            return jdbcTypes.get(i);
        }
    }
    return null;
}
```

###  2.3. <a name='loadMappedAndUnmappedColumnNames_'></a>_loadMappedAndUnmappedColumnNames_

初始化**有 mapped** 和**无 mapped**的字段的名字数组。

```java
/**
 * 有 mapped 的字段的名字的映射
 *
 * KEY：getMapKey(ResultMap, String)
 * VALUE：字段的名字的数组
 */
private final Map<String, List<String>> mappedColumnNamesMap = new HashMap<>();
/**
 * 无 mapped 的字段的名字的映射
 *
 * 和 {@link #mappedColumnNamesMap} 相反
 */
private final Map<String, List<String>> unMappedColumnNamesMap = new HashMap<>();

private void loadMappedAndUnmappedColumnNames(ResultMap resultMap, String columnPrefix) throws SQLException {
    List<String> mappedColumnNames = new ArrayList<>();
    List<String> unmappedColumnNames = new ArrayList<>();
    // <1> 将 columnPrefix 转换成大写，并拼接到 resultMap.mappedColumns 属性上
    final String upperColumnPrefix = columnPrefix == null ? null : columnPrefix.toUpperCase(Locale.ENGLISH);
    final Set<String> mappedColumns = prependPrefixes(resultMap.getMappedColumns(), upperColumnPrefix);
    // <2> 遍历 columnNames 数组，根据是否在 mappedColumns 中，分别添加到 mappedColumnNames 和 unmappedColumnNames 中
    for (String columnName : columnNames) {
        final String upperColumnName = columnName.toUpperCase(Locale.ENGLISH);
        if (mappedColumns.contains(upperColumnName)) {
            mappedColumnNames.add(upperColumnName);
        } else {
            unmappedColumnNames.add(columnName);
        }
    }
    // <3> 将 mappedColumnNames 和 unmappedColumnNames 结果，添加到 mappedColumnNamesMap 和 unMappedColumnNamesMap 中
    mappedColumnNamesMap.put(getMapKey(resultMap, columnPrefix), mappedColumnNames);
    unMappedColumnNamesMap.put(getMapKey(resultMap, columnPrefix), unmappedColumnNames);
}
```

##  3. <a name='-1'></a>剩下的写不动了。。。。下次一定



