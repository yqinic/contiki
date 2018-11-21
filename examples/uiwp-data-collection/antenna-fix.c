#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "button-sensor.h"
#include "core/net/netstack.h"

#include "dev/leds.h"

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

int count;

/*---------------------------------------------------------------------------*/
PROCESS(antenna_test, "Antenna Test");
AUTOSTART_PROCESSES(&antenna_test);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
    PRINTF("broadcast message received from %d.%d: '%s'\n",
            from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static void
broadcast_sent(struct broadcast_conn *c, int status, int num_tx)
{
    PRINTF("status: %d\n", status);
}

static void
unicast_recv(struct unicast_conn *c, const linkaddr_t *from)
{
    printf("%s\n", (char *)packetbuf_dataptr());
}

static void
unicast_sent(struct unicast_conn *c, int status, int num_tx)
{
    PRINTF("status: %d\n", status);
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv, broadcast_sent};
static struct broadcast_conn broadcast;

static const struct unicast_callbacks unicast_call = {unicast_recv, unicast_sent};
static struct unicast_conn unicast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(antenna_test, ev, data)
{
    PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

    PROCESS_BEGIN();

    broadcast_open(&broadcast, 129, &broadcast_call);
    unicast_open(&unicast, 137, &unicast_call);
    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, 0);

    static struct etimer et;

    while(1) {
        etimer_set(&et, CLOCK_SECOND * 0.2);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        // linkaddr_t addr;
        // addr.u8[0] = 99;
        // addr.u8[1] = 100;

        packetbuf_copyfrom("antennabc", 9);
        broadcast_send(&broadcast);
        // unicast_send(&unicast,&addr);
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
