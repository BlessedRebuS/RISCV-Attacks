[Black Hat conference ](https://www.youtube.com/watch?v=HCZ6DCu2ciE)

**Attack timing differences** in caches and predictors
**Exploit implementation bugs**
**Abusing physical properties** like rowhammer or power analysis

## Cache Flush and Reload (mitigated)

![flush-reload.png](img/flush-reload.png)

Usually after a cache miss the data is fetched from the DRAM, but the second time is fetched from the CPU cache that is much faster.

![shared-memory.png](img/shared-memory.png)


After the attacker flushes the cache, the victim now loads (due to a read operation) a data on the empty cache. 
Now the attacker can read data and due to its access time knows if the data is loaded in cache or not.

## Cache Flush and Fault (new approach)

Again the attacker flushes the cache, then it jump to the address containing victim cache line and handles the error. From the timing of the fault the attacker can predict if the data was on cache or not.

![fault-reload.png](img/fault-reload.png)

## Performance Counters

They give a benchmarking capability. They can count report events like cache missed, instruction executed, CPU frequency etc...
By default performance counters are available also to the attackers on the system. The mitigation for this type of attacks is making the interface root only.

![branch-taken.png](img/branch-taken.png)

*Some performance counters still remain privileges by standard.* The events are privileged apart from a few. The "branch taken" event is now privileged, but remains the number of **CPU cycles elapsed** and the **number of instruction retired**.

## KASLR: Kernel Address Space Layout Randomization

Randomization of address space at every boot of the system. An attacker to use a memory exploit has to know the addresses of the program before running it.

### BPU: Branch-Prediction Unit

The core idea of this methodolody is that branches impact execution speed. Knowing that there is a jump still doesn't tell what jump is taken in the code.
At this part it comes the **Optimize by Prediction**. The CPU has a history of last branches to optimize the future branches. For example if a code is "always" the processor tends to optimize the jump pre-loading the false branch condition.

## CPU OPTIMIZATION: Speculative Execution

Instead of only predicting the branch, the CPU could actually execute the prediction. That makes modern CPUs very fast. [Spectre](https://meltdownattack.com/) used this optimization to exploit the CPU. *RISC-V doesn't have speculative execution yet*, but they have **speculative prefetching**.
This means that the attacker can let the CPU prefetch whatever he wants.

![prefetch.png](img/prefetch.png)

This means that **the Speculative Prefetching is exploitable** and Spectre can be exploited on some RISC-V processors. On some specific CPUs it is present the limited speculation that mitigates this attacks.
