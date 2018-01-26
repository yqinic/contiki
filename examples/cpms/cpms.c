#include "contiki.h"
#include "channel-scan.c"
#include "net/rime/rime.h"
#include <stdio.h>

PROCESS(cpms, "cpms");
AUTOSTART_PROCESSES(&cpms);

PROCESS_THREAD(cpms, ev, data)
{
	PROCESS_BEGIN();

	static struct etimer et;

	while(1)
	{
		etimer_set(&et, CLOCK_SECOND * 2);

		int chn = channel_scan();

		printf("channel: %d\n", chn);

		PROCESS_WAIT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}
