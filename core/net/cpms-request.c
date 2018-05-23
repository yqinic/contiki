#include "net/cpms-request.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c\n"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
#else
#define PRINTF(...)
#endif

// static int cpms_request[CPMSREQUEST_NUM];

int 
cpmsrequest_frame_create(int channel, int order, int bytes, uint8_t *buf)
{
    uint8_t pos = 0;
    channel -= CHANNEL_LOWEST;

    buf[pos++] = ((2 & 3) << 6) | ((channel & 15) << 2) | (order & 3) ;
    buf[pos++] = (bytes >> 8) & 0xff;
    buf[pos++] = bytes & 0xff;

#if DEBUG
    int i;
	for (i = 0; i < pos; i++) {
		PRINTF(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buf[i]));
	}
#endif

    return (int)pos;
}

void 
cpmsrequest_frame_parse(uint8_t *buf, struct cpmsrequest_list *cpmsrequestlist)
{
    cpmsrequestlist->channel = (buf[0] >> 2) & 15;
    cpmsrequestlist->channel += CHANNEL_LOWEST;
    cpmsrequestlist->order = buf[0] & 3;
    cpmsrequestlist->bytes = (buf[1] << 8) + buf[2];
    
    PRINTF("%d\n%d\n%d\n", cpmsrequestlist->channel,
        cpmsrequestlist->order, cpmsrequestlist->bytes);

    return;
}