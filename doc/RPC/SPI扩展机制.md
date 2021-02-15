# Dubbo源码解析——SPI扩展机制

<!-- vscode-markdown-toc -->
* 1. [获取拓展配置](#)
	* 1.1. [获取扩展配置类:  _#getExtensionClasses_](#:_getExtensionClasses_)
	* 1.2. [加载获取的扩展配置类: _#loadExtensionClasses_](#:_loadExtensionClasses_)
	* 1.3. [从文件中加载: _#loadDirectory_](#:_loadDirectory_)
* 2. [获得扩展加载器 _ExtensionLoader_](#_ExtensionLoader_)
* 3. [获得指定的扩展对象](#-1)
	* 3.1. [按名获取扩展: _#getExtension_](#:_getExtension_)
	* 3.2. [创建扩展: _#createExtension_](#:_createExtension_)
	* 3.3. [反射setter注入扩展: _#injectExtension_](#setter:_injectExtension_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>获取拓展配置

###  1.1. <a name=':_getExtensionClasses_'></a>获取扩展配置类:  _#getExtensionClasses_

_#getExtensionClasses_ 用来获取**SPI扩展实现类数组**

<div align=center><img src="/assets/dubbo-spi-01.png"/></div>

`cachedClasses`属性，缓存的扩展实现类集合，它**不包含**如下的两种类型的扩展实现：
* **自适应扩展实现类** ：那些扩展`Adaptive`的实现类，会添加到`cachedAdaptiveClasses`属性中
* **带唯一参数为扩展接口的构造方法的实现类，或者是扩展Wrapper的实现类** :会添加到`cachedWrapperClasses`属性中

###  1.2. <a name=':_loadExtensionClasses_'></a>加载获取的扩展配置类: _#loadExtensionClasses_

<div align=center><img src="/assets/dubbo-spi-02.png"/></div>

###  1.3. <a name=':_loadDirectory_'></a>从文件中加载: _#loadDirectory_

<div align=center><img src="/assets/dubbo-spi-03.png"/></div>

下面总结一下加载的顺序：
1. 获取加载的文件的完整文件名，例如: `/META-INF/dubbo/internal/com.alibaba.dubbo.common.extension.ExtensionFactory`
2. 获得文件名对应的URL数组: <div align=center><img src="/assets/dubbo-spi-04.png"/></div>
3. 逐个文件URL遍历
4. 跳过当前被注释掉的行，以`#`开头
5. 按照`key=value`的配置拆分。其中 name 为拓展名，line 为拓展实现类名。注意，上文我们提到过 Dubbo SPI 会兼容 Java SPI 的配置格式，那么按照此处的解析方式，name 会为空。这种情况下，拓展名会自动生成
6. 判断扩展实现类，需要实现扩展接口
7. 添加各组件到缓存中，例如`cachedExtensionClasses`

##  2. <a name='_ExtensionLoader_'></a>获得扩展加载器 _ExtensionLoader_

<div align=center><img src="/assets/dubbo-spi-05.png"/></div>

##  3. <a name='-1'></a>获得指定的扩展对象

###  3.1. <a name=':_getExtension_'></a>按名获取扩展: _#getExtension_

<div align=center><img src="/assets/dubbo-spi-06.png"/></div>

###  3.2. <a name=':_createExtension_'></a>创建扩展: _#createExtension_

<div align=center><img src="/assets/dubbo-spi-07.png"/></div>

###  3.3. <a name='setter:_injectExtension_'></a>反射setter注入扩展: _#injectExtension_

这个方法是用来实现依赖注入的，如果被加载的实例中，有成员属性本身也是一个扩展点，则会通过 `setter` 方法进行注入。

分析到这里我们发现，所谓的扩展点，套路都一样，不管是 springfactorieyLoader，还是 Dubbo 的 spi。实际上，Dubbo 的功能 会更加强大，比如自适应扩展点，比如依赖注入。

**对于扩展点进行依赖注入，简单来说就是如果当前加载的扩展点中存在一个成员属性（对象），并且提供了 set 方法，那么这个 方法就会执行依赖注入.**

<div align=center><img src="/assets/dubbo-spi-08.png"/></div>

