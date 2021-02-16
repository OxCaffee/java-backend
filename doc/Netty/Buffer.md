# Netty源码解析正式篇——Buffer

<!-- vscode-markdown-toc -->
* 1. [前言](#)
* 2. [Buffer基本属性](#Buffer)
* 3. [构造参数](#-1)
* 4. [Buffer的读写模式](#Buffer-1)
* 5. [_DirectBuffer_ 和 _NonDirectBuffer_ 的区别](#DirectBuffer__NonDirectBuffer_)
* 6. [Buffer基本操作](#Buffer-1)
	* 6.1. [读写模式翻转——_#flip_](#_flip_)
	* 6.2. [重置读写——_#rewind_](#_rewind_)
	* 6.3. [重置Buffer——_#clear_](#Buffer_clear_)
	* 6.4. [设置标记——_#mark_](#_mark_)
	* 6.5. [恢复标记——_#reset_](#_reset_)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name=''></a>前言

一个 Buffer ，本质上是内存中的一块，我们可以将数据写入这块内存，之后从这块内存获取数据。通过将这块内存封装成 NIO Buffer 对象，并提供了一组常用的方法，方便我们对该块内存的读写。

Buffer 在 java.nio 包中实现，被定义成抽象类，从而实现一组常用的方法。整体类图如下：

<div align=center><img src="/assets/netty01.png"/></div>

我们可以将 Buffer 理解为一个数组的封装，例如 IntBuffer、CharBuffer、ByteBuffer 等分别对应 int[]、char[]、byte[] 等。
MappedByteBuffer 用于实现内存映射文件，不是本文关注的重点。因此，感兴趣的胖友，可以自己 Google 了解，还是蛮有趣的。

##  2. <a name='Buffer'></a>Buffer基本属性

Buffer 中有 4 个非常重要的属性：capacity、limit、position、mark 。代码如下：

<div align=center><img src="/assets/netty02.png"/></div>

**Buffer中4个变量的不等式关系为：`mark`<=`position`<=`limit`<=`capacity`**

##  3. <a name='-1'></a>构造参数

<div align=center><img src="/assets/netty03.png"/></div>

##  4. <a name='Buffer-1'></a>Buffer的读写模式

Buffer可以分为读模式和写模式两种情况，如下图所示:

<div align=center><img src="/assets/netty04.png"/></div>

* `position` 属性，位置，初始值为 0 。
  * 写模式下，每往 Buffer 中写入一个值，`position` 就自动加 1 ，代表下一次的写入位置。
  * 读模式下，每从 Buffer 中读取一个值，`position` 就自动加 1 ，代表下一次的读取位置。( 和写模式类似 )
* `limit` 属性，上限。
  * 写模式下，代表最大能写入的数据上限位置，这个时候 `limit` 等于 `capacity` 。
  * 读模式下，在 Buffer 完成所有数据写入后，通过调用 `#flip()` 方法，切换到读模式。此时，`limit` 等于 Buffer 中实际的数据大小。因为 Buffer 不一定被写满，所以不能使用 `capacity` 作为实际的数据大小。
* `mark` 属性，标记，通过 `#mark()` 方法，记录当前 `position` ；通过 `reset()` 方法，恢复 `position` 为标记。
  * 写模式下，标记上一次写位置。
  * 读模式下，标记上一次读位置。

##  5. <a name='DirectBuffer__NonDirectBuffer_'></a>_DirectBuffer_ 和 _NonDirectBuffer_ 的区别

* **Direct Buffer:** 所分配的内存**不在 JVM 堆上, 不受 GC 的管理**.(但是 Direct Buffer 的 Java 对象是由 GC 管理的, 因此当发生 GC, 对象被回收时, Direct Buffer 也会被释放)。因为 Direct Buffer 不在 JVM 堆上分配, 因此 Direct Buffer 对应用程序的内存占用的影响就不那么明显(实际上还是占用了这么多内存, 但是 JVM 不好统计到非 JVM 管理的内存.)。申请和释放 Direct Buffer 的开销比较大. 因此正确的使用 Direct Buffer 的方式是在初始化时申请一个 Buffer, 然后不断复用此 buffer, 在程序结束后才释放此 buffer.
使用 Direct Buffer 时, 当进行一些底层的系统 IO 操作时, 效率会比较高, 因为此时 JVM 不需要拷贝 buffer 中的内存到中间临时缓冲区中.

* **Non-Direct Buffer:** 直接在 **JVM 堆上进行内存的分配**, **本质上是 byte[] 数组的封装**.因为 Non-Direct Buffer 在 JVM 堆中, 因此当进行操作系统底层 IO 操作中时, 会将此 buffer 的内存复制到中间临时缓冲区中. 因此 Non-Direct Buffer 的效率就较低.

##  6. <a name='Buffer-1'></a>Buffer基本操作

###  6.1. <a name='_flip_'></a>读写模式翻转——_#flip_

如果要读取 Buffer 中的数据，需要切换模式，**从写模式切换到读模式**。对应的为 `#flip()` 方法，代码如下：

<div align=center><img src="/assets/netty05.png"/></div>

###  6.2. <a name='_rewind_'></a>重置读写——_#rewind_

`#rewind`代表重置`position=0` ，下次可以重新读或者写。**大多数情况下，使用在读模式**

<div align=center><img src="/assets/netty06.png"/></div>

###  6.3. <a name='Buffer_clear_'></a>重置Buffer——_#clear_

`#clear`表示重置当前Buffer的数据，这样下次可以重新读写

<div align=center><img src="/assets/netty07.png"/></div>

###  6.4. <a name='_mark_'></a>设置标记——_#mark_

<div align=center><img src="/assets/netty08.png"/></div>

###  6.5. <a name='_reset_'></a>恢复标记——_#reset_

<div align=center><img src="/assets/netty09.png"/><div>