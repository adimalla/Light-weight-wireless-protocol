/*
 * xbee_driver.c
 *
 * Crude test xbee driver file
 *
 *  Created on: Jan 13, 2020
 *      Author: root
 */


#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"

#include "xbee_driver.h"



/* Peripheral config defines */
#define COMMS_GPIO   GPIOC
#define COMMS_UART   UART1

#define COMMS_RX     4
#define COMMS_TX     5




void init_xbee_comm(void)
{
    gpio_handle_t xbee_io;
    uart_handle_t xbee;

    /* xBee takes PC4 to DOUT pin (pin 2) and PC5 to DIN/CONFIG pin (pin 3), see XBee Pinout diagram */

    /* Configure GPIO ports for UART */
    xbee_io.p_gpio_x = COMMS_GPIO;
    xbee_io.gpio_pin_config.pin_mode           = DIGITAL_ENABLE;             /*!<*/
    xbee_io.gpio_pin_config.alternate_function = ALTERNATE_FUNCTION_ENABLE;  /*!<*/

    xbee_io.gpio_pin_config.pin_number         = COMMS_RX;                   /*!<*/
    xbee_io.gpio_pin_config.pctl_val           = UART1RX_PC4;                /*!<*/

    /* Enable GPIO PC4 configs */
    gpio_init(&xbee_io);

    xbee_io.gpio_pin_config.pin_number         = COMMS_TX;     /*!<*/
    xbee_io.gpio_pin_config.pctl_val           = UART1TX_PC5;  /*!<*/

    /* Enable GPIO PC5 configs */
    gpio_init(&xbee_io);


    /* Configure UART1 for PC4 and PC5 (RX, TX) */
    xbee.p_uart_x                      = COMMS_UART;        /*!<*/
    xbee.uart_config.uart_clock_source = CLOCK_SYSTEM;      /*!<*/
    xbee.uart_config.uart_baudrate     = 115200;            /*!<*/
    xbee.uart_config.word_length       = EIGHT_BITS;        /*!<*/
    xbee.uart_config.uart_fifo         = FIFO_DISABLE;      /*!<*/
    xbee.uart_config.stop_bits         = ONE_STOP_BIT;      /*!<*/
    xbee.uart_config.uart_direction    = UART_TRANSCEIVER;  /*!<*/

    /* Enable UART configs */
    uart_init(&xbee);

    /* Configure UART RX interrupt */
    UART1->IM = (1 << 4);          /*!<*/
    NVIC->EN0 |= 1 << UART1_IRQn;  /*!<*/

}




int8_t xbee_send(char* message_buffer, uint8_t message_length)
{

    int8_t func_retval = 0;

    uart_write(COMMS_UART, message_buffer, (int16_t)message_length);

    return func_retval;
}


