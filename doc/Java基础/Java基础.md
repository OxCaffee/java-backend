# Java基础

* Java类是单继承但是接口可以多继承

```java
interface1 extends interface2, interface3
```

* 为什么Java只允许类的单继承?

```markdown
因为A同时继承类B和类C，而B和C都有一个D方法，那么A不知道该继承哪一个方法
```

* 多重继承是通过哪些方式实现的?

```markdown
1. 扩展一个类并实现一个或者多个接口
2. 实现两个或者更多的接口
```

* 类A和类B都在同一个包中，类A有个protected方法a()，类B不是类A的子类，类B可以访问类A中的方法a()吗?

```markdown
1. public: 可以被其他所有的类访问
2. private: 只能被自己访问和修改
3. protected: 自身，子类和同一个包中可以访问
4. default: 同一个包中的类可以访问，声明时没有加修饰符，认为是friendly
```

* (判断题): 用户不能调用构造方法，只能通过new关键字自动调用

```markdown
错误: 
1. 在类的其他方法中可以调用this()
2. 在类的子类中可以调用super()
3. 在反射中可以调用newInstance()调用
```

* 多态的表现形式和根本方法

```markdown
表现形式: 重写
根本方法: 继承
```

* 以下程序的执行结果为:

```java
static boolean foo(char c){
    System.out.println(c);
    return true;
}

public static void main(String[] args){
    int i = 0;
    for(foo('A'); foo('B') && (i < 2); foo('C')){
        i++;
        foo('D');
    }
}
```

```markdown
起始条件可以不为boolean类型
结果为:
ABDCBDCB
```

* 说说有关内部类的问题

```markdown
1. 成员内部类可以访问外部所有的资源但是自身不能定义静态资源
2. 静态内部类可以访问外部类的静态变量但是不可以访问非静态变量
3. 局部内部类往往定义在方法体中，只能访问代码块或者方法中被定义为final的变量
4. 匿名内部类不能使用class, implements, extends，没有构造方法
```

* 谈谈子类的构造函数

```markdown
1. 子类不能继承父类的构造器
2. 在创建子类的时候，如果子类中不含带参的构造函数，先执行父类的构造函数，然后执行自己的无参构造函数
```

* 下列代码的执行结果为:

```java
public static void main(String[] args){
    String s = "com.jd.".replaceAll(".", "/") + "MyClass.class"
}
```

```markdown
执行结果为:
///////MyClass.class

. 代表任何字符，这是一个坑
```

* 哪些接口直接继承自Collection接口

```markdown
             |---- List
Collection-------- Set
             |---- Queue
```

* 以下代码可以使用的修饰符是:

```java
abstract interface Status{
    /** Code here **/ int value = 10;
}
```

```markdown
1. public
2. final
3. static

接口字段的默认修饰符: public static final
方法字段的默认修饰符: public abstract
```

* 下面哪些方法是Object类的

```markdown
clone()
toString()
wait()
finalize()
```

* 下列哪些方法的定义是不正确的:

```java
public class A{
    float func0(){
        byte i = 1;
        return i;
    }

    float func1(){
        int i = 1;
        return;
    }

    float func2(){
        short i = 1;
        return i;
    }

    float func3(){
        long i = 3;
        return i;
    }

    float func4(){
        double i = 1;
        return i;
    }
}
```

```markdown
func1: 没有return value
func4: double无法assign给float

容易出错的点是func3: long可以assign给float
```

* Java程序的种类有:

```markdown
Applet: 嵌入在浏览器中的小程序
Servlet: web程序
Application
```

* Java局部变量如果不赋初始值会有默认值

```markdown
错误: 局部变量必须要有初始值，否则会报错
```

* 所有的异常类都直接继承于哪一个类?

```markdown
所有的异常类都直接继承于java.lang.Exception
所有的异常类都继承于Throwable
```

* 下列代码执行结果为:

```java
public class Demo{
    public static void main(String[] args){
        Integer i1 = 128;
        Integer i2 = 128;
        System.out.println((i1 == i2) + ", ");

        String i3 = "100";
        String i4 = "1" + new String("00");
        System.out.println((i3 == i4) + ", ");

        Integer i5 = 100;
        Integer i6 = 100;
        System.out.println((i5 == i6));
    }
}
```

