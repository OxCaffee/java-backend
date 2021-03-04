# MySQL面试题

<!-- vscode-markdown-toc -->
* 1. [MySQL的存储引擎](#MySQL)
* 2. [MySQL表的磁盘存储方式](#MySQL-1)
* 3. [InnoDB引擎的存储特性](#InnoDB)
* 4. [MyISAM存储引擎](#MyISAM)
* 5. [如何理解TEMPORARY类型的表](#TEMPORARY)
* 6. [MySQL的索引特性](#MySQL-1)
* 7. [请说说ALL,ANY和SOME子查询](#ALLANYSOME)
* 8. [如何改写用来查询非匹配(缺失)值的子查询为连接语句](#)
* 9. [说说UNION的特性](#UNION)
* 10. [如何级联删除](#-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='MySQL'></a>MySQL的存储引擎

MySQL支持多种存储引擎。每一种存储引擎所实现的表都具有一组特定的属性或者特性。在MySQL中我们最经常使用的就是InnoDB和MyISAM存储引擎。实际上MySQL的存储引擎远不止如此。见下表:

|  存储引擎  |                             描述                             |
| :--------: | :----------------------------------------------------------: |
|  ARCHIVE   |       用于数据存档的引擎(行被插入之后就不能在被修改了)       |
| BLACKHOLE  |        这种存储引擎会丢弃读写数据，读操作会返回空内容        |
|    CSV     | 这种存储引擎在存储数据的时候，会以逗号作为数据项之间的分隔符 |
| FEDERATED  |                   用来访问远程表的存储引擎                   |
| **InnoDB** |              **具备外键支持功能的事务处理引擎**              |
|   MEMORY   |                         置于内存的表                         |
|   MERGE    |               用来管理多个MyISAM表构成的表集合               |
| **MyISAM** |                 **主要的非事务处理存储引擎**                 |
|    NDB     |                    MySQL集群专属存储引擎                     |

如果我们要想查看MySQL有哪些存储引擎可以用，当然，这取决于MySQL的版本，可以通过下面的指令:

```mysql
mysql> SHOW ENGINES\G
```

或

```mysql
mysql> SELECT ENGINE FROM INFORMATION_SCHEMA.ENGINES;
```

如果我们想要查看支持事务的引擎，则可以通过下面的指令:

```mysql
mysql> SELECT ENGINE FROM INFORMATION_SCHEMA.ENGINES WHERE TRANSACTIONS = 'YES';
```

##  2. <a name='MySQL-1'></a>MySQL表的磁盘存储方式

每次在创建表的时候，MySQL都会创建一个磁盘文件，用来保存该表的格式，这个格式的文基本名与表名一致，扩展名为`.frm` 。 `frm`文件不会发生变化，因为无论管理这个表的存储引擎是哪一种，每个表都会只有一个与之对应的`.frm`文件。

各个存储引擎可能还会为表再创建几个特定的文件，用来存储表的内容，对于某个特定的表，与之相关的所有文件都会被放置到这个表的那个数据库对应的目录中。下表列出了几种由某个存储引擎创建的与表相关的文件扩展名。

| 存储引擎 |     磁盘文件扩展名      |
| :------: | :---------------------: |
|  InnoDB  |    .ibd(数据和索引)     |
|  MyISAM  |  .MYD(数据) .MYI(索引)  |
|   CSV    | .CSV(数据) .CSM(元数据) |

**默认情况下，InnoDB存储引擎会把表的数据和索引的数据存储在它的系统表空间中。也就是说，所有的InnoDB表的内容全都存储在一个共享的存储区域中，而不是某个特定的表所特有的文件里面**

##  3. <a name='InnoDB'></a>InnoDB引擎的存储特性

InnoDB存储引擎是MySQL的默认存储引擎，主要有下面的特性:

1. 其表在执行提交和回滚操作时是事务安全的，可以通过创建保存点(savepoint)来实现数据库的回滚。
2. 在系统崩溃之后可以自动恢复
3. 外键和引用完整性支持，包括级联删除和更新
4. 基于行级别的锁定和多版本化，使得在执行同时包含有检索和更新操作的组合条件查询时，可以表现出很好的并发性能。
5. 从MySQL5.6版本开始，InnoDB支持全文检索和FULLTEXT检索
6. 默认情况下，InnoDB存储引擎会把表集中存储在一个系统表空间中，而不是像其他存储引擎那样为不同的表创建不同的文件。**实际上，InnoDB存储引擎会把表空间当成是一个虚拟的文件系统，它会在其中存储和管理所有InnoDB表的内容，这样一来，表的长度就可以超过文件系统对单个文件的最大长度限制。同时，你也可以配置InnoDB使它为每一个表分别创建一个表空间，这样每一个表都会有.ibd文件**

##  4. <a name='MyISAM'></a>MyISAM存储引擎

MyISAM是MySQL**主要的非事务存储引擎** ，其特性如下:

1. 当保存连续相似的字符串索引值时，它会对键进行压缩，此外MyISAM还可以压缩相似的数字索引值，因为这些数值都是以高字节为优先的方式来进行保存的。 **MyISAM以低字节优先表示索引值的时候，检索速度更快，以高字节为优先表示时，更易于压缩** 。如果想启动MyISAM的数字压缩功能，在创建MyISAM的时候使用`PACK_KEYS=1`选项。
2. 与其他存储引擎相比，MyISAM为`AUTO_INCREMENT`提供了更多的功能。
3. 每个MyISAM都有一个标志，它会在执行表检查操作时被设置。MyISAM表还会有一个标志，用于表明该表在上次使用后是否被正常关闭，如果服务器发生意外的宕机，可以用这个标志来检查是否需要修复。如果想进行自动检查，则需要在启动服务器的时候，将`myisam_revover_options`系统变量设置为一个包含有`FORCE`选项的值，这样，在每次打开MyISAM表的时候，服务器便会自动检查该表的标志，并根据需要进行表的修复。
4. 支持全文检索和FULLTEXT索引
5. 支持空间数据类型和SPATIAL索引

##  5. <a name='TEMPORARY'></a>如何理解TEMPORARY类型的表

TEMPORAY表字如其名，是属于临时表，**其存储在的时间范畴仅在服务器会话存活的时间** ，具体创建的语句如下:

```mysql
mysql> CREATE TEMPORARY TABLE t (...)
```

TEMPORARY表与某个已有的永久表的名字相同，已经存在的永久表的内容也不会遭到损坏。

##  6. <a name='MySQL-1'></a>MySQL的索引特性

索引时加快访问表内容的基本手段，尤其是在涉及多个表的关联查询里面，这是一个非常重要的话题。MySQL提供了多个灵活的索引创建办法:

1. 可以对单个列或者多个列建立索引，多列索引也被称为复合索引
2. 索引可以只包含唯一值，也可以包含重复值
3. 可以为同一个表创建多个索引，帮助优化对表的不同类型的查询
4. 对于除`ENUM`和`SET`以外的字符串类型，可以利用列的前缀创建索引，也就是说，只对列名的最左边n个字符或者n个字节创建索引(对于BLOB和TEXT列，只有在制定了前缀长度的情况下才能创建索引) 。如果要在前缀长度范围内，大部分的列都具有唯一性，那么查询性能通常不会受到影响，并且还可能得到改善，具体表现在，为列的前缀而不是整个列进行索引，可以让索引变得更小，访问速度更快

**然后并非所有的MySQL存储引擎都支持索引功能** ，不同MySQL的存储引擎的特性见下表:

|        索引特性        |   InnoDB   | MyISAM | MEMORY |
| :--------------------: | :--------: | :----: | :----: |
| 索引列是否允许使用NULL |     Y      |   Y    |   Y    |
|  每个索引的列数(最大)  |     16     |   16   |   16   |
|     每个表的索引数     |     64     |   64   |   64   |
| 索引行的最大长度(字节) |    3072    |  1000  |  3072  |
|    是否支持前缀索引    |     Y      |   Y    |   Y    |
| 列前缀的最大长度(字节) |    767     |  1000  |  3072  |
| 是否支持BLOB/TEXT索引  |     Y      |   Y    |   N    |
|  是否支持FULLTEXT索引  | 5.6.4版本+ |   Y    |   N    |
|  是否支持SPATIAL索引   |     N      |   Y    |   N    |
|    是否支持HASH索引    |     N      |   N    |   Y    |

要想创建索引，我们可以通过下面的语句:

```mysql
mysql> CREATE INDEX index_name ON tbl_name (index_columns);
mysql> CREATE UNIQUE INDEX index_name ON tbl_name (index_columns);
mysql> CREATE FULLTEXT INDEX index_name ON tbl_name (index_columns);
mysql> CREATE SPATIAL INDEX index_name ON tbl_name (index_columns);
```

##  7. <a name='ALLANYSOME'></a>请说说ALL,ANY和SOME子查询

运算符ALL和ANY常与某个关系比较运算符结合在一起使用，以便测试列子查询的结果。他们会测试比较值与子查询返回的全部或者部分值是否匹配。 **注意ALL,ANY,SOME仅在子查询的情况下使用**

例如当我们想要查找某个**出生日期最早的那个人的信息** ，可以用下面的语句:

```mysql
mysql> SELECT name, birth FROM president
    -> WHERE birth <= ALL (SELECT birth FROM president);
```

当ALL, ANY, SOME与比较运算符"="配合使用的时候，子查询可以是表子查询，此外，需要使用一个**行构造器**来提供比较值:

```mysql
mysql> SELECT name, city, state FROM president
    -> WHERE (city, state) = ANY
    -> (SELECT city, state FROM president
    -> WHERE name = 'Robot');
```

**需要提一点的是，IN和NOT IN是=ANY和<>ANY的同义词，SOME是ANY的同义词**

##  8. <a name=''></a>如何改写用来查询非匹配(缺失)值的子查询为连接语句

比如我们想要测试哪些学生没有出现在absence表中:

```mysql
mysql> SELECT * FROM student
    -> WHERE student_id NOT IN (SELECT student_id FROM absence);
```

上述语句使用到了子查询，若想改成连接语句，可以使用下面的LEFT JOIN来实现:

```mysql
mysql> SELECT student.* FROM student
    -> LEFT JOIN absence ON student.student_id = absence.student_id
    -> WHERE absence.student_id IS NULL;
```

##  9. <a name='UNION'></a>说说UNION的特性

UNION有以下几种特性:

1. **列名和数据类型** :UNION结果集里的列名来自第一个SELECT里的列名。UNION中的第二个及后面的SELECT必须选取 相同个数的列，但各列可以有不同的列名和数据类型， **但是出于严谨的目的，数据类型最好相同**
2. **重复行的处理** :在默认情况下，MySQL使用UNION时会将结果集里的重复行剔除，默认情况下，UNION也是DISTINCT UNION的同义词
3. **ORDER BY和LIMIT处理** :如果想将UNION结果作为一个整体进行排序，那么需要用括号把每一个SELECT语句括起来，并在最后的那个SELECT语句后面再加上一个ORDER BY子句。因为UNION会使用第一个SELECT语句里的列名。所以ORDER BY子句必须引用那些句子，而不能引用最后那个SELECT语句。例如：

```mysql
mysql> (SELECT i, c FROM t1) UNION (SELECT i, d FROM t3) ORDER BY c;
```

```mysql
mysql> (SELECT * FROM a) UNION (SELECT * FROM b) UNION (SELECT * FROM c) LIMIT 2;
```

子句ORDER BY和LIMIT还可以用在被括号括起来的单个SELECT里，从而**只会对这条SELECT语句起作用** :

```mysql
mysql> (SELECT * FROM t1 ORDER BY i LIMIT 2)
    -> UNION (SELECT * FROM t2 ORDER BY j LIMIT 2)
    -> UNION (SELECT * FROM t2 ORDER BY d LIMIT 2);
```

##  10. <a name='-1'></a>如何级联删除

如果要从表t1中删除存在于表t2中的id的所有行，可以用如下语句:

```mysql
mysql> DELETE t1 FROM t1 INNER JOIN t2 ON t1.id = t2.id;
```

如果要同时删除t1, t2中含相同id的所有行，可以用如下语句：

```mysql
mysql> DELETE t1, t2 FROM t1 INNER JOIN t2 ON t1.id = t2.id;
```

如果想要删除t1中不存在于t2的id所有行，可以使用如下语句：

```mysql
mysql> DELETE t1 FROM t1 LEFT JOIN t2 ON t1.id = t2.id WHERE t2.id = NULL;
```

**总之，DELETE搭配连接可以完成几乎所有的级联删除操作**

## 

