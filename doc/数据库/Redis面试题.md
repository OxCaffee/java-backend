# Redis面试题

<!-- vscode-markdown-toc -->
* 1. [Redis的特点?](#Redis)
* 2. [为什么要把Redis所有的数据放在内存当中?](#Redis-1)
* 3. [Redis常见的性能问题有哪些?](#Redis-1)
* 4. [Redis最适合的场景?](#Redis-1)
* 5. [Memcache和Redis的区别?](#MemcacheRedis)
* 6. [Redis有哪几种数据结构?](#Redis-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='Redis'></a>Redis的特点?

Redis 是由意大利人 Salvatore Sanfilippo（网名：antirez）开发的一款内存高速缓存数据库。 Redis 全称为：Remote Dictionary Server（远程数据服务），该软件使用 C 语言编写，典型的 NoSQL 数据库服务器，Redis 是一个 key-value 存储系统，它支持丰富的数据类型，如：string、 list、set、zset(sorted set)、hash。Redis 本质上是一个 Key-Value 类型的内存数据库，很像 memcached，整个 数据库统统加载在内存当中进行操作，定期通过异步操作把数据库数据 flush 到硬盘 上进行保存。因为是纯内存操作，Redis 的性能非常出色，每秒可以处理超过 10 万次 读写操作，是已知性能最快的 Key-Value DB。Redis 的出色之处不仅仅是性能，Redis 最大的魅力 是支持保存多种数据结构，此外单 个 value 的最大限制是 1GB，不像 memcached 只能保存 1MB 的数据，另外 Redis 也可以对存入的 Key-Value 设置 expire 时间。Redis 的主要缺点是 数据库容量受到物理内存的限制，不能用作海量数据的高性能读写，因此 Redis 适合的场景主 要局限在较小数据量的高性能操作和运算上。

##  2. <a name='Redis-1'></a>为什么要把Redis所有的数据放在内存当中?

Redis 为了达到最快的读写速度将数据都读到内存中，并通过异步的方式将数据写入磁盘。 所以 redis 具有快速和数据持久化的特征。如果不将数据放在内存中，磁盘 I/O 速度为严重影响 redis 的性能。在内存越来越便宜的今天，redis 将会越来越受欢迎。如果设置了最大使用的内存， 则数据已有记录数达到内存限值后不能继续插入新值。

##  3. <a name='Redis-1'></a>Redis常见的性能问题有哪些?

* Master写内存快照，save命令调度rdbSave，会阻塞主线程，当快照比较大的时候对性能的影响是非常大的，会间断性暂停服务，所以Master最好不要写快照
* Master AOF持久化，如果不重写AOF文件，这种方式对性能影响是比较小的，但是随着AOF体积的增大，会影响Master的重启速度，因此Master最好不要做任何持久化的工作，包括内存快照和AOF日志文件，特别是不要启用内存快照做持久化，如果数据比较关键，可以采用Slave开启AOF备份数据，策略为每秒同步一次
* Master调用REWRITEAOF重写AOF文件，AOF会在重写的时候大量占用CPU资源和内存资源，导致服务load过高，出现短暂的暂停服务现象
* 主从复制问题，为了提高主从复制的速度和连接的稳定性，Slave和Master最好在同一个局域网内部

##  4. <a name='Redis-1'></a>Redis最适合的场景?

* 会话缓存
* 全页缓存
* 队列
* 排行榜
* 计数器
* 发布/订阅

##  5. <a name='MemcacheRedis'></a>Memcache和Redis的区别?

* 存储方式不同：Memcache 是把数据全部存在内存中，数据不能超过内存的大小，断电 后数据库会挂掉。Redis 有部分存在硬盘上，这样能保证数据的持久性
* 数据支持的类型不同 memcahe 对数据类型支持相对简单，redis 有复杂的数据类型
* 使用底层模型不同 它们之间底层实现方式 以及与客户端之间通信的应用协议不一样。 Redis 直接自己构建了 VM 机制 ，因为一般的系统调用系统函数的话，会浪费一定的时间去移动和请求
* 支持的 value 大小不一样 redis 最大可以达到 1GB，而 memcache 只有 1MB

##  6. <a name='Redis-1'></a>Redis有哪几种数据结构?



