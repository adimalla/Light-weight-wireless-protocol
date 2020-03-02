/*
 * applications_functions.c
 *
 *  Created on: Mar 1, 2020
 *      Author: root
 */


#include "tm4c123gh6pm.h"
#include "application_functions.h"




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
/*                    Clock and IO Configuration functions                    */
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



/******************************************************************************/
/*                                                                            */
/*                    Timer Configuration functions                           */
/*                                                                            */
/******************************************************************************/



void init_wide_timer_5(void)
{
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R5;                                       // turn-on timer
    WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;                                                  // turn-off counter before reconfiguring
    WTIMER5_CFG_R  = 4;                                                                // configure as 32-bit counter (A only)

    WTIMER5_TAMR_R = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TACDIR | TIMER_TAMR_TAMIE;    // 1-Shot mode, Count up, Match interrupt Enable
    WTIMER5_TAMATCHR_R = (40000 * (5 * 1)) - 1;                                        // Initial Match load value before sync, get from EEPROM, given by server

    NVIC_EN3_R |= 1 << (INT_WTIMER5A-16-96);                                           // Enable interrupt in NVIC
    WTIMER5_IMR_R = TIMER_IMR_TAMIM;                                                   // Match Interrupt Enable

    WTIMER5_CTL_R |= TIMER_CTL_TAEN;                                                   // start
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

        while(!( SYSCTL_PRWTIMER_R & (1 << 5)) );

        WTIMER5_TAMATCHR_R  = ( 40000 * (device_slot_time * device_slot_number) ) - 1;

        while(!( SYSCTL_PRWTIMER_R & (1 << 5)) );

        WTIMER5_CTL_R |= TIMER_CTL_TAEN;

        WTIMER5_TAV_R  = 0;
    }

    return func_retval;
}


int8_t rst_timer(void)
{

    /* Reset Send Timer */
    WTIMER5_CTL_R &= ~TIMER_CTL_TAEN;
    while(!( SYSCTL_PRWTIMER_R & (1 << 5)) );
    WTIMER5_TAV_R = 0;
    WTIMER5_CTL_R |= TIMER_CTL_TAEN;


    return 0;
}



/******************************************************************************/
/*                                                                            */
/*                     Status and Configuration functions                     */
/*                                                                            */
/******************************************************************************/


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


int8_t net_join_status(void)
{
    ONBOARD_BLUE_LED  ^= 1;
    ONBOARD_GREEN_LED ^= 1;

    return 0;
}


int8_t clear_led_status(void)
{

    ONBOARD_RED_LED   = 0;
    ONBOARD_BLUE_LED  = 0;
    ONBOARD_GREEN_LED = 0;

    return 0;
}


