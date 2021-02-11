# SpringBoot源码解析—— _@ConfigurationProperties_

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [_@EnableConfigurationProperties_](#EnableConfigurationProperties_)
* 3. [_@EnableConfigurationPropertiesImportSelector_](#EnableConfigurationPropertiesImportSelector_)
	* 3.1. [_ConfigurationPropertiesBeanRegistrar_](#ConfigurationPropertiesBeanRegistrar_)
	* 3.2. [_ConfigurationPropertiesBindingPostProcessorRegistrar_](#ConfigurationPropertiesBindingPostProcessorRegistrar_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

在项目中我们会经常使用@ConfigurationProperties，通过这个注解可以将配置文件中的内容自动设置到被注解的类中，@ConfigurationProperties注解的代码如下:

```java
// ConfigurationProperties.java

@Target({ElementType.TYPE, ElementType.METHOD})
@Retention(RetentionPolicy.RUNTIME)
@Documented
public @interface ConfigurationProperties {
	
    @AliasFor("prefix")
    String value() default "";
	
    @AliasFor("value")
    String prefix() default "";

    boolean ignoreInvalidFields() default false;

    boolean ignoreUnknownFields() default true;
}
```

@ConfigurationProperties注解有两种使用方法:

1. @Component+@ConfigurationProperties
2. @EnableConfigurationProperties+@ConfigurationProperties

两者的效果是等同的

##  2. <a name='EnableConfigurationProperties_'></a>_@EnableConfigurationProperties_

`org.springframework.boot.context.properties.@EnableConfigurationProperties` 注解，可以将指定带有 `@ConfigurationProperties` 的类，注册成 BeanDefinition ，从而创建成 Bean 对象。代码如下：

```java
// EnableConfigurationProperties.java

@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
@Documented
@Import(EnableConfigurationPropertiesImportSelector.class) //见下面解析
public @interface EnableConfigurationProperties {

	//指定的类们
	Class<?>[] value() default {};
}
```

##  3. <a name='EnableConfigurationPropertiesImportSelector_'></a>_@EnableConfigurationPropertiesImportSelector_

`org.springframework.boot.context.properties.EnableConfigurationPropertiesImportSelector` ，实现 ImportSelector 接口，处理 `@EnableConfigurationProperties` 注解。代码如下：

```java
// EnableConfigurationPropertiesImportSelector.java

private static final String[] IMPORTS = {
    	//返回的两个ImportBeanDefinitionRegistrar
		ConfigurationPropertiesBeanRegistrar.class.getName(),
		ConfigurationPropertiesBindingPostProcessorRegistrar.class.getName() };

@Override
public String[] selectImports(AnnotationMetadata metadata) {
	return IMPORTS;
}
```

###  3.1. <a name='ConfigurationPropertiesBeanRegistrar_'></a>_ConfigurationPropertiesBeanRegistrar_

ConfigurationPropertiesBeanRegistrar ，是 EnableConfigurationPropertiesImportSelector 的内部静态类，实现 ImportBeanDefinitionRegistrar 接口，将 `@EnableConfigurationProperties` 注解指定的类，逐个注册成对应的 BeanDefinition 对象。代码如下：

```java
// EnableConfigurationPropertiesImportSelector#ConfigurationPropertiesBeanRegistrar.java

@Override
public void registerBeanDefinitions(AnnotationMetadata metadata, BeanDefinitionRegistry registry) {
    getTypes(metadata) // <1>调用 #getTypes(AnnotationMetadata metadata) 方法，获得 @EnableConfigurationProperties 注解指定的类的数组。
            .forEach((type) -> register(registry, (ConfigurableListableBeanFactory) registry, type)); // <2>
}
```

###  3.2. <a name='ConfigurationPropertiesBindingPostProcessorRegistrar_'></a>_ConfigurationPropertiesBindingPostProcessorRegistrar_

`org.springframework.boot.context.properties.ConfigurationPropertiesBindingPostProcessorRegistrar` ，实现 ImportBeanDefinitionRegistrar 接口，代码如下：

```java
// ConfigurationPropertiesBindingPostProcessorRegistrar.java

@Override
public void registerBeanDefinitions(AnnotationMetadata importingClassMetadata, BeanDefinitionRegistry registry) {
    if (!registry.containsBeanDefinition(ConfigurationPropertiesBindingPostProcessor.BEAN_NAME)) {
        // <1> 注册 ConfigurationPropertiesBindingPostProcessor BeanDefinition
        registerConfigurationPropertiesBindingPostProcessor(registry);
        // <2> 注册 ConfigurationBeanFactoryMetadata BeanDefinition
        registerConfigurationBeanFactoryMetadata(registry);
    }
}
```

