#include "app.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_device.h"
#include "em_emu.h"
#include "em_letimer.h"
#include "iadc.h"
#include "cog.h"

/***************************************************************************//**
 * LETIEMR Interrupt Handler
 ******************************************************************************/
void LETIMER0_IRQHandler(void)
{
  // Clear all interrupt flags
  uint32_t flags = LETIMER_IntGet(LETIMER0);
  LETIMER_IntClear(LETIMER0, flags);
  if (pressureSensor.notifyAdc || pressureSensor.notifyCog){
      scanIADC();
  }
}

/***************************************************************************//**
 * Initialize CMU
 ******************************************************************************/
void initCMU(void)
{
  CMU_ClockEnable(cmuClock_LETIMER0, true);
}

/***************************************************************************//**
 * Initialize LETIMER.
 ******************************************************************************/
void initLetimer(void)
{
  // LETIMER initialization
  // Top value = LF Clock frequency, compare match frequency = 1 HZ
  LETIMER_Init_TypeDef initLetimer = LETIMER_INIT_DEFAULT;
  initLetimer.enable = false;  // Do not enable LETIMER when initializing
  const int FREQ = 1000;
  initLetimer.topValue = 32768 / FREQ;         //  for FREQ Hz, LFXO is 32768Hz
  LETIMER_Init(LETIMER0, &initLetimer);

  LETIMER_IntDisable(LETIMER0, _LETIMER_IEN_MASK);  // Disable all interrupts
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP0);  // Enable compare match 0

  NVIC_ClearPendingIRQ(LETIMER0_IRQn);
  NVIC_EnableIRQ(LETIMER0_IRQn);

  LETIMER_Enable(LETIMER0, true);
}
