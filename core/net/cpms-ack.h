#ifndef CPMSACK_H_
#define CPMSACK_H_

#ifndef BATTERY_VOLUMN_OFFSET
#define BATTERY_VOLUMN_OFFSET 2400
#endif

#ifndef ANTENNA_TYPE_OFFSET
#define ANTENNA_TYPE_OFFSET 0
#endif

#ifndef ANTENNA_ORIENTATION_OFFSET
#define ANTENNA_ORIENTATION_OFFSET 0
#endif

#ifndef CPMSACK_ACK
#define CPMSACK_ACK (0x01)
#endif

struct cpmsack {
    int val;
};

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

extern struct cpmsack cpms_acks[];

int cpmsack_set_attr(int type, const int value);

int cpmsack_get_attr(int type);

int cpmsack_frame_create();

int cpmsack_frame_parse();

#endif
