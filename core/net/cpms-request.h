#ifndef CPMSREQUEST_H_
#define CPMSREQUEST_H_

#include "contiki-conf.h"

#define CHANNEL_LOWEST 11
#define CPMSREQUEST_FRAME_LENGTH 3

enum {

	CPMSREQUEST_CHANNEL,

    CPMSREQUEST_ORDER,

    CPMSREQUEST_BYTES,

    CPMSREQUEST_NUM,
};

struct cpmsrequest_list{
    int channel;
    int order;
    int bytes;
};

int cpmsrequest_frame_create(int channel, int order, int bytes, uint8_t *buf);

void * cpmsrequest_frame_parse(uint8_t *buf);

#endif