```markdown
false, false, true

当我们在为Integer赋值的时候，会自动调用Integer.valueOf()
1. 对于-128~127之间的数字，Java会对其进行缓存，超过这个范围创建新的对象
2. 对于i3和i4，在编译期会在字符串常量池中创建一个"100"常量，然后创建一个"1"常量，但是在运行时才会创建"00"，将其拼接创建一个新的String对象"100"，i3和i4不同
```

* 下面程序代码的执行结果为:

```java
Boolean flag = false;

if(flag = true){
    System.out.println("true");
}else{
    System.out.println("false");
}
```

```markdown
执行结果为: true

if(flag = true) ==> if((flag = true) == true)条件为true
```

* 下面代码的执行结果为:

```java
public class Test{
    static String x = "1";
    static int y = 1;

    public static void main(String[] args){
        static int z = 2;
        System.out.println(x + y + z);
    }
}
```

```markdown
执行结果: 编译错误

static修饰类成员变量不能修饰局部变量
```

* 假定str0, ..., str4后续代码都是只读引用，Java7中，以上代码为基础，在发生过一次FullGC后，上述代码在Heap空间(不包括PermGen)的保留字符数为:

```java
static String str0 = "0123456789";
static String str1 = "0123456789";
String str2 = str1.substring(5);
String str3 = new String(str2);
String str4 = new String(str3.toCharArray());
str0 = null;
```

```markdown
Java垃圾回收主要针对堆区的回收，因为栈区的内存是随着线程而释放的，堆区主要分为三个区:
1. Young Gen: 对象被创建后通常放在Yound Gen，除了一些非常大的对象，经过一定的Minor GC还活着的对象被移到Old Gen
2. Old Gen: 从Young Gen移动过来的游戏额比较大的对象，Minor GC(Full GC)针对年老代的回收
3. Perm Gen(Java8改Meta Space): 存储的是final变量，static变量，常量池

str0和str1在编译时创建"0123456789"保存到常量池，即Perm Gen中
substring的本质还是会new一个String对象，str2为5 char，str3为5 char，str4也是创建一个新的对象为5 char，经过FullGC(Minor GC)之后
综上所述，不包含Perm Gen的一共有5 + 5+ 5 = 15 char
```

* 下面代码的执行结果为:

```java
byte b1 = 1, b2 = 2, b3, b6;
final byte b4 = 4, b5 = 6;
b6 = b4 + b5;
b3 = (b1 + b2);
System.out.println(b3 + b6);
```

```markdown
byte在运算的时候会自动提升至int类型
运算结果是int类型无法再赋给byte类型，要进行类型的强制转换
除此之外，short, char在计算的时候也会自动提升至int类型
```

* Java的关键字只能由数字字母和下划线组成

```markdown
错误
还有$也可以作为标识符的组成部分
```

* Java类名必须和文件名一致

```markdown
错误
内部类名可以不一样
```

* 下列流中，属于处理流的是:

```java
FileInputStream
InputStream
DataInputStream
BufferdInputStream
```

```markdown
区分节点流，处理流
1. 节点流: 从或者向一个地方读写数据
    Input/Output/Stream
    FileInput/Output/Stream
    String/Reader/Writer
    ByteArrayInputStream/Out
    Piped/Input/Output/Stream(Reader/Writer)
2. 处理流(处理节点流的流)
    Buffered...
    Input/Output/Stream/Reader/Writer
    Data/Input/Output/Stream
```

* 根据如下继承关系判断哪些是对的哪些是错的

```java
class A{}
class B extends A{}
class C extends A{}
class D extends B{}
```

```markdown
List = List<A>                          正确: 点到范围
List<A> = List<B>                       错误: 点到点
List<?> = List<Object>                  正确: 点到范围
List<? extends B> = List<D>             正确: 点到范围
List<A> = List<? extends A>             错误: 范围到点
List<? extends A> = List<? extends B>   正确: 小于A的范围小于小于B的范围
```

