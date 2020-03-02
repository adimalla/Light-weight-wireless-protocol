/**
 ******************************************************************************
 * @file    console_driver.c
 * @author  Aditya Mall
 * @brief
 *
 *  Info    serial console driver header file
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2020 Aditya Mall, MIT License </center></h2>
 *
 * MIT License
 *
 * Copyright (c) 2020 Aditya Mall
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */



/*
 * Standard header and driver header files
 */
#include <stdint.h>
#include <string.h>
#include "console_driver.h"

#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"



/******************************************************************************/
/*                                                                            */
/*                       Function Implementations                             */
/*                                                                            */
/******************************************************************************/



 /* Function to initialize system clock to 40 MHz */
 static void init_clock(void)
 {
     sysctl_t *pSYSCTL = SYSCTL;

     pSYSCTL->RCC = 0x00000540 | 0x00400000 | (0x04 << 23);
 }





/* Initialize UART for terminal */
static uint8_t init_uart(gpio_port_t *gpio_port_addr, uart_periph_t *port_address, uint32_t buad_rate)
{

    uint8_t func_retval = 0;

    gpio_handle_t console_io;
    uart_handle_t console_uart;

    init_clock();

    memset(&console_io, 0, sizeof(console_io));

    console_io.p_gpio_x                           = gpio_port_addr;
    console_io.gpio_pin_config.pin_mode           = DIGITAL_ENABLE;
    console_io.gpio_pin_config.alternate_function = ALTERNATE_FUNCTION_ENABLE;

    console_io.gpio_pin_config.pin_number = 0;
    console_io.gpio_pin_config.pctl_val   = UART0RX_PA0;

    gpio_init(&console_io);

    console_io.gpio_pin_config.pin_number = 1;
    console_io.gpio_pin_config.pctl_val   = UART0TX_PA1;

    gpio_init(&console_io);

    memset(&console_uart, 0, sizeof(console_uart));

    console_uart.p_uart_x = port_address;
    console_uart.uart_config.uart_clock_source = CLOCK_SYSTEM;
    console_uart.uart_config.uart_baudrate     = buad_rate;
    console_uart.uart_config.stop_bits         = ONE_STOP_BIT;
    console_uart.uart_config.uart_fifo         = FIFO_ENABLE;
    console_uart.uart_config.word_length       = EIGHT_BITS;
    console_uart.uart_config.uart_direction    = UART_TRANSCEIVER;

    uart_init(&console_uart);

    /* Needed for console API to work */
    func_retval = 1;

    return func_retval;
}




/* Wrapper function for open */
/* Must return 1 for success */
uint8_t serial_open(uint32_t baud_rate)
{
    int8_t func_retval = 0;

    func_retval = init_uart(GPIOA, UART0, baud_rate);

    return func_retval;
}


/* Wrapper function for print_char */
uint8_t write_char(char data)
{
    uart_putchar(UART0, data);

    return 0;
}


/* Wrapper function for read_char */
char read_char(void)
{
    return uart_getchar(UART0);
}




