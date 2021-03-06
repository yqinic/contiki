#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "button-sensor.h"
#include "core/net/netstack.h"

#include "dev/leds.h"

#include <stdio.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static int count = 0;
static int err = 0;
static linkaddr_t addr;
static unsigned long time_stat = 0;

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
    PRINTF("%s\n", (char *)packetbuf_dataptr());
}

static void
unicast_sent(struct unicast_conn *c, int status, int num_tx)
{
    if (status == 0) {
        printf("time: %lu, err: %d, ct: %d\n", rtimer_arch_now() - time_stat,
            err, count);
    } else {
        err += 1;
    }
    count += 1;

    packetbuf_copyfrom("thisisa40bytemessagefortestingpurpose!!!", 40);
    unicast_send(c, &addr);
    time_stat = rtimer_arch_now();
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

    // static struct etimer et;

    addr.u8[0] = 163;
    addr.u8[1] = 129;

    while(1) {
        // etimer_set(&et, CLOCK_SECOND);
        // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    
        packetbuf_copyfrom("thisisa40bytemessagefortestingpurpose!!!", 40);
        unicast_send(&unicast, &addr);
        count += 1;
        time_stat = rtimer_arch_now();

        PROCESS_YIELD();
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
