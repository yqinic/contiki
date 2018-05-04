#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

int degree = 0;

/*---------------------------------------------------------------------------*/
PROCESS(antenna_test_process, "Antenna Test");
AUTOSTART_PROCESSES(&antenna_test_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  // printf("unicast message received from %d.%d\n",
	//  from->u8[0], from->u8[1]);

  char buf[40];
  sprintf(buf, "deg: %s, rssi: %d.%c", 
    (char *)packetbuf_dataptr(), packetbuf_attr(PACKETBUF_ATTR_RSSI), '\0');
  
  printf("%s\n", buf);
}
/*---------------------------------------------------------------------------*/
static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

  printf("unicast message sent to %d.%d: status %d num_tx %d\n",
    dest->u8[0], dest->u8[1], status, num_tx);
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(antenna_test_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  unicast_open(&uc, 146, &unicast_callbacks);

  SENSORS_ACTIVATE(button_sensor);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			     data == &button_sensor);

    linkaddr_t addr;

    addr.u8[0] = 155;
    addr.u8[1] = 150;

    leds_toggle(LEDS_RED);
    degree += 2;

    char buf[10];
    sprintf(buf, "%d%c", degree, '\0');
    packetbuf_copyfrom(buf, strlen(buf));

    unicast_send(&uc, &addr);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
