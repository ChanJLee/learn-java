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