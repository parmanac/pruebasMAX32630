/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/




// REMOVE FOR NON MBED SITUATION. MODIFY BOARD INIT BASED ON PLATFORM!
#include <events/mbed_events.h>
#include <mbed.h>





#include "platform/max32630hsp.h"
#include "SHComm.h"
#include "cmdInterface.h"
#include "demoDefinitions.h"
#include "algoConfigAPI.h"
#include "simplest/simpleDataCapture.h"
#include "SSwrapper/authentication.h"


/*************** PLATFORM ***************************/

// Hardware serial port over DAPLink for Bootloder API Debug information
Serial daplink(USBTX, USBRX, 115200);

#include "USBSerial.h"
USBSerial microUSB(0x1f00, 0x2012, 0x0001, false);

// ICARUS Board initialization
InterruptIn interruptIn_PowerButton(P7_6);
MAX32630HSP icarus(MAX32630HSP::VIO_1V8, &interruptIn_PowerButton);


#define POLL_PERIOD_25MS   (1)
#define POLL_PERIOD_1000MS (25)


/***************APP CONFIGURATION******************/


//#define AUTHENTICATE_TO_SENSORHUB
//#define MEASURE_CONT_WHRM_CONT_WSPO2
//#define MEASURE_CONT_WHRM_ONESHOT_WSPO2
//#define MEASURE_CONT_HRM
//#define MEASURE_WHRM_WSPO2_EXTENDED_REPORT
//#define GET_RAW_GREEN_IR_RED_PPG
//#define BOOTLOADER_SEQUENCE
#define EVALUATE_HIGHLEVEL_ALGORITHMS

/***************APP ******************/

/*Higher Level Algorithms Demo Run Function*/
extern int run_algorithm_wrapper_consuming_whrm_outputs(void);

int main() {

#define WAIT_MS wait_ms
#define WAIT_SENSORHUB_STABLE_BOOTUP_MS  ((uint32_t)2000)

	WAIT_MS(WAIT_SENSORHUB_STABLE_BOOTUP_MS);

	sh_init_hwcomm_interface();

    int status;

#if defined(MEASURE_CONT_WHRM_CONT_WSPO2)
    status = measure_whrm_wspo2( (uint8_t) POLL_PERIOD_25MS , MXM_WEARABLE_ALGO_SUITE_CONTINUOUS_HRM_CONTINUOUS_SPO2_MODE);
#elif defined(MEASURE_CONT_WHRM_ONESHOT_WSPO2)
    status = measure_whrm_wspo2( (uint8_t) POLL_PERIOD_25MS , MXM_WEARABLE_ALGO_SUITE_CONTINUOUS_HRM_ONE_SHOT_SPO2_MODE);
#elif defined(MEASURE_CONT_HRM)
    status =  measure_whrm_wspo2( (uint8_t) POLL_PERIOD_25MS , MXM_WEARABLE_ALGO_SUITE_CONTINUOUS_HRM_MODE);
#elif defined(MEASURE_WHRM_WSPO2_EXTENDED_REPORT)
	status = measure_whrm_wspo2_extended_report();
#elif defined(GET_RAW_GREEN_IR_RED_PPG)
    status = get_raw_ppg();
#elif defined(AUTHENTICATE_TO_SENSORHUB)
    status = authenticate_to_sensorhub();
#elif defined(EVALUATE_HIGHLEVEL_ALGORITHMS)
    status = run_algorithm_wrapper_consuming_whrm_outputs();


#elif defined (BOOTLOADER_SEQUENCE)


    while(1) {


		//USBSerial *serial = &microUSB;
		char ch;
		while ( SERIAL_AVAILABLE()/*daplink.readable()*/) {
			//ch = daplink.getc();
            //printf("%c  " , ch );
			//ch = microUSB._getc();
			ch = SERIALIN();
			cmdIntf_build_command(ch);
		}


	}

#endif

}
