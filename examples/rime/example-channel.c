#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/cc2420/cc2420.h"
#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static void
broadcast_sent(struct broadcast_conn *c, int status, int num_tx)
{
  printf("status: %d\n", status);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv, broadcast_sent};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  
    cc2420_set_channel(20);
    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    printf("broadcast message sent, ch:%d\n", cc2420_get_channel());

    }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
