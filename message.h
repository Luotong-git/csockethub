#ifndef MESSAGE_H
#define MESSAGE_H

#include "gobang.h"

/* Message Type */
struct Message_Type{
    int type; /* 0 null,1 Operation Type,2 Message */
};

/* Operation Type */
struct GoBang_Operation{
    struct gb_coord coord;
};

/* message */
struct Message {
    char buffer[512];
};

#endif