#ifndef PROCESSES_H
#define PROCESSES_H

void help(int argc, char *argv[]);
void printmem(int argc, char *argv[]);
void mem(int argc, char *argv[]);
void getTime(int argc, char *argv[]);
void printPrime(int argc, char *argv[]);
void test_mm(int argc, char *argv[]);
void test_processes(int argc, char *argv[]);
void test_priority(int argc, char *argv[]);
void test_sem(int argc, char *argv[]);
void phylo(uint64_t argc, char *argv[]);
void getProcessesInfo(int argc, char *argv[]);
void loopProcess(int argc, char *argv[]);
void killProcess(int argc, char *argv[]);
void niceProcess(int argc, char *argv[]);
void blockProcess(int argc, char *argv[]);

#endif