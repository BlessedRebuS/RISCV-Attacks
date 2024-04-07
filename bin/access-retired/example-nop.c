#include "../../rlibsc.h"
#include <sched.h>
#include <stdio.h>
#include <string.h>

// Incresing the repetitions can reduce the noise making the PoC more precise
#define REP 100
#define COND NULL

int main() {
  size_t timings;
  size_t sum = 0, min = -1; 
  volatile FILE* f;
  size_t delta;
  for (int avg = 0; avg < REP; avg++) {
  sched_yield();
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
  delta = after - before;
  sum += delta;
  if (delta < min)
     min = delta;
  }
  printf("%zd (avg: %zd)\n",min, sum);
}
