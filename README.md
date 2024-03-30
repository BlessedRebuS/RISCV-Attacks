[Black Hat conference ](https://www.youtube.com/watch?v=HCZ6DCu2ciE) and [RISC-V Attacks POC](https://github.com/cispa/Security-RISC/tree/mlast weekain/flush-fault)

**Attack timing differences** in caches and predictors
**Exploit implementation bugs**
**Abusing physical properties** like rowhammer or power analysis

# Side channel attacks
In computer security, a side-channel attack is any attack based on extra information that can be gathered because of the fundamental way a computer protocol or algorithm is implemented, rather than flaws in the design of the protocol or algorithm itself. In this study case, the attacks will work only if the microarchitecture offers **[out of order execution](https://www.cs.uaf.edu/2011/spring/cs641/proj1/vsanditi/)** and **[speculative execution](https://en.wikipedia.org/wiki/Speculative_execution)**.

## Cache Flush and Reload (mitigated)

<img src='img/flush-reload.png' width='500'>

Usually after a cache miss the data is fetched from the DRAM, but the second time is fetched from the CPU cache that is much faster.

<img src='img/shared-memory.png' width='500'>

After the attacker flushes the cache, the victim now loads (due to a read operation) a data on the empty cache. 
Now the attacker can read data and due to its access time knows if the data is loaded in cache or not.

## Cache Flush and Fault (new approach)

Again the attacker flushes the cache, then it jump to the address containing victim cache line and handles the error. From the timing of the fault the attacker can predict if the data was on cache or not.

<img src='img/fault-reload.png' width='500'>

## [Performance Counters](https://docs.openhwgroup.org/projects/cv32e40x-user-manual/en/latest/perf_counters.html)

They give a benchmarking capability. They can count report events like cache missed, instruction executed, CPU frequency etc...
By default performance counters are available also to the attackers on the system. The mitigation for this type of attacks is making the interface root only.

<img src='img/branch-taken.png' width='500'>

*Some performance counters still remain privileges by standard.* The events are privileged apart from a few. The "branch taken" event is now privileged, but remains the number of **CPU cycles elapsed** and the **number of instruction retired**.

## KASLR: Kernel Address Space Layout Randomization

Randomization of address space at every boot of the system. An attacker to use a memory exploit has to know the addresses of the program before running it.

### BPU: Branch-Prediction Unit

The core idea of this methodolody is that branches impact execution speed. Knowing that there is a jump still doesn't tell what jump is taken in the code.
At this part it comes the **Optimize by Prediction**. The CPU has a history of last branches to optimize the future branches. For example if a code is "always" the processor tends to optimize the jump pre-loading the false branch condition.

## CPU OPTIMIZATION: Speculative Execution

Instead of only predicting the branch, the CPU could actually execute the prediction. That makes modern CPUs very fast. [Spectre](https://meltdownattack.com/) used this optimization to exploit the CPU. *RISC-V doesn't have speculative execution yet*, but they have **speculative prefetching**.
This means that the attacker can let the CPU prefetch whatever he wants.

<img src='img/prefetch.png' width='500'>


This means that **the Speculative Prefetching is exploitable** and Spectre can be exploited on some RISC-V processors. On some specific CPUs it is present the limited speculation that mitigates this attacks.

# Case of Study: Monte Cimone's RISC-V SiFive u74-mc


On the Monte Cimone's RISC-V cluster it is present the [SiFive U74-MC](https://starfivetech.com/uploads/u74mc_core_complex_manual_21G1.pdf). As the manual says and [as reported from the SiFive statement](https://www.sifive.com/blog/sifive-statement-on-meltdown-and-spectre), the IP core is not allowed to perform speculative execution:

"_Meltdown attacks (CVE-2017-5754) rely upon speculative access to memory that the processor does not have permission to access; our processors do not perform this form of speculation. The Spectre attacks (CVE-2017-5753 and CVE-2017-5715) rely upon speculative memory accesses causing cache state transitions; our processors do not speculatively refill or evict data cache lines._"

Let's try it in the SiFive present on the cluster 

<img src='img/sifive-arch.png' width='500'>

I will run [this](https://github.com/cispa/Security-RISC/tree/main/spectre) spectre POC for RISC-V from Lukas Gerlach, Michael Schwarz and Daniel Weber.

<img src='img/strace-sifive.png' width='500'>

As expected this type of attack is mitigated in this processor due to the **limited speculation**. In general, more optimized cores are more vulnerable. This attack actually works on C910 that allows speculative execution.

### Challenges
ROP: a function that calls other functions should not assume these registers hold their value across method calls.
Caller and callee: https://danielmangum.com/posts/risc-v-bytes-caller-callee-registers/
