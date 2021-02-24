# MyBatis源码解析——Executor

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_Executor_](#Executor_)
* 3. [_BaseExecutor_](#BaseExecutor_)
	* 3.1. [_#clearLocalCache_](#clearLocalCache_)
	* 3.2. [_#createCacheKey_](#createCacheKey_)
	* 3.3. [_#query_(不带CacheKey)](#query_CacheKey)
	* 3.4. [_#query_(带CacheKey)](#query_CacheKey-1)
	* 3.5. [_#queryFromDatabase_](#queryFromDatabase_)
	* 3.6. [_#doQuery_](#doQuery_)
	* 3.7. [_#deferLoad_](#deferLoad_)
	* 3.8. [_#update_](#update_)
	* 3.9. [_#doUpdate_](#doUpdate_)
	* 3.10. [_#commit_](#commit_)
	* 3.11. [_#rollback_](#rollback_)
* 4. [_SimpleExecutor_](#SimpleExecutor_)
	* 4.1. [_#doQuery_](#doQuery_-1)
	* 4.2. [_#doUpdate_](#doUpdate_-1)
	* 4.3. [_#doFlushStatements_](#doFlushStatements_)
* 5. [_ReuseExecutor_](#ReuseExecutor_)
	* 5.1. [_#doQuery_](#doQuery_-1)
	* 5.2. [_#doUpdate_](#doUpdate_-1)
	* 5.3. [_#doFlushStatements_](#doFlushStatements_-1)
	* 5.4. [_#prepareStatement_(Reuse体现的地方)](#prepareStatement_Reuse)
* 6. [_BatchExecutor_(重要)](#BatchExecutor_)
	* 6.1. [_#doQuery_](#doQuery_-1)
	* 6.2. [_#doUpdate_](#doUpdate_-1)
	* 6.3. [_#doFlushStatements_](#doFlushStatements_-1)
* 7. [_CachingExecutor_(待续.......)](#CachingExecutor_.......)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

在MyBatis中，SQL语句的执行涉及到多个组件，例如比较重要的Executor, StatementHandler, ParameterHandler, ResultSetHandler。

* Executor主要维护一级缓存和二级缓存，并提供事务管理的相关的操作，它会将数据库相关的操作交给StatementHandler完成
* StatementHandler首先通过ParameterHandler完成对SQL的实参绑定，然后通过`java.sql.Statement`对象执行SQL语句并得到结果集
* Statement获得的结果集会交给ResultHandler进行结果集的映射处理，得到相应的对象并返回

<div align=center><img src="/assets/mb2.png"/></div>

大概了解了上面的流程，下面就可以开始我们的源码探索之旅了。

##  2. <a name='Executor_'></a>_Executor_

```java
public interface Executor {

  //不需要ResultHandler
  ResultHandler NO_RESULT_HANDLER = null;

  //更新
  int update(MappedStatement ms, Object parameter) throws SQLException;

  //查询，带分页，带缓存，BoundSql
  <E> List<E> query(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler, CacheKey cacheKey, BoundSql boundSql) throws SQLException;

  //查询，带分页
  <E> List<E> query(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler) throws SQLException;

  //刷新批处理语句
  List<BatchResult> flushStatements() throws SQLException;

  //提交和回滚，参数是是否要强制
  void commit(boolean required) throws SQLException;

  void rollback(boolean required) throws SQLException;

  //创建CacheKey
  CacheKey createCacheKey(MappedStatement ms, Object parameterObject, RowBounds rowBounds, BoundSql boundSql);

  //判断是否缓存了
  boolean isCached(MappedStatement ms, CacheKey key);

  //清理Session缓存
  void clearLocalCache();

  //延迟加载
  void deferLoad(MappedStatement ms, MetaObject resultObject, String property, CacheKey key, Class<?> targetType);

  Transaction getTransaction();

  void close(boolean forceRollback);

  boolean isClosed();

  void setExecutorWrapper(Executor executor);

}
```

Executor的实现类如下：

<div align=center><img src="/assets/mb3.png"/></div>

上述除了CachingExecutor之外，其余的Executor实现类均只具备一级缓存功能，CachingExecutor开启了二级缓存功能， **其中BaseExecutor的本地缓存，就是一级缓存，并且需要注意的是，一级缓存是默认开启且无法关闭的** 。

##  3. <a name='BaseExecutor_'></a>_BaseExecutor_

BaseExecutor是所有Executor的基类，实现了最基础的功能，首先我们先来看看BaseExecutor的成员属性及构造函数:

```java
public abstract class BaseExecutor implements Executor {

  private static final Log log = LogFactory.getLog(BaseExecutor.class);

  protected Transaction transaction;
  protected Executor wrapper;

  //延迟加载队列（线程安全）
  protected ConcurrentLinkedQueue<DeferredLoad> deferredLoads;
  //本地缓存机制（Local Cache）防止循环引用（circular references）和加速重复嵌套查询(一级缓存)
  //本地缓存
  protected PerpetualCache localCache;
  //本地输出参数缓存
  protected PerpetualCache localOutputParameterCache;
  protected Configuration configuration;

  //查询堆栈
  protected int queryStack = 0;
  private boolean closed;

  protected BaseExecutor(Configuration configuration, Transaction transaction) {
    this.transaction = transaction;
    this.deferredLoads = new ConcurrentLinkedQueue<DeferredLoad>();
    this.localCache = new PerpetualCache("LocalCache");
    this.localOutputParameterCache = new PerpetualCache("LocalOutputParameterCache");
    this.closed = false;
    this.configuration = configuration;
    this.wrapper = this;
  }
  //........
}
```

###  3.1. <a name='clearLocalCache_'></a>_#clearLocalCache_

```java
  @Override
  public void clearLocalCache() {
    if (!closed) {
      //一级缓存清空
      localCache.clear();
      //输出缓存清空
      localOutputParameterCache.clear();
    }
  }
```

###  3.2. <a name='createCacheKey_'></a>_#createCacheKey_

```java
//创建缓存Key
  @Override
  public CacheKey createCacheKey(MappedStatement ms, Object parameterObject, RowBounds rowBounds, BoundSql boundSql) {
    if (closed) {
      throw new ExecutorException("Executor was closed.");
    }
    CacheKey cacheKey = new CacheKey();
    //MyBatis 对于其 Key 的生成采取规则为：[mappedStementId + offset + limit + SQL + queryParams + environment]生成一个哈希码
    cacheKey.update(ms.getId());
    cacheKey.update(Integer.valueOf(rowBounds.getOffset()));
    cacheKey.update(Integer.valueOf(rowBounds.getLimit()));
    cacheKey.update(boundSql.getSql());
    List<ParameterMapping> parameterMappings = boundSql.getParameterMappings();
    TypeHandlerRegistry typeHandlerRegistry = ms.getConfiguration().getTypeHandlerRegistry();
    // mimic DefaultParameterHandler logic
    //模仿DefaultParameterHandler的逻辑,不再重复，请参考DefaultParameterHandler
    for (int i = 0; i < parameterMappings.size(); i++) {
      ParameterMapping parameterMapping = parameterMappings.get(i);
      if (parameterMapping.getMode() != ParameterMode.OUT) {
        Object value;
        String propertyName = parameterMapping.getProperty();
        if (boundSql.hasAdditionalParameter(propertyName)) {
          value = boundSql.getAdditionalParameter(propertyName);
        } else if (parameterObject == null) {
          value = null;
        } else if (typeHandlerRegistry.hasTypeHandler(parameterObject.getClass())) {
          value = parameterObject;
        } else {
          MetaObject metaObject = configuration.newMetaObject(parameterObject);
          value = metaObject.getValue(propertyName);
        }
        cacheKey.update(value);
      }
    }
    if (configuration.getEnvironment() != null) {
      // issue #176
      cacheKey.update(configuration.getEnvironment().getId());
    }
    return cacheKey;
  }    
```

###  3.3. <a name='query_CacheKey'></a>_#query_(不带CacheKey)

```java
  //SqlSession.selectList会调用此方法
  @Override
  public <E> List<E> query(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler) throws SQLException {
    //得到绑定sql
    BoundSql boundSql = ms.getBoundSql(parameter);
    //创建缓存Key
    CacheKey key = createCacheKey(ms, parameter, rowBounds, boundSql);
    //查询
    return query(ms, parameter, rowBounds, resultHandler, key, boundSql);
 }
```

可以看到不带CacheKey的query方法调用了带CacheKey的query方法，在调用之前生成了一个CacheKey

###  3.4. <a name='query_CacheKey-1'></a>_#query_(带CacheKey)

```java
  @SuppressWarnings("unchecked")
  @Override
  public <E> List<E> query(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler, CacheKey key, BoundSql boundSql) throws SQLException {
    ErrorContext.instance().resource(ms.getResource()).activity("executing a query").object(ms.getId());
    //如果已经关闭，报错
    if (closed) {
      throw new ExecutorException("Executor was closed.");
    }
    //先清局部缓存，再查询.但仅查询堆栈为0，才清。为了处理递归调用
    if (queryStack == 0 && ms.isFlushCacheRequired()) {
      clearLocalCache();
    }
    List<E> list;
    try {
      //加一,这样递归调用到上面的时候就不会再清局部缓存了
      queryStack++;
      //先根据cachekey从localCache去查
      list = resultHandler == null ? (List<E>) localCache.getObject(key) : null;
      if (list != null) {
        //若查到localCache缓存，处理localOutputParameterCache
        handleLocallyCachedOutputParameters(ms, key, parameter, boundSql);
      } else {
        //从数据库查
        list = queryFromDatabase(ms, parameter, rowBounds, resultHandler, key, boundSql);
      }
    } finally {
      //清空堆栈
      queryStack--;
    }
    if (queryStack == 0) {
      //延迟加载队列中所有元素
      for (DeferredLoad deferredLoad : deferredLoads) {
        deferredLoad.load();
      }
      // issue #601
      //清空延迟加载队列
      deferredLoads.clear();
      if (configuration.getLocalCacheScope() == LocalCacheScope.STATEMENT) {
        // issue #482
    	//如果是STATEMENT，清本地缓存
        clearLocalCache();
      }
    }
    return list;
  }
```

###  3.5. <a name='queryFromDatabase_'></a>_#queryFromDatabase_

```java
  //从数据库查
  private <E> List<E> queryFromDatabase(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler, CacheKey key, BoundSql boundSql) throws SQLException {
    List<E> list;
    //先向缓存中放入占位符？？？
    localCache.putObject(key, EXECUTION_PLACEHOLDER);
    try {
      list = doQuery(ms, parameter, rowBounds, resultHandler, boundSql); //!!!!!!!!!!!!!!!!!!!!!!!!!!!
    } finally {
      //最后删除占位符
      localCache.removeObject(key);
    }
    //加入缓存
    localCache.putObject(key, list);
    //如果是存储过程，OUT参数也加入缓存
    if (ms.getStatementType() == StatementType.CALLABLE) {
      localOutputParameterCache.putObject(key, parameter);
    }
    return list;
  }
```

###  3.6. <a name='doQuery_'></a>_#doQuery_

由子类实现

```java
//query-->queryFromDatabase-->doQuery
  protected abstract <E> List<E> doQuery(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler, BoundSql boundSql)
      throws SQLException;
```

###  3.7. <a name='deferLoad_'></a>_#deferLoad_

```java
  //延迟加载，DefaultResultSetHandler.getNestedQueryMappingValue调用.属于嵌套查询，比较高级.
  @Override
  public void deferLoad(MappedStatement ms, MetaObject resultObject, String property, CacheKey key, Class<?> targetType) {
    if (closed) {
      throw new ExecutorException("Executor was closed.");
    }
    DeferredLoad deferredLoad = new DeferredLoad(resultObject, property, key, localCache, configuration, targetType);
    //如果能加载，则立刻加载，否则加入到延迟加载队列中
    if (deferredLoad.canLoad()) {
      deferredLoad.load();
    } else {
      //这里怎么又new了一个新的，性能有点问题
      deferredLoads.add(new DeferredLoad(resultObject, property, key, localCache, configuration, targetType));
    }
  }
```

而DeferLoad是BaseExecutor的内部类，源码如下:

```java
  //延迟加载
  private static class DeferredLoad {

    private final MetaObject resultObject;
    private final String property;
    private final Class<?> targetType;
    private final CacheKey key;
    private final PerpetualCache localCache;
    private final ObjectFactory objectFactory;
    private final ResultExtractor resultExtractor;

    // issue #781
    public DeferredLoad(MetaObject resultObject,
                        String property,
                        CacheKey key,
                        PerpetualCache localCache,
                        Configuration configuration,
                        Class<?> targetType) {
      this.resultObject = resultObject;
      this.property = property;
      this.key = key;
      this.localCache = localCache;
      this.objectFactory = configuration.getObjectFactory();
      this.resultExtractor = new ResultExtractor(configuration, objectFactory);
      this.targetType = targetType;
    }

    public boolean canLoad() {
      //缓存中找到，且不为占位符，代表可以加载
      return localCache.getObject(key) != null && localCache.getObject(key) != EXECUTION_PLACEHOLDER;
    }

	//加载
    public void load() {
      @SuppressWarnings( "unchecked" )
      // we suppose we get back a List
      List<Object> list = (List<Object>) localCache.getObject(key);
      //调用ResultExtractor.extractObjectFromList
      Object value = resultExtractor.extractObjectFromList(list, targetType);
      resultObject.setValue(property, value);
    }

  }
```

###  3.8. <a name='update_'></a>_#update_

```java
  //SqlSession.update/insert/delete会调用此方法
  @Override
  public int update(MappedStatement ms, Object parameter) throws SQLException {
    ErrorContext.instance().resource(ms.getResource()).activity("executing an update").object(ms.getId());
    if (closed) {
      throw new ExecutorException("Executor was closed.");
    }
    //先清局部缓存，再更新，如何更新交由子类，模板方法模式
    clearLocalCache();
    return doUpdate(ms, parameter);
  }
```

###  3.9. <a name='doUpdate_'></a>_#doUpdate_

```java
  protected abstract int doUpdate(MappedStatement ms, Object parameter)
      throws SQLException;
```

同样是由子类实现

###  3.10. <a name='commit_'></a>_#commit_

```java
  @Override
  public void commit(boolean required) throws SQLException {
    if (closed) {
      throw new ExecutorException("Cannot commit, transaction is already closed");
    }
    clearLocalCache();
    flushStatements();
    if (required) {	//如果强制commit
      transaction.commit();
    }
  }
```

###  3.11. <a name='rollback_'></a>_#rollback_

```java
  @Override
  public void rollback(boolean required) throws SQLException {
    if (!closed) {
      try {
        clearLocalCache();
        flushStatements(true);
      } finally {
        if (required) {
          transaction.rollback();
        }
      }
    }
  }
```

##  4. <a name='SimpleExecutor_'></a>_SimpleExecutor_

`org.apache.ibatis.executor.SimpleExecutor` ，继承 BaseExecutor 抽象类，简单的 Executor 实现类。

- 每次开始读或写操作，都创建对应的 Statement 对象。
- 执行完成后，关闭该 Statement 对象。

首先我们先来看看SimpleExecutor的构造方法:

```java
public class SimpleExecutor extends BaseExecutor {

  public SimpleExecutor(Configuration configuration, Transaction transaction) {
    super(configuration, transaction);
  }
  //....
}
```

SimpleExecutor重写了BaseExecutor的三个抽象方法，即`#doQuery`, `#doUpdate`, `#doFlushStatement`

###  4.1. <a name='doQuery_-1'></a>_#doQuery_

```java
  //select
  @Override
  public <E> List<E> doQuery(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler, BoundSql boundSql) throws SQLException {
    Statement stmt = null;
    try {
      Configuration configuration = ms.getConfiguration();
      //新建一个StatementHandler
      //这里看到ResultHandler传入了
      StatementHandler handler = configuration.newStatementHandler(wrapper, ms, parameter, rowBounds, resultHandler, boundSql);
      //准备语句
      stmt = prepareStatement(handler, ms.getStatementLog());
      //StatementHandler.query
      return handler.<E>query(stmt, resultHandler);
    } finally {
      closeStatement(stmt);
    }
  }
```

可以看到doQuery将查询操作交给StatementHandler来完成，StatementHandler首先完成参数绑定，然后准备语句，最后返回查询结果。比较简单。

###  4.2. <a name='doUpdate_-1'></a>_#doUpdate_

```java
  //update
  @Override
  public int doUpdate(MappedStatement ms, Object parameter) throws SQLException {
    Statement stmt = null;
    try {
      Configuration configuration = ms.getConfiguration();
      //新建一个StatementHandler
      //这里看到ResultHandler传入的是null
      StatementHandler handler = configuration.newStatementHandler(this, ms, parameter, RowBounds.DEFAULT, null, null);
      //准备语句
      stmt = prepareStatement(handler, ms.getStatementLog());
      //StatementHandler.update
      return handler.update(stmt);
    } finally {
      closeStatement(stmt);
    }
  }
```

同样交给StatementHandler来完成update操作。

###  4.3. <a name='doFlushStatements_'></a>_#doFlushStatements_

```java
  @Override
  public List<BatchResult> doFlushStatements(boolean isRollback) throws SQLException {
	//doFlushStatements只是给batch用的，所以这里返回空
    return Collections.emptyList();
  }
```

doFlushStatements只是给batch用的，所以这里返回emptyList

##  5. <a name='ReuseExecutor_'></a>_ReuseExecutor_

`org.apache.ibatis.executor.ReuseExecutor` ，继承 BaseExecutor 抽象类，可重用的 Executor 实现类。

- 每次开始读或写操作，优先从缓存中获取对应的 Statement 对象。如果不存在，才进行创建。
- **执行完成后，不关闭该 Statement 对象。**
- 其它的，和 SimpleExecutor 是一致的。

**ReuseExecutor内部维护了一个HashMap用来缓存使用过的Statement。**

###  5.1. <a name='doQuery_-1'></a>_#doQuery_

```java
  @Override
  public <E> List<E> doQuery(MappedStatement ms, Object parameter, RowBounds rowBounds, ResultHandler resultHandler, BoundSql boundSql) throws SQLException {
    Configuration configuration = ms.getConfiguration();
    StatementHandler handler = configuration.newStatementHandler(wrapper, ms, parameter, rowBounds, resultHandler, boundSql);
    Statement stmt = prepareStatement(handler, ms.getStatementLog());
    return handler.<E>query(stmt, resultHandler);
  }
```

###  5.2. <a name='doUpdate_-1'></a>_#doUpdate_

```java
  @Override
  public int doUpdate(MappedStatement ms, Object parameter) throws SQLException {
    Configuration configuration = ms.getConfiguration();
    //和SimpleExecutor一样，新建一个StatementHandler
    //这里看到ResultHandler传入的是null
    StatementHandler handler = configuration.newStatementHandler(this, ms, parameter, RowBounds.DEFAULT, null, null);
    //准备语句
    Statement stmt = prepareStatement(handler, ms.getStatementLog());
    return handler.update(stmt);
  }

```

###  5.3. <a name='doFlushStatements_-1'></a>_#doFlushStatements_

```java
  @Override
  public List<BatchResult> doFlushStatements(boolean isRollback) throws SQLException {
    for (Statement stmt : statementMap.values()) {
      closeStatement(stmt);
    }
    statementMap.clear();	//首先清空statement缓存
    return Collections.emptyList();
  }
```

###  5.4. <a name='prepareStatement_Reuse'></a>_#prepareStatement_(Reuse体现的地方)

```java
  private Statement prepareStatement(StatementHandler handler, Log statementLog) throws SQLException {
    Statement stmt;
    //得到绑定的SQL语句
    BoundSql boundSql = handler.getBoundSql();
    String sql = boundSql.getSql();
    //如果缓存中已经有了，直接得到Statement
    if (hasStatementFor(sql)) {
      stmt = getStatement(sql);
    } else {
      //如果缓存没有找到，则和SimpleExecutor处理完全一样，然后加入缓存
      Connection connection = getConnection(statementLog);
      stmt = handler.prepare(connection);
      putStatement(sql, stmt);
    }
    handler.parameterize(stmt);
    return stmt;
  }
```

##  6. <a name='BatchExecutor_'></a>_BatchExecutor_(重要)

执行update（没有select，JDBC批处理不支持select），将所有sql都添加到批处理中（addBatch()），等待统一执行（executeBatch()），它缓存了多个Statement对象，每个Statement对象都是addBatch()完毕后，等待逐一执行executeBatch()批处理的；BatchExecutor相当于维护了多个桶，每个桶里都装了很多属于自己的SQL，就像苹果蓝里装了很多苹果，番茄蓝里装了很多番茄，最后，再统一倒进仓库。（可以是Statement或PrepareStatement对象）

首先我们先来看看BatchExecutor的成员属性和构造函数:

```java
public class BatchExecutor extends BaseExecutor {

  public static final int BATCH_UPDATE_RETURN_VALUE = Integer.MIN_VALUE + 1002;

  //批处理的Statement
  private final List<Statement> statementList = new ArrayList<Statement>();
  //批处理的结果，实际上是一个个Statement各自的结果
  private final List<BatchResult> batchResultList = new ArrayList<BatchResult>();
  //当前执行的SQL语句
  private String currentSql;
  private MappedStatement currentStatement;

  public BatchExecutor(Configuration configuration, Transaction transaction) {
    super(configuration, transaction);
  }
}
```

###  6.1. <a name='doQuery_-1'></a>_#doQuery_

由于Batch操作不针对select语句，因此doQuery和正常的Executor并无差别

```java
  @Override
  public <E> List<E> doQuery(MappedStatement ms, Object parameterObject, RowBounds rowBounds, ResultHandler resultHandler, BoundSql boundSql)
      throws SQLException {
    Statement stmt = null;
    try {
      flushStatements();
      Configuration configuration = ms.getConfiguration();
      StatementHandler handler = configuration.newStatementHandler(wrapper, ms, parameterObject, rowBounds, resultHandler, boundSql);
      Connection connection = getConnection(ms.getStatementLog());
      stmt = handler.prepare(connection);
      handler.parameterize(stmt);
      return handler.<E>query(stmt, resultHandler);
    } finally {
      closeStatement(stmt);
    }
  }
```

###  6.2. <a name='doUpdate_-1'></a>_#doUpdate_

```java
  @Override
  public int doUpdate(MappedStatement ms, Object parameterObject) throws SQLException {
    final Configuration configuration = ms.getConfiguration();
    final StatementHandler handler = configuration.newStatementHandler(this, ms, parameterObject, RowBounds.DEFAULT, null, null);
    final BoundSql boundSql = handler.getBoundSql();
    final String sql = boundSql.getSql();
    final Statement stmt;
    if (sql.equals(currentSql) && ms.equals(currentStatement)) {
      int last = statementList.size() - 1;
      stmt = statementList.get(last);
      BatchResult batchResult = batchResultList.get(last);
      batchResult.addParameterObject(parameterObject);
    } else {
      Connection connection = getConnection(ms.getStatementLog());
      stmt = handler.prepare(connection);
      currentSql = sql;
      currentStatement = ms;
      statementList.add(stmt);
      batchResultList.add(new BatchResult(ms, sql, parameterObject));
    }
    handler.parameterize(stmt);
    handler.batch(stmt);
    return BATCH_UPDATE_RETURN_VALUE;
  }

```

###  6.3. <a name='doFlushStatements_-1'></a>_#doFlushStatements_

```java
  @Override
  public List<BatchResult> doFlushStatements(boolean isRollback) throws SQLException {
    try {
      List<BatchResult> results = new ArrayList<BatchResult>();
      if (isRollback) {
        return Collections.emptyList();
      }
      for (int i = 0, n = statementList.size(); i < n; i++) {
        Statement stmt = statementList.get(i);
        BatchResult batchResult = batchResultList.get(i);
        try {
          batchResult.setUpdateCounts(stmt.executeBatch());
          MappedStatement ms = batchResult.getMappedStatement();
          List<Object> parameterObjects = batchResult.getParameterObjects();
          KeyGenerator keyGenerator = ms.getKeyGenerator();
          if (Jdbc3KeyGenerator.class.equals(keyGenerator.getClass())) {
            Jdbc3KeyGenerator jdbc3KeyGenerator = (Jdbc3KeyGenerator) keyGenerator;
            jdbc3KeyGenerator.processBatch(ms, stmt, parameterObjects);
          } else if (!NoKeyGenerator.class.equals(keyGenerator.getClass())) { //issue #141
            for (Object parameter : parameterObjects) {
              keyGenerator.processAfter(this, ms, stmt, parameter);
            }
          }
        } catch (BatchUpdateException e) {
          StringBuilder message = new StringBuilder();
          message.append(batchResult.getMappedStatement().getId())
              .append(" (batch index #")
              .append(i + 1)
              .append(")")
              .append(" failed.");
          if (i > 0) {
            message.append(" ")
                .append(i)
                .append(" prior sub executor(s) completed successfully, but will be rolled back.");
          }
          throw new BatchExecutorException(message.toString(), e, results, batchResult);
        }
        results.add(batchResult);
      }
      return results;
    } finally {
      for (Statement stmt : statementList) {
        closeStatement(stmt);
      }
      currentSql = null;
      statementList.clear();
      batchResultList.clear();
    }
  }
```

##  7. <a name='CachingExecutor_.......'></a>_CachingExecutor_(待续.......)



