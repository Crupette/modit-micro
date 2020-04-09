/*  CPU.H - Data structures and simple functions to determine the topology, capabilities, and 
 *          processor count of the system
 *
 *  Author: Crupette
 * */
#ifndef MODULE_CPU_H
#define MODULE_CPU_H 1

/*  Wrapper for cpuid instruction
 *  cpuid: Issues a single request for cpuid, stores EAX in a, EDX in b
 *      code:   Command to issue to cpuid
 *      a:      Location to store EAX
 *      d:      Location to store EDX
 *  cpuid_data: Issues a request for cpuid, stores result as 16 byte string
 *      code:   Command to issue to cpuid
 *      buf:    Buffer to store result
 *      r:      Success
 * */
void cpuid(int code, uint32_t *a, uint32_t *d);
int cpuid_data(int code, uint32_t buf[4]);

#endif
