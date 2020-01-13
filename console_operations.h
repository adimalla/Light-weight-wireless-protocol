/*
 * console_operations.h
 *
 *  Created on: Oct 6, 2019
 *      Author: root
 */

#ifndef CONSOLE_OPERATIONS_H_
#define CONSOLE_OPERATIONS_H_





#include <stdint.h>

console_operations_t console =
{
     printchar : uart_putchar,
     readchar  : uart_getchar,
     write     : uart_write,
     read      : uart_read,

};



#endif /* CONSOLE_OPERATIONS_H_ */
