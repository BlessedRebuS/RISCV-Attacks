#include "../rlibsc.h"
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#define REP 3
#define THRESHOLD 20000

int main() {
  size_t timings[26];
  memset(timings, 0, sizeof(timings));
  char p[] = "/test";
  char tmp[256];
  DIR *d;
  volatile FILE *f;
  struct dirent *dir;
  d = opendir("/home");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      strcpy(tmp, "/home/");
      strcat(tmp, dir->d_name);
      strcat(tmp, p);
      // printf("%s\n", tmp);

    size_t sum = 0, min = -1;
    for (int avg = 0; avg < REP; avg++) {
      sched_yield();
      size_t before = rdinstret();
      f = fopen(tmp, "r");
      size_t after = rdinstret();
      size_t delta = after - before;
      sum += delta;
      if (delta < min)
        min = delta;
    }
    if(min > THRESHOLD){
    printf("----------File %s exists----------\n", tmp);
    } 
    printf("%s: %zd (avg: %zd)\n", tmp, min, sum);
    }
    closedir(d);
  }
}
