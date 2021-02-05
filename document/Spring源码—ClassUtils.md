# Sa-token源码解析

<!-- vscode-markdown-toc -->

* 1. [Sa-token配置选项](#Sa-token)

* 2. [Token(令牌应该如何设计，包含哪些参数)](#Token)

* 3. [Session的设计](#Session)

* 4. [`SaTokenManager`容器](#SaTokenManager)

* 5. [`StpLogic`核心验证模组](#StpLogic)

  * 5.1. [token相关操作逻辑](#token)
    * 5.1.1. [tokenValue的获取](#tokenValue)
    * 5.1.2. [检查token是否过期](#token-1)
  * 5.2. [session相关操作逻辑](#session)
    * 5.2.1. [在当前session上登录](#session-1)
    * 5.2.2. [在当前session上注销登录](#session-1)
    * 5.2.3. [指定loginId和device的session注销登录(踢人下线)](#loginIddevicesession)

* 6. [`StpUtil`对外暴露工具类](#StpUtil)
     * 6.1. [指定loginId的session注销登录(强制所有其他终端下线)](#loginIdsession)
     * 6.2. [指定loginId和device的session注销登录(强制特定终端下线)](#loginIddevicesession-1)
     * 6.3. [指定token的session注销登录](#tokensession)

* 7. [Sa-Token注解设计](#Sa-Token)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='Sa-token'></a>Sa-token配置选项

Sa-token可以零启动配置和通过配置文件方式进行参数的配置，所有的配置选项如下，配置文件对应的参数被封装为`SaTokenConfig`，`SaTokenConfig`含有配置参数的默认值，配置文件中未出现的配置选项会直接使用默认值

```yml
spring: 
    # sa-token配置
    sa-token: 
        # token名称 (同时也是cookie名称)
        #默认: satoken
        token-name: satoken
        # token有效期，单位s 默认30天, -1代表永不过期 
        # 默认: 30 * 24 * 60 * 60
        timeout: 2592000
        # token临时有效期 (指定时间内无操作就视为token过期) 单位: 秒
        # 默认: -1
        activity-timeout: -1
        # 是否允许同一账号并发登录 (为true时允许一起登录, 为false时新登录挤掉旧登录)
        # 默认: true 
        allow-concurrent-login: false
        # 在多人登录同一账号时，是否共用一个token (为true时所有登录共用一个token, 为false时每次登录新建一个token)
        # 默认： true 
        is-share: false
        # 是否尝试从请求体里面读取token
        # 默认: true
        is-read-body: true
        # 是否尝试从header里面读取token
        # 默认: true
        is-read-head: true
        # 是否尝试从cookie中读取文件
        # 默认: true
        is-read-cookie: true
        # token风格，可取值: uuid, simple-uuid, random-32, random-64, random-128, tik
        # 默认: uuid
        token-style: uuid
        # 默认dao层实现中，每次清理过期数据间隔的时间，单位：秒，-1代表不启动定时清理
        # 默认: 30
        data-refresh-period: 30
        # 获取[token专属session]时是否必须登录，如果配置为true，会在每次获取[token-session]时校验是否登录
        # 默认: true
        token-session-check-login: true
        # 是否在初始化配置的时候打印版本字符画
        # 默认: true
        is-v: true
```

`SaTokenConfigFactory`负责从`classpath:sa-token.properties`配置文件中读取参数，封装成`SaTokenConfig`，**在非IOC环境下不会使用到该类**。

##  2. <a name='Token'></a>Token(令牌应该如何设计，包含哪些参数)

Token在权限认证模块中的作用是无可替代的，所有的认证流程基本都是基于token，在Sa-token项目中，token的常用信息被封装在`SaTokenInfo`中，大致内容如下:

```java
public class SaTokenInfo {
	/** token名称 */
	public String tokenName;
	/** token值 */
	public String tokenValue;
	/** 此token是否已经登录 */
	public Boolean isLogin;
	/** 此token对应的LoginId，未登录时为null */
	public Object loginId;
	/** LoginKey账号体系标识 */
	public String loginKey;
	/** token剩余有效期 (单位: 秒) */
	public long tokenTimeout;
	/** User-Session剩余有效时间 (单位: 秒) */
	public long sessionTimeout;
	/** Token-Session剩余有效时间 (单位: 秒) */
	public long tokenSessionTimeout;
	/** token剩余无操作有效时间 (单位: 秒) */
	public long tokenActivityTimeout;
	/** 登录设备标识 */
	public String loginDevice;
}
```

关于token的一些常量则被封装在`SaTokenConsts`中:

```java
public class SaTokenConsts{
    /* 如果token为本次请求新创建的，则以此字符串作为key存储在request中 */
    public static final String JUST_CREATED_SAVE_KEY = "JUST_CREATED_SAVE_KEY_";
    /* 如果本次请求已经验证过并且操作没有过期，则以此值存储在当前的request中 */
    public static final String TOKEN_ACTIVITY_TIMEOUT_CHECKED_KEY = "TOKEN_ACTIVITY_TIMEOUT_CHECKDE_KEY_";
    /* 在登录时，进行临时身份切换时使用的key */
    public static final String SWITCH_TO_SAVE_KEY = "SWITCH_TO_SAVE_KEY_";
    /* 在登陆时，默认使用的设备名称 */
    public static final String DEFAULT_LOGIN_DEVICE = "default-device";
    /* Token风格: uuid */
    public static final String TOKEN_STYLE_UUID = "uuid";
    /* Token分割，简单uuid */
    public static final String TOKEN_STYLE_SIMPLE_UUID = "simple-uuid"; 
    /* token风格: 32位随机字符串 */
    public static final String TOKEN_STYLE_RANDOM_32 = "random-32"; 
    /* token风格: 64位随机字符串 */
    public static final String TOKEN_STYLE_RANDOM_64 = "random-64"; 
    /* token风格: 128位随机字符串 */
    public static final String TOKEN_STYLE_RANDOM_128 = "random-128"; 
    /* token风格: tik风格 (2_14_16)  */
    public static final String TOKEN_STYLE_RANDOM_TIK = "tik"; 
}
```

##  3. <a name='Session'></a>Session的设计

Session是会话中专业的数据缓存组件，在sa-token中Session分为三种, 分别是：

* **User-Session**: 指的是框架为每个loginId分配的Session
* **Token-Session**: 指的是框架为每个token分配的Session
* **自定义Session**: 指的是以一个特定的值作为SessionId，来分配的Session 

Sa-Token对于session的设计被保存在`SaSession`对象中，由于考虑到并发的原因，session中挂载的数据采用`ConcurrentHashMap`。

```java
public class SaSession implements Serializable {
	/** 此Session的id */
	private String id;
	/** 此Session的创建时间 */
	private long createTime;
	/** 此Session的所有挂载数据 */
	private Map<String, Object> dataMap = new ConcurrentHashMap<String, Object>();
        /** 此session绑定的token签名列表 */
        private List<TokenSign> tokenSignList = new Vector<TokenSign>();
}
```

`SaSession`包含一个`TokenSign`的签名列表，即token签名，被封装在`TokenSign`对象中，其作用就是**将token与用户的设备绑定起来，组成一个特殊的标识**:

```java
public class TokenSign{
    /* token值 */
    private String value;
    /* 设备标识 */
    private String device;
}
```

##  4. <a name='SaTokenManager'></a>`SaTokenManager`容器

`SaTokenManager`管理所有有关token操作的接口对象，是存放接口Bean的容器，基本定义如下:

```java
public class SaTokenManager{
    /* 配置文件bean */
    private static SaTokenManager saTokenConfig;
    /* token持久化bean */
    private static SaTokenDao satokenDao;
    /* 权限认证bean */
    private static StpInterface stpInterface;
    /* 框架行为bean */
    private static SaTokenAction saTokenAction;
    /* cookie操作bean */
    private static SaTokenCookie saTokenCookie;
    /* servlet操作bean，主要与获取HttpReq/Resp相关 */
    private static SaTokenServlet saTokenServlet;
}
```

##  5. <a name='StpLogic'></a>`StpLogic`核心验证模组

`StpLogic`是权限验证的核心实现类，实现的操作逻辑主要包含:

* **token相关操作逻辑**
* **session相关操作逻辑**
* **验证相关操作逻辑**

###  5.1. <a name='token'></a>token相关操作逻辑

####  5.1.1. <a name='tokenValue'></a>tokenValue的获取

```java
public String getTokenValue(){
    HttpServletRequest req = SaTokenManager.getSaTokenManager.getSaTokenServlet().getRequest();
    SaTokenConfig config = getConfig(); //获得配置参数
    String keyTokenName = getTokenName(); //配置文件中会配置token-name，默认是satoken
    String tokenValue = null;   //token值，name-value是一对键值对

    //1.尝试从request中获取
    if(req.getAttribute(getKeyJustCreatedSave()) != null){
        // 基本形式: JUST_CREATED_SAVE_KEY + loginKey
        tokenValue = String.valueOf(req.getAttribute(getKeyJustCreatedSave()));
    }
    //2. 尝试从req body中获取
    if(tokenValue == null && config.getIsReadBody()){
        tokenValue = req.getParameter(keyTokenName);
    }
    //3. 尝试从req header中获取
    if(tokenValue == null && config.getIsReadHeader()){
        tokenValue = req.getHeader(keyTokenName);
    }
    //4. 尝试从cookie中获取
    if(tokenValue == null && config.getIsReadCookie()){
        Cookie cookie = SaTokenManager.getSaTokenCookie().getCookie(req, keyTokenName);
        if(cookie != null){
            tokenValue = cookie.getValue();
        }
    }
}
```

####  5.1.2. <a name='token-1'></a>检查token是否过期

```java
public void checkActivityTimeout(String tokenValue){
    //如果tokenValue == null 获取设置了永不过期，不用检查
    if(tokenValue == null || getConfig().getActivityTimeout() == SaTokenDao.NEVER_EXPIRE){
        return;
    }
    //如果本次请求已经有了检查标记，即被检查过，放行
    HttpServletRequest req = SaTokenManager.getSaTokenServlet().getRequest();
    if(req.getAttribute(SaTokenConsts.TOKEN_ACTIVITY_CHECKED_KEY) != null){
        return;
    }
    //验证是否过期
    long timeout = getTokenActivityTimeoutByToken(tokenValue);
    //-1代表token已经被设置了永不过期
    if(timeout == SaTokenDao.NEVER_EXPIRE){
        return;
    }
    //-2表示已经过期
    if(timeout == SaTokenDao.NOT_VALUE_EXPIRE){
        throw ...
    }
    //打上标记检查
    req.setAttribute(SaTokenConsts.TOKEN_ACTIVITY_CHECKED_KEY, true);
}
```

###  5.2. <a name='session'></a>session相关操作逻辑

####  5.2.1. <a name='session-1'></a>在当前session上登录

```java
//loginId建立类型: long | int | String
public void setLoginId(Object loginId, String device){
    //1. 获取响应对象
    HttpServletRequest req = SaTokenManager.getSaTokenServlet().getRequest();
    SaTokenConfig config = getConfig();
    SaTokenDao dao = SaTokenManager.getSaTokenDao();

    //2. 根据loginId与device生成token
    String tokenValue = null;

    //为此loginId生成tokenValue
    if(config.getAllowConcurrentLogin()){   //如果同账号多终端登录
        if(config.getIsShare()){    //多终端下如果允许使用同一个token
            tokenValue = getTokenValueByLoginId(loginId, device);
        }
    }else{  //不允许并发登录
        //当前id对应的session不为null，说明这个账号已经在其他终端登录，需要强迫其他终端下线
        //下线的根据就是修改dao层的id-session为被取代状态
        SaSession session = getSessionByLoginId(loginId, false);
        if(session != null){
            List<TokenSign> tokenSigns = session.getTokenSignList();
            for(TokenSign tokenSign : tokenSigns){
                if(tokenSign.getDevice().equals(device)){ //找到异地登录的终端
                    //1. 将此token设置为BE_REPLACED
                    dao.updateValue(getKeyTokenValue(tokenSign.getTokenValue()), NotLoginException.BE_REPLACED);
                    //2. 清理掉该token对应的最后操作时间
                    clearLastActivity(tokenSign.getValue());
                    //3. 清理账号session上的token签名
                    session.removeTokenSign(tokenSign.getValue());
                }
            }
        }
    }

    //至此tokenValue如果生成失败，直接生成一个
    tokenValue = tokenValue != null ? tokenValue : createTokenValue(loginId);

    //获取User-Session，如果session未被创建，则创建，如果已经创建，则续期
    SaSession session = getSessionByLoginId(loginId, false);
    session = session == null ? 
        getSessionByLoginId(loginId) : dao.updateSessionTimeout(session.getId(), config.getTimeout());
    //在session上记录签名
    session.addTokenSign(new TokenSign(tokenValue, device));
    //token->uid
    dao.setValue(getTokenValue(tokenValue), String.valueOf(loginId), config.getTimeout());
    //将token保存到本次req中
    req.setAttribute(getKeyJustCreatedSave(), tokenValue);
    //记录该tokenValue的最后写入时间
    setLastActivityToNow(tokenValue);
    //cookie注入
    if(config.getIsReadCookie()){
        SaTokenManager.getSaTokenCookie().addCookie(SaTokenManager.getSaTokenServlet().getResponse(), 
            getTokenName(), tokenValue, "/", (int) config.getTimeout());
    }
}
```

####  5.2.2. <a name='session-1'></a>在当前session上注销登录

```java
public void logout(){
    //如果当前session并没有token，比如token已经过期，直接忽略
    String tokenValue = getTokenValue();
    if(tokenValue == null){
        return;
    }
    //如果打开了cookie模式，先把cookie清除
    if(getConfig().getIsReadCookie()){
        SaTokenManager.getSaTokenCookie().delCookie(SaTokenManager.getSaTokenServlet()
            .getRequest(), SaTokenManager.getSaTokenServlet().getResponse(), getTokenName());
        logoutByTokenValue(tokenValue);
    }
}

//根据token进行注销
public void logoutByTokenValue(String tokenValue) {
    // 1. 清理掉[token-最后操作时间] 
    clearLastActivity(tokenValue); 	
		
    // 2. 尝试清除token-id键值对 (先从db中获取loginId值，如果根本查不到loginId，那么无需继续操作 )
    String loginId = getLoginIdNotHandle(tokenValue);
    if(loginId == null || NotLoginException.ABNORMAL_LIST.contains(loginId)) { 			
        return;
    }
    SaTokenManager.getSaTokenDao().deleteKey(getKeyTokenValue(tokenValue));	
 		
    // 3. 尝试清理账号session上的token签名 (如果为null或已被标记为异常, 那么无需继续执行 )
    SaSession session = getSessionByLoginId(loginId, false);
    if(session == null) {
        return;
    }
    session.removeTokenSign(tokenValue); 
 	 	
    // 4. 尝试注销session
    session.logoutByTokenSignCountToZero();
}
```

####  5.2.3. <a name='loginIddevicesession'></a>指定loginId和device的session注销登录(踢人下线)

```java
//当被踢对象再次访问系统的时候，会抛出NotLoginException
public void logoutByLoginId(Object loginId, String device){
    //1. 先获取session
    SaSession session = getSessionByLoginId(loginId);
    if(session == null){ //session已不存在
        return;
    }
    //2. 循环tokenSign列表，删除相关信息
    List<TokenSign> tokenSignList = session.getTokenSignList();
    for(TokenSign tokenSign : tokenSignList){
        //device == null表示注销该loginId对应的所有设备
        if(device == null || tokenSign.getDevice().equals(device)){
            //1. 获取tokenValue
            String tokenValue = tokenSign.getValue();
            //2. 清理掉token最后操作时间
            clearLastActivity(tokenValue);
            //3. 标记该loginId对应的device会话下线
            SaTokenManager.getSaTokenDao().updateValue(getKeyTokenValue(tokenValue), NotLoginException.KICK_OUT);
            //4. 清理账号session上的token签名
            session.removeTokenSign(tokenValue);
        }
    }
    //3. 尝试注销session
    session.logoutByTokenSignCountToZero();
}
```

##  6. <a name='StpUtil'></a>`StpUtil`对外暴露工具类

`StpUtil`作为对外使用的Api，其内部的一系列操作是基于`StpLogic`逻辑实现的，同时将`StpLogic`提供的方法进行整合实现多种功能，整个`StpUtil`内置对象只有一个`StpLogic`，代码如下:

```java
public class StpUtil{
    //对应StpLogic中的loginKey为login
    public static StpLogic stpLogic = new StpLogic("login");
}
```

####  6.1. <a name='loginIdsession'></a>指定loginId的session注销登录(强制所有其他终端下线)

```java
public static void logoutByLoginId(Object loginId){
    stpLogic.logoutByLoginId(loginId);
}
```

####  6.2. <a name='loginIddevicesession-1'></a>指定loginId和device的session注销登录(强制特定终端下线)

```java
public static void logoutByLoginId(Object loginId, String device){
    stpLogic.logoutByLoginId(loginId, device);
}
```

####  6.3. <a name='tokensession'></a>指定token的session注销登录

```java
public static void logoutByTokenValue(String tokenValue){
    stpLogic.logoutByTokenValue(tokenValue);
}
```

##  7. <a name='Sa-Token'></a>Sa-Token注解设计

Sa-Token的权限验证是基于注解的，因此代码更加简洁优雅，供用户使用的注解主要有3个，分别是:

* **`@SaCheckLogin`**: 可作用于方法和类上，作用在类上的时候，该类所有的method均需要进行登录检查
* **`@SaCheckPermission`**: 可作用在方法和类上，作用在类上的时候，该类所有的method均需要进行权限验证
* **`@SaCheckRole`**: 可作用在方法和类上，作用在类上的时候，该类所有的method都需要进行角色验证