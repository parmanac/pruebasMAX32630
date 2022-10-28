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


/* NOTES
 *
 * 1. Integrate libMxmWellnessSuite.a library into biuld command
 * 2. Need API files:  AlgoWrapper.h
 *                     mxm_hrv_public.h
 *                     mxm_respiration_rate_manager.h
 *                     mxm_stress_monitoring.h
 *                     mxm_sleep_manager.h
 *
 * ******************************************************************************************************
 *
 * IMPORTANT :  SLEEP ALGORITHM STARTS TO GIVE OUTPUT AFTER ~ 25sps x 60secs x 40mins = 60000 samples !!!
 * 				SLEEP ALGORITHM STARTS TO GIVE OUTPUT AFTER ~ 25sps x 60secs x 40mins = 60000 samples !!!
 * 				SLEEP ALGORITHM STARTS TO GIVE OUTPUT AFTER ~ 25sps x 60secs x 40mins = 60000 samples !!!
 *              BEFORE THAT TIME SLEEP RELATED ALGORITHM OUTPUT is MXM_SLEEP_MANAGER_SP_UNDEFINED     !!!
 *
 *
 * ******************************************************************************************************
 * */


#include <mbed.h>  /*for type definitions*/
#include "SHComm.h"

#include "../simplest/simpleDataCapture.h"
#include "authentication.h"
#include "AlgoWrapper.h"
#include "mxm_hrv_public.h"
#include "mxm_respiration_rate_manager.h"
#include "mxm_stress_monitoring.h"
#include "mxm_sleep_manager.h"

#define ENABLE_ALGORITHMS_WITHIN_WRAPPER (MXM_ALGOSUITE_ENABLE_HRV |  MXM_ALGOSUITE_ENABLE_RESP | MXM_ALGOSUITE_ENABLE_STRESS | MXM_ALGOSUITE_ENABLE_SLEEP)
#define SUBJECT_PERSON_AGE           ((uint16_t) 30)
#define SUBJECT_PERSON_GENDER        ((mxm_sleep_manager_gender) MXM_MALE )
#define SUBJECT_PERSON_WEIGHT        ((uint16_t) 70)
#define SUBJECT_PERSON_SLEEPRESTHR   ((float) 70)
#define MIN_NUM_SENSORHUB_SAMPLES_TO_EVALUATE_ALL_WRAPPER_ALGORITHM ((int) 90000) /* 25sps x 60mins*/


#define SHIFT_FOR_UPPER_32BITS (32)
#define SHIFT_FOR_LOWER_32BITS (0)

static uint8_t accelBehavior = SH_INPUT_DATA_DIRECT_SENSOR; //SH_INPUT_DATA_FROM_HOST ;
static int sHubInputFifoSz = 5;


/* ******************************************************************************************************
*
* IMPORTANT :  THIS FUCNTIOJN IS FOR PROVIDING TIME STAMP TO WRAPPER INPUT SAMPLES .
*              THIS FUNCTION TO SUPPLY 64 BIT LOCAL UTC TIME WILL BE IMPLEMENTED BY CUSTOMER.
*              IT IS USED ANDROID PHONE TIME IN WELNESS ANDROID APPLICATION.
*
* ******************************************************************************************************
*/
long long int GET_LOCAL_64BIT_UTC_TIME(void){
	// will be implemented by Customer can be android/linux time stamp
	long long int u64_local_utc_time;
	return u64_local_utc_time;
}

