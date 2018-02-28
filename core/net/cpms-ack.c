#include "net/cpms-ack.h"
#include "dev/battery-sensor.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

struct cpmsack cpms_acks[CPMSACK_NUM];

static int 
cpmsack_update()
{
    
}

int 
cpmsack_set_attr(int type, const int value) 
{
    cpms_acks[type].val = value;
    return 1;
}

int
cpmsack_get_attr(int type)
{
    return cpms_acks[type].val;
}

