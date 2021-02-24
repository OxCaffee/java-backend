# MyBatis源码解析——StatementHandler

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_StatementHandler_](#StatementHandler_)
* 3. [_BaseStatementHandler_](#BaseStatementHandler_)
	* 3.1. [_#prepare_](#prepare_)
* 4. [_SimpleStatementHandler_](#SimpleStatementHandler_)
	* 4.1. [_#instantiateStatement_](#instantiateStatement_)
	* 4.2. [_#parameterize_](#parameterize_)
	* 4.3. [_#query_](#query_)
	* 4.4. [_#batch_](#batch_)
	* 4.5. [_#update_](#update_)
* 5. [_PreparedStatementHandler_](#PreparedStatementHandler_)
	* 5.1. [_#instantiateStatement_](#instantiateStatement_-1)
	* 5.2. [_#parameterize_](#parameterize_-1)
	* 5.3. [_#query_](#query_-1)
	* 5.4. [_#queryCursor_](#queryCursor_)
	* 5.5. [_#batch_](#batch_-1)
	* 5.6. [_#update_](#update_-1)
* 6. [_CallableStatementHandler_](#CallableStatementHandler_)
* 7. [_RoutingStatementHandler_](#RoutingStatementHandler_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

在[MyBatis源码解析——Executor执行器](/doc/ORM/MyBatis源码解析之Executor.md)中我们对StatementHandler有了初步的了解，Executor负责管理缓存，接收SQL操作请求，之后会把操作请求发给StatementHandler来完成，StatementHandler在将操作结果返还给Executor进行之后的操作。

StatementHandler的实现子类大致有4种:

* SimpleStatementHandler
* PreparedStatementHandler
* CallableStatementHandler
* RoutingStatementHandler

类UML图如下:

<div align=center><img src="/assets/mb4.png/"></div>



或者从一个SQL语句的执行过程我们可以看到StatementHandler的作用:

<div align=center><img src="/assets/mb5.png"/></div>

下面开始我们的源码探究之旅。

##  2. <a name='StatementHandler_'></a>_StatementHandler_

```java
/**
 * 语句处理器
 * 
 */
public interface StatementHandler {

  //准备语句
  Statement prepare(Connection connection)
      throws SQLException;

  //参数化
  void parameterize(Statement statement)
      throws SQLException;

  //批处理
  void batch(Statement statement)
      throws SQLException;

  //update
  int update(Statement statement)
      throws SQLException;

  //select-->结果给ResultHandler
  <E> List<E> query(Statement statement, ResultHandler resultHandler)
      throws SQLException;

  //得到绑定sql
  BoundSql getBoundSql();

  //得到参数处理器
  ParameterHandler getParameterHandler();

}
```

##  3. <a name='BaseStatementHandler_'></a>_BaseStatementHandler_

```java
public abstract class BaseStatementHandler implements StatementHandler {
    protected final Configuration configuration;
    protected final ObjectFactory objectFactory;
    protected final TypeHandlerRegistry typeHandlerRegistry;
    protected final ResultSetHandler resultSetHandler;
    protected final ParameterHandler parameterHandler;

    protected final Executor executor;
    protected final MappedStatement mappedStatement;
    protected final RowBounds rowBounds;

    protected BoundSql boundSql;

    protected BaseStatementHandler(Executor executor, MappedStatement mappedStatement, Object parameterObject, RowBounds rowBounds, ResultHandler resultHandler, BoundSql boundSql) {
        // 获得 Configuration 对象
        this.configuration = mappedStatement.getConfiguration();

        this.executor = executor;
        this.mappedStatement = mappedStatement;
        this.rowBounds = rowBounds;

        // 获得 TypeHandlerRegistry 和 ObjectFactory 对象
        this.typeHandlerRegistry = configuration.getTypeHandlerRegistry();
        this.objectFactory = configuration.getObjectFactory();

        // <1> 如果 boundSql 为空，一般是写类操作，例如：insert、update、delete ，则先获得自增主键，然后再创建 BoundSql 对象
        if (boundSql == null) { // issue #435, get the key before calculating the statement
            // <1.1> 获得自增主键
            generateKeys(parameterObject);
            // <1.2> 创建 BoundSql 对象
            boundSql = mappedStatement.getBoundSql(parameterObject);
        }
        this.boundSql = boundSql;

        // <2> 创建 ParameterHandler 对象
        this.parameterHandler = configuration.newParameterHandler(mappedStatement, parameterObject, boundSql);
        // <3> 创建 ResultSetHandler 对象
        this.resultSetHandler = configuration.newResultSetHandler(executor, mappedStatement, rowBounds, parameterHandler, resultHandler, boundSql);
    }
}
```

###  3.1. <a name='prepare_'></a>_#prepare_

```java
@Override
public Statement prepare(Connection connection, Integer transactionTimeout) throws SQLException {
    ErrorContext.instance().sql(boundSql.getSql());
    Statement statement = null;
    try {
        // <1> 创建 Statement 对象
        statement = instantiateStatement(connection);
        // 设置超时时间
        setStatementTimeout(statement, transactionTimeout);
        // 设置 fetchSize
        setFetchSize(statement);
        return statement;
    } catch (SQLException e) {
        // 发生异常，进行关闭
        closeStatement(statement);
        throw e;
    } catch (Exception e) {
        // 发生异常，进行关闭
        closeStatement(statement);
        throw new ExecutorException("Error preparing statement.  Cause: " + e, e);
    }
}
```

##  4. <a name='SimpleStatementHandler_'></a>_SimpleStatementHandler_

###  4.1. <a name='instantiateStatement_'></a>_#instantiateStatement_

```java
@Override
protected Statement instantiateStatement(Connection connection) throws SQLException {
    if (mappedStatement.getResultSetType() == ResultSetType.DEFAULT) {
        return connection.createStatement();
    } else {
        return connection.createStatement(mappedStatement.getResultSetType().getValue(), ResultSet.CONCUR_READ_ONLY);
    }
}
```

###  4.2. <a name='parameterize_'></a>_#parameterize_

```java
@Override
public void parameterize(Statement statement) throws SQLException {
    // N/A, 空，因为无需做占位符参数的处理。
}
```

###  4.3. <a name='query_'></a>_#query_

```java
  //select-->结果给ResultHandler
  @Override
  public <E> List<E> query(Statement statement, ResultHandler resultHandler) throws SQLException {
    String sql = boundSql.getSql();
    statement.execute(sql);
    //先执行Statement.execute，然后交给ResultSetHandler.handleResultSets
    return resultSetHandler.<E>handleResultSets(statement);
  }
```

###  4.4. <a name='batch_'></a>_#batch_

```java
  @Override
  public void batch(Statement statement) throws SQLException {
    String sql = boundSql.getSql();
    //调用Statement.addBatch
    statement.addBatch(sql);
  }
```

###  4.5. <a name='update_'></a>_#update_

```java
@Override
public int update(Statement statement) throws SQLException {
    String sql = boundSql.getSql();
    Object parameterObject = boundSql.getParameterObject();
    KeyGenerator keyGenerator = mappedStatement.getKeyGenerator();
    int rows;
    // 如果是 Jdbc3KeyGenerator 类型
    if (keyGenerator instanceof Jdbc3KeyGenerator) {
        // <1.1> 执行写操作
        statement.execute(sql, Statement.RETURN_GENERATED_KEYS);
        // <2.2> 获得更新数量
        rows = statement.getUpdateCount();
        // <1.3> 执行 keyGenerator 的后置处理逻辑
        keyGenerator.processAfter(executor, mappedStatement, statement, parameterObject);
    // 如果是 SelectKeyGenerator 类型
    } else if (keyGenerator instanceof SelectKeyGenerator) {
        // <2.1> 执行写操作
        statement.execute(sql);
        // <2.2> 获得更新数量
        rows = statement.getUpdateCount();
        // <2.3> 执行 keyGenerator 的后置处理逻辑
        keyGenerator.processAfter(executor, mappedStatement, statement, parameterObject);
    } else {
        // <3.1> 执行写操作
        statement.execute(sql);
        // <3.2> 获得更新数量
        rows = statement.getUpdateCount();
    }
    return rows;
}
```

##  5. <a name='PreparedStatementHandler_'></a>_PreparedStatementHandler_

###  5.1. <a name='instantiateStatement_-1'></a>_#instantiateStatement_

```java
@Override
protected Statement instantiateStatement(Connection connection) throws SQLException {
    String sql = boundSql.getSql();
    // <1> 处理 Jdbc3KeyGenerator 的情况
    if (mappedStatement.getKeyGenerator() instanceof Jdbc3KeyGenerator) {
        String[] keyColumnNames = mappedStatement.getKeyColumns();
        if (keyColumnNames == null) {
            return connection.prepareStatement(sql, PreparedStatement.RETURN_GENERATED_KEYS);
        } else {
            return connection.prepareStatement(sql, keyColumnNames);
        }
    // <2>和SimpleStatementHandler一样
    } else if (mappedStatement.getResultSetType() == ResultSetType.DEFAULT) {
        return connection.prepareStatement(sql);
    // <3>和SimpleStatementHandler一样 
    } else {
        return connection.prepareStatement(sql, mappedStatement.getResultSetType().getValue(), ResultSet.CONCUR_READ_ONLY);
    }
}
```

###  5.2. <a name='parameterize_-1'></a>_#parameterize_

```java
@Override
public void parameterize(Statement statement) throws SQLException {
    parameterHandler.setParameters((PreparedStatement) statement);//因为PreparedStatement使用到了占位符
}
```

###  5.3. <a name='query_-1'></a>_#query_

```java
@Override
public <E> List<E> query(Statement statement, ResultHandler resultHandler) throws SQLException {
    PreparedStatement ps = (PreparedStatement) statement;
    // 执行查询
    ps.execute();
    // 处理返回结果
    return resultSetHandler.handleResultSets(ps);
}
```

###  5.4. <a name='queryCursor_'></a>_#queryCursor_

```java
@Override
public <E> Cursor<E> queryCursor(Statement statement) throws SQLException {
    PreparedStatement ps = (PreparedStatement) statement;
    // 执行查询
    ps.execute();
    // 处理返回的 Cursor 结果
    return resultSetHandler.handleCursorResultSets(ps);
}
```

###  5.5. <a name='batch_-1'></a>_#batch_

```java
@Override
public void batch(Statement statement) throws SQLException {
    PreparedStatement ps = (PreparedStatement) statement;
    // 添加到批处理
    ps.addBatch();
}
```

###  5.6. <a name='update_-1'></a>_#update_

```java
@Override
public int update(Statement statement) throws SQLException {
    PreparedStatement ps = (PreparedStatement) statement;
    // 执行写操作
    ps.execute();
    int rows = ps.getUpdateCount();
    // 获得更新数量
    Object parameterObject = boundSql.getParameterObject();
    // 执行 keyGenerator 的后置处理逻辑
    KeyGenerator keyGenerator = mappedStatement.getKeyGenerator();
    keyGenerator.processAfter(executor, mappedStatement, ps, parameterObject);
    return rows;
}
```

##  6. <a name='CallableStatementHandler_'></a>_CallableStatementHandler_

待续......

##  7. <a name='RoutingStatementHandler_'></a>_RoutingStatementHandler_

本质是个自适应Handler，根据StatementType的不同，创建不同的StatementHandler

```java
/**
 * 被委托的 StatementHandler 对象
 */
private final StatementHandler delegate;

public RoutingStatementHandler(Executor executor, MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler, BoundSql boundSql) {
    // 根据不同的类型，创建对应的 StatementHandler 实现类
    switch (ms.getStatementType()) {
        case STATEMENT:
            delegate = new SimpleStatementHandler(executor, ms, parameter, rowBounds, resultHandler, boundSql);
            break;
        case PREPARED:
            delegate = new PreparedStatementHandler(executor, ms, parameter, rowBounds, resultHandler, boundSql);
            break;
        case CALLABLE:
            delegate = new CallableStatementHandler(executor, ms, parameter, rowBounds, resultHandler, boundSql);
            break;
        default:
            throw new ExecutorException("Unknown statement type: " + ms.getStatementType());
    }
}
```

