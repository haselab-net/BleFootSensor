#ifndef IADC_H
#define IADC_H

#define NUM_ADC_BITS  12
// Number of scan channels
#define NUM_ADC_INPUTS 8
#define NUM_ADC_DRIVINGPINS 8

extern volatile short adcValue[NUM_ADC_DRIVINGPINS][NUM_ADC_INPUTS];
void initIADC();
void scanIADC();
void printIADC();

#endif
