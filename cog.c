#include "cog.h"
#include "iadc.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"

struct PressureSensor pressureSensor;
int16_t cogValue[COG_FRAMES][3];
const short ySensorPos[] = {20, 40, 60, 148, 173, 198, 223, 248};
const short x1SensorPos[] = { 0, 7, 14, 21, 28, 35, 42, 49};
const short x2SensorPos[] = { 0, 13, 28, 41, 56, 70, 85, 98};
const int xCenter = 30;
const int yCenter = 120;

//  Convert adc in 12bits to force in 16 bits.
uint16_t adcToForce(short adc){
  if (adc < 100) adc = 100;
  uint16_t pres = 350000 / adc; //  85 < pres < 3500
  return pres;
}
void calcCog(int frame){
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
  cogValue[frame][0] = sum / 20;
  cogValue[frame][1] = xMom / 20;
  cogValue[frame][2] = yMom / 20;
}

void onIadcReadAll(){
  static int count = 0;
  calcCog(count % COG_FRAMES);
  count++;
  if (count % COG_FRAMES == 0){
      memcpy(pressureSensor.cogData, cogValue, sizeof(pressureSensor.cogData));
      sl_bt_external_signal(ES_COG);
  }
  if (count == COG_FRAMES * 10){
      count = 0;
      memcpy(pressureSensor.adcData, adcValue, sizeof(pressureSensor.adcData));
      sl_bt_external_signal(ES_ADC);
  }
}
