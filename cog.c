#include "cog.h"
#include "iadc.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"

struct PressureSensor pressureSensor;
volatile int16_t cogValue[3];
const short ySensorPos[] = {20, 40, 60, 148, 173, 198, 223, 248};
const short x1SensorPos[] = { 0, 7, 14, 21, 28, 35, 42, 49};
const short x2SensorPos[] = { 0, 13, 28, 41, 56, 70, 85, 98};
const int xCenter = 30;
const int yCenter = 120;

//  Convert adc in 12bits to force in 16 bits.
uint16_t adcToForce(uint16_t adc){
  return adc * 8;
}
void calcCog(){
  int xMom=0, yMom=0, sum=0;
  for(int y=0; y<NUM_ADC_DRIVINGPINS; ++y){
    for(int x=0; x<NUM_ADC_INPUTS; ++x){
      int yPos = ySensorPos[y];
      int xPos = x1SensorPos[x] * (248-yPos) / 248  +  x2SensorPos[x] * yPos / 248;
      uint16_t force = adcToForce(adcValue[y][x]);
      sum += force;
      xMom += force * (xPos - xCenter);
      yMom += force * (yPos - yCenter);
    }
  }
  cogValue[0] = sum / (NUM_ADC_INPUTS*NUM_ADC_DRIVINGPINS);
  cogValue[1] = xMom / (NUM_ADC_INPUTS*NUM_ADC_DRIVINGPINS*98);
  cogValue[2] = yMom / (NUM_ADC_INPUTS*NUM_ADC_DRIVINGPINS*248);
}

void onIadcReadAll(){
  calcCog();
  static int count = 0;
  count++;
  if (count == 100){
      count = 0;
      sl_bt_external_signal(ES_COG);
  }
}