int start_sensorhub_measurement_for_algowrapper(void){

	const int sensHubReportFifoThresh      = 1;

	int status;

	status  =  sh_set_report_period(1);
    if( status != SS_SUCCESS )
   	 return -1;

	 if(accelBehavior == SH_INPUT_DATA_FROM_HOST) {

		 status = sh_get_input_fifo_size(&sHubInputFifoSz);
	     if( status != SS_SUCCESS )
	    	 return -1;
	 }

   // enable data type to both raw sensor and algorithm data
    status = sh_set_data_type( SS_DATATYPE_BOTH, false );
    if( status != SS_SUCCESS )
   	 return -1;


    //set fifo threshold for mfio event frequency
    status = sh_set_fifo_thresh(sensHubReportFifoThresh);
    if( status != SS_SUCCESS )
    	 return -1;

    // enable accompanying accel sensor instance within sensorhub
    if(accelBehavior == SH_INPUT_DATA_FROM_HOST) {

   	 status = sh_sensor_enable_(SH_SENSORIDX_ACCEL, 1 , SH_INPUT_DATA_FROM_HOST);
   	 if( status != SS_SUCCESS )
   		 return -1;
    }

    status = sh_enable_algo_(SS_ALGOIDX_WHRM_WSPO2_SUITE , (int) ALGO_REPORT_MODE_EXTENDED);
    if( status != SS_SUCCESS )
    	 return -1;


   /*poll with timer*/
    start_hub_event_poll(200);


}


int init_wrapper_algos(void){

	int status = -1;

	mxm_algosuite_return_code algoStatus;
	mxm_algosuite_init_data  algosuite_init_data;

	algosuite_init_data.respConfig.led_code = MXM_RESPIRATION_RATE_MANAGER_GREEN_LED;
	algosuite_init_data.respConfig.sampling_rate = MXM_RESPIRATION_RATE_MANAGER_SAMPLING_RATE_25_HZ;
	algosuite_init_data.respConfig.signal_source_option = MXM_RESPIRATION_RATE_MANAGER_PPG_SOURCE_WRIST;

	algosuite_init_data.hrvConfig.samplingPeriod = (float) MXM_RESPIRATION_RATE_MANAGER_GREEN_LED;
	algosuite_init_data.hrvConfig.windowSizeInSec      =  256;
	algosuite_init_data.hrvConfig.windowShiftSizeInSec =  256;

	algosuite_init_data.stressConfig.dummy_config_for_compilation;

	algosuite_init_data.sleepConfig.is_activity_available = true;
	algosuite_init_data.sleepConfig.is_confidence_level_available_hr  = true;
	algosuite_init_data.sleepConfig.is_confidence_level_available_ibi = true;
	algosuite_init_data.sleepConfig.is_resting_hr_available           = true;
	algosuite_init_data.sleepConfig.mxm_sleep_detection_duration      = MXM_SLEEP_MANAGER_MINIMUM_SLEEP_30_MIN;
	algosuite_init_data.sleepConfig.user_info.age                     = SUBJECT_PERSON_AGE;
	algosuite_init_data.sleepConfig.user_info.gender                  = SUBJECT_PERSON_GENDER;
	algosuite_init_data.sleepConfig.user_info.weight                  = SUBJECT_PERSON_WEIGHT;
	algosuite_init_data.sleepConfig.user_info.sleep_resting_hr        = SUBJECT_PERSON_SLEEPRESTHR;


	algosuite_init_data.enabledAlgorithms = 0x0;
	algosuite_init_data.enabledAlgorithms = ENABLE_ALGORITHMS_WITHIN_WRAPPER;

	mxm_algosuite_manager_init( &algosuite_init_data , &algoStatus);

    if( algoStatus.hrv_status == MXM_HRV_SUCCESS &&
    	algoStatus.resp_status == MXM_RESPIRATION_RATE_MANAGER_SUCCESS  &&
		algoStatus.stress_status == MXM_STRESS_MONITORING_SUCCESS )
    	status  = 0;
    else
    	status  = -1;


}

