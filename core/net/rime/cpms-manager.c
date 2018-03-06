#include "contiki.h"
#include "net/rime/rime.h"
#include "sys/ctimer.h"
#include <stdlib.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define CPMS_ACK_EXPIRE 20
#define CPMS_REQUEST_EXPIRE 1
#define CPMS_DATA_EXPIRE 2

#define COMMAND_CHANNEL 26

PROCESS(sink_process, "Sink Process");
PROCESS(sensor_process, "Sensor Process");
PROCESS(broadcast_process, "Broadcast Process");

static struct broadcast_conn bc;
static struct unicast_conn uc;
static struct bunicast_conn buc;

static int unicast_ack_collecting = 0;

static void
unicast_request()
{
    // unicast_ack_collecting = 2;
    PRINTF("unicast ack received time expires, data request begins\n");
}

// broadcast communication
static void
recv_bc(struct broadcast_conn *c, const linkaddr_t *from)
{
    PRINTF("broadcast message received from %d.%d\n",
	    from->u8[0], from->u8[1]);

    // acknowledgment frame
    uint8_t *buf;
    buf = malloc(CPMSACK_FRAME_LENGTH);
    int num = 32;
    cpmsack_frame_create(num, buf);

    packetbuf_copyfrom(buf, CPMSACK_FRAME_LENGTH);
    unicast_send(&uc, from);
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
    int frame_type = (*(char *)packetbuf_dataptr() >> 6) & 3;

    PRINTF("unicast message received from %d.%d, frame type: %d\n",
	    from->u8[0], from->u8[1], frame_type);

    if (frame_type == 1) {
        // acknowledgment frame received
        struct cpmsack_list *cpmsacklist = cpmsack_frame_parse((uint8_t *)packetbuf_dataptr());
        int rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

        if (unicast_ack_collecting == 0) {
            // when first ack frame received
            // set expire time
            // stop broadcasting
            // initialize priority list

            struct ctimer ct;
            // ctimer_set(&ct, CLOCK_SECOND * CPMS_ACK_EXPIRE, unicast_request, NULL);
            cpmsplist_create(rssi, from, cpmsacklist);
            process_post(&sink_process, PROCESS_EVENT_MSG, NULL);
            unicast_ack_collecting = 1;
            PRINTF("first unicast ack received\n");

        } else if (unicast_ack_collecting == 1) {
            // update priority list
            cpmsplist_create(packetbuf_attr(PACKETBUF_ATTR_RSSI), from, cpmsacklist);
            PRINTF("keep receiving unicast ack...\n");

        } else {
            // ack collecting time expires, do nothing
            PRINTF("nothing here\n");
        }
    } else if (frame_type == 2) {
        // data request frame received
        // bunicast data to sink
    } else {
        // unrecognized frame, do nothing
        ;
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
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};

// bunicast communication
static void
recv_buc(struct bunicast_conn *c, const linkaddr_t *from)
{
    PRINTF("bunicast message received from %d.%d\n",
	    from->u8[0], from->u8[1]);
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

    static struct ctimer ct;

    while(1) {
        PRINTF("PROCESS_EVENT_MSG: %d\n", (int)ev);

        if(ev == PROCESS_EVENT_MSG) {
            process_exit(&broadcast_process);
            PRINTF("broadcast terminated\n");
            // clock_time_t delay = CLOCK_SECOND * 5;
            // ctimer_set(&ct, CPMS_DATA_EXPIRE, broadcast_process_start, &broadcast_process);
        } else {
            // process_start(&broadcast_process, NULL);
        }

        PROCESS_YIELD();
    }

    PROCESS_END();
}

PROCESS_THREAD(sensor_process, ev, data)
{
    PROCESS_BEGIN();

    com_init();

    static struct etimer et;

    while(1) {
        etimer_set(&et, CLOCK_SECOND * 1);

        // sensor applications to be put here 
        uint8_t *buf;
        buf = malloc(CPMSACK_FRAME_LENGTH);
        int num = 32;
        cpmsack_frame_create(num, buf);

        linkaddr_t addr;
        addr.u8[0] = 155;
        addr.u8[1] = 150;

        packetbuf_copyfrom(buf, CPMSACK_FRAME_LENGTH);
        unicast_send(&uc, &addr);

        PROCESS_WAIT_UNTIL(etimer_expired(&et));
    }

    PROCESS_END();
}

PROCESS_THREAD(broadcast_process, ev, data)
{
    PROCESS_BEGIN();

    static struct etimer et;

    while(1) {
        etimer_set(&et, CLOCK_SECOND);

        packetbuf_copyfrom("hi", 2);
        broadcast_send(&bc);
        PRINTF("broadcast msg sent by sink\n");

        PROCESS_WAIT_UNTIL(etimer_expired(&et));
    }

    PROCESS_END();
}

// sink node process will be initiated
void 
cpms_sink_init()
{
    process_start(&sink_process, NULL);
}

// sensor node process will be initiated
void 
cpms_sensor_init()
{
    process_start(&sensor_process, NULL);
}