#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include "dev/leds.h"

#define BUNICAST_SENDER 0
#define TEST_SIZE 6

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

int bsize = 0;
int bgmax = 3200;
int bmax = 3200 * TEST_SIZE;


#if BUNICAST_SENDER

char sp1[80];
char sp2[160];
char sp3[320];
char sp4[640];
char sp5[1280];
char sp6[2560];

char *str_pointer[TEST_SIZE] = {sp1, sp2, sp3, sp4, sp5, sp6};

unsigned long transmit_time[TEST_SIZE] = {0};

linkaddr_t addr;

static int power(int exponent) {
    int i, result = 1, base = 2;
    for(i=0; i<exponent; i++)
        result *= base;
    return result;
}

#else

unsigned long time[TEST_SIZE] = {0};

static unsigned long 
time_update(int idx, unsigned long time_tmp) {
    time[idx] += clock_time() - time_tmp;
    time_tmp = clock_time();

    return time_tmp;
}

static void print_result(int burecv[]) {

    int i;

    for (i = 0; i < TEST_SIZE; i++) {
        if (i == TEST_SIZE - 1)
            printf("time_b%d: %lu\n", i, time[i]);
        else
            printf("time_b%d: %lu, ", i, time[i]);
    }

    int sum_of_burecv = 0;

    for(i = 0; i < 6;i++)
        sum_of_burecv += burecv[i];

    if (sum_of_burecv != 0) {
        for (i = 0; i < TEST_SIZE; i++) {
            if (i == TEST_SIZE - 1)
                printf("loss_b%d: %d\n", i, burecv[i]);
            else
                printf("loss_b%d: %d, ", i, burecv[i]);
        }
    }   
}

#endif

PROCESS(cpms_bunicast_process, "cpms_bunicast");
AUTOSTART_PROCESSES(&cpms_bunicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_buc(struct bunicast_conn *c, const linkaddr_t *from)
{
#if !BUNICAST_SENDER

    static int brecv[TEST_SIZE] = {0};
    static unsigned long time_tmp = 0;
    static int burecv[TEST_SIZE] = {0};

    char *data = (char *)packetbuf_dataptr();

    int index = data[0] - '0';
    int idx_prev = (index + TEST_SIZE - 1) % TEST_SIZE;

    if (brecv[0] == 0)
        time_tmp = clock_time();
    else {
        if (brecv[idx_prev] % bgmax != 0) {
            int diff = bgmax - brecv[idx_prev];
            brecv[idx_prev] += diff;
            burecv[idx_prev] += diff;
            time_tmp = time_update(idx_prev, time_tmp);
            if (idx_prev == TEST_SIZE - 1)
                print_result(burecv);
        }
    }

    brecv[index] += 1;

    if (brecv[index] % bgmax == 0) {
        time_tmp = time_update(index, time_tmp);
        if (index == TEST_SIZE - 1)
            print_result(burecv);
            
    }

    int i;
    for(i = 0; i < TEST_SIZE; i++) {
        if (i == TEST_SIZE - 1)
            PRINTF("brecv%d: %d\n", i, brecv[i]);
        else
            PRINTF("brecv%d: %d, ", i, brecv[i]);
    }

    bsize += 1;

#endif
}
/*---------------------------------------------------------------------------*/
static void
sent_buc(struct bunicast_conn *c, int status)
{
#if BUNICAST_SENDER

    static unsigned long transmit_time_tmp = 0;

    transmit_time[(bsize % bmax) / bgmax] += 
        energest_type_time(ENERGEST_TYPE_TRANSMIT) - transmit_time_tmp;
    transmit_time_tmp = energest_type_time(ENERGEST_TYPE_TRANSMIT);

    bsize += c->block_size;
    int size_tmp = (bsize % bmax) / bgmax;

    if (bsize % bmax == 0) {
        int i;
        printf("energy estimated: ");
        for (i = 0; i < TEST_SIZE; i++) {
            if (i == TEST_SIZE - 1)
                printf("%lu\n", transmit_time[i]);
            else
                printf("%lu, ", transmit_time[i]);
        }
    }

    PRINTF("size index: %d, count: %d, status: %d\n", size_tmp, bsize, status);
    PRINTF("energy estimated: %lu, %lu, %lu, %lu, %lu\n",
        transmit_time[0]/RTIMER_ARCH_SECOND, transmit_time[1]/RTIMER_ARCH_SECOND,
        transmit_time[2]/RTIMER_ARCH_SECOND, transmit_time[3]/RTIMER_ARCH_SECOND,
        transmit_time[4]/RTIMER_ARCH_SECOND);    

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

    energest_init();

    int i,j;

    for(i = 0; i < TEST_SIZE; i++) {
        for(j = 0; j < power(i) * FRAME_DATA_SIZE; j++)
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