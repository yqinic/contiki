/*------------------UIWP Protocol Related Configuration -----------------------------*/
#define FRAME_DATA_SIZE 100
#define BLOCK_SIZE_OFFSET 8

// Offset channel
#define CHANNEL_OFFSET 26

/*----------------RIME Configuration Related-----------------*/
#define NETSTACK_CONF_RDC     nullrdc_driver
// #define NETSTACK_CONF_RDC     contikimac_driver

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 16
// #define WITH_PHASE_OPTIMIZATION 1
#define CSMA_CONF_MAX_FRAME_RETRIES 0