#include "net/cpms-ack.h"
#include "dev/battery-sensor.h"
#include <stdlib.h>

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

static int cpms_acks[CPMSACK_NUM];

static int 
cpmsack_update(int num)
{
    cpms_acks[CPMSACK_DATA_PRIORITY] = 2;
    cpms_acks[CPMSACK_DATA_BYTES] = num;

    SENSORS_ACTIVATE(battery_sensor);
    cpms_acks[CPMSACK_BATTERY_VOLTAGE] = battery_sensor.value(0);
    SENSORS_DEACTIVATE(battery_sensor);

    return 1;
}

int
cpmsack_init()
{
    cpms_acks[CPMSACK_DATA_PRIORITY] = 0;
    cpms_acks[CPMSACK_DATA_BYTES] = 0;
    cpms_acks[CPMSACK_BATTERY_VOLTAGE] = 0;
    cpms_acks[CPMSACK_CAMERA] = 0;

    cpms_acks[CPMSACK_BATTERY_VOLUMN] = BATTERY_VOLUMN_OFFSET;
    cpms_acks[CPMSACK_ANTENNA_TYPE] =  ANTENNA_TYPE_OFFSET;
    cpms_acks[CPMSACK_ANTENNA_ORIENTATION] = ANTENNA_ORIENTATION_OFFSET;

    return 1;
}

int 
cpmsack_set_attr(int type, const int value) 
{
    cpms_acks[type] = value;
    return 1;
}

int
cpmsack_get_attr(int type)
{
    return cpms_acks[type];
}

int 
cpmsack_frame_create(int num, uint8_t *buf)
{
    cpmsack_update(num);

    uint8_t pos = 0;

    buf[pos++] = ((1 & 3) << 6) | (cpms_acks[CPMSACK_DATA_PRIORITY] & 63);
    
    buf[pos++] = (cpms_acks[CPMSACK_DATA_BYTES] >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_DATA_BYTES] & 0xff;

    buf[pos++] = (cpms_acks[CPMSACK_BATTERY_VOLTAGE] >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_BATTERY_VOLTAGE] & 0xff;

    buf[pos++] = (cpms_acks[CPMSACK_BATTERY_VOLUMN] >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_BATTERY_VOLUMN] & 0xff;

    buf[pos++] = cpms_acks[CPMSACK_ANTENNA_TYPE] & 0xff;

    buf[pos++] = (cpms_acks[CPMSACK_ANTENNA_ORIENTATION] >> 8) & 0xff;
    buf[pos++] = cpms_acks[CPMSACK_ANTENNA_ORIENTATION] & 0xff;

    buf[pos++] = cpms_acks[CPMSACK_CAMERA] & 0xff;

#if DEBUG
    int i;
	for (i = 0; i < CPMSACK_FRAME_LENGTH; i++) {
		PRINTF(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buf[i]));
	}
#endif

    return (int)pos;
}

void *
cpmsack_frame_parse(uint8_t *buf)
{
    struct cpmsack_list *cpmsacklist;
    cpmsacklist = malloc(1);

    cpmsacklist->priority = buf[0] & 63;
    cpmsacklist->bytes = (buf[1] << 8) + buf[2];
    cpmsacklist->voltage = (buf[3] << 8) + buf[4];
    cpmsacklist->volumn = (buf[5] << 8) + buf[6];
    cpmsacklist->type = buf[7];
    cpmsacklist->orientation = (buf[8] << 8) + buf[9];
    cpmsacklist->camera = buf[10];

    PRINTF("%d\n%d\n%d\n%d\n%d\n%d\n%d\n", cpmsacklist->priority,
        cpmsacklist->bytes, cpmsacklist->voltage, cpmsacklist->volumn,
        cpmsacklist->type, cpmsacklist->orientation, cpmsacklist->camera);

    return cpmsacklist;
}