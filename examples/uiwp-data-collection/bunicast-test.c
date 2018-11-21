#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include "dev/leds.h"

PROCESS(cpms_bunicast_process, "cpms_bunicast");
AUTOSTART_PROCESSES(&cpms_bunicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_buc(struct bunicast_conn *c, const linkaddr_t *from)
{

}
/*---------------------------------------------------------------------------*/
static void
sent_buc(struct bunicast_conn *c, int status)
{

}
/*---------------------------------------------------------------------------*/
static const struct bunicast_callbacks bunicast_callbacks = {recv_buc, sent_buc};
static struct bunicast_conn buc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cpms_bunicast_process, ev, data)
{
    PROCESS_EXITHANDLER(bunicast_close(&buc));
    
    PROCESS_BEGIN();

    bunicast_open(&buc, 123, &bunicast_callbacks);

    while(1) {
        static struct etimer et;
        etimer_set(&et, CLOCK_SECOND);
            
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

        linkaddr_t addr;
        
        addr.u8[0] = 60;
        addr.u8[1] = 130;

        bunicast_send(&buc, &addr, "Hello");
        printf("header len: %d, data len: %d\n", 
            (int)packetbuf_hdrlen(), (int)packetbuf_datalen());
    }

    PROCESS_END();
}