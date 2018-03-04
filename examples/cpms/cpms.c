#include "contiki.h"

//#include "channel-scan.c"
#include "net/rime/rime.h"

#include <stdio.h>

PROCESS(cpms, "cpms");
AUTOSTART_PROCESSES(&cpms);

static void
recv_buc(struct bunicast_conn *c, const linkaddr_t *from)
{;
  	printf("bunicast message received from %d.%d: %s\n", from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static void
sent_buc(struct bunicast_conn *c, int status[])
{
	// const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  	// printf("bunicast message sent to %d.%d: status %d\n",
    // 	dest->u8[0], dest->u8[1], status);
}

static const struct bunicast_callbacks b_callbacks = {recv_buc, sent_buc};
static struct bunicast_conn buc;

PROCESS_THREAD(cpms, ev, data)
{
	PROCESS_BEGIN();

	// bunicast_open(&buc, 137, &b_callbacks);

	// static struct etimer et;

	// while(1)
	// {
	// 	etimer_set(&et, CLOCK_SECOND * 5);

		// // channel-scan test
		// int chn = channel_scan();

		// printf("channel: %d\n", chn);

		// // block transmission test 
		// char data[500];
		// int i;
		// for(i=0;i<500;i++) {
		// 	data[i] = i%10 + '0';
		// }

		// data[500] = '\0';
		// printf("%s\n", data);
		// linkaddr_t addr;

    	// addr.u8[0] = 155;
    	// addr.u8[1] = 150;
      
	  	// bunicast_send(&buc, &addr, data);

		// // cpmsack test
		// cpmsack_init();

		// uint8_t buffer[11];
		// cpmsack_frame_create(32, buffer);

		// cpmsack_frame_parse(buffer);

		// int i;
		// for (i=0; i<14; i++) {
		// 	printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buffer[i]));
		// }
		cpms_sink_init();

	// 	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	// }

	PROCESS_END();
}
