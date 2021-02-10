# JVM内存区域

<!-- vscode-markdown-toc -->
* 1. [运行时数据区域](#)
* 2. [堆](#-1)
	* 2.1. [MinorGC](#MinorGC)
	* 2.2. [MajorGC](#MajorGC)
* 3. [永久代和方法区](#-1)
	* 3.1. [为什么要将永久代替换成元空间呢?](#-1)
* 4. [运行时常量池](#-1)
* 5. [直接内存](#-1)
* 6. [揭秘HotSpot虚拟机](#HotSpot)
	* 6.1. [一个Java对象的创建过程](#Java)
		* 6.1.1. [Step1: 类加载的检查](#Step1:)
		* 6.1.2. [Step2: 分配内存](#Step2:)
		* 6.1.3. [Step3: 初始化零值](#Step3:)
		* 6.1.4. [Step4: 设置对象头](#Step4:)
		* 6.1.5. [Step5: 执行init方法](#Step5:init)
	* 6.2. [对象的内存布局](#-1)
	* 6.3. [对象的访问定位](#-1)
* 7. [垃圾收集与垃圾收集器](#-1)
	* 7.1. [分代收集算法](#-1)
	* 7.2. [分区收集算法](#-1)
	* 7.3. [GC垃圾收集器](#GC)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>运行时数据区域

**JDK1.8之前**

<div align=center><img src="/assets/jvm1.png"/></div>

**JDK1.8之后**

<div align=center><img src="/assets/jvm2.png"/></div>

* **虚拟机实例**

一个程序开始运行，虚拟机就开始实例化了，多个程序启动会有多个虚拟机实例，程序关闭或者退出，虚拟机实例就会消失，多个虚拟机实例之间**数据**不能共享

* **Java线程和操作系统线程是映射关系**

Hotspot虚拟机中的Java线程和原生操作系统的线程是**映射**的关系，原生线程创建完毕之后，就会调用Java线程的run()方法

* **线程私有的**
  * **程序计数器**: 唯一一个不会出现OOM的区域
  * **虚拟机栈**: 可以粗糙的分为堆内存和栈内存，每次调用一个方法都会创建一个虚拟机栈帧(栈帧的结构要知道)
  * **本地方法栈**: 为native方法提供，注意native方法被底层而不是java虚拟机服务，可能会抛出OOM和SOF异常 
* **线程公有的**
  * **堆**: 内存当中最大的一块区域，这里的目的就是存放实例对象
  * **方法区**: 各个线程共享的内存区域，一定一定要记住这个区域是**存放类信息，常量，静态变量，即时编译器编译后的代码等数据**
  * **直接内存(非运行时数据区的一部分)**

##  2. <a name='-1'></a>堆

堆是Java虚拟机管理的最大的一块内存区域，此区域存在的唯一目的就是**存放类的实例**

Java堆是垃圾收集器管理的主要区域，现在的GC主要都采用分代收集算法，Java堆还分为Eden，From Suvivor, To Suvivor空间，这样做的目的就是**更好的回收内存，或者更快的分配内存**

**JDK7之前**

<div align=center><img src="/assets/jvm3.jfif"/></div>

**JDK7之后**

<div align=center><img src="/assets/jvm4.jfif"/></div>

**容易产生的一个误区就是: Eden区就是新生代，这样是不正确的，新生代包括Eden和两个Suvivor区**

大部分情况下，对象会在Eden区分配，但是如果对象太大，直接进入老年代，Eden区的对象和Survivor From的对象在经过一次MinorGC之后进入Survivor To, Survivor To空间不够的情况下会直接进入老年代。同时年龄加1，Survivor From和Survivor To的角色互换，也就是上一次的From变成下一次的To，当对象的年龄达到15(默认)的时候，进入老年代，要修改这个阈值，可以通过`-XX:MaxTenuringThreshold`来设置

###  2.1. <a name='MinorGC'></a>MinorGC

Eden区不够会触发MinorGC，MinorGC采用**复制算法**，流程为:

* 复制: 将Eden和from区的对象复制到to区域，(若有对象年龄到达了老年，转到old区)，同时年龄+1，年龄达到15就会被移到old区中
* 清除: 清除eden和from区的对象
* 交换: from区和to区交换，上一次的to区作为下一次GC的from区

###  2.2. <a name='MajorGC'></a>MajorGC

老年代不够会会触发MajorGC，MajorGC采用**标记清除算法**，一般老年代的GC不是很频繁，最频繁的是新生代，所以MajorGC触发之前一般会先触发MinorGC，**当新分配的对象很大时会直接进入老年代，当老年代空间不够时，会触发MajorGC再分配**

##  3. <a name='-1'></a>永久代和方法区

其实很多人都会困惑于，永久代和方法区的关系

形象地比喻他们两个之间地关系就好比**接口和实现类**之间的关系

```markdown
Java虚拟机只是提出了由方法区这个概念和它的作用，但是没有实现它(接口)

永久代则是HotSpot对于Java虚拟机方法区的实现(实现类)
```

JDK8之前设置永久代的参数

```markdown
-XX:PermSize=N
-XX:MaxPermSize=N
```

JDK8之后设置元空间的参数

```markdown
-XX:MetaspaceSize=N
-XX:MaxMetaspaceSize=N
```

**元空间与永久代的不同之处就在于: 元空间使用了操作系统的直接内存，随着对象的不断创建，可能会耗尽操作系统的内存空间，但是通过实验得到的结果是:虚拟机会调用GC来阻止这一现象的发生**

###  3.1. <a name='-1'></a>为什么要将永久代替换成元空间呢?

因为JDK8之前，永久代有上限且无法修改上限，但是使用元空间之后就有了更多可用的空间

##  4. <a name='-1'></a>运行时常量池

JDK8之前，运行时常量池是方法区的一部分，是线程间共享的，除了类的版本，字段，方法，接口等描述信息，还有常量池信息(主要是编译器生成的各种字面量和符号引用，**关于符号引用，会在动态链接阶段将符号引用替换为直接引用**

JDK8之后，**随着方法区被移除，常量池被放置在堆开辟的一段空间之内**

<div align=center><img src="/assets/jvm5.png"/></div>

##  5. <a name='-1'></a>直接内存

**直接内存是在堆之外，但是jdk8之后，直接内存也会被频繁使用，会导致OOM**

**Q: 直接内存和堆内存之间怎么交互?**

基于NIO模型，**在堆中创建一个引用到直接内存的DirectByteBuffer，可以显著提性能,避免了Java堆和native内存之间来回复制数据**

##  6. <a name='HotSpot'></a>揭秘HotSpot虚拟机

###  6.1. <a name='Java'></a>一个Java对象的创建过程

<div align=center><img src="/assets/jvm6.jpg"/></div>

####  6.1.1. <a name='Step1:'></a>Step1: 类加载的检查

虚拟机收到一个new指令，首先检查这个new的对象是否**存在于常量池中**，检查符号引用代表的类是否已经被加载，解析和初始化过，如果没有，首先先加载类

####  6.1.2. <a name='Step2:'></a>Step2: 分配内存

加载过之后，就开始为这个对象分配内存，分配的方式有“**碰撞指针**”和“**空闲列表**”，采用哪种方式根据Java堆是否规整

<div align=center><img src="/assets/jvm7.png"/></div>

**Q：分配内存面临并发问题的时候该怎么办**

* **CAS+失败重试**: CAS是乐观的
* **TLAB**: 为每个线程在Eden区分配一块内存，JVM给线程中的对象分配内存时，首先在TLAB分配，TLAB空间不够，再采用CAS分配

####  6.1.3. <a name='Step3:'></a>Step3: 初始化零值

为已分配的内存空间分配上初始值，以供直接使用

**注意!!!!!局部变量在声明的时候一定要赋初值，局部变量不在这个阶段初始化**

####  6.1.4. <a name='Step4:'></a>Step4: 设置对象头

这个实例属于哪个类，如何才能找到元空间中的类的元数据信息，对象的GC生命

####  6.1.5. <a name='Step5:init'></a>Step5: 执行init方法

上面的初始化零值是所有的对象都赋予初始零值，在这个阶段，调用`<init>`来给类变量赋予相应的值(一定不要和Step3搞混了)

###  6.2. <a name='-1'></a>对象的内存布局

没什么好说的，HotSpot规定对象的起始地址必须8字节的整数倍，因此要必要时可以通过**填充字节**来达到内存对齐的功能

###  6.3. <a name='-1'></a>对象的访问定位

```markdown
首先要先明白一个问题:

Java的程序是运行在栈上面的
对象的引用是存在于堆上面的

栈如何访问堆?????
```

**Java栈的本地本地变量表里面存在`reference`指向堆**

**1. 句柄**

<div align=center><img src="/assets/jvm8.png"/></div>

**2. 直接指针**

<div align=center><img src="/assets/jvm9.png"/></div>

**使用句柄的好处就是reference存储的地址稳定，是句柄地址**

**使用直接指针的好处就是，访问速度快，一次访问，句柄是二次访问**


##  7. <a name='-1'></a>垃圾收集与垃圾收集器

<div align=center><img src="/assets/jvm4.png"></div>

###  7.1. <a name='-1'></a>分代收集算法

* **新生代**: 复制算法
* **老年代**: 标记整理算法

###  7.2. <a name='-1'></a>分区收集算法

将整个**堆**空间分成若干个不同的小区间，每个区间独立使用，独立回收，优点是**可以一次回收多个小区间**，根据目标停顿，合理回收若干个小区间，减少GC停顿时间

###  7.3. <a name='GC'></a>GC垃圾收集器

Hostspot虚拟机GC

<div align=center><img src="/assets/HotSpot GC.png"></div>
