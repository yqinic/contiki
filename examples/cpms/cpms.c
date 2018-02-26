#include "contiki.h"
#include "channel-scan.c"
#include "net/rime/rime.h"
#include "bunicast.h"
#include <stdio.h>

PROCESS(cpms, "cpms");
AUTOSTART_PROCESSES(&cpms);

static void
recv_uc(struct bunicast_conn *c, const linkaddr_t *from)
{
  printf("bunicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);
}
/*---------------------------------------------------------------------------*/
static void
sent_uc(struct bunicast_conn *c, int status)
{
  printf("bunicast message sent to %d.%d: status %d\n",
    dest->u8[0], dest->u8[1], status);
}
/*---------------------------------------------------------------------------*/
static const struct bunicast_callbacks bunicast_callbacks = {recv_uc, sent_uc};
static struct bunicast_conn buc;

PROCESS_THREAD(cpms, ev, data)
{
	PROCESS_BEGIN();

	unicast_open(&buc, 137, &bunicast_callbacks);

	static struct etimer et;

	while(1)
	{
		etimer_set(&et, CLOCK_SECOND * 2);

		// int chn = channel_scan();

		// printf("channel: %d\n", chn);

		char data[700];
		int i;
		for(i=0;i<700;i++) {
			data[i] = char(i%10);
		}

		linkaddr_t addr;

    	addr.u8[0] = 155;
    	addr.u8[1] = 150;
      
	  	bunicast_send(&buc, &addr, data);

		PROCESS_WAIT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}
