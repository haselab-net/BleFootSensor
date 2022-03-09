#ifndef COG_H
#define COG_H
#include "iadc.h"
#include "sl_bluetooth.h"

#define COG_FRAMES  10
extern struct PressureSensor{
	union {
		struct {
			unsigned char notifyAdc:1;
			unsigned char notifyCog:1;
		};
		unsigned char flags;
	};
  short adcData[NUM_ADC_DRIVINGPINS][NUM_ADC_INPUTS];
  short cogData[COG_FRAMES][3];  //  sum, LR, FB
} pressureSensor;
extern int16_t cogValue[COG_FRAMES][3];

void onIadcReadAll();

#endif
