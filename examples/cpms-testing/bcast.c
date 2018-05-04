#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include <math.h>

char d[80];
int count = 0;
int select = 0;
int power = 1;

int before = 4;

unsigned long time[] = {0, 0, 0, 0, 0};
unsigned long time_prev;

linkaddr_t addr;
/*---------------------------------------------------------------------------*/
PROCESS(unicast_process, "unicast");
AUTOSTART_PROCESSES(&unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  char swit = *(char *)packetbuf_dataptr();

  if (swit == 'h') {
    time_prev = clock_time();
  }

  int ss = swit - '0';
  // printf("%c %d", swit, ss);

  if (ss - before == -4) {
    time[before] = clock_time() - time_prev;
    time_prev = clock_time();
    printf("1: %lu, 2: %lu, 4: %lu, 8: %lu, 16: %lu\n", time[0], time[1], time[2], time[3], time[4]);
    before = ss;
  } else if (ss - before == 1) {
    time[before] = clock_time() - time_prev;
    time_prev = clock_time();
    before = ss;
  } else {;}
}
/*---------------------------------------------------------------------------*/
static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{
  count += 1;
  // printf("power:%d, count:%d, select:%d\n", power, count, select);

  if (count%1600 == 0) {
    count = 0;
    select += 1;
    if (select >= 5)
      select = 0;
    // power = (int)pow(2, (double)select);
    switch(select) {
      case 0:
        power = 1;
        break;
      case 1:
        power = 2;
        break;
      case 2:
        power = 4;
        break;
      case 3:
        power = 8;
        break;
      case 4:
        power = 16;
        break;
      default:
        printf("wrong!\n");
    }

    d[0] = select + '0';
    printf("switching to sel: %d, pow: %d\n", select, power);
  }

  if(count%power == 0) {
    int i;
    for(i = 0; i < power; i++) {
      packetbuf_copyfrom(d, 80);
      unicast_send(c, &addr);
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();

  unicast_open(&uc, 146, &unicast_callbacks);

  int i;

  for (i = 0; i < 80; i++) {
    d[i] = 'a';
  }
  d[0] = '0';

  addr.u8[0] = 155;
  addr.u8[1] = 150;

  while(1) {
    static struct etimer et;
    
    etimer_set(&et, CLOCK_SECOND * 1000);

    packetbuf_copyfrom("hello", 5);
    unicast_send(&uc, &addr);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
