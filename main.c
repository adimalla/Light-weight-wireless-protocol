




#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "tm4c123gh6pm.h"
#include "mcu_tm4c123gh6pm.h"
#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"

#include "network_protocol_configs.h"
#include "wi_network.h"
#include "comms_protocol.h"




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


/* Peripheral config defines */
#define COMMS_GPIO   GPIOC
#define COMMS_UART   UART1

#define COMMS_RX     4
#define COMMS_TX     5



typedef enum fsm_state_values
{
    START_STATE       = 15,
    MSG_READ_STATE    = 10,
    SYNC_STATE         = 1,
    JOINREQ_STATE      = 2,
    JOINRESP_STATE     = 3,
    STATUSMSG_STATE    = 4,
    STATUSACK_STATE    = 5,
    CONTROLMSG_STATE   = 6,
    EVENTMSG_STATE     = 7,
    TIMEOUT_STATE      = 8,
    EXIT_STATE         = 9

}fsm_states_t;






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
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF | SYSCTL_RCGC2_GPIOC;

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



void init_comm(void)
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
    xbee.uart_config.uart_fifo         = FIFO_DISABLE;       /*!<*/
    xbee.uart_config.stop_bits         = ONE_STOP_BIT;      /*!<*/
    xbee.uart_config.uart_direction    = UART_TRANSCEIVER;  /*!<*/

    /* Enable UART configs */
    uart_init(&xbee);

    /* Configure UART RX interrupt */
    UART1->IM = (1 << 4);          /*!<*/
    NVIC->EN0 |= 1 << UART1_IRQn;  /*!<*/

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



void uart1ISR(void)
{
    protocol_handle_t server_device;

    static uint8_t rx_index = 0;
    uint8_t checksum = 0;

    char c = UART1_DR_R & 0xFF;

    buffer.receive_message[rx_index] = c;

    if(buffer.receive_message[rx_index] == 't' && buffer.receive_message[rx_index - 1] == '\r')
    {

        UART1->ICR |= (1 << 4);

        server_device.packet_type = (void*)buffer.receive_message;

        /* Validate checksum */
        checksum = wi_network_checksum((char*)buffer.receive_message, 5, rx_index + 1);

        rx_index = 0;

        if(server_device.packet_type->fixed_header.message_checksum == checksum)
        {
            checksum = 0;

            /* Manage Network Access */
            if(server_device.packet_type->fixed_header.message_type < 10)
            {

                if(server_device.packet_type->fixed_header.message_type == COMMS_JOINREQ_MESSAGE)
                {
                    buffer.flag_state = JOINREQ_FLAG;

                    /* Copy data to read buffer  */
                    memset(buffer.read_message, 0, sizeof(buffer.read_message));
                    memcpy(buffer.read_message, buffer.receive_message, server_device.packet_type->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH);

                }

                if(server_device.packet_type->fixed_header.message_type == COMMS_STATUS_MESSAGE)
                {
                    buffer.flag_state = STATUSMSG_FLAG;

                    /* Copy data to read buffer  */
                    memset(buffer.read_message, 0, sizeof(buffer.read_message));
                    memcpy(buffer.read_message, buffer.receive_message, server_device.packet_type->fixed_header.message_length + COMMS_PREAMBLE_LENTH + COMMS_FIXED_HEADER_LENGTH);

                }


            }
        }

        /* Clear receive buffer */
        memset(buffer.receive_message, 0, sizeof(buffer.receive_message));

        rx_index = 0;

    }
    else
    {
        rx_index++;

    }


}




typedef struct client_devices
{
    uint8_t client_id;
    char    client_mac[6];
    uint8_t client_number_of_slots;
    uint8_t client_state;

}client_devices_t;



typedef struct table_return_values
{
    uint8_t table_index;
    int8_t table_retval;

}table_retval_t;