* 那个修饰符是所有同一个类生成的对象共享?

```markdown
static
static修饰的某个字段只有一个存储空间，所有的实例共享
```

* 下面为true的是

```java
Integer i = 42;
Long l = 42l;
Double d = 42.0;
```

```markdown
(i == l)        false: 不同类型的引用比较，编译错误
(i == d)        false: 同上
(l == d)        false: 同上
i.equals(d)     false: 包装类的equals()不处理数据转型
d.equals(l)     false: 同上
i.equals(l)     false: 同上
l.equals(42l)   true 
```

* 下面一段代码，当T分别为引用类型和值类型的时候，分别产生了多少个T对象

```java
T t = new T();
Func(t);

public void Func(T t){}
```

```markdown
引用类型作为参数的时候，传递的是引用对象的地址，不产生新的对象
值类型作为参数的时候，传递的是对象的值副本，产生了一个新的对象

答案为: 1, 2
```

* 下列代码的输出为:

```java
public class P{
    public static int abc = 123;

    static{
        System.out.println("P is int");
    }
}

public class S extends P{
    static{
        System.out.println("S is int");
    }
}

public class Test{
    public static void main(String[] args){
        System.out.println(S.abc);
    }
}
```

```markdown
虚拟机严格规定了有且只有5中情况必须立即对类进行“初始化”:

1. 使用new关键字，读取或者设置一个类的静态字段的时候，或者调用一个类的静态方法的时候
2. 使用java.lang.reflect包的方法对类进行反射调用的时候，如果类没有被初始化，必须对其进行初始化
3. 当初始化一个类的时候，如果发现其父类没有被初始化就先初始化它的父类
4. 当虚拟机启动的时候，用户需要指定一个要执行的主类(包含main)，虚拟机会先初始化这个类
5. 使用jdk1.7动态语言支持的时候的一些情况

而属于被动引用不会触发子类的初始化
1. 子类引用父类的静态字段，只会出发子类的加载，父类的初始化，不会导致子类的初始化
2. 通过数组定义来引用类，不会触发此类的初始化
3. 常量在编译阶段被存入调用类的常量池，本质上没有引用到定义常量的类，因此不会触发定义常量的类的初始化
```

* 判断下列关于HashMap和HashTable的说法是否正确

```markdown
1. HashMap是线程不安全的，HashTable是线程不安全的(正确)
2. HashTable键和值均不允许null，HashMap也是(错误)： HashMap允许值为null
3. HashMap通过get()可判断是否含有键(错误): HashTable是的
```

* final修饰的变量不允许被赋值?

```markdown
final修饰的属性值已经固定，无法再被赋值
```

* 假设下面两个赋值语句

```java
a = Integer.parseInt("1024");
b = Integer.valueOf("1024").intValue();
```

判断a和b的类型

```markdown
a和b都是int

intValue()是把Integer变成int
parseInt()是把String变成int
valueOf()是把String转化为Integer
```

* off-heap指的是哪种内存

```markdown
JVM的内存结构:

1. 程序计数器: 几乎不占用内存，用于取下一条执行的指令
2. 堆: 所有通过new生成的对象全部都放在heap中
3. 栈: 每个线程执行每个方法的时候都会在栈中申请一个栈帧，每个栈帧包含局部变量区和操作数栈，用于存放此方法调用过程中的临时变量，参数和中间结果
4. 本地方法栈: 用于支持native方法的执行，存储了每个native方法调用的状态
5. 方法区: 存放了主要加载的类信息，静态变量，final类型的常量，属性和方法信息，JVM用永久代来存放方法区

off-heap意味着把内存对象分配在Java虚拟机的堆以外的内存当中，这些内存直接受操作系统管理，JVM负责回收方法区和堆，因此off-heap指的是进程管理的内存
```

* 下面的import语句可以访问的空间是:

```java
import java.util.*;
```

```markdown
能访问java.util目录下的所有类，不能访问java.util子目录下的所有类
```

* `URL url = new URL("http://www.123.com")`，如果www.123.com不存在，返回的结果是:

