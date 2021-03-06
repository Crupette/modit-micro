/*  INTERRUPT.H -   Module interface for a general interrupt handler.
 *          Provides structures and functions to define and setup all 256 possible
 *          interrupts, and replace interrupt calls with other handlers
 *
 *  Author: Crupette
 * */

#ifndef MOD_INTERRUPT_H
#define MOD_INTERRUPT_H 1

#include <stdint.h>
#include <stddef.h>

#define IRQ_ENABLE asm volatile("sti")
#define IRQ_DISABLE asm volatile("cli")

typedef struct idt_entry {
    uint16_t offset_low;    //Lowest part of function
    uint16_t selector;  //Selector of interrupt function
    uint8_t zero;       //Has to be 0, unused
    union {
        uint8_t flags;
        struct {
            uint8_t type    : 4;    //Gate type
            uint8_t ss  : 1;    //Storage segment
            uint8_t dpl : 2;    //Descriptor Privilege Level
            uint8_t present : 1;    //Is interrupt valid
        }__attribute__((packed));
    };
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr {
    uint16_t size;
    uintptr_t addr;
} __attribute__((packed)) idt_ptr_t;

typedef void (*int_handler_t)();

typedef struct interrupt_state {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t num, err;
    uint32_t eip, cs, eflags, usresp, ss;
} interrupt_state_t;

typedef void (*isr_handler_t)(interrupt_state_t*);

/*  Creates a new IDT entry based on passed arguments
 *  i:    Interrupt index
 *  func:     Pointer to interrupt handler function
 *  selector: Selector of interrupt function
 *  flags:    Extra interrupt flags
 * */
void idt_createEntry(uint8_t i, int_handler_t func, uint16_t selector, uint8_t flags);

/*  Adds a hook for process execution to be passed in response to an interrupt
 *  i:  Interrupt to hook
 *  func:   Function to jump to
 * */
void isr_addHandler(uint8_t i, isr_handler_t func);
void irq_addHandler(uint8_t i, isr_handler_t func);

/*  Loads the current IDT in the current processor
 * */
void setup_idt(void);

void disable_interrupts(void);
void enable_interrupts(void);

//Handler functions
extern void _isr0();
extern void _isr1();
extern void _isr2();
extern void _isr3();
extern void _isr4();
extern void _isr5();
extern void _isr6();
extern void _isr7();
extern void _isr8();
extern void _isr9();
extern void _isr10();
extern void _isr11();
extern void _isr12();
extern void _isr13();
extern void _isr14();
extern void _isr15();
extern void _isr16();
extern void _isr17();
extern void _isr18();
extern void _isr19();
extern void _isr20();
extern void _isr21();
extern void _isr22();
extern void _isr23();
extern void _isr24();
extern void _isr25();
extern void _isr26();
extern void _isr27();
extern void _isr28();
extern void _isr29();
extern void _isr30();
extern void _isr31();
extern void _irq0();
extern void _irq1();
extern void _irq2();
extern void _irq3();
extern void _irq4();
extern void _irq5();
extern void _irq6();
extern void _irq7();
extern void _irq8();
extern void _irq9();
extern void _irq10();
extern void _irq11();
extern void _irq12();
extern void _irq13();
extern void _irq14();
extern void _irq15();
extern void _irq16();
extern void _irq17();
extern void _irq18();
extern void _irq19();
extern void _irq20();
extern void _irq21();
extern void _irq22();
extern void _irq23();
extern void _irq24();
extern void _irq25();
extern void _irq26();
extern void _irq27();
extern void _irq28();
extern void _irq29();
extern void _irq30();
extern void _irq31();
extern void _irq32();
extern void _irq33();
extern void _irq34();
extern void _irq35();
extern void _irq36();
extern void _irq37();
extern void _irq38();
extern void _irq39();
extern void _irq40();
extern void _irq41();
extern void _irq42();
extern void _irq43();
extern void _irq44();
extern void _irq45();
extern void _irq46();
extern void _irq47();
extern void _irq48();
extern void _irq49();
extern void _irq50();
extern void _irq51();
extern void _irq52();
extern void _irq53();
extern void _irq54();
extern void _irq55();
extern void _irq56();
extern void _irq57();
extern void _irq58();
extern void _irq59();
extern void _irq60();
extern void _irq61();
extern void _irq62();
extern void _irq63();
extern void _irq64();
extern void _irq65();
extern void _irq66();
extern void _irq67();
extern void _irq68();
extern void _irq69();
extern void _irq70();
extern void _irq71();
extern void _irq72();
extern void _irq73();
extern void _irq74();
extern void _irq75();
extern void _irq76();
extern void _irq77();
extern void _irq78();
extern void _irq79();
extern void _irq80();
extern void _irq81();
extern void _irq82();
extern void _irq83();
extern void _irq84();
extern void _irq85();
extern void _irq86();
extern void _irq87();
extern void _irq88();
extern void _irq89();
extern void _irq90();
extern void _irq91();
extern void _irq92();
extern void _irq93();
extern void _irq94();
extern void _irq95();
extern void _irq96();
extern void _irq97();
extern void _irq98();
extern void _irq99();
extern void _irq100();
extern void _irq101();
extern void _irq102();
extern void _irq103();
extern void _irq104();
extern void _irq105();
extern void _irq106();
extern void _irq107();
extern void _irq108();
extern void _irq109();
extern void _irq110();
extern void _irq111();
extern void _irq112();
extern void _irq113();
extern void _irq114();
extern void _irq115();
extern void _irq116();
extern void _irq117();
extern void _irq118();
extern void _irq119();
extern void _irq120();
extern void _irq121();
extern void _irq122();
extern void _irq123();
extern void _irq124();
extern void _irq125();
extern void _irq126();
extern void _irq127();
extern void _irq128();
extern void _irq129();
extern void _irq130();
extern void _irq131();
extern void _irq132();
extern void _irq133();
extern void _irq134();
extern void _irq135();
extern void _irq136();
extern void _irq137();
extern void _irq138();
extern void _irq139();
extern void _irq140();
extern void _irq141();
extern void _irq142();
extern void _irq143();
extern void _irq144();
extern void _irq145();
extern void _irq146();
extern void _irq147();
extern void _irq148();
extern void _irq149();
extern void _irq150();
extern void _irq151();
extern void _irq152();
extern void _irq153();
extern void _irq154();
extern void _irq155();
extern void _irq156();
extern void _irq157();
extern void _irq158();
extern void _irq159();
extern void _irq160();
extern void _irq161();
extern void _irq162();
extern void _irq163();
extern void _irq164();
extern void _irq165();
extern void _irq166();
extern void _irq167();
extern void _irq168();
extern void _irq169();
extern void _irq170();
extern void _irq171();
extern void _irq172();
extern void _irq173();
extern void _irq174();
extern void _irq175();
extern void _irq176();
extern void _irq177();
extern void _irq178();
extern void _irq179();
extern void _irq180();
extern void _irq181();
extern void _irq182();
extern void _irq183();
extern void _irq184();
extern void _irq185();
extern void _irq186();
extern void _irq187();
extern void _irq188();
extern void _irq189();
extern void _irq190();
extern void _irq191();
extern void _irq192();
extern void _irq193();
extern void _irq194();
extern void _irq195();
extern void _irq196();
extern void _irq197();
extern void _irq198();
extern void _irq199();
extern void _irq200();
extern void _irq201();
extern void _irq202();
extern void _irq203();
extern void _irq204();
extern void _irq205();
extern void _irq206();
extern void _irq207();
extern void _irq208();
extern void _irq209();
extern void _irq210();
extern void _irq211();
extern void _irq212();
extern void _irq213();
extern void _irq214();
extern void _irq215();
extern void _irq216();
extern void _irq217();
extern void _irq218();
extern void _irq219();
extern void _irq220();
extern void _irq221();
extern void _irq222();
extern void _irq223();

#endif