table_retval_t update_client_table(client_devices_t *device_table, protocol_handle_t *protocol_server, device_config_t *server)
{
    table_retval_t return_value;

    uint8_t index = 0;
    uint8_t found = 0;

    uint16_t requested_slots = 0;

    /* Get number of slots from the payload */
    requested_slots = atoi(protocol_server->joinrequest_msg->payload);

    /* Error check */
    if(device_table == NULL || protocol_server == NULL || server == NULL )
    {
        return_value.table_retval = -1;
    }
    else if(requested_slots > COMMS_SERVER_MAX_SLOTS)
    {
        return_value.table_retval = -2;
    }
    else
    {
        /* get data from join request */
        for(index = 0; index < 5; index++)
        {
            if(memcmp(device_table[index].client_mac, protocol_server->joinrequest_msg->source_mac,6) == 0)
            {
                found = 1;

                return_value.table_retval = -3;

                return_value.table_index = index;

                break;
            }
            else if(device_table[index].client_id == 0) /* Fill the next available row */
            {
                found = 0;

                /* Add new device ID to table */
                device_table[index].client_id = server->total_slots;

                /* Add MAC address to table */
                memcpy(device_table[index].client_mac, protocol_server->joinrequest_msg->source_mac, 6);

                /* Update slot */
                if(requested_slots > 1)
                {
                    server->total_slots += requested_slots;
                }
                else
                {
                    server->total_slots++;
                }

                /* Add client slots to table */
                device_table[index].client_number_of_slots = requested_slots;


                /* update device count */
                server->device_count++;

                /* return client ID */
                return_value.table_index = index;

                break;
            }

        }
    }

    return return_value;
}



int8_t read_client_table(char *client_mac_address, int8_t *client_id, client_devices_t *device_table, int16_t table_index)
{
    int8_t func_retval;

    if(table_index < 0)
    {
        //strncpy(client_mac_address, device_table[table_index].client_mac, 6);
        *client_id = table_index;

        func_retval = -4;
    }
    else
    {
        strncpy(client_mac_address, device_table[table_index].client_mac, 6);
        *client_id = device_table[table_index].client_id;

        func_retval = 0;
    }

    return func_retval;

}



int8_t calibrate_tx_timer(uint16_t device_slot_time, uint8_t device_slot_number)
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




