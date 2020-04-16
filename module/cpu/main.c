#include "module/cpu.h"
#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/memory.h"
#include "kernel/logging.h"
#include "kernel/io.h"

bool cpuid_supported = true;
bool msr_supported = true;

void cpuid(int code, uint32_t *a, uint32_t *d){
    if(!cpuid_supported) return;
    asm volatile("cpuid": "=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}

int cpuid_data(int code, uint32_t buf[4]){
    if(!cpuid_supported) return -1;
    asm volatile("cpuid": "=a"(*buf),"=b"(*(buf + 1)),
                          "=c"(*(buf + 3)),"=d"(*(buf + 2)):
                          "a"(code));
    return ((int*)buf)[0];
}

bool cpu_chk_msr(void){
    if(!cpuid_supported) return false;
    uint32_t a, d;
    cpuid(1, &a, &d);   //Read CPU features
    return d & CPUID_FEAT_EDX_MSR;
}

void cpu_read_msr(uint32_t id, uint32_t *lo, uint32_t *hi){
    if(!msr_supported) return;
    asm volatile("rdmsr": "=a"(*lo), "=d"(*hi): "c"(id));
}

void cpu_write_msr(uint32_t id, uint32_t lo, uint32_t hi){
    asm volatile("wrmsr": "=a"(lo), "=d"(hi): "c"(id));
}

void _invl_opcode_hook(interrupt_state_t *state){
    (void)state;
    cpuid_supported = false;
}

int cpu_init(){
    isr_addHandler(6, _invl_opcode_hook);
    uint32_t vendor[5];
    memset(vendor, 0, 20);

    cpuid_data(0, vendor);

    if(cpuid_supported == false){
        log_printf(LOG_WARNING, "CPU does not support instruction CPUID!\n");
    }else{
        log_printf(LOG_DEBUG, "CPU Vendor reported as %s\n", (char*)(vendor + 1));
    }

    isr_addHandler(6, 0);

    msr_supported = cpu_chk_msr();

    return 0;
}

int cpu_fini(){
    return 0;
}

module_name(cpu);

module_load(cpu_init);
module_unload(cpu_fini);

module_depends(paging);
module_depends(heap);
module_depends(dthelper);
module_depends(idt);
module_depends(isr);
