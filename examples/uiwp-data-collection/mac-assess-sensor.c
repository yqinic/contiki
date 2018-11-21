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

int count = 0;
static linkaddr_t addr;
static unsigned long time_stat = 0;
static struct ctimer send_timer;

static void send_packet();
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
    // if (status == 0)
    //     PRINTF("status: %d, time elapse: %lu\n", status, rtimer_arch_now() - time_stat);
    
    ctimer_set(&send_timer, random_rand() % CLOCK_SECOND, send_packet, NULL);
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv, broadcast_sent};
static struct broadcast_conn broadcast;

static const struct unicast_callbacks unicast_call = {unicast_recv, unicast_sent};
static struct unicast_conn unicast;

static void send_packet() {
    packetbuf_copyfrom("thisisa40bytemessagefortestingpurpose!!!", 40);
    unicast_send(&unicast, &addr);
    // if (count % 30 == 0) {
    //     PRINTF("count: %d, transmit time: %lu, listening time: %lu, run time: %lu\n", 
    //         count,
    //         energest_type_time(ENERGEST_TYPE_TRANSMIT), 
    //         energest_type_time(ENERGEST_TYPE_LISTEN),
    //         rtimer_arch_now());
    // }
    // count += 1;
    
    // time_stat = rtimer_arch_now();
}
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
