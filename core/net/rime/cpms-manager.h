#ifndef CPMSMANAGER_H_
#define CPMSMANAGER_H_

#include "net/rime/broadcast.h"
#include "net/rime/unicast.h"
#include "net/rime/bunicast.h"

// sink node process will be initiated
void cpms_sink_init();
// sensor node process will be initiated
void cpms_sensor_init();

#endif