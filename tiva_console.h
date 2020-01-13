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

#ifndef TIVA_CONSOLE_H_
#define TIVA_CONSOLE_H_

#include <stdint.h>
#include "tiva_console_config.h"


#define CONSOLE_CLEAR_SCREEN  "\033[2J\033[H"
#define CONSOLE_LOCAL_ECHO    "\033[12l"
#define END_OF_LINE           "\r\n"





typedef struct console_operations
{
    void   (*printchar)(uart_periph_t *port_address, char data);
    char   (*readchar)(uart_periph_t *port_address);
    int8_t (*write)(uart_periph_t *p_uart_x, const char *buffer, int16_t length);
    int8_t (*read)(uart_periph_t *port_address, char *read_buffer, int16_t length);

}console_operations_t;




typedef struct console
{
    char    string_input[MAX_INPUT_SIZE];
    uint8_t arguments_count;
    int8_t  return_values;

}console_t;




int8_t init_console(uart_periph_t *port_address);

int8_t console_print(char *string_input);

int8_t console_getstring(console_t *command_line_input, uint16_t input_length);



#endif /* TIVA_CONSOLE_H_ */
