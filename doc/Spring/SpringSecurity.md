# Spring Security源码解析

<!-- vscode-markdown-toc -->
* 1. [关键类](#)
	* 1.1. [SecurityContextHolder](#SecurityContextHolder)
	* 1.2. [Principal](#Principal)
	* 1.3. [Authentication](#Authentication)
	* 1.4. [AuthenticationManager](#AuthenticationManager)
		* 1.4.1. [ProviderManager](#ProviderManager)
		* 1.4.2. [DaoAuthenticationProvider](#DaoAuthenticationProvider)
* 2. [架构图](#-1)
* 3. [配置信息](#-1)
	* 3.1. [@EnableWebSecurity](#EnableWebSecurity)
		* 3.1.1. [WebSecurityConfiguration](#WebSecurityConfiguration)
		* 3.1.2. [AuthenticationConfiguration](#AuthenticationConfiguration)
* 4. [过滤器](#-1)
	* 4.1. [Spring Security默认核心过滤器](#SpringSecurity)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>关键类

###  1.1. <a name='SecurityContextHolder'></a>SecurityContextHolder

SecurityContextHolder用于存储 **安全上下文(security context)** 信息，一般来说，当前操作对应的用户，该用户是否被认证，拥有哪些权限等。

SecurityContextHolder默认采用ThreadLocal存储认证信息，这就意味着用户和线程绑定，当用户登陆时自动绑定到当前线程。

 ```java
 	static {
 		initialize();
 	}
 
 	private static void initialize() {
 		if (!StringUtils.hasText(strategyName)) {
 			// Set default
 			strategyName = MODE_THREADLOCAL;
 		}
 		if (strategyName.equals(MODE_THREADLOCAL)) {
 			strategy = new ThreadLocalSecurityContextHolderStrategy();	//默认是ThreadLocal
 		}
 		else if (strategyName.equals(MODE_INHERITABLETHREADLOCAL)) {
 			strategy = new InheritableThreadLocalSecurityContextHolderStrategy();
 		}
 		else if (strategyName.equals(MODE_GLOBAL)) {
 			strategy = new GlobalSecurityContextHolderStrategy();
 		}
 		else {
 			// Try to load a custom strategy
 			try {
 				Class<?> clazz = Class.forName(strategyName);
 				Constructor<?> customStrategy = clazz.getConstructor();
 				strategy = (SecurityContextHolderStrategy) customStrategy.newInstance();
 			}
 			catch (Exception ex) {
 				ReflectionUtils.handleReflectionException(ex);
 			}
 		}
 		initializeCount++;
 	}
 ```

###  1.2. <a name='Principal'></a>Principal

Principal是对身份信息的高度抽象， **可以表示一个个人，公司或者login id**

```java
/**
 * This interface represents the abstract notion of a principal, which
 * can be used to represent any entity, such as an individual, a
 * corporation, and a login id.
 *
 * @see java.security.cert.X509Certificate
 *
 * @author Li Gong
 */
public interface Principal {
	...
}
```

###  1.3. <a name='Authentication'></a>Authentication

```java
public interface Authentication extends Principal, Serializable {

    /**
     * 由AuthenticationManager设定，来指明principal被授权
     * 返回: 该principal被授予的权限，返回的被授予的权限的列表
     */
	Collection<? extends GrantedAuthority> getAuthorities();

	/**
	 * 密码信息，用户输入的密码字符串，在认证过后通常会被移除来保障安全
	 */
	Object getCredentials();

	/**
	 * 该principal的细节信息，通常为WebAuthenticationDeatails类型，记录了访问者的ip和sessionId信息
	 */
	Object getDetails();

	/**
	 * 最重要的身份信息，大部分情况下返回的是UserDetails接口的实现类，也是框架中的常用接口之一
	 */
	Object getPrincipal();

	/**
	 * 是否被鉴权
	 */
	boolean isAuthenticated();

	/**
	 * 设置是否被鉴权
	 */
	void setAuthenticated(boolean isAuthenticated) throws IllegalArgumentException;

}
```

###  1.4. <a name='AuthenticationManager'></a>AuthenticationManager

Authentication是认证过程的发起点，只是一个抽象接口，具体的认证过程由其实现类执行。

```java
public interface AuthenticationManager {

	Authentication authenticate(Authentication authentication) throws AuthenticationException;

}
```

####  1.4.1. <a name='ProviderManager'></a>ProviderManager

```java
public class ProviderManager implements AuthenticationManager, MessageSourceAware, InitializingBean {

	private static final Log logger = LogFactory.getLog(ProviderManager.class);

	private AuthenticationEventPublisher eventPublisher = new NullEventPublisher();
	//这里维持了一个认证器列表，从AuthenticationManager开始发起认证
    //依次比对AuthenticationProvider，如果可以认证，即执行认证过程
    //这种模式称为“委托模式”
	private List<AuthenticationProvider> providers = Collections.emptyList();

	protected MessageSourceAccessor messages = SpringSecurityMessageSource.getAccessor();

	private AuthenticationManager parent;

	private boolean eraseCredentialsAfterAuthentication = true;
    
    @Override
	public Authentication authenticate(Authentication authentication) throws AuthenticationException {
        //toTest一定是Authentication的实现类，获取Authentication的类别信息
		Class<? extends Authentication> toTest = authentication.getClass();
		AuthenticationException lastException = null;
		AuthenticationException parentException = null;
		Authentication result = null;
		Authentication parentResult = null;
		int currentPosition = 0;
		int size = this.providers.size();
        //依次验证
		for (AuthenticationProvider provider : getProviders()) {
            //不支持就跳到下一个
			if (!provider.supports(toTest)) {
				continue;
			}
			if (logger.isTraceEnabled()) {
				logger.trace(LogMessage.format("Authenticating request with %s (%d/%d)",
						provider.getClass().getSimpleName(), ++currentPosition, size));
			}
			try {
                //验证结束，将验证结果替换
				result = provider.authenticate(authentication);
				if (result != null) {
					copyDetails(authentication, result);
					break;
				}
			}
			catch (AccountStatusException | InternalAuthenticationServiceException ex) {
				prepareException(ex, authentication);
				// SEC-546: Avoid polling additional providers if auth failure is due to
				// invalid account status
				throw ex;
			}
			catch (AuthenticationException ex) {
				lastException = ex;
			}
		}
        //如果当前ProviderManager验证不通过，可以尝试通过父类AuthenticationManager进行验证
		if (result == null && this.parent != null) {
			// Allow the parent to try.
			try {
				parentResult = this.parent.authenticate(authentication);
				result = parentResult;
			}
			catch (ProviderNotFoundException ex) {
				// ignore as we will throw below if no other exception occurred prior to
				// calling parent and the parent
				// may throw ProviderNotFound even though a provider in the child already
				// handled the request
			}
			catch (AuthenticationException ex) {
				parentException = ex;
				lastException = ex;
			}
		}
		if (result != null) {
            //验证通过，擦除敏感信息，用户的关键数据
			if (this.eraseCredentialsAfterAuthentication && (result instanceof CredentialsContainer)) {
				// Authentication is complete. Remove credentials and other secret data
				// from authentication
				((CredentialsContainer) result).eraseCredentials();
			}
			// If the parent AuthenticationManager was attempted and successful then it
			// will publish an AuthenticationSuccessEvent
			// This check prevents a duplicate AuthenticationSuccessEvent if the parent
			// AuthenticationManager already published it
			if (parentResult == null) {
				this.eventPublisher.publishAuthenticationSuccess(result);
			}

			return result;
		}

		// Parent was null, or didn't authenticate (or throw an exception).
		if (lastException == null) {
			lastException = new ProviderNotFoundException(this.messages.getMessage("ProviderManager.providerNotFound",
					new Object[] { toTest.getName() }, "No AuthenticationProvider found for {0}"));
		}
		// If the parent AuthenticationManager was attempted and failed then it will
		// publish an AbstractAuthenticationFailureEvent
		// This check prevents a duplicate AbstractAuthenticationFailureEvent if the
		// parent AuthenticationManager already published it
		if (parentException == null) {
			prepareException(lastException, authentication);
		}
		throw lastException;
	}
```

总结来说，ProviderManager 中的List，会依照次序去认证，**认证成功则立即返回null** ,下一个AuthenticationProvider会继续尝试认证，如果所有认证器都无法认证成功，则ProviderManager 会抛出一个ProviderNotFoundException异常。

####  1.4.2. <a name='DaoAuthenticationProvider'></a>DaoAuthenticationProvider

AuthenticationProvider最最最常用的一个实现便是DaoAuthenticationProvider。顾名思义，Dao正是数据访问层的缩写，也暗示了这个身份认证器的实现思路。

在Spring Security中。提交的用户名和密码，被封装成了UsernamePasswordAuthenticationToken，而根据用户名加载用户的任务则是交给了UserDetailsService，在DaoAuthenticationProvider中，对应的方法便是retrieveUser，虽然有两个参数，但是retrieveUser只有第一个参数起主要作用，返回一个UserDetails。还需要完成UsernamePasswordAuthenticationToken和UserDetails密码的比对，这便是交给additionalAuthenticationChecks方法完成的，如果这个void方法没有抛异常，则认为比对成功。比对密码的过程，用到了PasswordEncoder和SaltSource

```java
public class DaoAuthenticationProvider extends AbstractUserDetailsAuthenticationProvider {

	/**
	 * The plaintext password used to perform PasswordEncoder#matches(CharSequence,
	 * String)} on when the user is not found to avoid SEC-2056.
	 */
	private static final String USER_NOT_FOUND_PASSWORD = "userNotFoundPassword";

	private PasswordEncoder passwordEncoder;

	/**
	 * The password used to perform {@link PasswordEncoder#matches(CharSequence, String)}
	 * on when the user is not found to avoid SEC-2056. This is necessary, because some
	 * {@link PasswordEncoder} implementations will short circuit if the password is not
	 * in a valid format.
	 */
	private volatile String userNotFoundEncodedPassword;

	private UserDetailsService userDetailsService;

	private UserDetailsPasswordService userDetailsPasswordService;

	public DaoAuthenticationProvider() {
		setPasswordEncoder(PasswordEncoderFactories.createDelegatingPasswordEncoder());
	}

	@Override
	@SuppressWarnings("deprecation")
	protected void additionalAuthenticationChecks(UserDetails userDetails,
			UsernamePasswordAuthenticationToken authentication) throws AuthenticationException {
		if (authentication.getCredentials() == null) {
			this.logger.debug("Failed to authenticate since no credentials provided");
			throw new BadCredentialsException(this.messages
					.getMessage("AbstractUserDetailsAuthenticationProvider.badCredentials", "Bad credentials"));
		}
		String presentedPassword = authentication.getCredentials().toString();
		if (!this.passwordEncoder.matches(presentedPassword, userDetails.getPassword())) {
			this.logger.debug("Failed to authenticate since password does not match stored value");
			throw new BadCredentialsException(this.messages
					.getMessage("AbstractUserDetailsAuthenticationProvider.badCredentials", "Bad credentials"));
		}
	}

	@Override
	protected void doAfterPropertiesSet() {
		Assert.notNull(this.userDetailsService, "A UserDetailsService must be set");
	}

	@Override
	protected final UserDetails retrieveUser(String username, UsernamePasswordAuthenticationToken authentication)
			throws AuthenticationException {
		prepareTimingAttackProtection();
		try {
			UserDetails loadedUser = this.getUserDetailsService().loadUserByUsername(username);
			if (loadedUser == null) {
				throw new InternalAuthenticationServiceException(
						"UserDetailsService returned null, which is an interface contract violation");
			}
			return loadedUser;
		}
		catch (UsernameNotFoundException ex) {
			mitigateAgainstTimingAttack(authentication);
			throw ex;
		}
		catch (InternalAuthenticationServiceException ex) {
			throw ex;
		}
		catch (Exception ex) {
			throw new InternalAuthenticationServiceException(ex.getMessage(), ex);
		}
	}

	@Override
	protected Authentication createSuccessAuthentication(Object principal, Authentication authentication,
			UserDetails user) {
		boolean upgradeEncoding = this.userDetailsPasswordService != null
				&& this.passwordEncoder.upgradeEncoding(user.getPassword());
		if (upgradeEncoding) {
			String presentedPassword = authentication.getCredentials().toString();
			String newPassword = this.passwordEncoder.encode(presentedPassword);
			user = this.userDetailsPasswordService.updatePassword(user, newPassword);
		}
		return super.createSuccessAuthentication(principal, authentication, user);
	}

	private void prepareTimingAttackProtection() {
		if (this.userNotFoundEncodedPassword == null) {
			this.userNotFoundEncodedPassword = this.passwordEncoder.encode(USER_NOT_FOUND_PASSWORD);
		}
	}

	private void mitigateAgainstTimingAttack(UsernamePasswordAuthenticationToken authentication) {
		if (authentication.getCredentials() != null) {
			String presentedPassword = authentication.getCredentials().toString();
			this.passwordEncoder.matches(presentedPassword, this.userNotFoundEncodedPassword);
		}
	}

	/**
	 * Sets the PasswordEncoder instance to be used to encode and validate passwords. If
	 * not set, the password will be compared using
	 * {@link PasswordEncoderFactories#createDelegatingPasswordEncoder()}
	 * @param passwordEncoder must be an instance of one of the {@code PasswordEncoder}
	 * types.
	 */
	public void setPasswordEncoder(PasswordEncoder passwordEncoder) {
		Assert.notNull(passwordEncoder, "passwordEncoder cannot be null");
		this.passwordEncoder = passwordEncoder;
		this.userNotFoundEncodedPassword = null;
	}

	protected PasswordEncoder getPasswordEncoder() {
		return this.passwordEncoder;
	}

	public void setUserDetailsService(UserDetailsService userDetailsService) {
		this.userDetailsService = userDetailsService;
	}

	protected UserDetailsService getUserDetailsService() {
		return this.userDetailsService;
	}

	public void setUserDetailsPasswordService(UserDetailsPasswordService userDetailsPasswordService) {
		this.userDetailsPasswordService = userDetailsPasswordService;
	}

}
```

##  2. <a name='-1'></a>架构图

<div align=center><img src="/assets/ss1.png"/></div>

##  3. <a name='-1'></a>配置信息

###  3.1. <a name='EnableWebSecurity'></a>@EnableWebSecurity

我们自己定义的配置类WebSecurityConfig加上了@EnableWebSecurity注解，同时继承了WebSecurityConfigurerAdapter。你可能会在想谁的作用大一点，毫无疑问@EnableWebSecurity起到决定性的配置作用，它其实是个组合注解。

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
@Documented
@Import({ 
    WebSecurityConfiguration.class,   //用来配置web安全的
    SpringWebMvcImportSelector.class, //判断当前的环境是否包含springmvc，因为spring security可以在非spring环境下使用，为了避免DispatcherServlet的重复配置
    OAuth2ImportSelector.class,
	HttpSecurityConfiguration.class 
})
@EnableGlobalAuthentication
@Configuration
public @interface EnableWebSecurity {

	/**
	 * Controls debugging support for Spring Security. Default is false.
	 * @return if true, enables debug support with Spring Security
	 */
	boolean debug() default false;

}
```

####  3.1.1. <a name='WebSecurityConfiguration'></a>WebSecurityConfiguration

WebSecurityConfiguration中完成了声明springSecurityFilterChain的作用，并且最终交给DelegatingFilterProxy这个代理类，负责拦截请求（注意DelegatingFilterProxy这个类不是spring security包中的，而是存在于web包中，spring使用了代理模式来实现安全过滤的解耦）

```java
@Configuration(proxyBeanMethods = false)
public class WebSecurityConfiguration implements ImportAware, BeanClassLoaderAware {

	private WebSecurity webSecurity;

	private Boolean debugEnabled;

	private List<SecurityConfigurer<Filter, WebSecurity>> webSecurityConfigurers;

	private List<SecurityFilterChain> securityFilterChains = Collections.emptyList();

	private List<WebSecurityCustomizer> webSecurityCustomizers = Collections.emptyList();

	private ClassLoader beanClassLoader;

	@Autowired(required = false)
	private ObjectPostProcessor<Object> objectObjectPostProcessor;
    
    //,,,,,,,
}
```

####  3.1.2. <a name='AuthenticationConfiguration'></a>AuthenticationConfiguration

AuthenticationConfiguration的主要任务，便是负责生成全局的身份认证管理者AuthenticationManager。

```java
@Configuration(proxyBeanMethods = false)
@Import(ObjectPostProcessorConfiguration.class)
public class AuthenticationConfiguration {

	private AtomicBoolean buildingAuthenticationManager = new AtomicBoolean();

	private ApplicationContext applicationContext;

	private AuthenticationManager authenticationManager;

	private boolean authenticationManagerInitialized;

	private List<GlobalAuthenticationConfigurerAdapter> globalAuthConfigurers = Collections.emptyList();

	private ObjectPostProcessor<Object> objectPostProcessor;
    
    //.....
}
```

##  4. <a name='-1'></a>过滤器

###  4.1. <a name='SpringSecurity'></a>Spring Security核心过滤器

```java
o.s.s.web.context.SecurityContextPersistenceFilter@8851ce1,
o.s.s.web.header.HeaderWriterFilter@6a472566, o.s.s.web.csrf.CsrfFilter@61cd1c71,
o.s.s.web.authentication.logout.LogoutFilter@5e1d03d7,
o.s.s.web.authentication.UsernamePasswordAuthenticationFilter@122d6c22,
o.s.s.web.savedrequest.RequestCacheAwareFilter@5ef6fd7f,
o.s.s.web.servletapi.SecurityContextHolderAwareRequestFilter@4beaf6bd,
o.s.s.web.authentication.AnonymousAuthenticationFilter@6edcad64,
o.s.s.web.session.SessionManagementFilter@5e65afb6,
o.s.s.web.access.ExceptionTranslationFilter@5b9396d3,
o.s.s.web.access.intercept.FilterSecurityInterceptor@3c5dbdf8
```

Spring security的过滤器日志有一个特点：log打印顺序与实际配置顺序符合，也就意味着`SecurityContextPersistenceFilter`是整个过滤器链的第一个过滤器，而`FilterSecurityInterceptor`则是末置的过滤器。另外通过观察过滤器的名称，和所在的包名，可以大致地分析出他们各自的作用，如`UsernamePasswordAuthenticationFilter`明显便是与使用用户名和密码登录相关的过滤器，而`FilterSecurityInterceptor`我们似乎看不出它的作用，但是其位于`web.access`包下，大致可以分析出他与访问限制相关。第四篇文章主要就是介绍这些常用的过滤器，对其中关键的过滤器进行一些源码分析。先大致介绍下每个过滤器的作用：

- **SecurityContextPersistenceFilter** 两个主要职责：请求来临时，创建`SecurityContext`安全上下文信息，请求结束时清空`SecurityContextHolder`。
- HeaderWriterFilter (文档中并未介绍，非核心过滤器) 用来给http响应添加一些Header,比如X-Frame-Options, X-XSS-Protection*，X-Content-Type-Options.
- CsrfFilter 在spring4这个版本中被默认开启的一个过滤器，用于防止csrf攻击，了解前后端分离的人一定不会对这个攻击方式感到陌生，前后端使用json交互需要注意的一个问题。
- LogoutFilter 顾名思义，处理注销的过滤器
- **UsernamePasswordAuthenticationFilter** 这个会重点分析，表单提交了username和password，被封装成token进行一系列的认证，便是主要通过这个过滤器完成的，在表单认证的方法中，这是最最关键的过滤器。
- RequestCacheAwareFilter (文档中并未介绍，非核心过滤器) 内部维护了一个RequestCache，用于缓存request请求
- SecurityContextHolderAwareRequestFilter 此过滤器对ServletRequest进行了一次包装，使得request具有更加丰富的API
- **AnonymousAuthenticationFilter** 匿名身份过滤器，这个过滤器个人认为很重要，需要将它与UsernamePasswordAuthenticationFilter 放在一起比较理解，spring security为了兼容未登录的访问，也走了一套认证流程，只不过是一个匿名的身份。
- SessionManagementFilter 和session相关的过滤器，内部维护了一个SessionAuthenticationStrategy，两者组合使用，常用来防止`session-fixation protection attack`，以及限制同一用户开启多个会话的数量
- **ExceptionTranslationFilter** 直译成异常翻译过滤器，还是比较形象的，这个过滤器本身不处理异常，而是将认证过程中出现的异常交给内部维护的一些类去处理，具体是那些类下面详细介绍
- **FilterSecurityInterceptor** 这个过滤器决定了访问特定路径应该具备的权限，访问的用户的角色，权限是什么？访问的路径需要什么样的角色和权限？这些判断和处理都是由该类进行的。

### SecurityContextPersistenceFilter

SecurityContextHolder存储在HttpSession中，当用户来访问的时候，先获取安全上下文，从上下文中再获取用户的信息。

```java
public class SecurityContextPersistenceFilter extends GenericFilterBean {

   static final String FILTER_APPLIED = "__spring_security_scpf_applied";
   //安全上下文存储的仓库
   private SecurityContextRepository repo;

   public SecurityContextPersistenceFilter() {
      //HttpSessionSecurityContextRepository是SecurityContextRepository接口的一个实现类
      //使用HttpSession来存储SecurityContext
      this(new HttpSessionSecurityContextRepository());
   }

   public void doFilter(ServletRequest req, ServletResponse res, FilterChain chain)
         throws IOException, ServletException {
      HttpServletRequest request = (HttpServletRequest) req;
      HttpServletResponse response = (HttpServletResponse) res;

      if (request.getAttribute(FILTER_APPLIED) != null) {
         // ensure that filter is only applied once per request
         chain.doFilter(request, response);
         return;
      }
      request.setAttribute(FILTER_APPLIED, Boolean.TRUE);
      //包装request，response
      HttpRequestResponseHolder holder = new HttpRequestResponseHolder(request,
            response);
      //从Session中获取安全上下文信息
      SecurityContext contextBeforeChainExecution = repo.loadContext(holder);
      try {
         //请求开始时，设置安全上下文信息，这样就避免了用户直接从Session中获取安全上下文信息
         SecurityContextHolder.setContext(contextBeforeChainExecution);
         chain.doFilter(holder.getRequest(), holder.getResponse());
      }
      finally {
         //请求结束后，清空安全上下文信息
         SecurityContext contextAfterChainExecution = SecurityContextHolder
               .getContext();
         SecurityContextHolder.clearContext();
         repo.saveContext(contextAfterChainExecution, holder.getRequest(),
               holder.getResponse());
         request.removeAttribute(FILTER_APPLIED);
         if (debug) {
            logger.debug("SecurityContextHolder now cleared, as request processing completed");
         }
      }
   }
}
```

### UsernamePasswordAuthenticationFilter

<div align=center><img src="/assets/ss2.jpg"/></div>

### AnonymousAuthenticationFilter

对于Anonymous匿名身份的理解是Spirng Security为了整体逻辑的统一性，即使是未通过认证的用户，也给予了一个匿名身份。而`AnonymousAuthenticationFilter`该过滤器的位置也是非常的科学的，它位于常用的身份认证过滤器（如`UsernamePasswordAuthenticationFilter`、`BasicAuthenticationFilter`、`RememberMeAuthenticationFilter`）之后，意味着只有在上述身份过滤器执行完毕后，SecurityContext依旧没有用户信息，`AnonymousAuthenticationFilter`该过滤器才会有意义—-基于用户一个匿名身份。

