#include <stdio.h>

FILE __stdin = { 0 };
FILE __stdout = { 0 };
FILE __stderr = { 0 };

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;

//Will request from the filesystem server handles for stdin, stdout, stderr
//These will be sent from the process server by call from sc_spawn
void __stdio_init(){
    //For now, assume we are alone

    stdin->fd = FD_PRINT;
    stdout->fd = FD_PRINT;
    stderr->fd = FD_PRINT;

    stdin->buf  = malloc(BUFSIZ);
    stdout->buf = malloc(BUFSIZ);
    stderr->buf = malloc(BUFSIZ);

    stdin->bufsz = stdout->bufsz = stderr->bufsz = BUFSIZ;

    memset(stdin->buf, 0, BUFSIZ);
    memset(stdout->buf, 0, BUFSIZ);
    memset(stderr->buf, 0, BUFSIZ);
}

size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream){
    if(size == 0 || nmemb == 0) return 0;
    if(stream == 0) return 0;   //TODO: Set errno

    int32_t rsz = size * nmemb;
    size_t wrcnt = 0;
    while(rsz > 0){
        wrcnt += (rsz + stream->bufi >= stream->bufsz ? stream->bufsz - stream->bufi : rsz);
        if(rsz + stream->bufi >= stream->bufsz){
            memcpy(stream->buf + stream->bufi, ptr, stream->bufsz - stream->bufi);
            fflush(stream);
        }else{
            memcpy(stream->buf + stream->bufi, ptr, rsz);
            stream->bufi += rsz;
        }
        rsz -= stream->bufsz - stream->bufi;
    }
    return wrcnt;
}

int fflush(FILE *stream){
    if(stream == 0) {
        //TODO: Flush all streams
        return 0;
    }
    micro_write(stream, stream->buf, stream->bufi);
    stream->bufi = 0;
    return 0;
} 
