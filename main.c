



/*
 * Standard Header and API Header files
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "tm4c123gh6pm.h"
#include "mcu_tm4c123gh6pm.h"
#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"

#include "xbee_driver.h"

#include "comms_network.h"
#include "comms_protocol.h"

#include "comms_server_fsm.h"



/******************************************************************************/
/*                                                                            */
/*                  Data Structures and Defines                               */
/*                                                                            */
/******************************************************************************/



//****************** Bit Banding defines for Pins *********************//      (for TM4C123GXL-TIVA-C LAUNCHPAD)

//Port F
#define PF1               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define PF2               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
#define PF3               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))
#define PF4               (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 4*4)))



//***************************** Board Modules Pins *************************// (for TM4C123GXL-TIVA-C LAUNCHPAD)

#define ONBOARD_RED_LED           PF1
#define ONBOARD_BLUE_LED          PF2
#define ONBOARD_GREEN_LED         PF3
#define ONBOARD_PUSH_BUTTON       PF4






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



void init_clocks(void)
{
    //******************************************************* Clock Configs ******************************************************************//

    // Configure System clock as 40Mhz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (0x04 << SYSCTL_RCC_SYSDIV_S);

    // Enable GPIO port A, and F peripherals
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

}



void init_board_io(void)
{

    //**************************************************** On Board Modules ******************************************************************//

    // Configure On boards RED, GREEN and BLUE led and Pushbutton Pins
    GPIO_PORTF_DEN_R |= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);                  // Enable Digital
    GPIO_PORTF_DIR_R |= (1 << 1) | (1 << 2) | (1 << 3);                             // Enable as Output
    GPIO_PORTF_DIR_R &= ~(0x10);                                                    // Enable push button as Input
    GPIO_PORTF_PUR_R |= 0x10;                                                       // Enable internal pull-up for push button


    // GPIOF Interrupt
    GPIO_PORTF_IM_R |= 0x10;
    NVIC_EN0_R |= (1<<(INT_GPIOF-16));

}






void init_wide_timer_5(void)
{
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R5;                                       // turn-on timer
    WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;                                                  // turn-off counter before reconfiguring
    WTIMER5_CFG_R  = 4;                                                                // configure as 32-bit counter (A only)

    WTIMER5_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR | TIMER_TAMR_TAMIE;    // 1-Shot mode, Count up, Match interrupt Enable
    WTIMER5_TAMATCHR_R = (40000 * (1 * 1)) - 1;                                        // Initial Match load value before sync, get from EEPROM, given by server

    NVIC_EN3_R |= 1 << (INT_WTIMER5A-16-96);                                           // Enable interrupt in NVIC
    WTIMER5_IMR_R = TIMER_IMR_TAMIM;                                                   // Match Interrupt Enable

    WTIMER5_CTL_R |= TIMER_CTL_TAEN;                                                   // start
}





void gpioPortFIsr(void)
{

    GPIO_PORTF_ICR_R = 0x10;

    buffer.application_data.network_join_response = 1;

}


int8_t clear_uart_recv_interrupt(void)
{
    /* Clear UART interrupt */
    UART1->ICR |= (1 << 4);

    return 0;
}





void uart1ISR(void)
{

    access_control_t *wireless_network;

    network_operations_t net_ops =
    {
     .clear_recv_interrupt = clear_uart_recv_interrupt,
    };

    wireless_network = create_network_handle(&net_ops);

    static uint8_t rx_index = 0;

    char c = UART1->DR & 0xFF;

    buffer.receive_message[rx_index] = c;

    comms_server_recv_it(wireless_network, &buffer, &rx_index);

}




int8_t set_tx_timer(uint16_t device_slot_time, uint8_t device_slot_number)
{
    int8_t func_retval;

    if(device_slot_number == 0 || device_slot_time == 0)
    {
        func_retval = -1;
    }
    else
    {
        /* Calibrate Timer ISR to slot number received */
        WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;

        while(!( SYSCTL->PRWTIMER & (1 << 5)) );

        WTIMER5_TAMATCHR_R  = ( 40000 * (device_slot_time * device_slot_number) ) - 1;

        while(!( SYSCTL->PRWTIMER & (1 << 5)) );

        WTIMER5_CTL_R |= TIMER_CTL_TAEN;

        WTIMER5_TAV_R  = 0;
    }

    return func_retval;
}




int8_t sync_led_status(void)
{

    ONBOARD_RED_LED    = 0;
    ONBOARD_BLUE_LED   = 0;
    ONBOARD_GREEN_LED ^= 1;

    return 0;
}


int8_t recv_led_status(void)
{

    ONBOARD_BLUE_LED = 0;
    ONBOARD_RED_LED ^= 1;

    return 0;
}

int8_t send_led_status(void)
{
    ONBOARD_RED_LED   = 0;
    ONBOARD_BLUE_LED ^= 1;

    return 0;
}


int8_t clear_led_status(void)
{

    ONBOARD_RED_LED   = 0;
    ONBOARD_BLUE_LED  = 0;
    ONBOARD_GREEN_LED = 0;

    return 0;
}



void wTimer5Isr(void)
{
    // Clear Interrupt
    WTIMER5_ICR_R = TIMER_ICR_TAMCINT;
    WTIMER5_TAV_R = 0;

    access_control_t *wireless_network;

    network_operations_t net_ops =
    {

     .send_message          = xbee_send,
     .set_tx_timer          = set_tx_timer,
     .sync_activity_status  = sync_led_status,
     .send_activity_status  = send_led_status,
     .recv_activity_status  = recv_led_status,
     .clear_status          = clear_led_status

    };

    wireless_network = create_network_handle(&net_ops);

    device_config_t *server_device;

    server_device = create_server_device("11:22:33:44:55:66", 1441, 4, 3);

    static client_devices_t client_devices[5];

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
