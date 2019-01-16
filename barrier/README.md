# BARRIER

##  实例

```java
package com.chan;

import java.util.concurrent.*;

public class Main {
    public static void main(String args[]) {
        int num = 3;
        Executor executor = Executors.newFixedThreadPool(num);
        final CyclicBarrier cyclicBarrier = new CyclicBarrier(num + 1);

        for (int i = 0; i < num; ++i) {
            executor.execute(new Runnable() {
                @Override
                public void run() {
                    try {
                        cyclicBarrier.await();
                        System.out.println("fuck: " + Thread.currentThread());
                        cyclicBarrier.await();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    } catch (BrokenBarrierException e) {
                        e.printStackTrace();
                    }
                }
            });
        }

        System.out.println("prepare");
        try {
            Thread.sleep(10000);
            cyclicBarrier.await();
            cyclicBarrier.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (BrokenBarrierException e) {
            e.printStackTrace();
        }

        ((ExecutorService) executor).shutdownNow();
    }
}
```

## 源码分析

CyclicBarrier的构造函数

```java
public CyclicBarrier(int parties, Runnable barrierAction) {
    // parties 为需要协同工作的线程数，从上面的代码来看就是num + 1
    if (parties <= 0) throw new IllegalArgumentException();
    this.parties = parties;
    this.count = parties;
    this.barrierCommand = barrierAction;
}
```

从上面的代码来看CyclicBarrier的核心函数是await

```java
public int await() throws InterruptedException, BrokenBarrierException {
    try {
        return dowait(false, 0L);
    } catch (TimeoutException toe) {
        throw new Error(toe); // cannot happen
    }
}
```

请求转发到了dowait

```java
    private int dowait(boolean timed, long nanos) throws InterruptedException, BrokenBarrierException,
               TimeoutException {
        final ReentrantLock lock = this.lock;
        // 对当前代码加锁
        lock.lock();
        try {
            // 代表没一个阶段，比如上面的代码，一开始先调用await等待所有线程到位，然后再调用await等待所有线程一起执行结束
            // 上面就代表了两个generation
            final Generation g = generation;

            if (g.broken)
                throw new BrokenBarrierException();

            if (Thread.interrupted()) {
                breakBarrier();
                throw new InterruptedException();
            }

            // 可以看到每次调用await，计数都减一
            // 当减少到最后一个的时候 说明所有的线程都就位了
            int index = --count;
            if (index == 0) {  // tripped
                boolean ranAction = false;
                try {
                    final Runnable command = barrierCommand;
                    if (command != null)
                        command.run();
                    ranAction = true;
                    // 最后一个人要开启下一个generation
                    nextGeneration();
                    return 0;
                } finally {
                    if (!ranAction)
                        breakBarrier();
                }
            }

            // 如果当前不是最后一个线程，那么其他人要等待还未完成的线程，所以这里开始不停的循环
            // loop until tripped, broken, interrupted, or timed out
            for (;;) {
                try {
                    if (!timed)
                        trip.await();
                    else if (nanos > 0L)
                        nanos = trip.awaitNanos(nanos);
                } catch (InterruptedException ie) {
                    if (g == generation && ! g.broken) {
                        breakBarrier();
                        throw ie;
                    } else {
                        // We're about to finish waiting even if we had not
                        // been interrupted, so this interrupt is deemed to
                        // "belong" to subsequent execution.
                        Thread.currentThread().interrupt();
                    }
                }

                if (g.broken)
                    throw new BrokenBarrierException();

                // 循环什么时候结束呢，在这里
                // 当发现当前的generation已经没了的时候
                if (g != generation)
                    return index;

                if (timed && nanos <= 0L) {
                    breakBarrier();
                    throw new TimeoutException();
                }
            }
        } finally {
            lock.unlock();
        }
    }
```

看到这里你也许和我一样感到困惑，上面的代码解释起来看起来很有道理，但感觉还是哪里不对。

要知道在lock和unlock之间，只能有一个线程访问临界区。但是无论从我们的注释里看，还是从执行效果来看，大家都是并行在临界区进行的啊。

这里有个非常恶心的地方，就是上面代码提到的trip。纵观整个代码，目测那里可能会将lock解开。


我们看下trip是怎么来的

```java
    /** The lock for guarding barrier entry */
    private final ReentrantLock lock = new ReentrantLock();
    /** Condition to wait on until tripped */
    private final Condition trip = lock.newCondition();
```

在CyclicBarrier里可以找到这段代码。

看来他是来自ReentrantLock内部对象，我们去ReentrantLock看下。


## ReentrantLock

ReentrantLock是在jdk1.5的时候引入的，它主要是为了替换内置锁。
内置锁由synchronized关键字声明。为什么要用来替换内置所呢，因为它是公平锁，也就是说大家都有平等的机会去获得内置锁，那么考虑这个场景。线程A获得了内置锁，这时候线程B想要获得，但是发现锁已经被占用，这时候线程B只能休眠。当A释放内置锁的时候，这时候不巧来了线程C也要这个锁，因为B刚刚在休眠，当它从休眠中好不容易恢复，准备去获得锁的时候，发现又被C占用了，因而可能面临无休止的休眠。ReentrantLock就是为了解决这个问题而诞生的。

ReentrantLock的构造函数
```java

```