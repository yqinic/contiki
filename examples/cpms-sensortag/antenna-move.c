#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "button-sensor.h"
#include "core/net/netstack.h"
#include "dev/leds.h"

#include <stdio.h>

#define NUM_PACKETS_SEND 100

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

int count = NUM_PACKETS_SEND;

static int antenna_send_set(int txset) {
    static int txsend = 0;

    if (txset) {
        txsend = (txsend + 1) % 2;
        return -1;
    } else
        return txsend;
}

/*---------------------------------------------------------------------------*/
PROCESS(button_click, "Button Click");
// PROCESS(antenna_test, "Antenna Test");
AUTOSTART_PROCESSES(&button_click);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);

static void
broadcast_sent(struct broadcast_conn *c, int status, int num_tx)
{
    PRINTF("status: %d\n", status);
}

static void
unicast_recv(struct unicast_conn *c, const linkaddr_t *from)
{
    PRINTF("unicast message received from %d.%d: '%s'\n",
        from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
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

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
    PRINTF("broadcast message received from %d.%d: '%s'\n",
            from->u8[0], from->u8[1], (char *)packetbuf_dataptr());

    if (antenna_send_set(0) == 1) {
        if (count > 0) {
            char buf[20];
            sprintf(buf, "%hd,%d", 
                packetbuf_attr(PACKETBUF_ATTR_RSSI), count);
            packetbuf_copyfrom(buf, strlen(buf));
            count -= 1;
            unicast_send(&unicast, from);
        } else {
            count = NUM_PACKETS_SEND;
            antenna_send_set(1);
        }
    }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(button_click, ev, data)
{
    PROCESS_BEGIN();

    broadcast_open(&broadcast, 129, &broadcast_call);
    unicast_open(&unicast, 137, &unicast_call);
    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, 5);

    // SENSORS_ACTIVATE(button_left_sensor);
    // SENSORS_ACTIVATE(button_right_sensor);

    while(1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);

        if (data == &button_left_sensor) {
            antenna_send_set(1);
            // process_start(&antenna_test, NULL);
        } else 
            PRINTF("do nothing\n");
        
    }

    PROCESS_END();
}

// PROCESS_THREAD(antenna_test, ev, data)
// {
//     PROCESS_EXITHANDLER(unicast_close(&unicast);)

//     PROCESS_BEGIN();

//     unicast_open(&unicast, 137, &unicast_call);

//     static struct etimer et;

//     for (count = 0; count < 20; count++) {
//         etimer_set(&et, CLOCK_SECOND * 0.5);
//         PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

//         packetbuf_copyfrom("antennatx", 9);
//         unicast_send(&unicast);
//         leds_toggle(LEDS_RED);
//     }

//     unicast_close(&unicast);

//     PROCESS_END();
// }
/*---------------------------------------------------------------------------*/
