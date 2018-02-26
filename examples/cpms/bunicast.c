#include "net/rime/rime.h"
#include "bunicast.h"
#include <string.h>

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS)
#else
#define PRINTF(...)
#endif

#ifndef BLOCK_SIZE_OFFSET
#define BLOCK_SIZE_OFFSET 8
#endif

#ifndef FRAME_DATA_SIZE
#define FRAME_DATA_SIZE 80
#endif

static int block_size = SIZE_OFFSET;

static void 
recv_from_unciast(struct unicast_conn *unicast, const linkaddr_t *from)
{
	struct bunicast_conn *c = (struct bunicast_conn *)unicast;

	if(c->u->recv) {
		c->u->recv(c, from);
	}
}

static void
sent_by_unicast(struct unicast_conn *unicast, int status, int num_tx)
{
	struct bunicast_conn *c = (struct bunicast_conn *c)unicast;

	static int counter;
	static int status_sum;

	status_sum += status;

	if (++counter == size) {
		if (status_sum == 0) {
			PRINTF("No transmission error\n");
			if(c->u->sent) {
				c->u->sent(c, BLOCK_TX_OK);
			}
		} else {
			PRINTF("Transmission error\n");
			if(c->u->sent) {
				c->u->sent(c, BLOCK_TX_ERR);
			}
		}
		
		counter = 0;
		status_sum = 0;
	}
}

static const struct unicast_callbacks bc = {recv_from_unciast, sent_by_unicast};

void
bunicast_open(struct bunicast_conn *c, uint16_t channel,
	const struct bunicast_callbacks *u)
{
	unicast_open(&c->c, channel, &bc);
	c->u = u;
}

void
bunicast_close(struct bunicast_conn *c)
{
	unicast_close(&c->c);
}

void
bunicast_send(struct bunicast_conn *c, const linkaddr_t *receiver, const char *data)
{
	int num_of_tx, total_num_tx;
	// max data size = FRAME_DATA_SIZE * block_size
	int data_len = MIN(strlen(data), FRAME_DATA_SIZE * size);

	// total number of unicast
	total_num_tx = data_len / FRANE_DATA_SIZE;
	if (data_len % FRAME_DATA_SIZE != 0) 
		total_num_tx += 1;
		
	PRINTF("total number of transmission: %d\n", total_num_tx);

	int *data_ptr;

	// block transmission
	for(num_of_tx = 0; num_of_tx < total_num_tx; num_of_tx++) {
		data_ptr = data + FRAME_DATA_SIZE * num_of_tx;
		packetbuf_copyfrom(data_ptr, MIN(strlen(data_ptr), FRAME_DATA_SIZE));
		unicast_send(&c->c, receiver);
	}
}

int
bunicast_size(int size)
{
	block_size = size;
}