```markdown
URL url = new URL("XXX");

如果XXX不符合URL的格式会抛出异常，但是除此之外，返回的就只有一个URL链接，不管目标地址是否存在

例如:
URL url = new URL("http://www.123.com")
返回的是
http://www.123.com
```

* 写出下列语句1和语句2的执行结果

```java
public class Test{
    public static void main(String[] args){
        String s = "tommy";
        Object o = s;
        sayHello(o);    //语句1
        sayHello(s);    //语句2
    }

    public static void sayHello(String to){
        System.out.println(String.format("Hello, %s", to));
    }

    public static void sayHello(Object to){
        System.out.println(String.format("Welcome, %s", to));
    }
}
```

```markdown
语句1: Welcome, tommy
语句2: Hello, tommy
```

* `static abstract void f1();`是否正确?

```markdown
错误

abstract不能和final, private, static同时使用
abstract方法的默认修饰符是public
```

* `null, false, null, friendly, sizeof`是不是java关键字?

```markdown
不是java关键字，是java保留字
```

* 下面的switch语句中，x可以是哪些类型的数据:

```java
switch(x){
    default:
        System.out.println("Hello");
}
```

```markdown
在Java7之前: byte, short, char, int
在Java7之后: String, byte, short, int, char, Enum
```

* 往OuterClass类的代码段中插入内部类声明，哪一个是错误的:

```java
public class OuterClass{
    private float f = 1.0f;
    //插入代码段
}

//1
class InnerClass{
    public static float func(){
        return f;
    }
}

//2
abstract class InnerClass{
    public abstract float func(){}
}

//3
static class InnerClass{
    protected static float func(){
        return f;
    }
}

//4
public class InnerClass{
    static float func(){
        return f;
    }
}
```

```markdown
1错误: 成员内部类不能有任何static方法或者字段
2错误: abstract没有方法体
3错误: 静态内部类不能访问外部非静态变量
4错误: 同1
```

* 下面代码执行之后cnt的值是:

```java
public class Test{
    static int cnt = 6;

    static{
        cnt += 9;
    }

    static{
        cnt /= 3;
    }

    public static void main(String[] args){
        System.out.println("cnt = " + cnt);
    }
}
```

```markdown
这道题很重要，要知道静态代码块和静态成员的初始化和执行顺序

执行顺序如下:
1. 父类静态成员和静态代码块，按照在代码中出现的顺序依次执行
2. 子类静态成员和静态代码块，按照在代码中出现的顺序依次执行
3. 父类实例成员和实例初始化块，按照在代码中出现的顺序依次执行
4. 执行父类构造函数
5. 子类实例成员和实例初始化块，按照在代码中出现的顺序依次执行
6. 执行子类构造函数

其中要注意的是，静态成员变量和静态代码块是同一个级别的，有自己出现的顺序
```

* 下面代码的执行顺序是:

```java
public class IfTest{
    public static void main(String[] args){
        int x = 3;
        int y = 1;
        if(x = y)
            System.out.println("Not Equal");
        else
            System.out.println("Equal");
    }
}
```

```markdown
输出结果: Not Equal

if(x = y) ==> if((x = y) == true)
```

* 下面程序运行的结果是:

```java
public static void main(String[] args){
    Thread t = new Thread(){
        public void run(){
            do();
        }
    };

    t.run();
    System.out.println("b");
}

public static void do(){
    System.out.println("a");
}
```

```markdown
输出结果: ab

原因: 代码中是t.run()，只是简单的方法调用，并不是启动线程，
若是t.start()，执行的结果可能是ab也可能是ba
```

* 以下程序执行之后将会有多少个字节被写入到a.txt中

```java
try{
    FileOutputStream fos = new FileOutputStream("a.txt");
    DataOutputStream dos = new DataOutputStream(fos);

    dos.writeInt(3);
    dos.writeChar(1);
    
    dos.close();
    fos.close();
}catch(Exception e){
    e.printStackTrace();
}
```

```markdown
Java采用Unicode编码，一个char占两个字节，一个int占4个字节，一共六个字节
```

* 判断下面说法是否正确

