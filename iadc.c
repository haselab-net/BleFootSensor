/***************************************************************************//**
 * @file main_scan_interrupt.c
 * @brief Use the ADC to take repeated nonblocking measurements on multiple
 * inputs
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Evaluation Quality
 * This code has been minimally tested to ensure that it builds and is suitable
 * as a demonstration for evaluation purposes only. This code will be maintained
 * at the sole discretion of Silicon Labs.
 ******************************************************************************/

#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_iadc.h"
#include "em_gpio.h"
#include "iadc.h"
#include "cog.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Set CLK_ADC to 10MHz
#define CLK_SRC_ADC_FREQ          10000000 // CLK_SRC_ADC
#define CLK_ADC_FREQ              10000000 // CLK_ADC - 10MHz max in normal mode


//  pin info: https://www.silabs.com/documents/public/user-guides/ug465-brd4314a.pdf

/*
 * Specify the IADC input using the IADC_PosInput_t typedef.  This
 * must be paired with a corresponding macro definition that allocates
 * the corresponding ABUS to the IADC.  These are...
 *
 * GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AEVEN0_ADC0
 * GPIO->ABUSALLOC |= GPIO_ABUSALLOC_AODD0_ADC0
 * GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BEVEN0_ADC0
 * GPIO->BBUSALLOC |= GPIO_BBUSALLOC_BODD0_ADC0
 * GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDEVEN0_ADC0
 * GPIO->CDBUSALLOC |= GPIO_CDBUSALLOC_CDODD0_ADC0
 *
 * ...for port A, port B, and port C/D pins, even and odd, respectively.
 */
/*
#define IADC_INPUT_0_PORT_PIN     iadcPosInputPortBPin0;
#define IADC_INPUT_1_PORT_PIN     iadcPosInputPortBPin1;

#define IADC_INPUT_0_BUS          BBUSALLOC
#define IADC_INPUT_0_BUSALLOC     GPIO_BBUSALLOC_BEVEN0_ADC0
#define IADC_INPUT_1_BUS          BBUSALLOC
#define IADC_INPUT_1_BUSALLOC     GPIO_BBUSALLOC_BODD0_ADC0
*/
#define IADC_INPUT_0_PORT_PIN     iadcPosInputPortCPin3
#define IADC_INPUT_1_PORT_PIN     iadcPosInputPortCPin2
#define IADC_INPUT_2_PORT_PIN     iadcPosInputPortCPin1
#define IADC_INPUT_3_PORT_PIN     iadcPosInputPortCPin0
#define IADC_INPUT_4_PORT_PIN     iadcPosInputPortDPin0
#define IADC_INPUT_5_PORT_PIN     iadcPosInputPortDPin1
#define IADC_INPUT_6_PORT_PIN     iadcPosInputPortDPin2
#define IADC_INPUT_7_PORT_PIN     iadcPosInputPortDPin3

#define IADC_INPUT_0_BUS          CDBUSALLOC
#define IADC_INPUT_0_BUSALLOC     GPIO_CDBUSALLOC_CDEVEN0_ADC0
#define IADC_INPUT_1_BUS          CDBUSALLOC
#define IADC_INPUT_1_BUSALLOC     GPIO_CDBUSALLOC_CDODD0_ADC0
#define IADC_INPUT_2_BUS          CDBUSALLOC
#define IADC_INPUT_2_BUSALLOC     GPIO_CDBUSALLOC_CDEVEN0_ADC0
#define IADC_INPUT_3_BUS          CDBUSALLOC
#define IADC_INPUT_3_BUSALLOC     GPIO_CDBUSALLOC_CDODD0_ADC0
#define IADC_INPUT_4_BUS          CDBUSALLOC
#define IADC_INPUT_4_BUSALLOC     GPIO_CDBUSALLOC_CDEVEN0_ADC0
#define IADC_INPUT_5_BUS          CDBUSALLOC
#define IADC_INPUT_5_BUSALLOC     GPIO_CDBUSALLOC_CDODD0_ADC0
#define IADC_INPUT_6_BUS          CDBUSALLOC
#define IADC_INPUT_6_BUSALLOC     GPIO_CDBUSALLOC_CDEVEN0_ADC0
#define IADC_INPUT_7_BUS          CDBUSALLOC
#define IADC_INPUT_7_BUSALLOC     GPIO_CDBUSALLOC_CDODD0_ADC0


//#define IADC_INPUT_6_BUS          BBUSALLOC
//#define IADC_INPUT_6_BUSALLOC     GPIO_BBUSALLOC_BEVEN0_ADC0



