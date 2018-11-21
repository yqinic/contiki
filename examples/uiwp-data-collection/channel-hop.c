
#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/button-sensor.h"
#include "core/net/netstack.h"

#include "rf-core/rf-core.h"

#include "dev/leds.h"

#include <stdio.h>

PROCESS(channel_hop_test, "channel hop");
AUTOSTART_PROCESSES(&channel_hop_test);

PROCESS_THREAD(channel_hop_test, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();

  while(1) {

    etimer_set(&et, CLOCK_SECOND*10);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    leds_toggle(LEDS_GREEN);

    int op_ch;
    op_ch = channel_scan();

    printf("optimum channel: %d\n", op_ch);
  }

  PROCESS_END();
}