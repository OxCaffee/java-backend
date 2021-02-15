# Dubbo源码解析——SPI扩展机制

<!-- vscode-markdown-toc -->
* 1. [前言](#)
	* 1.1. [什么是SPI](#SPI)
	* 1.2. [Dubbo SPI使用示例](#DubboSPI)
* 2. [获取拓展配置](#-1)
	* 2.1. [获取扩展配置类:_#getExtensionClasses_](#:_getExtensionClasses_)
	* 2.2. [加载获取的扩展配置类:_#loadExtensionClasses_](#:_loadExtensionClasses_)
	* 2.3. [从文件中加载:_#loadDirectory_](#:_loadDirectory_)
* 3. [获得扩展加载器:_ExtensionLoader_](#:_ExtensionLoader_)
* 4. [获得指定的扩展对象](#-1)
	* 4.1. [按名获取扩展:_#getExtension_](#:_getExtension_)
	* 4.2. [创建扩展:_#createExtension_](#:_createExtension_)
	* 4.3. [反射setter注入扩展:#injectExtension_](#setter:injectExtension_)
* 5. [获得自适应的拓展对象](#-1)
	* 5.1. [_#getAdaptiveExtension_](#getAdaptiveExtension_)
	* 5.2. [_#createAdaptiveExtension_](#createAdaptiveExtension_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

###  1.1. <a name='SPI'></a>什么是SPI

SPI 全称为 Service Provider Interface，是一种服务发现机制。SPI 的本质是将接口实现类的全限定名配置在文件中，并由服务加载器读取配置文件，加载实现类。这样可以在运行时，动态为接口替换实现类。正因此特性，我们可以很容易的通过 SPI 机制为我们的程序提供拓展功能。SPI 机制在第三方框架中也有所应用，比如 Dubbo 就是通过 SPI 机制加载所有的组件。不过，Dubbo 并未使用 Java 原生的 SPI 机制，而是对其进行了增强，使其能够更好的满足需求。在 Dubbo 中，SPI 是一个非常重要的模块。基于 SPI，我们可以很容易的对 Dubbo 进行拓展。如果大家想要学习 Dubbo 的源码，SPI 机制务必弄懂。接下来，我们先来了解一下 Java SPI 与 Dubbo SPI 的用法，然后再来分析 Dubbo SPI 的源码。

###  1.2. <a name='DubboSPI'></a>Dubbo SPI使用示例

* [Dubbo SPI使用示例](https://dubbo.apache.org/zh/docs/v2.7/dev/source/dubbo-spi/)

##  2. <a name='-1'></a>获取拓展配置

###  2.1. <a name=':_getExtensionClasses_'></a>获取扩展配置类:_#getExtensionClasses_

_#getExtensionClasses_ 用来获取**SPI扩展实现类数组**

<div align=center><img src="/assets/dubbo-spi-01.png"/></div>

`cachedClasses`属性，缓存的扩展实现类集合，它**不包含**如下的两种类型的扩展实现：
* **自适应扩展实现类** ：那些扩展`Adaptive`的实现类，会添加到`cachedAdaptiveClasses`属性中
* **带唯一参数为扩展接口的构造方法的实现类，或者是扩展Wrapper的实现类** :会添加到`cachedWrapperClasses`属性中

###  2.2. <a name=':_loadExtensionClasses_'></a>加载获取的扩展配置类:_#loadExtensionClasses_

<div align=center><img src="/assets/dubbo-spi-02.png"/></div>

###  2.3. <a name=':_loadDirectory_'></a>从文件中加载:_#loadDirectory_

<div align=center><img src="/assets/dubbo-spi-03.png"/></div>

下面总结一下加载的顺序：
1. 获取加载的文件的完整文件名，例如: `/META-INF/dubbo/internal/com.alibaba.dubbo.common.extension.ExtensionFactory`
2. 获得文件名对应的URL数组: <div align=center><img src="/assets/dubbo-spi-04.png"/></div>
3. 逐个文件URL遍历
4. 跳过当前被注释掉的行，以`#`开头
5. 按照`key=value`的配置拆分。其中 name 为拓展名，line 为拓展实现类名。注意，上文我们提到过 Dubbo SPI 会兼容 Java SPI 的配置格式，那么按照此处的解析方式，name 会为空。这种情况下，拓展名会自动生成
6. 判断扩展实现类，需要实现扩展接口
7. 添加各组件到缓存中，例如`cachedExtensionClasses`

##  3. <a name=':_ExtensionLoader_'></a>获得扩展加载器:_ExtensionLoader_

<div align=center><img src="/assets/dubbo-spi-05.png"/></div>

##  4. <a name='-1'></a>获得指定的扩展对象

###  4.1. <a name=':_getExtension_'></a>按名获取扩展:_#getExtension_

<div align=center><img src="/assets/dubbo-spi-06.png"/></div>

###  4.2. <a name=':_createExtension_'></a>创建扩展:_#createExtension_

<div align=center><img src="/assets/dubbo-spi-07.png"/></div>

Wrapper 类同样实现了扩展点接口，但是 Wrapper 不是扩展点的真正实现。它的用途主要是用于从 ExtensionLoader 返回扩展点时，包装在真正的扩展点实现外。即从 ExtensionLoader 中返回的实际上是 Wrapper 类的实例，Wrapper 持有了实际的扩展点实现类。

扩展点的 Wrapper 类可以有多个，也可以根据需要新增。

通过 Wrapper 类可以把所有扩展点公共逻辑移至 Wrapper 中。新加的 Wrapper 在所有的扩展点上添加了逻辑，有些类似 AOP，即 Wrapper 代理了扩展点。

###  4.3. <a name='setter:injectExtension_'></a>反射setter注入扩展:#injectExtension_

这个方法是用来实现依赖注入的，如果被加载的实例中，有成员属性本身也是一个扩展点，则会通过 `setter` 方法进行注入。

分析到这里我们发现，所谓的扩展点，套路都一样，不管是 springfactorieyLoader，还是 Dubbo 的 spi。实际上，Dubbo 的功能 会更加强大，比如自适应扩展点，比如依赖注入。

**对于扩展点进行依赖注入，简单来说就是如果当前加载的扩展点中存在一个成员属性（对象），并且提供了 set 方法，那么这个 方法就会执行依赖注入.**

<div align=center><img src="/assets/dubbo-spi-08.png"/></div>

##  5. <a name='-1'></a>获得自适应的拓展对象

在 Dubbo 的代码里，常常能看到如下的代码：

```java
ExtensionLoader.getExtensionLoader(Protocol.class).getAdaptiveExtension()
```

###  5.1. <a name='getAdaptiveExtension_'></a>_#getAdaptiveExtension_

<div align=center><img src="/assets/dubbo-spi-09.png"></div>

###  5.2. <a name='createAdaptiveExtension_'></a>_#createAdaptiveExtension_

<div align=center><img src="/assets/dubbo-spi-10.png"/></div>
