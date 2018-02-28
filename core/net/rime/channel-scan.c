#include "channel-scan.h"
#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef CHANNEL_OFFSET
#define CHANNEL_OFFSET 26
#endif

#ifndef RSSI_MIN
#define RSSI_MIN -100
#endif

#include "net/netstack.h"
#include "channel-scan.h"

static int
optimal_channel(int channel_rssi[], int num_of_channel, int channel_min)
{
	int i, min = 0;
	for (i = 0; i < num_of_channel; i++)
	{
		int ch_rssi = channel_rssi[i];
		if (min > ch_rssi
			&& i+channel_min != CHANNEL_OFFSET
			&& ch_rssi >= RSSI_MIN)
			min = ch_rssi;
	}

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

	// get max and min channel no.
	int num_of_channel, channel_min, channel_max;
	if (NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &channel_min) != RADIO_RESULT_OK)
		return 0;
	if (NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &channel_max) != RADIO_RESULT_OK)
		return 0;

	num_of_channel = channel_max - channel_min + 1;
	PRINTF("number of channels: %d\n", num_of_channel);

	// get rssi of each channel
	int channel_rssi[num_of_channel];
	for (i = 0; i < num_of_channel; i++)
	{
		if (channel_hop(i+channel_min) == 0)
			return 0;
		do {
			NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &(channel_rssi[i]));
		} while (channel_rssi[i] < RSSI_MIN);
		
		PRINTF("rssi of channel %d: %d\n", i+channel_min, channel_rssi[i]);
	}

	// return to offset channel
	if (channel_hop(CHANNEL_OFFSET) == 0)
		return 0;

	// return the optimal channel
	return optimal_channel(channel_rssi, num_of_channel, channel_min);	
}

int
channel_hop(int channel)
{
	if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel) != RADIO_RESULT_OK)
		return 0;

	return 1;
}

