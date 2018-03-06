#ifndef PRIORITYLIST_H_
#define PRIORITYLIST_H_

#include "contiki-conf.h"
#include "net/rime/rime.h"

struct cpmspriority_list {
    struct cpmspriority_list *next;
    int rssi;
    uint8_t re_tx;
    linkaddr_t addr;
    struct cpmsack_list *cpmsacklist;
};

int cpmsplist_create(int rssi, const linkaddr_t *addr, struct cpmsack_list *cpmsacklist);

void * cpmsplist_get();

int cpmsplist_retx_update(struct cpmspriority_list *cpmsplist);

int cpmsplist_free();

#endif