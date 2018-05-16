#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include "dev/leds.h"

#define BUNICAST_SENDER 0

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

int bsize = 0;
int bmax = 800;
int bgmax = 160;

#if BUNICAST_SENDER

char sp1[80];
char sp2[160];
char sp3[320];
char sp4[640];
char sp5[1280];

char *str_pointer[5] = {sp1, sp2, sp3, sp4, sp5};

linkaddr_t addr;

static int power(int exponent) {
    int i, result = 1, base = 2;
    for(i=0; i<exponent; i++)
        result *= base;
    return result;
}

#else

unsigned long time[5] = {0};

#endif

PROCESS(cpms_bunicast_process, "cpms_bunicast");
AUTOSTART_PROCESSES(&cpms_bunicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_buc(struct bunicast_conn *c, const linkaddr_t *from)
{
#if !BUNICAST_SENDER

    static int brecv[5] = {0};
    static unsigned long time_tmp = 0;

    char *data = (char *)packetbuf_dataptr();

    switch(data[0]) {
        case '0':
            if (brecv[0] == 0)
                time_tmp = clock_time();
            brecv[0] += 1;
            if (brecv[0]%bgmax == 0) {
                time[0] += clock_time() - time_tmp;
                time_tmp = clock_time();
            }
            break;
        case '1':
            brecv[1] += 1;
            if (brecv[1]%bgmax == 0) {
                time[1] += clock_time() - time_tmp;
                time_tmp = clock_time();
            }
            break;
        case '2':
            brecv[2] += 1;
            if (brecv[2]%bgmax == 0) {
                time[2] += clock_time() - time_tmp;
                time_tmp = clock_time();
            }
            break;
        case '3':
            brecv[3] += 1;
            if (brecv[3]%bgmax == 0) {
                time[3] += clock_time() - time_tmp;
                time_tmp = clock_time();
            }
            break;
        case '4':
            brecv[4] += 1;
            if (brecv[4]%bgmax == 0) {
                time[4] += clock_time() - time_tmp;
                time_tmp = clock_time();
                printf("time_b0: %lu, time_b1: %lu, time_b2: %lu, time_b3: %lu, time_b4: %lu\n",
                    time[0], time[1], time[2], time[3], time[4]);
            }
            break;
        default:
            printf("Error in receiving!\n");
    }

    PRINTF("brecv1:%d, brecv2:%d, brecv3:%d, brecv4:%d, brecv5:%d\n",
        brecv[0],brecv[1],brecv[2],brecv[3],brecv[4]);
    
    bsize += 1;

#endif
}
/*---------------------------------------------------------------------------*/
static void
sent_buc(struct bunicast_conn *c, int status)
{
#if BUNICAST_SENDER
    bsize += c->block_size;
    int size_tmp = (bsize%bmax)/bgmax;

    PRINTF("size index: %d, count: %d, status: %d\n", size_tmp, bsize, status);    

    bunicast_size(c, power(size_tmp));
    bunicast_send(c, &addr, str_pointer[size_tmp]);
#endif
}
/*---------------------------------------------------------------------------*/
static const struct bunicast_callbacks bunicast_callbacks = {recv_buc, sent_buc};
static struct bunicast_conn buc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cpms_bunicast_process, ev, data)
{
    PROCESS_EXITHANDLER(bunicast_close(&buc));
    
    PROCESS_BEGIN();

    bunicast_open(&buc, 123, &bunicast_callbacks);

#if BUNICAST_SENDER
    int i,j;

    for(i=0; i<5; i++) {
        for(j=0; j<power(i)*FRAME_DATA_SIZE; j++)
            str_pointer[i][j] = i + '0';
        str_pointer[i][power(i)*FRAME_DATA_SIZE-1] = '\0';
    }

    addr.u8[0] = 60;
    addr.u8[1] = 130;

    bunicast_size(&buc, 1);
    bunicast_send(&buc, &addr, str_pointer[0]);
#else
    while(1) {
        static struct etimer et;
        etimer_set(&et, CLOCK_SECOND * 60);
            
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }
#endif

    PROCESS_END();
}