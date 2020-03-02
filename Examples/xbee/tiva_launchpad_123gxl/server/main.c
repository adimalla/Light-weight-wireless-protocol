



/*
 * Standard Header and API Header files
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "tm4c123gh6pm.h"
#include "xbee_driver.h"
#include "application_functions.h"

#include "comms_network.h"
#include "comms_protocol.h"

#include "comms_server_fsm.h"



/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/

#define SLOT_TIME_MS      6
#define STARTING_SLOTS    3


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/


comms_network_buffer_t buffer;



/******************************************************************************/
/*                                                                            */
/*                         Functions Definitions                              */
/*                                                                            */
/******************************************************************************/




int8_t clear_uart_recv_interrupt(void)
{
    /* Clear UART interrupt */
    UART1_ICR_R |= (1 << 4);

    return 0;
}



network_operations_t net_ops =
{

 .send_message         = xbee_send,
 .set_tx_timer         = set_tx_timer,
 .clear_recv_interrupt = clear_uart_recv_interrupt,
 .sync_activity_status = sync_led_status,
 .send_activity_status = send_led_status,
 .recv_activity_status = recv_led_status,
 .clear_status         = clear_led_status

};



void gpioPortFIsr(void)
{

    GPIO_PORTF_ICR_R = 0x10;

    buffer.application_flags.network_join_response = 1;

}


void uart1ISR(void)
{

    access_control_t *wireless_network;

    wireless_network = create_network_handle(&net_ops);

    static uint8_t rx_index = 0;

    char c = UART1_DR_R & 0xFF;

    buffer.receive_message[rx_index] = c;

    comms_server_recv_it(wireless_network, &buffer, &rx_index);

}

char user_name[10]   = "sens_net";
uint8_t password[10] = "1234";

void wTimer5Isr(void)
{
    // Clear Interrupt
    WTIMER5_ICR_R = TIMER_ICR_TAMCINT;
    WTIMER5_TAV_R = 0;

    access_control_t *wireless_network;

    wireless_network = create_network_handle(&net_ops);

    device_config_t *server_device;

    server_device = create_server_device("11:22:33:44:55:66", 1441, SLOT_TIME_MS, STARTING_SLOTS, user_name, password);

    /* non object based, easy to debug */
    static client_devices_t client_devices[CLIENT_TABLE_SIZE];

    /* Start Server state machine */
    comms_start_server(wireless_network, server_device, &buffer, client_devices, WI_LOCAL_SERVER);

}



/*
 * main.c
 */
int main(void)
{
    bool loop = false;

    init_clocks();

    init_board_io();

    init_xbee_comm();

    init_wide_timer_5();


    loop = true;
    while(loop)
    {

    }

    return 0;
}
