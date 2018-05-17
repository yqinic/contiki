#ifndef BUNICAST_H_
#define BUNICAST_H_

#define DYNAMIC_MEMB 0

#include "net/rime/unicast.h"
#if DYNAMIC_MEMB
#include <stdlib.h>
#endif

struct bunicast_conn;

struct bunicast_callbacks {
	void (* recv)(struct bunicast_conn *c, const linkaddr_t *from);
#if DYNAMIC_MEMB
	void (* sent)(struct bunicast_conn *ptr, int status[]);
#else
	void (* sent)(struct bunicast_conn *ptr, int status);
#endif
};

struct bunicast_conn {
	struct unicast_conn c;
	const struct bunicast_callbacks *u;
	int block_size;
	int total_num_tx;
	int counter;
};

void bunicast_open(struct bunicast_conn *c, uint16_t channel, const struct bunicast_callbacks *u);
void bunicast_close(struct bunicast_conn *c);

void bunicast_send(struct bunicast_conn *c, const linkaddr_t *receiver, char *data);

int bunicast_size(struct bunicast_conn *c, int size);

#endif