void wTimer5Isr(void)
{
    // Clear Interrupt
    WTIMER5_ICR_R = TIMER_ICR_TAMCINT;
    WTIMER5_TAV_R = 0;


    /* WI Network related declarations */
    access_control_t       slot_network;
    static device_config_t sync_gen;
    protocol_handle_t      device_server;

    /* Device DB related declarations */
    static client_devices_t client_devices[5];
    static table_retval_t table_values;

    static uint8_t contrl_flag = 0;


    char    message_buffer[144]     = {0};
    uint8_t message_length          = 0;
    char    destination_mac_addr[6] = {0};

    static int8_t  client_id                 = 0;    /*!< from device table   */
    static uint8_t destination_client_id     = 0;    /*!< from status message */
    static uint8_t source_client_id          = 0;
    static char    status_message_buffer[20] = {0};

    static int8_t fsm_state = START_STATE;

    switch(fsm_state)
    {

    case START_STATE:

        memset(&sync_gen, 0 ,sizeof(sync_gen));
        memset(client_devices, 0 ,sizeof(client_devices));

        sync_gen.device_mac[0] = 11;
        sync_gen.device_mac[1] = 22;
        sync_gen.device_mac[2] = 33;
        sync_gen.device_mac[3] = 44;
        sync_gen.device_mac[4] = 55;
        sync_gen.device_mac[5] = 66;

        sync_gen.device_network_id  = 1441;
        sync_gen.device_slot_number = COMMS_SERVER_SLOTNUM;
        sync_gen.device_slot_time   = COMMS_SERVER_SLOT_TIME;
        sync_gen.total_slots        = 4;

        /* calibrate timer */
        calibrate_tx_timer(COMMS_SERVER_SLOT_TIME, sync_gen.total_slots);

        fsm_state = SYNC_STATE;

        break;


    case MSG_READ_STATE:

        ONBOARD_RED_LED = 0;
        ONBOARD_BLUE_LED = 0;

        fsm_state = buffer.flag_state;
        if(fsm_state == 0)
        {
            fsm_state = SYNC_STATE;

        }
        else
        {
            /* change state according to message type */
            device_server.packet_type = (void*)buffer.read_message;

            fsm_state = device_server.packet_type->fixed_header.message_type;


        }

        buffer.flag_state = CLEAR_FLAG;

        break;



    case SYNC_STATE:

        ONBOARD_RED_LED    = 0;
        ONBOARD_GREEN_LED ^= 1;

        slot_network.sync_message = (void*)message_buffer;

        message_length = wi_network_sync_message(&slot_network, sync_gen.device_network_id, sync_gen.device_slot_time, "sync");

        uart_write(UART1, (char*)slot_network.sync_message, message_length);

        fsm_state = MSG_READ_STATE;

        if(contrl_flag)
        {
            //calibrate_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

            fsm_state = CONTROLMSG_STATE;

        }

        break;


    case JOINREQ_STATE:

        ONBOARD_BLUE_LED = 0;
        ONBOARD_RED_LED ^= 1;

        device_server.joinrequest_msg = (void*)buffer.read_message;

        /* Check network id */
        if(device_server.joinrequest_msg->network_id == sync_gen.device_network_id && buffer.application_data.network_join_response == 1)
        {

            table_values = update_client_table(client_devices, &device_server, &sync_gen);
            if(table_values.table_retval == -1)
            {
                /* function parameter error */
                fsm_state = SYNC_STATE;
            }
            else
            {
                /* calibrate timer to broadcast slot */
                calibrate_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

                fsm_state = JOINRESP_STATE;

                buffer.application_data.network_join_response = 0;
            }

        }
        else
        {
            fsm_state = SYNC_STATE;
        }


        buffer.flag_state = CLEAR_FLAG;

        memset(buffer.read_message, 0, sizeof(buffer.read_message));


        break;



    case JOINRESP_STATE:

        /* send join response at broadcast slot and reset to updated slot */

        ONBOARD_RED_LED = 0;

        memset(message_buffer, 0, sizeof(message_buffer));

        device_server.joinresponse_msg = (void*)message_buffer;

        /* Get client data from the device table */
        read_client_table(destination_mac_addr, &client_id, client_devices, table_values.table_index);

        /* Set join response message type */
        comms_set_joinresp_message_status(&device_server, table_values.table_retval);

        /* Configure JOINRESP message */
        message_length = comms_joinresp_message(&device_server, sync_gen, destination_mac_addr, client_id);

        /* Send JOINRESP message */
        uart_write(UART1, (char*)device_server.joinresponse_msg, message_length);

        /* update timer to accommodate new slot */
        calibrate_tx_timer(COMMS_SERVER_SLOT_TIME, sync_gen.total_slots);

        fsm_state = SYNC_STATE;

        break;



    case STATUSMSG_STATE:

        /* Read Status message and send control message to the destination device */

        device_server.status_msg = (void*)buffer.read_message;

        /* Check network id */
        if(device_server.status_msg->network_id == sync_gen.device_network_id)
        {
            /* get destination client and payload from status */
            comms_get_status_message(status_message_buffer, &source_client_id, &destination_client_id, device_server);

            /* handle server message condition (not done) */
            if(destination_client_id == 1)
            {
                /* calibrate timer to broadcast message */
                calibrate_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

                fsm_state = CONTROLMSG_STATE;

            }
            else
            {
                /* search table for */
                read_client_table(destination_mac_addr, &client_id, client_devices, destination_client_id - 4);

                /* calibrate timer to broadcast message */
                calibrate_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

                fsm_state = CONTROLMSG_STATE;

            }

            memset(buffer.read_message, 0, sizeof(buffer.read_message));


        }
        else
        {
            fsm_state = SYNC_STATE;
        }

        buffer.flag_state = CLEAR_FLAG;


        break;


    case STATUSACK_STATE:

        memset(message_buffer, 0, sizeof(message_buffer));

        device_server.statusack_msg = (void*)message_buffer;

        message_length = comms_statusack_message(&device_server, sync_gen, client_id, destination_client_id);

        uart_write(UART1, (char*)device_server.statusack_msg, message_length);

        contrl_flag = 1;

        fsm_state = MSG_READ_STATE;

        break;



    case CONTROLMSG_STATE:

        ONBOARD_BLUE_LED = 1;

        memset(message_buffer, 0, sizeof(message_buffer));

        device_server.contrl_msg = (void*)message_buffer;

        /* echo condition (compare ID gotten from status message with ID gotten from device table )*/

        message_length = comms_control_message(&device_server, sync_gen, source_client_id, destination_client_id, status_message_buffer);

        uart_write(UART1, (char*)device_server.contrl_msg, message_length);

        calibrate_tx_timer(COMMS_SERVER_SLOT_TIME, sync_gen.total_slots);

        memset(status_message_buffer, 0, sizeof(status_message_buffer));

        contrl_flag = 0;


        fsm_state = SYNC_STATE;

        break;


    default:

        ONBOARD_GREEN_LED = 1;

        fsm_state = MSG_READ_STATE;

        break;

    }


}




/**
 * main.c
 */
int main(void)
{
    bool loop = false;

    init_clocks();

    init_board_io();

    init_comm();

    init_wide_timer_5();


    loop = true;
    while(loop)
    {

    }

    return 0;
}
