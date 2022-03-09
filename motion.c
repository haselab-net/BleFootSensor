#include "em_common.h"
#include "app_assert.h"
#include "sl_i2cspm.h"
#include "sl_i2cspm_instances.h"
#include "em_i2c.h"
#include "em_gpio.h"

I2C_TransferSeq_TypeDef seq;

void readLSM9DS(){
  GPIO_PinModeSet(gpioPortA, 7, gpioModeWiredAndAlternatePullUp, 0);
  GPIO_PinModeSet(gpioPortA, 8, gpioModeWiredAndAlternatePullUp, 0);

  static uint8_t bufs[2][10];
  seq.buf[0].data = bufs[0];
  seq.buf[1].data = bufs[1];
  seq.buf[0].len = 10;
  seq.buf[1].len = 10;

  seq.flags = I2C_FLAG_WRITE_WRITE;
  seq.addr = 0xD4;
  seq.buf[0].data[0] = 0x20;
  seq.buf[1].data[0] = 0x20;
  seq.buf[0].data[1] = 0xFF;
  seq.buf[1].data[1] = 0xFF;
  I2C_TransferReturn_TypeDef rv = I2CSPM_Transfer(sl_i2cspm_inst, &seq);


  app_log_info("I2C rv:%x  w:%x r:%x\n", rv, bufs[0][0], bufs[1][0]);
}
