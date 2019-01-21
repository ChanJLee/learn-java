 双重锁定机制最简单的应用就是单例模式，就从单例模式的双重锁定机制来记录一下其中并发的原理。
public class Singleton
{
    public class Singleton
    {
        private static Singleton singleton；
        private Singleton()
        { }
 
        public static Singleton GetInstance()
        {
            
            if (singleton == null)
            {
                lock (Singleton.class)
                {
                    //第二重 singleton == null
                    if (singleton == null)
                    {
                        singleton = new Singleton();
                    }
                }
            }
            return singleton;
        }
    }
} 

         
       先说说为什么要双重锁的问题。在早期的jvm中，synchronized存在巨大的性能开销。如果getInstance的竞争很小，甚至没有竞争，那么synchronized就存在很大的冗余性能开销。所以通过双重检查机制避免不必要的锁操作。
    那么问题来了，这样的单例模式就是没有问题的了吗？当时不是。代码执行到instance不为null时，可能会出现instance的引用还没有被完全初始化的情况。这是为什么呢？
    问题的原因：instance＝new SingleTon这一行可以分解为如下代码
    1.memory＝allocate（）；
    2.ctorInstance（memory）； //对象初始化
    3.instance＝memory；    //设置instance指向刚被分配的内存；
    这里面涉及到java并发编程的一些基础。执行代码时，为了提高性能，编译器和处理器常常会对指令进行重排序。重排序分为三种：编译器重排序，指令级并行重排序，内存系统重排序。这些重排序势必会对java的代码的执行带来一些问题，如果代码随便执行，那么写程序的人肯定都疯了，那么对一定程度上限制代码的重排序当然就是jvm必须要做的事。对于编译器，jmm（java memory model）的编译器重排序规则会禁止特定的类型的编译器重排序（不是所有都要禁止，都禁止编译器的性能优化就不存在了，肯定不可以啦）。对于处理器重排序，jmm的处理器重排序规则会要求java编译器在生成指令序列时，插入特定的内存屏障指令，通过内存屏障来进制特定类型的处理器重排序。至于各种重排序规则与屏障类型这里就不一一介绍了（自行百度即可，不知道也不影响理解）。
    现在可以说会上面的例子了，很显然只要使用jmm的某种限制就可以让上面的重排序不会发生，这种重排序限制就是volatile。
    volatile有两种内存语义，一种是缓存一致性，另一种就是加屏障了。所谓缓存一致性就是当读volatile变量时，jmm会把该线程对应的本地内存置为无效。线程接下来将从主内存中读取共享变量。至于屏障类型有四种，StoreStore屏障，StoreLoad屏障，LoadLoad屏障，LoadStore屏障。简单来说，只要volatile变量于普通变量之间的重排序可能破坏volatile的内存语义，这种重排序就会被编译器重排序规则和处理器内存屏障插入策略禁止。
    说完volatile，接下来就是锁了。说到锁注意一个问题，锁指的不仅仅是synchronized，synchronized是锁的一种而已，而且还不是最好的一种。锁的内存语义和volatile 是完全一样的，那么锁的实现是什么原理呢？我总结为volatile变量＋CAS轮询。volatile变量保护状态的正确性，CAS保证状态的原子性。至于CAS的原理参看本人另一篇博客。
    暂时通过一个例子简单介绍并发的一点基础，后续慢慢补充锁的其他知识。
--------------------- 
作者：wander_sky 
来源：CSDN 
原文：https://blog.csdn.net/yu280265067/article/details/50894540 
版权声明：本文为博主原创文章，转载请附上博文链接！