int fetch_measurement_samples_from_sensorhub(mxm_algosuite_input_data *highLevelAlgoSampleArray) {

	static int sample_cnt = 0;

	const int MAX_WHRMWSPO2_SAMPLE_COUNT   = 45;
    const int WHRMWSPO2_FRAME_SIZE         = sizeof(accel_mode1_data)
			                                  + sizeof(max8614x_mode1_data)
									          + sizeof(whrm_wspo2_suite_mode2_data);

	 static uint8_t databuf[WHRMWSPO2_FRAME_SIZE * MAX_WHRMWSPO2_SAMPLE_COUNT + 1];

	int num_samples = 0;
    int status;

	if( sh_has_mfio_event()) {

   		 sh_clear_mfio_event_flag();

   		 uint8_t hubStatus = 0;
   	     status = sh_get_sensorhub_status(&hubStatus);

   		 if ( status == SS_SUCCESS && (hubStatus & SS_MASK_STATUS_DATA_RDY) == SS_MASK_STATUS_DATA_RDY ) {

  		    	 status = sh_num_avail_samples(&num_samples);
   	    		 if(status == SS_SUCCESS ) {

						 wait_ms(5);
						 status = sh_read_fifo_data(num_samples, PPG_REPORT_SIZE + ACCEL_REPORT_SIZE + ALGO_EXTENDED_REPORT_SIZE, &databuf[0], sizeof(databuf));
						 if(status == SS_SUCCESS){

							 //SERIALOUT(" data pull >> %d \r\n" , num_samples);

							 max8614x_mode1_data             ppgDataSample;
							 accel_mode1_data                accelDataSamp;
							 whrm_wspo2_suite_mode2_data     algoDataSamp;

							 uint8_t *ptr = &databuf[1]; //first byte is status so skip it.
							 //uint8_t *end = &databuf[num_samples*WHRM_FRAME_SIZE];
							 //while( ptr < end )

							 int sampleIdx = 0;
							 while( sampleIdx < num_samples ) {

								 ppgDataSample.led1  			     =  (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 ppgDataSample.led2  			     =  (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 ppgDataSample.led3  			     =  (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 ppgDataSample.led4  				 =  (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 ppgDataSample.led5  				 =  (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 ppgDataSample.led6  				 =  (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);

								 accelDataSamp.x                     =  (*ptr++ << 8)  + (*ptr++ << 0);
								 accelDataSamp.y                     =  (*ptr++ << 8)  + (*ptr++ << 0);
								 accelDataSamp.z                     =  (*ptr++ << 8)  + (*ptr++ << 0);

								 algoDataSamp.current_operating_mode =  (*ptr++);
								 algoDataSamp.hr                     =  (*ptr++ << 8)  + (*ptr++ << 0);
								 algoDataSamp.hr_conf                =  (*ptr++);
								 algoDataSamp.rr                     =  (*ptr++ << 8)  + (*ptr++ << 0);
								 algoDataSamp.rr_conf      			 =  (*ptr++);
								 algoDataSamp.activity_class         =  (*ptr++);

								 algoDataSamp.walk_steps             =  (*ptr++ << 24) + (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 algoDataSamp.run_steps              =  (*ptr++ << 24) + (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 algoDataSamp.kcal                   =  (*ptr++ << 24) + (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);
								 algoDataSamp.cadence                =  (*ptr++ << 24) + (*ptr++ << 16) + (*ptr++ << 8) + (*ptr++ << 0);

								 algoDataSamp.is_led_cur1_adj        =  (*ptr++);
								 algoDataSamp.adj_led_cur1           =  (*ptr++ << 8)  + (*ptr++ << 0);
								 algoDataSamp.is_led_cur2_adj        =  (*ptr++);
								 algoDataSamp.adj_led_cur2           =  (*ptr++ << 8)  + (*ptr++ << 0);
								 algoDataSamp.is_led_cur3_adj        =  (*ptr++);
								 algoDataSamp.adj_led_cur3           =  (*ptr++ << 8)  + (*ptr++ << 0);

								 algoDataSamp.is_int_time_adj        =  (*ptr++);
								 algoDataSamp.t_int_code             =  (*ptr++);
								 algoDataSamp.is_f_smp_adj           =  (*ptr++);
								 algoDataSamp.adj_f_smp 			 =  (*ptr++);
								 algoDataSamp.smp_ave                =  (*ptr++);
								 algoDataSamp.hrm_afe_state          =  (*ptr++);
								 algoDataSamp.is_high_motion         =  (*ptr++);
								 algoDataSamp.scd_contact_state      =  (*ptr++);

								 algoDataSamp.r                      =  (*ptr++ << 8)  + (*ptr++ << 0);
								 algoDataSamp.spo2_conf              =  (*ptr++);
								 algoDataSamp.spo2                   =  (*ptr++ << 8)  + (*ptr++ << 0);
								 algoDataSamp.percentComplete 		 =  (*ptr++);
								 algoDataSamp.lowSignalQualityFlag   =  (*ptr++);
								 algoDataSamp.motionFlag 			 =  (*ptr++);
								 algoDataSamp.lowPiFlag 			 =  (*ptr++);
								 algoDataSamp.unreliableRFlag 		 =  (*ptr++);
								 algoDataSamp.spo2State 			 =  (*ptr++);

								 sampleIdx += 1;


                                 // MAP TO WRAPPER INPUT SAMPLE FORMAT______________________________:
								 mxm_algosuite_input_data highLevelAlgoSample;

								 highLevelAlgoSample.inp_sample_count	    	= sample_cnt++;
								 highLevelAlgoSample.grn_count					= ppgDataSample.led1;
								 highLevelAlgoSample.grn2Cnt			  		= ppgDataSample.led4;
								 highLevelAlgoSample.irCnt	    		  		= ppgDataSample.led2;
								 highLevelAlgoSample.redCnt				  		= ppgDataSample.led3;
								 highLevelAlgoSample.accelx						= accelDataSamp.x;
								 highLevelAlgoSample.accely						= accelDataSamp.y;
								 highLevelAlgoSample.accelz						= accelDataSamp.z;
								 highLevelAlgoSample.whrm_suite_curr_opmode 	= algoDataSamp.current_operating_mode;
								 highLevelAlgoSample.hearth_rate_estim      	= algoDataSamp.hr;
								 highLevelAlgoSample.hr_confidence  	  		= algoDataSamp.hr_conf;
								 highLevelAlgoSample.rr_interbeat_interval		= algoDataSamp.rr;
								 highLevelAlgoSample.rr_confidence  	  		= algoDataSamp.rr_conf;
								 highLevelAlgoSample.activity_class				= algoDataSamp.activity_class;
								 highLevelAlgoSample.r_spo2						= algoDataSamp.r;
								 highLevelAlgoSample.spo2_confidence  	  		= algoDataSamp.spo2_conf;
								 highLevelAlgoSample.spo2_estim					= algoDataSamp.spo2;
								 highLevelAlgoSample.spo2_calc_percentage		= algoDataSamp.percentComplete;
								 highLevelAlgoSample.spo2_low_sign_quality_flag	= algoDataSamp.lowSignalQualityFlag;
								 highLevelAlgoSample.spo2_motion_flag       	= algoDataSamp.motionFlag;
								 highLevelAlgoSample.spo2_low_pi_flag	    	= algoDataSamp.lowPiFlag;
								 highLevelAlgoSample.spo2_unreliable_r_flag 	= algoDataSamp.unreliableRFlag;
								 highLevelAlgoSample.spo2_state			  		= algoDataSamp.spo2State;
								 highLevelAlgoSample.skin_contact_state	    	= algoDataSamp.scd_contact_state;
								 highLevelAlgoSample.walk_steps           		= algoDataSamp.walk_steps;
								 highLevelAlgoSample.run_steps            		= algoDataSamp.run_steps;
								 highLevelAlgoSample.kcal                 		= algoDataSamp.kcal;
								 highLevelAlgoSample.cadence              		= algoDataSamp.cadence;

								 long long int u64_sample_time = GET_LOCAL_64BIT_UTC_TIME();
								 highLevelAlgoSample.timestampUpper32bit        = (u64_sample_time >> SHIFT_FOR_UPPER_32BITS) & 0xFFFFFFFF;
								 highLevelAlgoSample.timestampLower32bit        = (u64_sample_time >> SHIFT_FOR_LOWER_32BITS) & 0xFFFFFFFF;


							 } //eof loop reading bytyes from hub report fifo

						 } // eof datas pull request form hub

	          } // eof fifo data count query

   		 }

   	 }

    return num_samples;

}


int run_algorithm_wrapper_consuming_whrm_outputs(void){


    // HOLD DATA FROM SENSORHUB
	const int MAX_WHRMWSPO2_SAMPLE_COUNT   = 45;


	int num_samples_fetched_from_sensorhub;
    int totalSamplesProcessed;

	int status;
	// Authenticate to sensorhub:
	status = authenticate_to_sensorhub();
    if( 0 != status)
       return -1;

    //init algorithms within wrapper
    status = init_wrapper_algos();
    if( 0 != status)
       return -1;

   // start measurement from SensorHub
    status = start_sensorhub_measurement_for_algowrapper();
    if( 0 != status)
       return -1;


    totalSamplesProcessed = 0;

    while( totalSamplesProcessed < MIN_NUM_SENSORHUB_SAMPLES_TO_EVALUATE_ALL_WRAPPER_ALGORITHM ){

		 /* THIS IS FOR SHOWING HOW TO PUSH ACCEL DATA TO SENSOR HUB. IN REALITY ACCEL DATA SHOULD BE COLLECTED IN 25HZ ie 40ms intervals
			and be pushed to sensor hub if accel count acceeds 5 samples if there is enaough free space in sesnor hub input FIFO
		 */
		 if( accelBehavior == SH_INPUT_DATA_FROM_HOST) {

			 FeedAccDataIntoSensHub ();
		 }

		num_samples_fetched_from_sensorhub = 0;
		mxm_algosuite_input_data algoWrapperInputSamples[MAX_WHRMWSPO2_SAMPLE_COUNT];
		// FETCH MEASUREMENT SAMPLES FROM SENSORHUB in WRAPPER INPUT FORMAT
		num_samples_fetched_from_sensorhub = fetch_measurement_samples_from_sensorhub( &algoWrapperInputSamples[0]);

		int samplesFedToWrapper = 0;
		while( samplesFedToWrapper <  num_samples_fetched_from_sensorhub) {

			// _____________________Instant Wrapper Run STATUS__________________
		    mxm_algosuite_return_code algoWrapperRunStatus;
		    // _____________________Instant Wrapper OUTPUT for all enabled algorithms_________________
		    mxm_algosuite_output_data algoWrapperOutput;

			mxm_algosuite_manager_run(  &algoWrapperInputSamples[samplesFedToWrapper],
										&algoWrapperOutput,
										&algoWrapperRunStatus);


			samplesFedToWrapper += 1;
		}

		totalSamplesProcessed += num_samples_fetched_from_sensorhub;


    }//eof main measurement loop

    // FINALIZE WRAPPER OPERATION
    mxm_algosuite_return_code algoWrapperEndStatus;
    mxm_algosuite_manager_end( (unsigned char) (MXM_ALGOSUITE_ENABLE_HRV |  MXM_ALGOSUITE_ENABLE_RESP | MXM_ALGOSUITE_ENABLE_STRESS | MXM_ALGOSUITE_ENABLE_SLEEP),
    												  &algoWrapperEndStatus);

    if ( algoWrapperEndStatus.hrv_status == MXM_HRV_SUCCESS &&
    	 algoWrapperEndStatus.resp_status == MXM_RESPIRATION_RATE_MANAGER_SUCCESS &&
		 algoWrapperEndStatus.stress_status == MXM_STRESS_MONITORING_SUCCESS &&
		 algoWrapperEndStatus.sleep_status == MXM_SLEEP_MANAGER_SUCCESS)
    	 status = 0;
    else
    	 status = -1;

    // stop Sensorhub measurement
    sh_disable_algo(SS_ALGOIDX_WHRM_WSPO2_SUITE);
	sh_disable_sensor_list();


}

