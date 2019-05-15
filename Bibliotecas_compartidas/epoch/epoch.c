/*
 * epoch.c
 *
 *  Created on: 15 may. 2019
 *      Author: facusalerno
 */

#include "epoch.h"

timestamp_t getCurrentTime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((timestamp_t)tv.tv_sec) * 1000 + ((timestamp_t)tv.tv_usec) / 1000;
}

void seeCurrentTime(){
	printf("%llu", getCurrentTime());
}
