#include "contiki.h"
#include "net/rime/rime.h"
#include "sys/ctimer.h"
#include <stdlib.h>
#include <stdio.h>

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define CPMS_ACK_EXPIRE 1
#define CPMS_REQUEST_EXPIRE 2
#define CPMS_GLOREQ_EXPIRE 10
#define CPMS_DATA_EXPIRE 2

#define COMMAND_CHANNEL CHANNEL_OFFSET

PROCESS(sink_process, "Sink Process");
PROCESS(broadcast_process, "Broadcast Process");
AUTOSTART_PROCESSES(&sink_process);

static struct broadcast_conn bc;
static struct unicast_conn uc;
static struct bunicast_conn buc;

static struct ctimer request_timer;
static struct ctimer globalreq_timer;
static struct ctimer data_timer;
static struct ctimer globaldata_timer;

struct cpmspriority_list *global_cpmsplist;

static int data_channel = 11;
static int unicast_ack_collecting = 0;
static int bunicast_data_receiving = 0;

#if DEBUG
static unsigned long time = 0;
#endif

static void
request_list_runout()
{
    PRINTF("priority list run out, start broadcast process\n");

    unicast_ack_collecting = 0;
    ctimer_stop(&request_timer);
    ctimer_stop(&globalreq_timer);
    process_start(&broadcast_process, NULL);
}


static void 
unicast_request()
{
    // create data request frame
    uint8_t *buf;
    buf = malloc(CPMSREQUEST_FRAME_LENGTH);
    struct cpmspriority_list *cpmsplist = cpmsplist_get();

    if (cpmsplist == NULL) {

        request_list_runout();
        return;
    }

    cpmsrequest_frame_create(data_channel, 0, cpmsplist->cpmsacklist->bytes, buf);

    PRINTF("data channel: %d, send to %d.%d\n", 
        data_channel, cpmsplist->addr.u8[0], cpmsplist->addr.u8[1]);

    // send data request frame on command channel
    packetbuf_copyfrom(buf, CPMSREQUEST_FRAME_LENGTH);
    unicast_send(&uc, &cpmsplist->addr);
    free(buf);

    // // set expire time
    // ctimer_set(&request_timer, CLOCK_SECOND * CPMS_REQUEST_EXPIRE, request_expire, cpmsplist);
    global_cpmsplist = cpmsplist;
}

static void
request_expire(void *ptr)
{
    PRINTF("data request expire, resend from priority list\n");

    // hop back to command channel
    channel_hop(COMMAND_CHANNEL);

    struct cpmspriority_list *cpmsplist = ptr;

    // retransmission + 1
    if (!cpmsplist_retx_update(cpmsplist)) {

        request_list_runout();
        return;
    }

    unicast_request();
}

static void
global_request_expire()
{
    PRINTF("global data request expires, back to broadcast\n");

    channel_hop(COMMAND_CHANNEL);
    
    // data request unicast expires, back to broadcast
    ctimer_stop(&request_timer);
    cpmsplist_free();
    unicast_ack_collecting = 0;
    process_start(&broadcast_process, NULL);
}

static void
unicast_request_from_ack()
{
    // unicast ack received time expires, disable it
    unicast_ack_collecting = 2;
    PRINTF("unicast ack received time expires, data request begins\n");

    // channel scan to obtain optimal data channel
    data_channel = channel_scan();

    unicast_request();

    // set global unicast request expire time
    ctimer_set(&globalreq_timer, CLOCK_SECOND * CPMS_GLOREQ_EXPIRE, global_request_expire, NULL);
}

// broadcast communication
static void
recv_bc(struct broadcast_conn *c, const linkaddr_t *from)
{
    PRINTF("broadcast message received from %d.%d\n",
	    from->u8[0], from->u8[1]);
}

static void
sent_bc(struct broadcast_conn *c, int status, int num_tx)
{
    PRINTF("broadcast message sent: status %d num_tx %d\n", status, num_tx);
}

static const struct broadcast_callbacks broadcast_callbacks = {recv_bc, sent_bc};

