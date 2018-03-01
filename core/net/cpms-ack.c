#include "net/cpms-ack.h"
#include "dev/battery-sensor.h"

#define DEBUG 1
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

struct cpmsack cpms_acks[CPMSACK_NUM];

static int 
cpmsack_update(int num)
{
    cpms_acks[CPMSACK_DATA_PRIORITY].val = 1;
    cpms_acks[CPMSACK_DATA_BYTES].val = num;

    SENSORS_ACTIVATE(battery_sensor);
    cpms_acks[CPMSACK_BATTERY_VOLTAGE].val = battery_sensor.value(0);
    SENSORS_DEACTIVATE(battery_sensor);

    // to be modified
    cpms_acks[CPMSACK_BATTERY_VOLUMN].val = BATTERY_VOLUMN_OFFSET;
    cpms_acks[CPMSACK_ANTENNA_TYPE].val =  ANTENNA_TYPE_OFFSET;
    cpms_acks[CPMSACK_ANTENNA_ORIENTATION].val = ANTENNA_ORIENTATION_OFFSET;

    return 1;
}

int 
cpmsack_set_attr(int type, const int value) 
{
    cpms_acks[type].val = value;
    return 1;
}

int
cpmsack_get_attr(int type)
{
    return cpms_acks[type].val;
}

int 
cpmsack_frame_create(int num, uint8_t *buf)
{
    cpmsack_update(num);

    uint8_t pos = 0;

    buf[pos++] = ((1 & 3) << 6) | (cpms_acks[CPMSACK_DATA_PRIORITY].val & 63);
    
    buf[pos++] = (cpms_acks[CPMSACK_DATA_BYTES].val >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_DATA_BYTES].val & 0xff;

    buf[pos++] = (cpms_acks[CPMSACK_BATTERY_VOLTAGE].val >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_BATTERY_VOLTAGE].val & 0xff;

    buf[pos++] = (cpms_acks[CPMSACK_BATTERY_VOLUMN].val >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_BATTERY_VOLUMN].val & 0xff;

    buf[pos++] = cpms_acks[CPMSACK_ANTENNA_TYPE].val & 0xff;

    buf[pos++] = (cpms_acks[CPMSACK_ANTENNA_ORIENTATION].val >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_ANTENNA_ORIENTATION].val & 0xff;

#if DEBUG
    int i;
	for (i = 0; i < 10; i++) {
		PRINTF(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buf[i]));
	}
#endif

    return (int)pos;
}

int
cpmsack_frame_parse(uint8_t *buf)
{
    return 1;
}