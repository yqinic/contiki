#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*------------------UIWP Protocol Related Configuration -----------------------------*/

// Applications
#define UIWP_APPLICATION WIRELESS_CHARGING

// Broadcast interval
#define UWIP_BROADCAST_INTERVAL 5

// Expire timers in clock ticks
#define CPMS_ACK_EXPIRE 16
#define CPMS_REQUEST_EXPIRE 8
#define CPMS_GLOREQ_EXPIRE 128
#define CPMS_DATA_EXPIRE 16
#define CPMS_GLODATA_EXPIRE 256

// Channel switching
#define CHANNEL_SWITCHING 1

// Max num of acks to receive
#define MAX_ACK_NUM 9

#define DEBUG_TIMER 0
#define DEBUG_RELIABLE 0

#define COMMAND_CHANNEL CHANNEL_OFFSET

#define EVENT_BC_STOP 0xA0

// Timers
enum {
    // request timer 
    REQ_TIMER,
    // global request timer
    GLOBALREQ_TIMER,
    // data timer
    DATA_TIMER,
    // global data timer
    GLOBALDATA_TIMER,

    TIMER_NUM
};

// Applications
enum {
    // check node presence
    CHECK_PRESENCE,
    // data collection
    DATA_COLLECTION,
    // inspection
    INSPECTION,
    // wireless charging
    WIRELESS_CHARGING,

    APPLICATION_NUM
};