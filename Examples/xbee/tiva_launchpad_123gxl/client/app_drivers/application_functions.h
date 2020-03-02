/*
 * application_functions.h
 *
 *  Created on: Mar 1, 2020
 *      Author: root
 */

#ifndef APPLICATION_FUNCTIONS_H_
#define APPLICATION_FUNCTIONS_H_



#include <stdint.h>



void init_clock(void);

void init_board_io(void);

void gpioPortFIsr(void);



void init_wide_timer_5(void);

int8_t set_tx_timer(uint16_t device_slot_time, uint8_t device_slot_number);

int8_t reset_timer(void);


int8_t sync_led_status(void);

int8_t recv_led_status(void);

int8_t send_led_status(void);

int8_t network_joined_status(void);

int8_t clear_led_status(void);


#endif /* EXAMPLES_XBEE_TIVA_LAUNCHPAD_123GXL_CLIENT_APP_DRIVERS_APPLICATION_FUNCTIONS_H_ */