```markdown
异常分为Error和Exception                        正确
Throwable是所有异常类的父类                      正确
Exception是所有异常类的父类                      错误
Exception包括RuntimeException和除此之外的异常    正确
```

* 以下哪个选项不属于java类加载的过程

```markdown
A. 生成java.lang.Class对象
B. int类型对象成员变量赋予默认值
C. 执行static块代码
D. 类方法的解析
```

```markdown
Java类加载的过程如下:

加载，验证，准备，解析，初始化

B对象成员变量的初始化是在实例化对象的时候才赋予默认值的
而类成员变量的赋值是在类加载的过程中进行的
```

* 当我们都需要所有的线程都执行到某处，才进行后面代码的执行，应该使用哪一个?

```markdown
1. CountDownLatch
2. CyclicBarrier
```

```markdown
CyclicBarrier是所有的线程都执行完毕之后，才执行后面的操作
而CountDownLatch可以使所有的线程都执行到某一处的时候才进行后面的操作
```

* Java抽象类能不能有构造方法?

```markdown
可以
```

* 直接调用Thread的run()是否会报错?

```markdown
不会
```

* ThreadLocal用于创建线程的本地变量，变量是不是线程间共享的？

```markdown
ThreadLocal存放的值是线程内共享，线程间互斥的，主要用于线程内共享一些数据，避免通过参数来进行传递
```

* Java每个中文字符占用2个字节，每个英文字符占用一个字节?

```markdown
错误

Java一律采用Unicode编码，不论是中文字符还是英文字符均占用2个字节
```

* Java的char类型是怎么来存放的?

```markdown
Java的char类型，通常是以UTF-16 Big Endian存放的
```

* 下列赋值语句正确的是:

```java
double d = 5.3e12;
float f = 11.1;
int i = 0.0;
Double oD = 3;
```

```markdown
1. 正确
2. 11.1默认为double，需要强制转换
3. 0.0转int也需要强制转换
4. 包装类型的数值必须严格对应
```

* (这题我服了)以下代码执行的结果是多少

```java
public static void main(String[] args){
    int count = 0;
    int num = 0;
    for(int i = 0; i <= 100; i++){
        num = num + i;
        count = count++;
    }
    System.out.println("num * count = " + (num * count));
}
```

* 关于以下访问权限的说明正确的是:

```markdown
1. 外部类定义前面可以修饰public, protected和private
2. 局部内部类前面可以修饰public, protected和private
3. 匿名内部类前面可以修饰public, protected和private
4. 以上说法都不正确
```

```markdown
1. 错误: 外部类放在包中，只有可见和不可见之分，即public和default
2. 错误: 局部内部类是定义在方法里面的，不能有public, protected和private
3. 错误: 匿名内部类不能有访问修饰符和static修饰符的

所以
4. 正确
```

* (很重要)给出下面代码的运行结果

```java
class Two{
    Byte x;
}

class Pass0{
    public static void main(String[] args){
        Pass0 p = new Pass0();
        p.start();
    }

    void start(){
        Two t = new Two();
        System.out.println(t.x + " ");

        Two t2 = fix(t);
        System.out.println(t.x + " " + t2.x);
    }

    Two fix(Two tt){
        tt.x = 42;
        return tt;
    }
}
```

```markdown
基本类型和String=""创建的字符串
传递给方法的参数的时候是值传递
其余情况为引用传递

所以，输出为:
null 42 42
```

* 谈谈有关finalize()方法

```markdown
GC Roots失去引用链，第一次标记，调用finalize()，之后由Finalizer线程执行是否可重新到达GC Roots，如果能到，移出队列，如果不能到，第二次标记，回收内存
```

* 下列程序运行的结果为:

```java
public static void main(String[] args){
    Object o1 = true ? new Integer(1) : new Double(2.0);
    Object o2;

    if(true){
        o2 = new Integer(1);
    }
    System.out.println(o1);
    System.out.println(" ");
    System.out.println(o2);
}
```

```markdown
1.0 1

三元操作符类型提升，Integer提升为Double
```

* 下列程序的输出是:

