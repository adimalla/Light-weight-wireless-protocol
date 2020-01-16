




#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "tm4c123gh6pm.h"
#include "mcu_tm4c123gh6pm.h"
#include "gpio_tm4c123gh6pm.h"
#include "uart_tm4c123gh6pm.h"

#include "xbee_driver.h"

#include "network_protocol_configs.h"
#include "comms_network.h"
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

    char c = UART1->DR & 0xFF;

    buffer.receive_message[rx_index] = c;

    if(buffer.receive_message[rx_index] == 't' && buffer.receive_message[rx_index - 1] == '\r')
    {

        /* Clear UART interrupt */
        UART1->ICR |= (1 << 4);

        server_device.packet_type = (void*)buffer.receive_message;

        /* Validate checksum */
        checksum = comms_network_checksum((char*)buffer.receive_message, 5, rx_index + 1);

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


/* -2 : JOINRESP_NACK, -3: JOINRESP_DUP */
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

                /* Request for already present device */
                return_value.table_retval = -3;

                return_value.table_index = index;

                break;
            }
            else if(device_table[index].client_id == 0) /* Fill the next available row */
            {
                found = 0;

                /* Add new device ID to table */
                device_table[index].client_id = server->total_slots + 1;

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

                return_value.table_retval = 0;

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



int dummy_func(char *msh, uint8_t length)
{

    __asm(" NOP");

    return 0;
}



int8_t comms_start_server(uint8_t send_message_buffer_size);



void wTimer5Isr(void)
{
    // Clear Interrupt
    WTIMER5_ICR_R = TIMER_ICR_TAMCINT;
    WTIMER5_TAV_R = 0;


    /* WI Network related declarations */
    access_control_t *wireless_network;

    network_operations_t net_ops = {

    .send_message = xbee_send


    };

    wireless_network = create_network_handle(&net_ops);

    device_config_t *server_device;

    server_device = create_server_device("11:22:33:44:55:66", 1441, COMMS_SERVER_SLOT_TIME, 3);

    protocol_handle_t  server;

    static uint8_t contrl_flag = 0;

    char    send_message_buffer[144] = {0};
    uint8_t message_length           = 0;
    char    destination_mac_addr[6]  = {0};

    static int8_t  client_id                 = 0;    /*!< from device table   */
    static uint8_t destination_client_id     = 0;    /*!< from status message */
    static uint8_t source_client_id          = 0;
    static char    status_message_buffer[20] = {0};


    /* Device DB related declarations */
    static client_devices_t client_devices[5];
    static table_retval_t   table_values;


    static int8_t fsm_state = START_STATE;

    switch(fsm_state)
    {

    case START_STATE:

        memset(client_devices, 0 ,sizeof(client_devices));

        /* calibrate timer */
        set_tx_timer(server_device->device_slot_time, server_device->total_slots);

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
            server.packet_type = (void*)buffer.read_message;

            fsm_state = server.packet_type->fixed_header.message_type;


        }

        buffer.flag_state = CLEAR_FLAG;

        break;



    case SYNC_STATE:

        ONBOARD_RED_LED    = 0;
        ONBOARD_GREEN_LED ^= 1;

        wireless_network->sync_message = (void*)send_message_buffer;

        message_length = comms_network_sync_message(wireless_network, server_device->device_network_id, server_device->device_slot_time, "sync");

        comms_send(wireless_network, (char*)wireless_network->sync_message, message_length);

        fsm_state = MSG_READ_STATE;

        if(contrl_flag)
        {
            //set_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

            fsm_state = CONTROLMSG_STATE;

        }

        break;


    case JOINREQ_STATE:

        ONBOARD_BLUE_LED = 0;
        ONBOARD_RED_LED ^= 1;

        server.joinrequest_msg = (void*)buffer.read_message;

        /* Check network id */
        if(server.joinrequest_msg->network_id == server_device->device_network_id && buffer.application_data.network_join_response == 1)
        {

            table_values = update_client_table(client_devices, &server, server_device);
            if(table_values.table_retval == -1)
            {
                /* function parameter error */
                fsm_state = SYNC_STATE;
            }
            else
            {
                /* calibrate timer to broadcast slot */
                set_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

                fsm_state = JOINRESP_STATE;

                buffer.application_data.network_join_response = 0;
            }

        }
        else
        {
            buffer.application_data.network_join_response = 0;
            fsm_state = SYNC_STATE;
        }


        buffer.flag_state = CLEAR_FLAG;

        memset(buffer.read_message, 0, sizeof(buffer.read_message));


        break;



    case JOINRESP_STATE:

        /* send join response at broadcast slot and reset to updated slot */

        ONBOARD_RED_LED = 0;

        memset(send_message_buffer, 0, sizeof(send_message_buffer));

        server.joinresponse_msg = (void*)send_message_buffer;

        /* Get client data from the device table */
        read_client_table(destination_mac_addr, &client_id, client_devices, table_values.table_index);

        /* Set join response message type */
        comms_set_joinresp_message_status(&server, table_values.table_retval);

        /* Configure JOINRESP message */
        message_length = comms_joinresp_message(&server, *server_device, destination_mac_addr, client_id);

        /* Send JOINRESP message */
        comms_send(wireless_network, (char*)server.joinresponse_msg, message_length);

        /* update timer to accommodate new slot */
        set_tx_timer(COMMS_SERVER_SLOT_TIME, server_device->total_slots);

        fsm_state = SYNC_STATE;

        break;



    case STATUSMSG_STATE:

        /* Read Status message and send control message to the destination device */

        server.status_msg = (void*)buffer.read_message;

        /* Check network id */
        if(server.status_msg->network_id == server_device->device_network_id)
        {
            /* get destination client and payload from status */
            comms_get_status_message(status_message_buffer, &source_client_id, &destination_client_id, server);

            /* handle server message condition (not done) */
            if(destination_client_id == 1)
            {
                /* calibrate timer to broadcast message */
                set_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

                fsm_state = CONTROLMSG_STATE;

            }
            else
            {
                /* search table for */
                read_client_table(destination_mac_addr, &client_id, client_devices, destination_client_id - 4);

                /* calibrate timer to broadcast message */
                set_tx_timer(COMMS_SERVER_SLOT_TIME, COMMS_BRODCAST_SLOTNUM);

                fsm_state = CONTROLMSG_STATE;

            }

        }
        else
        {
            fsm_state = SYNC_STATE;
        }

        memset(buffer.read_message, 0, sizeof(buffer.read_message));

        buffer.flag_state = CLEAR_FLAG;


        break;


    case STATUSACK_STATE:

        memset(send_message_buffer, 0, sizeof(send_message_buffer));

        server.statusack_msg = (void*)send_message_buffer;

        message_length = comms_statusack_message(&server, *server_device, client_id, destination_client_id);

        uart_write(UART1, (char*)server.statusack_msg, message_length);

        contrl_flag = 1;

        fsm_state = MSG_READ_STATE;

        break;



    case CONTROLMSG_STATE:

        ONBOARD_BLUE_LED = 1;

        memset(send_message_buffer, 0, sizeof(send_message_buffer));

        server.contrl_msg = (void*)send_message_buffer;

        /* echo condition (compare ID gotten from status message with ID gotten from device table )*/

        message_length = comms_control_message(&server, *server_device, source_client_id, destination_client_id, status_message_buffer);

        comms_send(wireless_network, (char*)server.contrl_msg, message_length);

        set_tx_timer(COMMS_SERVER_SLOT_TIME, server_device->total_slots);

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

    init_xbee_comm();

    init_wide_timer_5();


    loop = true;
    while(loop)
    {

    }

    return 0;
}