/* This example enters EM2 in the infinite while loop; Setting this define to 1
 * enables debug connectivity in the EMU_CTRL register, which will consume about
 * 0.5uA additional supply current */
#define EM2DEBUG                  1

/*******************************************************************************
 ***************************   GLOBAL VARIABLES   ******************************
 ******************************************************************************/

volatile short adcValue[NUM_ADC_DRIVINGPINS][NUM_ADC_INPUTS];
const short drivingPins[NUM_ADC_DRIVINGPINS] = {4, 3, 2, 1, 0, 0, 4, 6};
const short drivingPorts[NUM_ADC_DRIVINGPINS] = {gpioPortB, gpioPortB, gpioPortB, gpioPortB,
    gpioPortB, gpioPortA, gpioPortA, gpioPortC};


/**************************************************************************//**
 * @brief  IADC Initializer
 *****************************************************************************/
void initIADC (void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitScan_t initScan = IADC_INITSCAN_DEFAULT;
  IADC_ScanTable_t initScanTable = IADC_SCANTABLE_DEFAULT;    // Scan Table

  // Enable IADC0 and GPIO clock branches
  CMU_ClockEnable(cmuClock_IADC0, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* Note: For EFR32xG21 radio devices, library function calls to
   * CMU_ClockEnable() have no effect as oscillators are automatically turned
   * on/off based on demand from the peripherals; CMU_ClockEnable() is a dummy
   * function for EFR32xG21 for library consistency/compatibility.
   */

  // Reset IADC to reset configuration in case it has been modified by
  // other code
  IADC_reset(IADC0);

  // Select clock for IADC
  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);  // FSRCO - 20MHz

  // Modify init structs and initialize
  init.warmup = iadcWarmupKeepWarm;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Configuration 0 is used by both scan and single conversions by default
  // Use unbuffered AVDD as reference
  initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                             CLK_ADC_FREQ,
                                             0,
                                             iadcCfgModeNormal,
                                             init.srcClkPrescale);

  // Scan initialization
  initScan.dataValidLevel = _IADC_SCANFIFOCFG_DVL_VALID4;     // Set the SCANFIFODVL flag when 4 entries in the scan FIFO are valid;
                                                              // Not used as an interrupt or to wake up LDMA; flag is ignored

  // Tag FIFO entry with scan table entry id.
  initScan.showId = true;

  // Configure entries in scan table, CH0 is single-ended from input 0, CH1 is
  // single-ended from input 1
  initScanTable.entries[0].posInput = IADC_INPUT_0_PORT_PIN;
  initScanTable.entries[0].negInput = iadcNegInputGnd;
  initScanTable.entries[0].includeInScan = true;

  initScanTable.entries[1].posInput = IADC_INPUT_1_PORT_PIN;
  initScanTable.entries[1].negInput = iadcNegInputGnd;
  initScanTable.entries[1].includeInScan = true;

  initScanTable.entries[2].posInput = IADC_INPUT_2_PORT_PIN;
  initScanTable.entries[2].negInput = iadcNegInputGnd;
  initScanTable.entries[2].includeInScan = true;

  initScanTable.entries[3].posInput = IADC_INPUT_3_PORT_PIN;
  initScanTable.entries[3].negInput = iadcNegInputGnd;
  initScanTable.entries[3].includeInScan = true;

  initScanTable.entries[4].posInput = IADC_INPUT_4_PORT_PIN;
  initScanTable.entries[4].negInput = iadcNegInputGnd;
  initScanTable.entries[4].includeInScan = true;

  initScanTable.entries[5].posInput = IADC_INPUT_5_PORT_PIN;
  initScanTable.entries[5].negInput = iadcNegInputGnd;
  initScanTable.entries[5].includeInScan = true;

  initScanTable.entries[6].posInput = IADC_INPUT_6_PORT_PIN;
  initScanTable.entries[6].negInput = iadcNegInputGnd;
  initScanTable.entries[6].includeInScan = true;

  initScanTable.entries[7].posInput = IADC_INPUT_7_PORT_PIN;
  initScanTable.entries[7].negInput = iadcNegInputGnd;
  initScanTable.entries[7].includeInScan = true;

  initScanTable.entries[8].includeInScan = false;


  // Initialize IADC
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Scan
  IADC_initScan(IADC0, &initScan, &initScanTable);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_0_BUS |= IADC_INPUT_0_BUSALLOC;
  GPIO->IADC_INPUT_1_BUS |= IADC_INPUT_1_BUSALLOC;
  GPIO->IADC_INPUT_2_BUS |= IADC_INPUT_2_BUSALLOC;
  GPIO->IADC_INPUT_3_BUS |= IADC_INPUT_3_BUSALLOC;
  GPIO->IADC_INPUT_4_BUS |= IADC_INPUT_4_BUSALLOC;
  GPIO->IADC_INPUT_5_BUS |= IADC_INPUT_5_BUSALLOC;
  GPIO->IADC_INPUT_6_BUS |= IADC_INPUT_6_BUSALLOC;
  GPIO->IADC_INPUT_7_BUS |= IADC_INPUT_7_BUSALLOC;

  // Clear any previous interrupt flags
  IADC_clearInt(IADC0, _IADC_IF_MASK);

  // Enable Scan interrupts
  IADC_enableInt(IADC0, IADC_IEN_SCANTABLEDONE);

  // Enable ADC interrupts
  NVIC_ClearPendingIRQ(IADC_IRQn);
  NVIC_EnableIRQ(IADC_IRQn);

  //  pull up adc pins
  GPIO_PinModeSet(gpioPortC, 0, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortC, 0);
  GPIO_PinModeSet(gpioPortC, 1, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortC, 1);
  GPIO_PinModeSet(gpioPortC, 2, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortC, 2);
  GPIO_PinModeSet(gpioPortC, 3, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortC, 3);
  GPIO_PinModeSet(gpioPortD, 0, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortD, 0);
  GPIO_PinModeSet(gpioPortD, 1, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortD, 1);
  GPIO_PinModeSet(gpioPortD, 2, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortD, 2);
  GPIO_PinModeSet(gpioPortD, 3, gpioModeDisabled, 0); //  pull up = disable + DOUT:1
  GPIO_PinOutSet(gpioPortD, 3);

  //  driving GPIO settings
  for(int i=0; i<NUM_ADC_DRIVINGPINS; ++i){
      GPIO_PinModeSet(drivingPorts[i], drivingPins[i], gpioModeWiredAnd, 0); //  gpioModeWiredAnd=openDrain
      GPIO_PinOutSet(drivingPorts[i], drivingPins[i]); // set high level to driving pins
  }
}

