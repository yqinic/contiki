#ifndef CPMSACK_H_
#define CPMSACK_H_

#include "contiki-conf.h"

#ifndef BATTERY_VOLUMN_OFFSET
#define BATTERY_VOLUMN_OFFSET 2400
#endif

#ifndef ANTENNA_TYPE_OFFSET
#define ANTENNA_TYPE_OFFSET 0
#endif

#ifndef ANTENNA_ORIENTATION_OFFSET
#define ANTENNA_ORIENTATION_OFFSET 0
#endif

#define CPMSACK_ACK (0x01)
#define DATA_PRIORITY_BIN 6
#define DATA_BYTES_OCT 2
#define BATTERY_VOLTAGE_OCT 2
#define BATTERY_VOLUMN_OCT 2
#define ANTENNA_TYPE_OCT 1
#define ANTENNA_ORIENTATION_OCT 2

enum {
    // how much data to be transferred
	CPMSACK_DATA_PRIORITY,
    CPMSACK_DATA_BYTES,

    // node battery information
    CPMSACK_BATTERY_VOLTAGE,
    CPMSACK_BATTERY_VOLUMN,

    // node antenna information
    CPMSACK_ANTENNA_TYPE,
    CPMSACK_ANTENNA_ORIENTATION,

    // camera inspection information
    CPMSACK_CAMERA,

    CPMSACK_NUM,
};

struct cpmsack_list{
    int priority;
    int bytes;
    int voltage;
    int volumn;
    int type;
    int orientation;
    int camera;
};

int cpmsack_init();

int cpmsack_set_attr(int type, const int value);

int cpmsack_get_attr(int type);

int cpmsack_frame_create(int num, uint8_t *buf);

int * cpmsack_frame_parse(uint8_t *buf);

#endif
