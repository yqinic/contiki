#include "net/rime/priority-list.h"
#include "lib/memb.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else 
#define PRINTF(...)
#endif 

#ifndef PLIST_NUM
#define PLIST_NUM 16
#endif

MEMB(plist_memb, struct cpmspriority_list, PLIST_NUM);
LIST(plist);

int
cpmsplist_create(int rssi, linkaddr_t *addr, struct cpmsack_list *cpmsacklist)
{
    struct cpmspriority_list *cpmsplist;

    cpmsplist = memb_alloc(&plist_memb);
    cpmsplist->rssi = rssi;
    cpmsplist->re_tx = 0;
    linkaddr_copy(&n->addr, addr);
    cpmsplist->cpmsacklist = cpmsacklist;

    struct cpmspriority_list *list_handle = list_head(plist);

    if (list_handle == NULL) {
        // no priority list, create one
        list_push(plist, cpmsplist);
        return 1;
    } else {
        if (list_length(plist) >= PLIST_NUM) {
            // maximum plist reaches, return error
            return 0;
        }

        // have list, but current struct has highest priority, or priority corresponded highest rssi
        if ((list_handle->cpmsacklist->priority > cpmsplist->cpmsacklist->priority) ||
            ((list_handle->cpmsacklist->priority == cpmsplist->cpmsacklist->priority) && 
            (list_handle->rssi <= cpmsplist->rssi))) {
                
            list_push(plist, cpmsplist);
            return 1;
        } else {
            // neither highest priority priority, nor lowest rssi
            struct cpmspriority_list *list_previous = list_handle;

            while (list_handle != NULL) {
                list_handle = list_item_next(list_handle);

                if (list_handle->cpmsacklist->priority == cpmsplist->cpmsacklist->priority) {
                    if (list_handle->rssi <= cpmsplist->rssi) {
                        list_insert(plist, list_previous, cpmsplist);
                        return 1;
                    }
                } else if (list_handle->cpmsacklist->priority > cpmsplist->cpmsacklist->priority) {
                    list_insert(plist, list_previous, cpmsplist);
                    return 1;
                } else {
                    // do nothing
                }
                list_previous = list_handle;
            }

            // current struct has lowest priority with this priority's lowest rssi
            list_add(plist, cpmsplist);
            return 1;
        } 
    }

    return 0;
}