```java
class Person{
    String name = "No name";

    public Person(String name){
        this.name = name;
    }
}

class Employee extends Pereson{
    String emId = "0000";

    public Employee(String emId){
        this.emId = emId;
    }
}

public static void main(String[] args){
    Employee employee = new Employee("123");
    System.out.println(employee.emId);
}
```

```markdown
子类创建的过程中会首先调用父类的默认构造函数
如果父类没有默认构造函数，需要显式指定父类的构造函数

因此上述代码会报错
```

* 以下代码运行的结果为:

```java
public class Main{
    public static void main(String[] args){
        String s;
        System.out.println(s);
    }
}
```

```markdown
会报错，因为方法内部的局部变量需要赋初始值
而类内的成员变量会在类加载的时候赋予初值
```

* 下面哪些属于正确地声明一个二维数组

```java
A. int[][] a = new int[][];
B. int []a[] = new int[10][10];
C. int[10][10] a = new int[][];
D. int[][] a = new int[10][10];
```

```markdown
A. 一维长度必须确定
B. 正确
C. 一维长度必须确定
D. 正确
```

* 下面代码是否正确

```java
public class Base{
    private int a, b, c, d, e;

    public Base(int a, int b){
        this.a = a;
        this.b = b;
    }

    public Base(int c, int d){
        Base(1, 2);
        this.c = c;
        this.d = d;
    }
}
```

```markdown
错误: 只能调用this(1, 2)
```

* Java以Stream结尾都是字节流，以Reader/Writer结尾都是字符流

* 下列代码的运行结果是:

```java
class Animal{
    public void move(){
        System.out.println("动物可以移动");
    }
}
class Dog extends Animal{
    public void move(){
        System.out.println("狗可以跑和走");
    }
    public void bark(){
        System.out.println("狗可以吠叫");
    }
}
public class TestDog{
    public static void main(String args[]){
        Animal a = new Animal();
        Animal b = new Dog(); 
        a.move();
        b.move();
        b.bark();
    }
}
```

```markdown
运行错误

Animal b = new Dog();

Animal没有bark()这个方法
```

* volatile可以实现多线程下的计数器

```markdown
必须加锁
```

* 下面哪些方法可以被继承

```java
public class Parent { 
    private void m1(){} 
    void m2(){} 
    protected void m3(){} 
    public static void m4(){} 
}
```

```markdown
通过继承，子类可以拥有所有父类对其可见的方法和域 
A.私有方法只能在本类中可见，故不能继承，A错误 
B.缺省访问修饰符只在本包中可见，在外包中不可见，B错误 
C.保护修饰符凡是继承自该类的子类都能访问，当然可被继承覆盖；C正确 
D.static修饰的成员属于类成员，父类字段或方法只能被子类同名字段或方法遮蔽，不能被继承覆盖，D错误 
```

* goto是java关键字吗

```markdown
是的
```

* 下列List扩充几次

```java
List<Integer> list = new ArrayList<>(10);
```

```markdown
Arraylist默认数组大小是10，扩容后的大小是扩容前的1.5倍，最大值小于Integer 的最大值减8，如果新创建的集合有带初始值，默认就是传入的大小，也就不会扩容 
```

* Java重写和重载需要注意的事项是

```markdown
方法重写 
    参数列表必须完全与被重写方法的相同； 
    返回类型必须完全与被重写方法的返回类型相同； 
    访问权限不能比父类中被重写的方法的访问权限更低。例如：如果父类的一个方法被声明为public，那么在子类中重写该方法就不能声明为protected。 
    父类的成员方法只能被它的子类重写。 
    声明为final的方法不能被重写。 
    声明为static的方法不能被重写，但是能够被再次声明。 
    子类和父类在同一个包中，那么子类可以重写父类所有方法，除了声明为private和final的方法。 
    子类和父类不在同一个包中，那么子类只能够重写父类的声明为public和protected的非final方法。 
    重写的方法能够抛出任何非强制异常，无论被重写的方法是否抛出异常。但是，重写的方法不能抛出新的强制性异常，或者比被重写方法声明的更广泛的强制性异常，反之则可以。 
    构造方法不能被重写。 
    如果不能继承一个方法，则不能重写这个方法。 
方法重载
    被重载的方法必须改变参数列表(参数个数或类型或顺序不一样)； 
    被重载的方法可以改变返回类型； 
    被重载的方法可以改变访问修饰符； 
    被重载的方法可以声明新的或更广的检查异常； 
    方法能够在同一个类中或者在一个子类中被重载。 
    无法以返回值类型作为重载函数的区分标准。 
```

