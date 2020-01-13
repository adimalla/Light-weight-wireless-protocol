


/**
 ******************************************************************************
 * @file    uart_tm4c123gh6pm.h, file name will change
 * @author  Aditya Mall,
 * @brief   TM4C123GH6PM Device Peripheral Access Layer Header File.
 *
 *  This file contains:
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2019 Aditya Mall, Hari Haran Krishnan </center></h2>
 *
 * TODO Add license, add your name as you make changes
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
 * @brief Standard Headers
 */

#include  <strings.h>
#include "mcu_tm4c123gh6pm.h"
#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"
#include "tiva_console.h"
#include "console_operations.h"




/******************************************************************************/
/*                                                                            */
/*                     Console Structures and defines                         */
/*                                                                            */
/******************************************************************************/



typedef enum func_retval
{
    func_param_error  = -1,
    func_exec_success = 0,
    func_retval_error = -2,
    gpio_init_error   = -1,
    uart_init_error   = -1

}func_retval_t;


typedef enum console_command
{
    BACKSPACE         = 8,
    NEWLINE_CHARACTER = 10,
    CARRIAGE_RETURN   = 13,
    ASCII_ESCAPE      = 27,
    ASCII_DELETE      = 127

}console_command_t;




/******************************************************************************/
/*                                                                            */
/*                         Console Functions                                  */
/*                                                                            */
/******************************************************************************/


/*
 * @brief   Initializes UART
 * @param   *p_uart_hanlde : pointer to the uart handle structure
 * @retval  int8_t         : -1 = error, 0 = success
 */
int8_t init_console(uart_periph_t *port_address)
{

    int8_t func_retval = 0;
    int8_t gpio_retval = 0;
    int8_t uart_retval = 0;

    gpio_handle_t console_io;
    uart_handle_t console_uart;

    memset(&console_io, 0, sizeof(console_io));

    console_io.p_gpio_x                           = GPIOA;
    console_io.gpio_pin_config.pin_mode           = DIGITAL_ENABLE;
    console_io.gpio_pin_config.alternate_function = ALTERNATE_FUNCTION_ENABLE;

    console_io.gpio_pin_config.pin_number = 0;
    console_io.gpio_pin_config.pctl_val   = UART0RX_PA0;

    gpio_retval = gpio_init(&console_io);
    if(gpio_retval == gpio_init_error)
    {
        func_retval = gpio_init_error;
    }
    else
    {
        console_io.gpio_pin_config.pin_number = 1;
        console_io.gpio_pin_config.pctl_val   = UART0TX_PA1;

        gpio_retval = gpio_init(&console_io);
        if(gpio_retval == gpio_init_error)
        {
            func_retval = gpio_init_error;
        }
        else
        {
            memset(&console_uart, 0, sizeof(console_uart));

            console_uart.p_uart_x = port_address;
            console_uart.uart_config.uart_clock_source = CLOCK_SYSTEM;
            console_uart.uart_config.uart_baudrate     = 115200;
            console_uart.uart_config.stop_bits         = ONE_STOP_BIT;
            console_uart.uart_config.uart_fifo         = FIFO_ENABLE;
            console_uart.uart_config.word_length       = EIGHT_BITS;
            console_uart.uart_config.uart_direction    = UART_TRANSCEIVER;

            uart_retval = uart_init(&console_uart);
            if(uart_retval == uart_init_error)
            {
                func_retval = uart_init_error;
            }
        }
    }

    return (func_retval_t)func_retval;
}




int8_t console_print(char *string_input)
{
    int8_t   func_retval   = 0;
    uint16_t string_length = 0;
    uint8_t  index         = 0;

    if(string_input == NULL)
    {
        func_retval = -1;
    }
    else
    {
        string_length = strlen(string_input);

        for(index = 0; index < string_length; index++)
        {
            if(string_input[index] == '\n')
            {
                console.printchar(STDOUT,'\r');
                console.printchar(STDOUT,'\n');
            }

            else
                console.printchar(STDOUT, string_input[index]);
        }

        func_retval = func_exec_success;
    }

    return func_retval;
}





int8_t console_getstring(console_t *command_line_input, uint16_t input_length)
{

    char     char_input  = 0;
    char     next_1      = 0;
    char     next_2      = 0;
    uint16_t char_count  = 0;

    func_retval_t  func_retval;


    /* Clear string before writing */
    memset(command_line_input->string_input, 0, input_length);


    /*Prompt sign*/
    console_print("\033[1;32m$>\033[0m");

    while(1)
    {
        char_input = console.readchar(STDIN);

        /* Carriage return and Backspace Processing */
        if(char_input == CARRIAGE_RETURN)
        {
            command_line_input->string_input[char_count] = '\0';

            console.printchar(STDOUT,'\r');
            console.printchar(STDOUT,'\n');

            char_count = 0;

            func_retval = func_exec_success;

            break;
        }
        else if (char_input == BACKSPACE || char_input == ASCII_DELETE)
        {
            if(char_count <= 0)
            {
                console_print("\033[C");
            }
            else
            {
                uart_putchar(UART0, ' ');
                console_print("\033[D");
                char_count--;
            }
        }
        else if (char_input == ASCII_ESCAPE)
        {
            next_1 = uart_getchar(STDIN);
            next_2 = uart_getchar(STDIN);

            if(next_1 == 91 && next_2 == 65)            // Up, don't process UP key press
            {
                console_print("\033[B");
                char_input = '\0';
            }
            else if(next_1 == 91 && next_2 == 66)       // Down, Don't process Down key press
            {
                console_print("\033[A");
                char_input = '\0';
            }
        }
        else
        {
            command_line_input->string_input[char_count++] =  char_input;
        }

        /* Buffer over flow check */
        if(char_count == input_length - 1)
        {

            command_line_input->string_input[char_count] = '\0';

            char_count = 0;

            console_print("\n");

            func_retval = func_retval_error;

            break;
        }

    }

    return func_retval;
}
