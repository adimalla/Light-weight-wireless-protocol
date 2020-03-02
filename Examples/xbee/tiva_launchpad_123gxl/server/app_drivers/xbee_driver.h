/*
 * xbee_driver.h
 *
 *  Created on: Jan 13, 2020
 *      Author: root
 */

#ifndef XBEE_DRIVER_H_
#define XBEE_DRIVER_H_


#include <stdint.h>


void init_xbee_comm(void);

int8_t xbee_send(char* message_buffer, uint16_t message_length);


#endif /* XBEE_DRIVER_H_ */