* 关于ThreadLocal

```markdown
ThreadLocal不是一个线程而是一个线程的本地化对象。当工作于多线
程环境中的对象采用ThreadLocal维护变量时，ThreadLocal为每个使
用该变量的线程分配一个独立的副本。每个线程都可以独立的改变自己
的副本，而不影响其他线程的副本。
```

* 下列关于会话跟踪技术说法正确的是:

```markdown
A. Cookie是Web服务器发送给客户端的一小段信息，客户端请求时，可以读取该信息发送到服务器端
B. 关闭浏览器意味着临时会话ID丢失，但所有与原会话关联的会话数据仍保留在服务器上，直至会话过期
C. 在禁用Cookie时可以使用URL重写技术跟踪会话
D. 隐藏表单域将字段添加到HTML表单并在客户端浏览器中显示
```

```markdown
A,B,C均正确

隐藏域在页面中对于用户（浏览器）是不可见的，在表单中插入隐藏域
的目的在于收集或发送信息，以利于被处理表单的程序所使用。浏览者
单击发送按钮发送表单的时候，隐藏域的信息也被一起发送到服务器
```

* Java虚函数

```markdown
虚函数的存在是为了多态。

Java中其实没有虚函数的概念，它的普通函数就相当于C++的虚函数，动态绑定是Java的默认行为。如果Java中不希望某个函数具有虚函数特性，可以加上final关键字变成非虚函数

PS: 其实C++和Java在虚函数的观点大同小异，异曲同工罢了。
```

* 下列代码的输出结果为:

```java
public class Demo {
  public static void main(String args[])
  {
    String str=new String("hello");
    if(str=="hello")
    {
      System.out.println("true");
    }      
    else     {
      System.out.println("false");
    }
  }
}
```

```markdown
new String() 创建了一个新的String对象

与常量池中的String地址不同
```

* 什么情况下会发生永久代溢出，什么情况下会发生老年代溢出:

```markdown
CGIB动态代理大量生成代理对象导致Perm溢出
大量创造对象导致OOM(堆的老年代溢出)
```

* 创建派生类的时候构造顺序为:

```markdown
基类构造函数，派生类对象成员构造函数，派生类本身的构造函数
```

* 以下JSP代码定义了一个变量，如何输出这个变量的值？ 
  `<bean:define id="stringBean" value="helloWorld"/>`

```jsp
1. <%=stringBean%>
2. <bean:write name="stringBean"/>
3. <%String myBean=(String)pageContext.getAttribute("stringBean",PageContext.PAGE_SCOPE);%>
<%=myBean%>
```

* 在try的括号里面有return一个值，那在哪里执行finally里的代码?

```markdown
return 前返回
```

* 关于下列jsp说法错误的是:

```jsp
<%@ page language="java" import="java.util.*" errorPage="error.jsp" isErrorPage="false" %>  
```

```markdown
A. 该页面可以使用 exception 对象(错误)
B. 该页面发生异常会转向 error.jsp(正确)
C. 存在 errorPage 属性时，isErrorPage 是默认为 false(正确)
D. error.jsp 页面一定要有isErrorPage 属性且值为 true(正确)

exception是JSP九大内置对象之一，其实例代表其他页面的异常和错
误。只有当页面是错误处理页面时，即isErroePage为 true时，该对象
才可以使用。对于C项，errorPage的实质就是JSP的异常处理机制,发生
异常时才会跳转到 errorPage指定的页面，没必要给errorPage再设置
一个errorPage。所以当errorPage属性存在时， isErrorPage属性值
为false 
```

* Java通过垃圾回收回收不再引用的变量，垃圾回收时对象的finallize方法一定会得到执行

```markdown
错误

GC线程属于优先级十分低的线程
(但是这个解释有失偏颇)
```