// unicast communication
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{

    PRINTF("unicast message received from %d.%d\n", from->u8[0], from->u8[1]);

    // acknowledgment frame received
    struct cpmsack_list *cpmsacklist = cpmsack_frame_parse((uint8_t *)packetbuf_dataptr());
    int rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

    if (unicast_ack_collecting == 0) {
        // when first ack frame received
        // free previous priority list
        // set expire time
        // stop broadcasting
        // initialize priority list

        unicast_ack_collecting = 1;
    #if DEBUG
        time = clock_time();
    #endif
        static struct ctimer ack_timer;
        ctimer_set(&ack_timer, CLOCK_SECOND * CPMS_ACK_EXPIRE, unicast_request_from_ack, NULL);
        cpmsplist_free();
        cpmsplist_create(rssi, from, cpmsacklist);
        process_post(&sink_process, PROCESS_EVENT_MSG, NULL);
        PRINTF("first unicast ack received\n");

    } else if (unicast_ack_collecting == 1) {
        // update priority list
        cpmsplist_create(rssi, from, cpmsacklist);
        PRINTF("keep receiving unicast ack...\n");

    } else {
        // ack collecting time expires, do nothing
        PRINTF("ack collecting time expires, nothing here\n");
    }  
}

static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{
    const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
    if(linkaddr_cmp(dest, &linkaddr_null)) {
        return;
    }
    PRINTF("unicast message sent to %d.%d: status %d num_tx %d\n",
        dest->u8[0], dest->u8[1], status, num_tx);


    if (unicast_ack_collecting == 2) {
        // channel hop
        channel_hop(data_channel);

        // set expire time
        ctimer_set(&request_timer, CLOCK_SECOND * CPMS_REQUEST_EXPIRE, request_expire, global_cpmsplist);
    }
    
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};

// bunicast communication
static void
recv_buc(struct bunicast_conn *c, const linkaddr_t *from)
{
    printf("bunicast message received from %d.%d, msg: %s\n",
	    from->u8[0], from->u8[1], (char *)packetbuf_dataptr());

    unicast_ack_collecting = 0;

#if DEBUG
    time = clock_time() - time;
    double t = (double)time/CLOCK_SECOND;
    PRINTF("time used: %d.%d\n", (int)t, (int)(100*(t-(int)t)));
#endif

    if (bunicast_data_receiving == 0) {
        ctimer_stop(&request_timer);
        ctimer_stop(&globalreq_timer);
    }
    
}

static void
sent_buc(struct bunicast_conn *c, int *status)
{
    const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
    PRINTF("bunicast message sent to %d.%d: status %s\n",
        dest->u8[0], dest->u8[1], (char *)status);
}

static const struct bunicast_callbacks bunicast_callbacks = {recv_buc, sent_buc};

static void
com_init()
{
    broadcast_open(&bc, 127, &broadcast_callbacks);
    unicast_open(&uc, 146, &unicast_callbacks);
    bunicast_open(&buc, 135, &bunicast_callbacks);
}

static void
broadcast_process_start(void *ptr)
{
    PRINTF("sensor data transmission expired, back to broadcast process\n");

    channel_hop(COMMAND_CHANNEL); 

    struct process *b_process = ptr;
    process_start(b_process, NULL);
}

PROCESS_THREAD(sink_process, ev, data)
{
    PROCESS_BEGIN();

    com_init();
    channel_hop(COMMAND_CHANNEL);

    // static struct ctimer ct;

    while(1) {
        PRINTF("PROCESS_EVENT_MSG: %d\n", (int)ev);

        if (ev == PROCESS_EVENT_MSG) {
            process_exit(&broadcast_process);
            // unicast_ack_collecting = 0;
            PRINTF("broadcast terminated\n");
            // clock_time_t delay = CLOCK_SECOND * 5;
            // ctimer_set(&ct, CPMS_DATA_EXPIRE, broadcast_process_start, &broadcast_process);
        } else if (ev == PROCESS_EVENT_INIT) {
            process_start(&broadcast_process, NULL);
        } else {
            ;
        }

        PROCESS_YIELD();
        PRINTF("process yielded\n");
        // PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_MSG);
    }

    PROCESS_END();
}

PROCESS_THREAD(broadcast_process, ev, data)
{
    PROCESS_BEGIN();

    static struct etimer et;

    while(1) {
        etimer_set(&et, CLOCK_SECOND * 5);

        packetbuf_copyfrom("hi", 2);
        broadcast_send(&bc);
        PRINTF("broadcast msg sent by sink\n");

        PROCESS_WAIT_UNTIL(etimer_expired(&et));
    }

    PROCESS_END();
}