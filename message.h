#ifndef MESSAGE_H
#define MESSAGE_H

#include "gobang.h"

/* Message Type */
typedef struct message_type{
    int type; /* 0 null,1 Operation Type,2 Message */
} MessageType;

/* Operation Type */
typedef struct gobang_operation{
    struct gb_coord coord;
} GongBangOperation;

/* message */
typedef struct messagebuffer {
    char buffer[512];
} MessageBuffer;

/* 用户类 */
typedef struct user_model{
    char Name[20]; // 用户名
} UserModel;

#endif