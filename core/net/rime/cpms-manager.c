#include "contiki.h"
#include "net/rime/rime.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(sink_process, "Sink Process");
PROCESS(sensor_process, "Sensor Process");

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
static struct broadcast_conn bc;

// unicast communication
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
    PRINTF("unicast message received from %d.%d\n",
	    from->u8[0], from->u8[1]);
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
static struct unicast_conn uc;

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
    PRINTF("bunicast message sent to %d.%d: status %s\n",
        dest->u8[0], dest->u8[1], (char *)status);
}

static const struct bunicast_callbacks bunicast_callbacks = {recv_buc, sent_buc};
static struct bunicast_conn buc;

static void
com_init()
{
    broadcast_open(&bc, 127, &broadcast_callbacks);
    unicast_open(&uc, 146, &unicast_callbacks);
    bunicast_open(&buc, 135, &bunicast_callbacks);
}

PROCESS_THREAD(sink_process, ev, data)
{
    PROCESS_BEGIN();

    com_init();

    while(1) {
 

    }

    PROCESS_END();
}

PROCESS_THREAD(sensor_process, ev, data)
{
    PROCESS_BEGIN();

    com_init();

    while(1) {
 

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