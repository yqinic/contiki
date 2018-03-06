#ifndef PRIORITYLIST_H_
#define PRIORITYLIST_H_

#include "contiki-conf.h"
#include "net/rime/rime.h"

struct cpmspriority_list {
    int rssi;
    int re_tx;
    linkaddr_t addr;
    struct cpmsack_list *cpmsacklist;
};

int cpmsplist_create(int rssi, linkaddr_t *addr, struct cpmsack_list *cpmsacklist);

int cpmsplist_free();

#endif