/**************************************************************************//**
 * @brief  ADC Handler
 *****************************************************************************/
static int drivingCount = 0;

void IADC_IRQHandler(void)
{
  IADC_Result_t result = {0, 0};

  // Get ADC results
  while(IADC_getScanFifoCnt(IADC0))
  {
    // Read data from the scan FIFO
    result = IADC_pullScanFifoResult(IADC0);

    // Calculate input voltage:
    //  For single-ended the result range is 0 to +Vref, i.e.,
    //  for Vref = AVDD = 3.30V, 12 bits represents 3.30V full scale IADC range.
    if (result.id < NUM_ADC_INPUTS){
        adcValue[drivingCount][result.id] = result.data;
    }
  }
  //printf("iadc %d\n", result.id);

  if(result.id == 3) // first set of table entries are read.
  {
      IADC_setScanMask(IADC0, 0x00F0);  // configure scan mask to measure second set of table entries
      IADC_command(IADC0, iadcCmdStartScan);
  } else {
      IADC_setScanMask(IADC0, 0x000F);  // configure scan mask to measure first set of table entries
      GPIO_PinOutSet(gpioPortB, drivingPins[drivingCount]);
      drivingCount ++;
      if (drivingCount >= NUM_ADC_DRIVINGPINS){
          //  read all sensors
          drivingCount = 0;
          onIadcReadAll();
      }else{
          //  continue to scan to read next sensor groups
          scanIADC();
      } //  continue to read for all driving pins.
  }

  // Clear scan interrupt flag
  IADC_clearInt(IADC0, IADC_IF_SCANTABLEDONE); // flags are sticky; must be cleared in software
}

void scanIADC(){
  GPIO_PinOutClear(gpioPortB, drivingPins[drivingCount]);
  IADC_command(IADC0, iadcCmdStartScan);
}

void printIADC(){
  for(int d=0; d<NUM_ADC_DRIVINGPINS; ++d){
      printf("ADC B%d", drivingPins[d]);
      for(int i=0; i<NUM_ADC_INPUTS; ++i){
          printf(" %d", adcValue[d][i]);
      }
      printf("\n");
  }
}



/**************************************************************************//**
 * @brief  Main function not used
 *****************************************************************************/
int invalid__main__not__called(void)
{
  CHIP_Init();

  // Initialize the IADC
  initIADC();

#ifdef EM2DEBUG
  // Enable debug connectivity in EM2
  EMU->CTRL_SET = EMU_CTRL_EM2DBGEN;
#endif

  // Infinite loop
  while(1){
    // Start scan
    IADC_command(IADC0, iadcCmdStartScan);

    // Enter EM2
    EMU_EnterEM2(false);
  }
}
