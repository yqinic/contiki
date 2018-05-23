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

#define COMMAND_CHANNEL CHANNEL_OFFSET

PROCESS(sensor_process, "Sensor Process");
AUTOSTART_PROCESSES(&sensor_process);

static struct broadcast_conn bc;
static struct unicast_conn uc;
static struct bunicast_conn buc;

// broadcast communication
static void
recv_bc(struct broadcast_conn *c, const linkaddr_t *from)
{
    PRINTF("broadcast message received from %d.%d\n",
	    from->u8[0], from->u8[1]);

    // acknowledgment frame

    uint8_t buf[CPMSACK_FRAME_LENGTH];
    int num = 32;
    cpmsack_frame_create(num, buf);

    PRINTF("send ack frame\n");

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

    PRINTF("unicast message received from %d.%d\n", from->u8[0], from->u8[1]);

    // data request frame received
    // bunicast data to sink

    struct cpmsrequest_list cpmsrequestlist;
    cpmsrequest_frame_parse((uint8_t *)packetbuf_dataptr(), &cpmsrequestlist);

    if (channel_hop(cpmsrequestlist.channel)) {
        PRINTF("bunicast send to sink, channel: %d\n", cpmsrequestlist.channel);

        char *buf = "hello world\n";
        bunicast_send(&buc, from, buf);
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
    printf("bunicast message received from %d.%d, msg: %s\n",
	    from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static void
sent_buc(struct bunicast_conn *c, int status)
{
    const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
    PRINTF("bunicast message sent to %d.%d: status %d\n",
        dest->u8[0], dest->u8[1], status);

    channel_hop(COMMAND_CHANNEL);
}

static const struct bunicast_callbacks bunicast_callbacks = {recv_buc, sent_buc};

static void
com_init()
{
    broadcast_open(&bc, 127, &broadcast_callbacks);
    unicast_open(&uc, 146, &unicast_callbacks);
    bunicast_open(&buc, 135, &bunicast_callbacks);
    channel_hop(COMMAND_CHANNEL);
}

PROCESS_THREAD(sensor_process, ev, data)
{
    PROCESS_BEGIN();

    com_init();

    static struct etimer et;

    while(1) {
        etimer_set(&et, CLOCK_SECOND * 40);

        // sensor applications to be put here 

        PROCESS_WAIT_UNTIL(etimer_expired(&et));
    }

    PROCESS_END();
}