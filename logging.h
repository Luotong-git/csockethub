#ifndef _LOGGING_H
#define _LOGGING_H

// Minskiter 2020-10-03 LOGGING

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h> 

#define MESSAGE_MAX_SIZE 1024


#define MESSAGE_ERROR 0
#define MESSAGE_WARNING 1
#define MESSAGE_INFO 2

typedef struct logging
{
    int level; //2 info(output all),1 warning(output error also),0 error(output error only)
    FILE * output;
    bool is_debug;
    void (*info)(struct logging * self,const char * message,...);
    void (*warning)(struct logging * self,const char * message,...);
    void (*error)(struct logging * self,const char * message,...);
    void (*debug)(struct logging * self,const char * message,...);
} LOG;

void logging_warning(LOG * self,const char * message,...);
void logging_info(LOG * self,const char * message,...);
void logging_error(LOG * self,const char * message,...);
void logging_debug(LOG * self,const char * message,...);

LOG * logging_create_debug(FILE * output,int level,bool debug){
    LOG * log = (LOG *)malloc(sizeof(LOG));
    log->is_debug=debug;
    log->output = output;
    log->level = MESSAGE_INFO;
    log->warning = &logging_warning;
    log->info = &logging_info;
    log->debug = &logging_debug;
    log->error = &logging_error;
    return log;
}

LOG * logging_create(FILE * output){
    return logging_create_debug(output,MESSAGE_INFO,false);
}

LOG * logging_create_level(FILE * output,int level){
    return logging_create_debug(output,level,false);
}

void logging_release(LOG * self){
    free(self);
}

// 记录警告
void logging_warning(LOG * self,const char * message,...){
    if (!self->output) return;
    if (self->level<MESSAGE_WARNING) return;
    char buffer[MESSAGE_MAX_SIZE] = "";
    va_list args;
    va_start(args,message);
    vsnprintf(buffer,MESSAGE_MAX_SIZE,message,args);
    va_end(args);
    fprintf(self->output,"%s %s\n","[WARNING]",buffer);
}

/* Record Error */
void logging_error(LOG * self,const char * message,...){
    if (!self->output) return;
    if (self->level<MESSAGE_ERROR) return;
    char buffer[MESSAGE_MAX_SIZE] = "";
    va_list args;
    va_start(args,message);
    vsnprintf(buffer,MESSAGE_MAX_SIZE,message,args);
    va_end(args);
    fprintf(self->output,"%s %s\n","[ERROR]",buffer);
}

/* Record Infomation */
void logging_info(LOG * self,const char * message,...){
    if (!self->output) return;
    if (self->level<MESSAGE_INFO) return;
    char buffer[MESSAGE_MAX_SIZE] = "";
    va_list args;
    va_start(args,message);
    vsnprintf(buffer,MESSAGE_MAX_SIZE,message,args);
    va_end(args);
    fprintf(self->output,"%s %s\n","[INFO]",buffer);
}

/* Record Debug Infomation */
void logging_debug(LOG * self,const char *message,...){
    if (!self->output) return;
    if (!self->is_debug) return;
    char buffer[MESSAGE_MAX_SIZE] = "";
    va_list args;
    va_start(args,message);
    vsnprintf(buffer,MESSAGE_MAX_SIZE,message,args);
    va_end(args);
    fprintf(self->output,"%s %s\n","[INFO]",buffer);
}

#endif