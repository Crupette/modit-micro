#include "module/interrupt.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/io.h"

typedef struct idt {
    idt_entry_t entries[256];
    idt_ptr_t ptr;
} idt_t;

static idt_t _idt;
static isr_handler_t _handlers[256] = { 0 };

void idt_createEntry(uint8_t i, int_handler_t func, uint16_t selector, uint8_t flags){
    idt_entry_t *entry = &_idt.entries[i];
    entry->offset_low = (uintptr_t)func & 0xFFFF;
    entry->offset_high = ((uintptr_t)func >> 16) & 0xFFFF;
    entry->selector = selector;
    entry->flags = flags;
}

void idt_addHandler(uint8_t i, isr_handler_t func){
    _handlers[i] = func;
}

void _interrupt_handler(interrupt_state_t *state){
    //Handler must exist to be called
    if(_handlers[state->num] != 0){
        _handlers[state->num](state);
    }else
    if(state->num >= 32){
        //IRQ's need to be acknowleged. IRQ's in the secondary PIC need another ack
        if(state->num > 40){
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
    }else{
        //Damaging ISR's need to be caught to prevent destructive triple-faults
        log_printf(LOG_FATAL, "Unhandled exception %i\n", state->num);
        while(true) asm("hlt");
    }
}

static void setup_irq(){
    //Setup
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    //Remap offset of IDR
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    //Setup cascading
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    //Environment info
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    //Masking
    outb(0x21, 0x00);
    outb(0x21, 0x00);
}

void setup_idt(){
    _idt.ptr.size = (sizeof(idt_entry_t) * 256) - 1;
    _idt.ptr.addr = (uintptr_t)&_idt.entries[0];

    //Oh god
    idt_createEntry(0, _isr0, 0x08, 0x8E);
    idt_createEntry(1, _isr1, 0x08, 0x8E);
    idt_createEntry(2, _isr2, 0x08, 0x8E);
    idt_createEntry(3, _isr3, 0x08, 0x8E);
    idt_createEntry(4, _isr4, 0x08, 0x8E);
    idt_createEntry(5, _isr5, 0x08, 0x8E);
    idt_createEntry(6, _isr6, 0x08, 0x8E);
    idt_createEntry(7, _isr7, 0x08, 0x8E);
    idt_createEntry(8, _isr8, 0x08, 0x8E);
    idt_createEntry(9, _isr9, 0x08, 0x8E);
    idt_createEntry(10, _isr10, 0x08, 0x8E);
    idt_createEntry(11, _isr11, 0x08, 0x8E);
    idt_createEntry(12, _isr12, 0x08, 0x8E);
    idt_createEntry(13, _isr13, 0x08, 0x8E);
    idt_createEntry(14, _isr14, 0x08, 0x8E);
    idt_createEntry(15, _isr15, 0x08, 0x8E);
    idt_createEntry(16, _isr16, 0x08, 0x8E);
    idt_createEntry(17, _isr17, 0x08, 0x8E);
    idt_createEntry(18, _isr18, 0x08, 0x8E);
    idt_createEntry(19, _isr19, 0x08, 0x8E);
    idt_createEntry(20, _isr20, 0x08, 0x8E);
    idt_createEntry(21, _isr21, 0x08, 0x8E);
    idt_createEntry(22, _isr22, 0x08, 0x8E);
    idt_createEntry(23, _isr23, 0x08, 0x8E);
    idt_createEntry(24, _isr24, 0x08, 0x8E);
    idt_createEntry(25, _isr25, 0x08, 0x8E);
    idt_createEntry(26, _isr26, 0x08, 0x8E);
    idt_createEntry(27, _isr27, 0x08, 0x8E);
    idt_createEntry(28, _isr28, 0x08, 0x8E);
    idt_createEntry(29, _isr29, 0x08, 0x8E);
    idt_createEntry(30, _isr30, 0x08, 0x8E);
    idt_createEntry(31, _isr31, 0x08, 0x8E);
    idt_createEntry(32, _irq0, 0x08, 0x8E);
    idt_createEntry(33, _irq1, 0x08, 0x8E);
    idt_createEntry(34, _irq2, 0x08, 0x8E);
    idt_createEntry(35, _irq3, 0x08, 0x8E);
    idt_createEntry(36, _irq4, 0x08, 0x8E);
    idt_createEntry(37, _irq5, 0x08, 0x8E);
    idt_createEntry(38, _irq6, 0x08, 0x8E);
    idt_createEntry(39, _irq7, 0x08, 0x8E);
    idt_createEntry(40, _irq8, 0x08, 0x8E);
    idt_createEntry(41, _irq9, 0x08, 0x8E);
    idt_createEntry(42, _irq10, 0x08, 0x8E);
    idt_createEntry(43, _irq11, 0x08, 0x8E);
    idt_createEntry(44, _irq12, 0x08, 0x8E);
    idt_createEntry(45, _irq13, 0x08, 0x8E);
    idt_createEntry(46, _irq14, 0x08, 0x8E);
    idt_createEntry(47, _irq15, 0x08, 0x8E);
    idt_createEntry(48, _irq16, 0x08, 0x8E);
    idt_createEntry(49, _irq17, 0x08, 0x8E);
    idt_createEntry(50, _irq18, 0x08, 0x8E);
    idt_createEntry(51, _irq19, 0x08, 0x8E);
    idt_createEntry(52, _irq20, 0x08, 0x8E);
    idt_createEntry(53, _irq21, 0x08, 0x8E);
    idt_createEntry(54, _irq22, 0x08, 0x8E);
    idt_createEntry(55, _irq23, 0x08, 0x8E);
    idt_createEntry(56, _irq24, 0x08, 0x8E);
    idt_createEntry(57, _irq25, 0x08, 0x8E);
    idt_createEntry(58, _irq26, 0x08, 0x8E);
    idt_createEntry(59, _irq27, 0x08, 0x8E);
    idt_createEntry(60, _irq28, 0x08, 0x8E);
    idt_createEntry(61, _irq29, 0x08, 0x8E);
    idt_createEntry(62, _irq30, 0x08, 0x8E);
    idt_createEntry(63, _irq31, 0x08, 0x8E);
    idt_createEntry(64, _irq32, 0x08, 0x8E);
    idt_createEntry(65, _irq33, 0x08, 0x8E);
    idt_createEntry(66, _irq34, 0x08, 0x8E);
    idt_createEntry(67, _irq35, 0x08, 0x8E);
    idt_createEntry(68, _irq36, 0x08, 0x8E);
    idt_createEntry(69, _irq37, 0x08, 0x8E);
    idt_createEntry(70, _irq38, 0x08, 0x8E);
    idt_createEntry(71, _irq39, 0x08, 0x8E);
    idt_createEntry(72, _irq40, 0x08, 0x8E);
    idt_createEntry(73, _irq41, 0x08, 0x8E);
    idt_createEntry(74, _irq42, 0x08, 0x8E);
    idt_createEntry(75, _irq43, 0x08, 0x8E);
    idt_createEntry(76, _irq44, 0x08, 0x8E);
    idt_createEntry(77, _irq45, 0x08, 0x8E);
    idt_createEntry(78, _irq46, 0x08, 0x8E);
    idt_createEntry(79, _irq47, 0x08, 0x8E);
    idt_createEntry(80, _irq48, 0x08, 0x8E);
    idt_createEntry(81, _irq49, 0x08, 0x8E);
    idt_createEntry(82, _irq50, 0x08, 0x8E);
    idt_createEntry(83, _irq51, 0x08, 0x8E);
    idt_createEntry(84, _irq52, 0x08, 0x8E);
    idt_createEntry(85, _irq53, 0x08, 0x8E);
    idt_createEntry(86, _irq54, 0x08, 0x8E);
    idt_createEntry(87, _irq55, 0x08, 0x8E);
    idt_createEntry(88, _irq56, 0x08, 0x8E);
    idt_createEntry(89, _irq57, 0x08, 0x8E);
    idt_createEntry(90, _irq58, 0x08, 0x8E);
    idt_createEntry(91, _irq59, 0x08, 0x8E);
    idt_createEntry(92, _irq60, 0x08, 0x8E);
    idt_createEntry(93, _irq61, 0x08, 0x8E);
    idt_createEntry(94, _irq62, 0x08, 0x8E);
    idt_createEntry(95, _irq63, 0x08, 0x8E);
    idt_createEntry(96, _irq64, 0x08, 0x8E);
    idt_createEntry(97, _irq65, 0x08, 0x8E);
    idt_createEntry(98, _irq66, 0x08, 0x8E);
    idt_createEntry(99, _irq67, 0x08, 0x8E);
    idt_createEntry(100, _irq68, 0x08, 0x8E);
    idt_createEntry(101, _irq69, 0x08, 0x8E);
    idt_createEntry(102, _irq70, 0x08, 0x8E);
    idt_createEntry(103, _irq71, 0x08, 0x8E);
    idt_createEntry(104, _irq72, 0x08, 0x8E);
    idt_createEntry(105, _irq73, 0x08, 0x8E);
    idt_createEntry(106, _irq74, 0x08, 0x8E);
    idt_createEntry(107, _irq75, 0x08, 0x8E);
    idt_createEntry(108, _irq76, 0x08, 0x8E);
    idt_createEntry(109, _irq77, 0x08, 0x8E);
    idt_createEntry(110, _irq78, 0x08, 0x8E);
    idt_createEntry(111, _irq79, 0x08, 0x8E);
    idt_createEntry(112, _irq80, 0x08, 0x8E);
    idt_createEntry(113, _irq81, 0x08, 0x8E);
    idt_createEntry(114, _irq82, 0x08, 0x8E);
    idt_createEntry(115, _irq83, 0x08, 0x8E);
    idt_createEntry(116, _irq84, 0x08, 0x8E);
    idt_createEntry(117, _irq85, 0x08, 0x8E);
    idt_createEntry(118, _irq86, 0x08, 0x8E);
    idt_createEntry(119, _irq87, 0x08, 0x8E);
    idt_createEntry(120, _irq88, 0x08, 0x8E);
    idt_createEntry(121, _irq89, 0x08, 0x8E);
    idt_createEntry(122, _irq90, 0x08, 0x8E);
    idt_createEntry(123, _irq91, 0x08, 0x8E);
    idt_createEntry(124, _irq92, 0x08, 0x8E);
    idt_createEntry(125, _irq93, 0x08, 0x8E);
    idt_createEntry(126, _irq94, 0x08, 0x8E);
    idt_createEntry(127, _irq95, 0x08, 0x8E);
    idt_createEntry(128, _irq96, 0x08, 0x8E);
    idt_createEntry(129, _irq97, 0x08, 0x8E);
    idt_createEntry(130, _irq98, 0x08, 0x8E);
    idt_createEntry(131, _irq99, 0x08, 0x8E);
    idt_createEntry(132, _irq100, 0x08, 0x8E);
    idt_createEntry(133, _irq101, 0x08, 0x8E);
    idt_createEntry(134, _irq102, 0x08, 0x8E);
    idt_createEntry(135, _irq103, 0x08, 0x8E);
    idt_createEntry(136, _irq104, 0x08, 0x8E);
    idt_createEntry(137, _irq105, 0x08, 0x8E);
    idt_createEntry(138, _irq106, 0x08, 0x8E);
    idt_createEntry(139, _irq107, 0x08, 0x8E);
    idt_createEntry(140, _irq108, 0x08, 0x8E);
    idt_createEntry(141, _irq109, 0x08, 0x8E);
    idt_createEntry(142, _irq110, 0x08, 0x8E);
    idt_createEntry(143, _irq111, 0x08, 0x8E);
    idt_createEntry(144, _irq112, 0x08, 0x8E);
    idt_createEntry(145, _irq113, 0x08, 0x8E);
    idt_createEntry(146, _irq114, 0x08, 0x8E);
    idt_createEntry(147, _irq115, 0x08, 0x8E);
    idt_createEntry(148, _irq116, 0x08, 0x8E);
    idt_createEntry(149, _irq117, 0x08, 0x8E);
    idt_createEntry(150, _irq118, 0x08, 0x8E);
    idt_createEntry(151, _irq119, 0x08, 0x8E);
    idt_createEntry(152, _irq120, 0x08, 0x8E);
    idt_createEntry(153, _irq121, 0x08, 0x8E);
    idt_createEntry(154, _irq122, 0x08, 0x8E);
    idt_createEntry(155, _irq123, 0x08, 0x8E);
    idt_createEntry(156, _irq124, 0x08, 0x8E);
    idt_createEntry(157, _irq125, 0x08, 0x8E);
    idt_createEntry(158, _irq126, 0x08, 0x8E);
    idt_createEntry(159, _irq127, 0x08, 0x8E);
    idt_createEntry(160, _irq128, 0x08, 0x8E);
    idt_createEntry(161, _irq129, 0x08, 0x8E);
    idt_createEntry(162, _irq130, 0x08, 0x8E);
    idt_createEntry(163, _irq131, 0x08, 0x8E);
    idt_createEntry(164, _irq132, 0x08, 0x8E);
    idt_createEntry(165, _irq133, 0x08, 0x8E);
    idt_createEntry(166, _irq134, 0x08, 0x8E);
    idt_createEntry(167, _irq135, 0x08, 0x8E);
    idt_createEntry(168, _irq136, 0x08, 0x8E);
    idt_createEntry(169, _irq137, 0x08, 0x8E);
    idt_createEntry(170, _irq138, 0x08, 0x8E);
    idt_createEntry(171, _irq139, 0x08, 0x8E);
    idt_createEntry(172, _irq140, 0x08, 0x8E);
    idt_createEntry(173, _irq141, 0x08, 0x8E);
    idt_createEntry(174, _irq142, 0x08, 0x8E);
    idt_createEntry(175, _irq143, 0x08, 0x8E);
    idt_createEntry(176, _irq144, 0x08, 0x8E);
    idt_createEntry(177, _irq145, 0x08, 0x8E);
    idt_createEntry(178, _irq146, 0x08, 0x8E);
    idt_createEntry(179, _irq147, 0x08, 0x8E);
    idt_createEntry(180, _irq148, 0x08, 0x8E);
    idt_createEntry(181, _irq149, 0x08, 0x8E);
    idt_createEntry(182, _irq150, 0x08, 0x8E);
    idt_createEntry(183, _irq151, 0x08, 0x8E);
    idt_createEntry(184, _irq152, 0x08, 0x8E);
    idt_createEntry(185, _irq153, 0x08, 0x8E);
    idt_createEntry(186, _irq154, 0x08, 0x8E);
    idt_createEntry(187, _irq155, 0x08, 0x8E);
    idt_createEntry(188, _irq156, 0x08, 0x8E);
    idt_createEntry(189, _irq157, 0x08, 0x8E);
    idt_createEntry(190, _irq158, 0x08, 0x8E);
    idt_createEntry(191, _irq159, 0x08, 0x8E);
    idt_createEntry(192, _irq160, 0x08, 0x8E);
    idt_createEntry(193, _irq161, 0x08, 0x8E);
    idt_createEntry(194, _irq162, 0x08, 0x8E);
    idt_createEntry(195, _irq163, 0x08, 0x8E);
    idt_createEntry(196, _irq164, 0x08, 0x8E);
    idt_createEntry(197, _irq165, 0x08, 0x8E);
    idt_createEntry(198, _irq166, 0x08, 0x8E);
    idt_createEntry(199, _irq167, 0x08, 0x8E);
    idt_createEntry(200, _irq168, 0x08, 0x8E);
    idt_createEntry(201, _irq169, 0x08, 0x8E);
    idt_createEntry(202, _irq170, 0x08, 0x8E);
    idt_createEntry(203, _irq171, 0x08, 0x8E);
    idt_createEntry(204, _irq172, 0x08, 0x8E);
    idt_createEntry(205, _irq173, 0x08, 0x8E);
    idt_createEntry(206, _irq174, 0x08, 0x8E);
    idt_createEntry(207, _irq175, 0x08, 0x8E);
    idt_createEntry(208, _irq176, 0x08, 0x8E);
    idt_createEntry(209, _irq177, 0x08, 0x8E);
    idt_createEntry(210, _irq178, 0x08, 0x8E);
    idt_createEntry(211, _irq179, 0x08, 0x8E);
    idt_createEntry(212, _irq180, 0x08, 0x8E);
    idt_createEntry(213, _irq181, 0x08, 0x8E);
    idt_createEntry(214, _irq182, 0x08, 0x8E);
    idt_createEntry(215, _irq183, 0x08, 0x8E);
    idt_createEntry(216, _irq184, 0x08, 0x8E);
    idt_createEntry(217, _irq185, 0x08, 0x8E);
    idt_createEntry(218, _irq186, 0x08, 0x8E);
    idt_createEntry(219, _irq187, 0x08, 0x8E);
    idt_createEntry(220, _irq188, 0x08, 0x8E);
    idt_createEntry(221, _irq189, 0x08, 0x8E);
    idt_createEntry(222, _irq190, 0x08, 0x8E);
    idt_createEntry(223, _irq191, 0x08, 0x8E);
    idt_createEntry(224, _irq192, 0x08, 0x8E);
    idt_createEntry(225, _irq193, 0x08, 0x8E);
    idt_createEntry(226, _irq194, 0x08, 0x8E);
    idt_createEntry(227, _irq195, 0x08, 0x8E);
    idt_createEntry(228, _irq196, 0x08, 0x8E);
    idt_createEntry(229, _irq197, 0x08, 0x8E);
    idt_createEntry(230, _irq198, 0x08, 0x8E);
    idt_createEntry(231, _irq199, 0x08, 0x8E);
    idt_createEntry(232, _irq200, 0x08, 0x8E);
    idt_createEntry(233, _irq201, 0x08, 0x8E);
    idt_createEntry(234, _irq202, 0x08, 0x8E);
    idt_createEntry(235, _irq203, 0x08, 0x8E);
    idt_createEntry(236, _irq204, 0x08, 0x8E);
    idt_createEntry(237, _irq205, 0x08, 0x8E);
    idt_createEntry(238, _irq206, 0x08, 0x8E);
    idt_createEntry(239, _irq207, 0x08, 0x8E);
    idt_createEntry(240, _irq208, 0x08, 0x8E);
    idt_createEntry(241, _irq209, 0x08, 0x8E);
    idt_createEntry(242, _irq210, 0x08, 0x8E);
    idt_createEntry(243, _irq211, 0x08, 0x8E);
    idt_createEntry(244, _irq212, 0x08, 0x8E);
    idt_createEntry(245, _irq213, 0x08, 0x8E);
    idt_createEntry(246, _irq214, 0x08, 0x8E);
    idt_createEntry(247, _irq215, 0x08, 0x8E);
    idt_createEntry(248, _irq216, 0x08, 0x8E);
    idt_createEntry(249, _irq217, 0x08, 0x8E);
    idt_createEntry(250, _irq218, 0x08, 0x8E);
    idt_createEntry(251, _irq219, 0x08, 0x8E);
    idt_createEntry(252, _irq220, 0x08, 0x8E);
    idt_createEntry(253, _irq221, 0x08, 0x8E);
    idt_createEntry(254, _irq222, 0x08, 0x8E);
    idt_createEntry(255, _irq223, 0x08, 0x8E);

    idt_ptr_t *paddr = &_idt.ptr;

    extern void idt_flush(idt_ptr_t *ptr);
    idt_flush(paddr);
    log_printf(LOG_OK, "Setup IDT\n");
}

extern void idt_flush(idt_ptr_t *idt);
int _init(){
    memset(&_idt, 0, sizeof(_idt));

    setup_irq();
    setup_idt();

    IRQ_ENABLE;
    log_printf(LOG_OK, "Loaded Interrupt Structures\n");
    return 0;
}

int _fini(){
    IRQ_DISABLE;
    return 0;
}

module_name(interrupt);

module_load(_init);
module_unload(_fini);

module_depends(gdt);
