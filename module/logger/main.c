#include "module/heap.h"

#include "kernel/modloader.h"
#include "kernel/logging.h"
#include "kernel/vgaterm.h"
#include "kernel/io.h"

#include <stdarg.h>

char **log_buffer = 0;

extern logger_t *logger;
extern int l_sprintf(char *buf, const char *fmt, ...);
extern int l_vsprintf(char *buf, const char *fmt, va_list ap);

void add_entry(char *msg){
    char **obuf = log_buffer;
    log_buffer = kalloc((logger->buffer_len + 1) * sizeof(char*));
    memcpy(log_buffer, obuf, logger->buffer_len * sizeof(char*));
    kfree(obuf);

    log_buffer[logger->buffer_len] = msg;
    logger->buffer_len++;
}

const char *lvlltbl[LOG_FATAL + 1] = {
    "\033[34mDEBUG\033[97m",
    "\033[32mOK\033[97m",
    "\033[94mINFO\033[97m",
    "\033[93mWARN\033[97m",
    "\033[91mERROR\033[97m",
    "\033[91;41mFATAL\033[97;40m"
};

void _log_printf(char *file, uint32_t line, log_level_t lvl, char *fmt, ...){
    char *buf = kalloc(256 + (strlen(fmt) * 2));
    char *obuf = buf;
    va_list ap;

    va_start(ap, fmt); 

    //Stores message in print buffer
    buf += l_sprintf(buf, "(\033[94m%s\033[97m:\033[93m%i\033[97m): [%s]: ",
            file, line, lvlltbl[lvl]);
    buf += l_vsprintf(buf, fmt, ap);

    va_end(ap);

    //Shrinks buffer to fit string
    buf = kalloc(strlen(obuf) + 1);
    strcpy(buf, obuf);
    kfree(obuf);

    add_entry(buf);

    //Outputs to VGA and serial
    vga_printf("%s", buf);
}

char *_log_getmsg(uint32_t i){
    if(i >= logger->buffer_len) return 0;
    return log_buffer[i];
}

char **_log_getmsgs() { return log_buffer; }

int logging_init(){
    //Initial length of 4 entries
    log_buffer = kalloc(16);
    memset(log_buffer, 0, 16);

    logger->_printf = _log_printf;
    logger->getmsg = _log_getmsg;
    logger->getmsgs = _log_getmsgs;

    //Recover logs from VGA buffer
    uintptr_t vgabuf = 0xE00B8000;
    for(int i = -1; i < VGA_HEIGHT; i++){
        uint8_t *line = (uint8_t*)(vgabuf + (i * (VGA_WIDTH * 2)));
        char *cmsg = kalloc(VGA_WIDTH);
        memset(cmsg, 0, VGA_WIDTH);
        char *mi = cmsg;
        for(char *c = (char*)line; *c; c += 2){
            *mi = *c;
            mi++;
        }
        if(strlen(cmsg) != 0){
            add_entry(cmsg);
        }else{
            kfree(cmsg);
        }
    }

    log_printf(LOG_OK, "Setup new log (%i stored messages)\n", logger->buffer_len);

    return 0;
}

int logging_fini(){
    return 0;
}

module_name(logger);

module_load(logging_init);
module_unload(logging_fini);

module_depends(heap);
module_depends(paging);
module_depends(interrupt);
