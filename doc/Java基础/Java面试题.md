# Java面试题汇总

<!-- vscode-markdown-toc -->
* 1. [一个java源文件中是否可以包括多个类(不是内部类)?有什么限制?](#java)
* 2. [Java中有没有goto?](#Javagoto)
* 3. [switch是否可以作用在byte上，能否作用在long上，能否作用在String上?](#switchbytelongString)
* 4. [short s=1;s+=1;有什么错误？](#shorts1s1)
* 5. [char能否存储一个汉字?](#char)
* 6. [使用final关键字修饰一个变量的时候，是引用不能变还是引用的对象不能变?](#final)
* 7. [==和equals的区别?](#equals)
* 8. [静态变量和实例变量的区别?](#)
* 9. [是否可以从一个static方法内部发出对非static方法的调用?](#staticstatic)
* 10. [Integer和int的区别?](#Integerint)
* 11. [Math.round(11.5)等于多少?Math.round(-11.5)等于什么?](#Math.round11.5Math.round-11.5)
* 12. [请说出public,private,protected以及不写时的区别?](#publicprivateprotected)
* 13. [面向对象编程的特点?](#-1)
* 14. [Java中多态的机制是什么样的?](#Java)
* 15. [abstract class和interface的区别?](#abstractclassinterface)
* 16. [abstract class中是否可以使用static?是否可以使用native?是否可以使用synchronized?](#abstractclassstaticnativesynchronized)
* 17. [String拼接之后,原始的String对象中的内容到底变化了没有?](#StringString)
* 18. [String是否可以被继承?](#String)
* 19. [String s=new String("xyz")这条语句到底创建了多少个String对象?](#StringsnewStringxyzString)
* 20. [String和StringBuffer的区别?](#StringStringBuffer)
* 21. [String s="a"+"b"+"c"+"d"到底创建了多少String对象?](#StringsabcdString)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->

##  1. <a name='java'></a>一个java源文件中是否可以包括多个类(不是内部类)?有什么限制?

可以有多个类，但是只能有一个public的类，并且public的类名必须和文件名一致

##  2. <a name='Javagoto'></a>Java中有没有goto?

goto是Java的保留字，但是没有在java中使用

##  3. <a name='switchbytelongString'></a>switch是否可以作用在byte上，能否作用在long上，能否作用在String上?

在Java中，switch(expr1)中，expr1只能是一个整数表达式或者枚举常量(更大字体)，整数表达式可以是int基本类型或者Integer包装类型，由于byte,short,char可以隐含转换为int，因此这些类型及其这些类型的包装类型都是可以的。显然long和String类型都不符合switch的语法规定，并且不能被隐含地转换为int类型，所以他们也都不能作用于switch语句中。

##  4. <a name='shorts1s1'></a>short s=1;s+=1;有什么错误？

short变量在自增时会默认升级为int类型，然后将int赋值给short类型时，编译器将报告需要强制转换类型的错误。

##  5. <a name='char'></a>char能否存储一个汉字?

char字符是用来存储Unicode编码的字符的，Unicode编码字符集中包含了汉字，所以，char类型变量可以存储汉字。但是某些特殊的汉字并不能存储。注意，char只占2byte。

##  6. <a name='final'></a>使用final关键字修饰一个变量的时候，是引用不能变还是引用的对象不能变?

final修饰的变量，引用不能改变，引用的对象的内容可以发生改变。例如下面的语句:

```java
final StringBuffer s = new StringBuffer("immutable");
```

执行下面的语句会发生编译期错误：

```java
s = new StringBuffer("");
```

但是，执行下面的编译通过：

```java
s.append(" changed!");
```

##  7. <a name='equals'></a>==和equals的区别?

==操作符专门用来比较两个变量的值是否相等，也就是比较内存中两个对象的值是否相等，要比较两个基本类型的数据或者两个引用变量是否相等，只能用==操作符。

equals用来比较两个独立对象的内容是否相等，就好比比较两个人的长相是否相等，它比较的两个对象是独立的。例如：

```java
String a = new String("xxxx");
String b = new String("xxxx");
```

两条 new 语句创建了两个对象，然后用 a,b 这两个变量分别指向了其中一个对象，这是两个 不同的对象，它们的首地址是不同的，即 a 和 b 中存储的数值是不相同的，所以，表达式 a==b 将返回 false，而这两个对象中的内容是相同的，所以，表达式 a.equals(b)将返回 true。

在实际开发中，我们经常要比较传递进行来的字符串内容是否等，例如， String input = …;input.equals(“quit”)，许多人稍不注意就使用==进行比较了，这是错误的，随便从网上找几 个项目实战的教学视频看看，里面就有大量这样的错误。记住，字符串的比较基本上都是使用 equals 方法。

如果一个类没有自己定义 equals 方法，那么它将继承 Object 类的 equals 方法，Object 类的 equals 方法的实现代码如下： boolean equals(Object o){ return this==o; } 这说明，如果一个类没有自己定义 equals 方法，它默认的 equals 方法（从 Object 类继承的） 就是使用==操作符，也是在比较两个变量指向的对象是否是同一对象，这时候使用 equals 和使 用==会得到同样的结果，如果比较的是两个独立的对象则总返回 false。**如果你编写的类希望能 够比较该类创建的两个实例对象的内容是否相同，那么你必须覆盖 equals 方法，由你自己写代 码来决定在什么情况即可认为两个对象的内容是相同的** 。

##  8. <a name=''></a>静态变量和实例变量的区别?

在语法定义上的区别：静态变量前要加 static 关键字，而实例变量前则不加。 在程序运行时的区别：实例变量属于某个对象的属性，必须创建了实例对象，其中的实例变量 才会被分配空间，才能使用这个实例变量。静态变量不属于某个实例对象，而是属于类，所以也称为 类变量，只要程序加载了类的字节码，不用创建任何实例对象，静态变量就会被分配空间， 静态变量就 可以被使用了。总之，实例变量必须创建对象后才可以通过这个对象来使用，静态变量则可以直接 使用类名来引用。

例如，对于下面的程序，无论创建多少个实例对象，永远都只分配了一个 staticVar 变量，并 且每创建一个实例对象，这个 staticVar 就会加 1；但是，每创建一个实例对象，就会分配一个 instanceVar，即可能分配多个 instanceVar，并且每个 instanceVar 的值都只自加了 1 次。

```java
public class VariantTest{
    public static int staticVar = 0;
    public int instanceVar = 0;
    public VariantTest(){
        staticVar++;
        instanceVar++;
        System.out.println(“staticVar=” + staticVar + ”,instanceVar=” + instanceVar);
    }
}
```

##  9. <a name='staticstatic'></a>是否可以从一个static方法内部发出对非static方法的调用?

**不可以。因为非 static 方法是要与对象关联在一起的，必须创建一个对象后，才可以在该对 象上进行方法调用，而 static 方法调用时不需要创建对象，可以直接调用。**也就是说，当一个 static 方法被调用时，可能还没有创建任何实例对象，如果从一个 static 方法中发出对非 static 方法的 调用，那个非 static 方法是关联到哪个对象上的呢？这个逻辑无法成立，所以，一个 static 方法 内部发出对非 static 方法的调用。

##  10. <a name='Integerint'></a>Integer和int的区别?

int 是 java 提供的 8 种原始数据类型之一。Java 为每个原始类型提供了封装类，Integer 是 java 为 int 提供的封装类。int 的默认值为 0，而 Integer 的默认值为 null，即 Integer 可以区分出未赋 值和值为 0 的区别，int 则无法表达出未赋值的情况，例如，要想表达出没有参加考试和考试成 绩为 0 的区别，则只能使用 Integer。在 JSP 开发中，Integer 的默认为 null，所以用 el 表达式在 文本框中显示时，值为空白字符串，而 int 默认的默认值为 0，所以用 el 表达式在文本框中显示 时，结果为 0，所以，int 不适合作为 web 层的表单数据的类型。

另外，Integer 提供了多个与整数相关的操作方法，例如，将一个字符串转换成整数，Integer 中还定义了表示整数的最大值和最小值的常量。

##  11. <a name='Math.round11.5Math.round-11.5'></a>Math.round(11.5)等于多少?Math.round(-11.5)等于什么?

round 方法，它表示“四舍五入”，算法为Math.floor(x+0.5)，即将原来的数字加上0.5 后再向下取整，所以，Math.round(11.5) 的结果为 12，Math.round(-11.5)的结果为-11。

##  12. <a name='publicprivateprotected'></a>请说出public,private,protected以及不写时的区别?

|    作用域     | 当前类 | 同一package | 子孙类 | 其他package |
| :-----------: | :----: | :---------: | :----: | :---------: |
|  **public**   |  YES   |     YES     |  YES   |     YES     |
| **protected** |  YES   |     YES     |  YES   |     NO      |
| **friendly**  |  YES   |     YES     |   NO   |     NO      |
|  **private**  |  YES   |     NO      |   NO   |     NO      |

##  13. <a name='-1'></a>面向对象编程的特点?

面向对象编程的特点主要包含：封装，继承，多态和抽象4个特征。

* **封装** :封装是保证软件部件具有优良的模块性的基础，**封装的目标就是要实现软件部件的高内聚，低耦合** 。防止程序相互依赖性而带来的变动影响。在面向对象的编程语言中，对象是封装的最基本单位， 面向对象的封装比传统语言的封装更为清晰、更为有力。面向对象的封装就是把描述一个对象的属性 和行为的代码封装在一个“模块”中，也就是一个类中，属性用变量定义，行为用方法进行定义，方法可 以直接访问同一个对象中的属性。通常情况下，只要记住让变量和访问这个变量的方法放在一起， 将一个类中的成员变量全部定义成私有的， **只有这个类自己的方法才可以访问到这些成员变量** ， 这就基本上实现对象的封装，就很容易找出要分配到这个类上的方法了，就基本上算是会面向对象的编程了。把握一个原则：把对同一事物进行操作的方法和相关的方法放在同一个类中，把方 法和它操作的数据放在同一个类中。
* **抽象** :抽象就是**找出一些事物的相似和共性之处，然后将这些事物归为一个类** ，这个类只考虑这些 事物的相似和共性之处，并且会忽略与当前主题和目标无关的那些方面，将注意力集中在与当前目 标有关的方面。例如，看到一只蚂蚁和大象，你能够想象出它们的相同之处，那就是抽象。抽象包 括行为抽象和状态抽象两个方面。例如，定义一个 Person 类，如下：

```java
class Person{
    String name;
    int age;
}
```

* **继承** :在定义和实现一个类的时候，可以在一个已经存在的类的基础之上来进行，把这个已经存在的 类所定义的内容作为自己的内容，并可以加入若干新的内容，或修改原来的方法使之更适合特殊的需 要，这就是继承。继承是子类自动共享父类数据和方法的机制，这是类之间的一种关系， 提高了软 件的可重用性和可扩展性。
* **多态** :多态是指**程序中定义的引用变量所指向的具体类型和通过该引用变量发出的方法调用在编程时并不确定，而是在程序运行期间才确定，即一个引用变量倒底会指向哪个类的实例对象，该引用变量发出的方法调用到底是哪个类中实现的方法，必须在由程序运行期间才能决定** 。因为在程序 运行时才确定具体的类，这样，不用修改源程序代码，就可以让引用变量绑定到各种不同的类实现 上，从而导致该引用调用的具体方法随之改变，即不修改程序代码就可以改变程序运行时所绑定的具 体代码，让程序可以选择多个运行状态，这就是多态性。多态性增强了软件的灵活性和扩展性。例如， 下面代码中的 UserDao 是 一 个 接 口 ， 它 定 义 引 用 变 量 userDao 指 向 的 实 例 对 象由 daofactory.getDao()在执行的时候返回，有时候指向的是 UserJdbcDao 这个实现，有时候指向的 是 UserHibernateDao 这个实现，这样，不用修改源代码，就可以改变 userDao 指向的具体类实现， 从而导致 userDao.insertUser() 方法调用的具体代码也随之改变， 即有时候调用的是 UserJdbcDao 的 insertUser 方法，有时候调用的是 UserHibernateDao 的 insertUser 方法。

##  14. <a name='Java'></a>Java中多态的机制是什么样的?

靠的是父类或接口定义的引用变量可以指向子类或具体实现类的实例对象，而程序调用的方法在运行期才动态绑定，就是引用变量所指向的具体实例对象的方法，也就是内存里正在运行的那 个对象的方法，而不是引用变量的类型中定义的方法。

##  15. <a name='abstractclassinterface'></a>abstract class和interface的区别?

含有 abstract 修饰符的 class 即为抽象类，abstract 类不能创建的实例对象。含有 abstract 方 法的类必须定义为 abstract class，abstract class 类中的方法不必是抽象的。abstract class 类中定义 抽象方法必须在具体(Concrete)子类中实现，所以，不能有抽象构造方法或抽象静态方法。如果子类没有实现抽象父类中的所有抽象方法，那么子类也必须定义为 abstract 类型。

接口（interface）可以说成是抽象类的一种特例，接口中的所有方法都必须是抽象的。

* 抽象类可以有构造方法，接口中不能有构造方法。 

* 抽象类中可以有普通成员变量，接口中没有普通成员变量 

* 抽象类中可以包含非抽象的普通方法，接口中的所有方法必须都是抽象的，不能有非抽象的普通方法。 
* 抽象类中的抽象方法的访问类型可以是 public，protected 和（默认类型,虽然 eclipse 下不报错，但应该也不行），但接口中的抽象方法只能是 public 类型的，并且默认即 为 public abstract 类型。 

* 抽象类中可以包含静态方法，接口中不能包含静态方法 6. 抽象类和接口中都可以包含静态成员变量，抽象类中的静态成员变量的访问类型可以任意， 但接口中定义的变量只能是 public static final 类型，并且默认即为 public static final 类型。 7. 一个类可以实现多个接口，但只能继承一个抽象类。

##  16. <a name='abstractclassstaticnativesynchronized'></a>abstract class中是否可以使用static?是否可以使用native?是否可以使用synchronized?

* abstract 的 method 不可以是 static 的，因为抽象的方法是要被子类实现的，而 static 与子类 扯不上关系！
* native 方法表示该方法要用另外一种依赖平台的编程语言实现的，不存在着被子类实现的问 题，所以，它也不能是抽象的，不能与 abstract 混用。例如，FileOutputSteam 类要硬件打交道， 底层的实现用的是操作系统相关的 api 实现，例如，在 windows 用 c 语言实现的，所以，查看 jdk 的源代码，可以发现 FileOutputStream 的 open 方法的定义如下： `private native void open(String name) throws FileNotFoundException;` 如果我们要用 java 调用别人写的 c 语言函数，我们是无法直接调用的，我们需要按照 java 的要求写一个 c 语言的函数，又我们的这个 c 语言函数去调用别人的 c 语言函数。由于我们的c 语言函数是按 java 的要求来写的，我们这个 c 语言函数就可以与 java 对接上，java 那边的对接 方式就是定义出与我们这个 c 函数相对应的方法，java 中对应的方法不需要写具体的代码，但需 要在前面声明 native。
* synchronized 应该是作用在一个具体的方法上才有意义。而 且，方法上的 synchronized 同步所使用的同步锁对象是 this，而抽象方法上无法确定 this 是什么。

##  17. <a name='StringString'></a>String拼接之后,原始的String对象中的内容到底变化了没有?

没有。因为 String 被设计成不可变(immutable)类，所以它的所有对象都是不可变对象。在这 段代码中，s 原先指向一个 String 对象，内容是 "Hello"，然后我们对 s 进行了+操作，那么 s 所 指向的那个对象是否发生了改变呢？答案是没有。这时，s 不指向原来那个对象了，而指向了另 一个 String 对象，内容为"Hello world!"，原来那个对象还存在于内存之中，只是 s 这个引用变量 不再指向它了。 通过上面的说明，我们很容易导出另一个结论，如果经常对字符串进行各种各样的修改， 或者说，不可预见的修改，那么使用 String 来代表字符串的话会引起很大的内存开销。因为 String 对象建立之后不能再改变，所以对于每一个不同的字符串，都需要一个 String 对象来表示。这时， 应该考虑使用 StringBuffer 类，它允许修改，而不是每个不同的字符串都要生成一个新的对象。 并且，这两种类的对象转换十分容易。 至于为什么要把 String 类设计成不可变类，是它的用途决定的。其实不只 String，很多 Java 标准类库中的类都是不可变的。在开发一个系统的时候，我们有时候也需要设计不可变类，来传 递一组相关的值，这也是面向对象思想的体现。不可变类有一些优点，比如因为它的对象是只读的， 所以多线程并发访问也不会有任何问题。当然也有一些缺点，比如每个不同的状态都要一个对象来 代 表 ， 可 能 会 造 成 性 能 上 的 问 题 。 所 以 Java 标准类库还提供了一个可变版本，即 StringBuffer。

##  18. <a name='String'></a>String是否可以被继承?

String 类是 final 类故不可以继承。

##  19. <a name='StringsnewStringxyzString'></a>String s=new String("xyz")这条语句到底创建了多少个String对象?

两个或一个，”xyz”对应一个对象，这个对象放在字符串常量缓冲区，常量”xyz”不管出现多少 遍，都是缓冲区中的那一个。New String 每写一遍，就创建一个新的对象，它一句那个常量”xyz” 对象的内容来创建出一个新 String 对象。如果以前就用过’xyz’，这句代表就不会创建”xyz”自己了， 直接从缓冲区拿。

##  20. <a name='StringStringBuffer'></a>String和StringBuffer的区别?

JAVA 平台提供了两个类：String 和 StringBuffer，它们可以储存和操作字符串，即包含多个字 符的字符数据。这个 String 类提供了数值不可改变的字符串。而这个 StringBuffer 类提供的字符串 进行修改。当你知道字符数据要改变的时候你就可以使用 StringBuffer。典型地，你可以使用 StringBuffers 来动态构造字符数据。另外，String 实现了 equals 方法，`new String(“abc”).equals(new String(“abc”)` 的 结 果 为 true, 而 StringBuffer 没 有 实 现 equals 方 法 ， 所 以 `new StringBuffer(“abc”).equals(new StringBuffer(“abc”)`的结果为 false。

##  21. <a name='StringsabcdString'></a>String s="a"+"b"+"c"+"d"到底创建了多少String对象?

对于下面的代码：

```java
String s1 = "a";
String s2 = s1 + "b";
String s3 = "a" + "b";
System.out.println(s2 == "ab");	//false
System.out.println(s3 == "ab");	//true
```

实验结果表明，javac 编译可以对字符 串常量直接相加的表达式进行优化，不必要等到运行期去进行加法运算处理，而是在编译时去掉 其中的加号，直接将其编译成一个这些常量相连的结果。

因此，`String s = "a" + "b" + "c" + "d";` 只会生成一个"abcd"的字符串String对象。

