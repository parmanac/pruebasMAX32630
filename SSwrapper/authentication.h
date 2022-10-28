/*
 * authentication.h
 *
 *  Created on: Dec 18, 2019
 *      Author: Yagmur.Gok
 */

#ifndef SOURCE_SIMPLEST_AUTHENTICATION_H_
#define SOURCE_SIMPLEST_AUTHENTICATION_H_

#include <stdint.h>

#define AUTH_INITIALS_SEQUENCE_SIZE  ((int)12)
#define AUTH_PUBLIC_STRING_SIZE      ((int)12)
#define AUTH_FINAL_SEQUENCE_SIZE     ((int)32)

typedef struct{
	uint8_t authInitialsArray[AUTH_INITIALS_SEQUENCE_SIZE];
	uint8_t authLocalPublicString[AUTH_PUBLIC_STRING_SIZE];
	uint8_t authSensorhubPublicString[AUTH_PUBLIC_STRING_SIZE];
	uint8_t authFinalAuthString[AUTH_FINAL_SEQUENCE_SIZE];
}authentication_data_t;


int authenticate_to_sensorhub(void);

extern authentication_data_t sessionAuthData;


#endif /* SOURCE_SIMPLEST_AUTHENTICATION_H_ */
