/*
 * authentication.cpp
 *
 *  Created on: Dec 18, 2019
 *      Author: Yagmur.Gok
 */

#include <mbed.h>  /*for type definitions*/
#include "SHComm.h"


#include "demoDefinitions.h"
#include "authentication.h"
#include "AlgoWrapper.h"

/*
 * IMPORTANT NOTE: AUTHENTICATION PROCESS IS ONLY REUQIRED FOR USING MAXIM SINGLE LIBARY OF ALGORITHMS
 *                 HRV + STRESS + SLEEP + RESPIRATION RATE which accepts WHRM suite outputs of Sensorhub
 *                 USERS WHO JUST USE SENSORHUB SHALL SKIP THIS PROCESS!
 *
 * AUTHENTICATION STEPS:
 * 1. Ask Sensorhub for session authentication initials : call to sh_get_dhparams()
 * 2. Pass authentication initials to Maxim Single Library and get session local string
 *                        from Maxim Single Library
 * 3. Send session local string of Maxim Single Library to senbsorhub : sh_set_dhlocalpublic()
 * 4. Ask Sensorhub for session public string of sensorhub : call to sh_get_dhremotepublic()
 * 5. Ask Sensorhub for session final authentication string : call to sh_get_authentication()
 * 6. Pass session public string of sensorhub and session final authentication string from sensorhub
 *                        to Maxim Single Library
 *
 *
 * */


authentication_data_t sessionAuthData;

int authenticate_to_sensorhub(void) {

    int idx;
    /* Step 1*/
	int status = sh_get_dhparams( &sessionAuthData.authInitialsArray[0], AUTH_INITIALS_SEQUENCE_SIZE);
	if( status != 0)
		return -1;

	/* Step 2*/
	/* PASS AUTH INITIALS TO MAXIM SINGLE LIBRARY OF ALOGORITHMS HRV + SLEEP +STRESS + RESPIRATION RATE
	  get form Maxim single library and pass session local string of Maxim Single Library to senbsorhub */
	mxm_algosuite_manager_getauthinitials( &sessionAuthData.authInitialsArray[0], &sessionAuthData.authLocalPublicString[0]);
	status = sh_set_dhlocalpublic( &sessionAuthData.authLocalPublicString[0] , AUTH_PUBLIC_STRING_SIZE);
	if( status != 0)
		return -1;

	wait_ms(500);

	/* Step 4 : Ask Sensorhub for session public string of sensorhub : call to sh_get_dhremotepublic() */
    status =  sh_get_dhremotepublic( &sessionAuthData.authSensorhubPublicString[0], AUTH_PUBLIC_STRING_SIZE);
    if( status != 0)
		return -1;

	wait_ms(500);

	/* Step 5: Ask Sensorhub for session final authentication string : call to sh_get_authentication()*/
	status = sh_get_authentication( &sessionAuthData.authFinalAuthString[0] , AUTH_FINAL_SEQUENCE_SIZE );
    if( status != 0)
		return -1;

    /*Pass session public string of sensorhub and session final authentication string from sensorhub to Maxim Single Library*/
    mxm_algosuite_manager_authenticate( &sessionAuthData.authSensorhubPublicString[0],
    		                                  &sessionAuthData.authFinalAuthString[0]);


	return status;

}





