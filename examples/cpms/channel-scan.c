#include  <stdio.h>

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef CHANNEL_OFFSET
#define CHANNEL_OFFSET 26
#endif

#include "net/netstack.h"
#include "channel-scan.h"

static int
optimal_channel(int channel_rssi[], int num_of_channel, int channel_min)
{
	int i, min = 0;
	for (i = 0; i < num_of_channel; i++)
	{
		if (min > channel_rssi[i] && i+channel_min != CHANNEL_OFFSET)
			min = channel_rssi[i];
	}

	int optimal;
	for (i = 0; i < num_of_channel; i++)
	{
		if (min == channel_rssi[i] && i+channel_min != CHANNEL_OFFSET)
			return i + channel_min;
	}

	return 0;
}

int
channel_scan(void)
{
	int i;

	int num_of_channel, channel_min, channel_max;
	if (NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &channel_min) != RADIO_RESULT_OK)
		return 0;
	if (NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &channel_max) != RADIO_RESULT_OK)
		return 0;

	num_of_channel = channel_max - channel_min + 1;
	PRINTF("number of channels: %d\n", num_of_channel);

	int channel_rssi[num_of_channel];
	for (i = 0; i < num_of_channel; i++)
	{
		if (channel_hop(i+channel_min) == 0)
			return 0;
		NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &(channel_rssi[i]));
		PRINTF("rssi of channel %d: %d\n", i+channel_min, channel_rssi[i]);
	}

	if (channel_hop(CHANNEL_OFFSET) == 0)
		return 0;

	return optimal_channel(channel_rssi, num_of_channel, channel_min);	
}

int
channel_hop(int channel)
{
	if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel) != RADIO_RESULT_OK)
		return 0;

	return 1;
}

