#include "net/rime/priority-list.h"
#include "lib/memb.h"
#include <stdlib.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTFUNC(...) cpmsplist_print()
#else 
#define PRINTF(...)
#define PRINTFUNC(...)
#endif 

#ifndef PLIST_NUM
#define PLIST_NUM 16
#endif

MEMB(plist_memb, struct cpmspriority_list, PLIST_NUM);
LIST(plist);

#if DEBUG
static void
cpmsplist_print()
{
    struct cpmspriority_list *list_handle = list_head(plist);
    int count = 0;

    while (list_handle != NULL) {
        count += 1;
        printf("no. %d element in plist, priority: %d, rssi: %d, no. of retx: %d, address: %d.%d\n", 
            count, list_handle->cpmsacklist->priority, list_handle->rssi, list_handle->re_tx,
            list_handle->addr.u8[0], list_handle->addr.u8[1]);

        list_handle = list_item_next(list_handle);
    }
}
#endif

static int
update_cpmsplist(struct cpmspriority_list *cpmsplist)
{
    struct cpmspriority_list *list_handle = list_head(plist);

    if (list_handle == NULL) {
        // no priority list, create one
        list_init(plist);
        list_add(plist, cpmsplist);

        PRINTF("first element in plist created\n");
        PRINTFUNC();
        return 1;

    } else {
        if (list_length(plist) >= PLIST_NUM) {
            // maximum plist reaches, return error
            PRINTF("maximum plist reaches\n");
            return 0;
        }
        if (cpmsplist->re_tx == 1) {
            if (list_handle->cpmsacklist->priority > cpmsplist->cpmsacklist->priority) {
                    
                list_push(plist, cpmsplist);
                PRINTF("retx push to the beginning\n");
                PRINTFUNC();
                return 1;

            } else {

                struct cpmspriority_list *list_previous = list_handle;

                while (list_handle != NULL) {

                    if (list_handle->cpmsacklist->priority > cpmsplist->cpmsacklist->priority) {
                    
                        list_insert(plist, list_previous, cpmsplist);
                        PRINTF("retx element inserted to a specific place\n");
                        PRINTFUNC();
                        return 1;
                    }

                    list_previous = list_handle;
                    list_handle = list_item_next(list_handle);
                }

                list_add(plist, cpmsplist);
                PRINTF("retx element add to the tail\n");
                PRINTFUNC();
                return 1;
            }
        } else {
            // have list, but current struct has highest priority, or priority corresponded highest rssi
            if ((list_handle->cpmsacklist->priority > cpmsplist->cpmsacklist->priority) ||
                ((list_handle->cpmsacklist->priority == cpmsplist->cpmsacklist->priority) && 
                (list_handle->rssi <= cpmsplist->rssi))) {

                list_push(plist, cpmsplist);
                PRINTF("push to the beginning\n");
                PRINTFUNC();
                return 1;
            } else {
                // neither highest priority priority, nor lowest rssi
                struct cpmspriority_list *list_previous = list_handle;

                while (list_handle != NULL) {
                    
                    if (list_handle->cpmsacklist->priority == cpmsplist->cpmsacklist->priority) {
                        if (list_handle->rssi <= cpmsplist->rssi) {
                            list_insert(plist, list_previous, cpmsplist);
                            PRINTF("element inserted to a specific place\n");
                            PRINTFUNC();
                            return 1;
                        }
                    } else if (list_handle->cpmsacklist->priority > cpmsplist->cpmsacklist->priority) {
                        list_insert(plist, list_previous, cpmsplist);
                        PRINTF("element inserted to a specific place\n");
                        PRINTFUNC();
                        return 1;
                        
                    } else {
                        // do nothing
                    }

                    list_previous = list_handle;
                    list_handle = list_item_next(list_handle);
                }

                // current struct has lowest priority with this priority's lowest rssi
                list_add(plist, cpmsplist);
                PRINTF("element add to the tail\n");
                PRINTFUNC();
                return 1;
            } 
        }
        
    }

    return 0;
}

int
cpmsplist_create(int rssi, const linkaddr_t *addr, struct cpmsack_list *cpmsacklist)
{
    struct cpmspriority_list *cpmsplist;

    cpmsplist = memb_alloc(&plist_memb);

    if (cpmsplist != NULL) {
        cpmsplist->rssi = rssi;
        cpmsplist->re_tx = 0;
        linkaddr_copy(&cpmsplist->addr, addr);
        cpmsplist->cpmsacklist = cpmsacklist;

        PRINTF("plist struct created, rssi: %d, addr: %d.%d\n", cpmsplist->rssi, 
        cpmsplist->addr.u8[0], cpmsplist->addr.u8[1]);
    }
    
    update_cpmsplist(cpmsplist);

    return 1;
}

void *
cpmsplist_get()
{
    return list_pop(plist);
}

int 
cpmsplist_retx_update(struct cpmspriority_list *cpmsplist)
{
    if (++cpmsplist->re_tx == 2) {
        if (++cpmsplist->cpmsacklist->priority > CPMS_PRIORITY_LEVEL_MAX) {
            return 0;
        }
    } else if (cpmsplist->re_tx == 3) {
        return 0;
    } else {
        // do nothing here
    }

    PRINTF("updated priority: %d, updated re_tx: %d\n", 
        cpmsplist->cpmsacklist->priority, cpmsplist->re_tx);

    return update_cpmsplist(cpmsplist);
}

int 
cpmsplist_free()
{
    // free the list and its corresponding memory
    while (list_head(plist) != NULL) {
        struct cpmspriority_list *cpmsplist = list_chop(plist);
        free(cpmsplist->cpmsacklist);
        memb_free(&plist_memb, cpmsplist);
    }
    
    PRINTF("plist is freed\n");

    return 1;
}