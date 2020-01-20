/*
 ******************************************************************************
 * @file    comms_protocol_configs.h
 * @author  Aditya Mall,
 * @brief   (6314)comms protocol header file.
 *
 *  Info
 *          (6314)comms protocol header file, based on custom protocol,
 *          part of EE6314 IOT project (University of Texas, Arlington)
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2019 Aditya Mall, MIT License </center></h2>
 *
 * MIT License
 *
 * Copyright (c) 2019 Aditya Mall
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

#ifndef NETWORK_PROTOCOL_CONFIGS_H_
#define NETWORK_PROTOCOL_CONFIGS_H_


#define ACTIVITY_OPERATIONS  1
#define DEBUG_OPERATIONS     1


/* Message Premable defines */
#define PREAMBLE_SYNC       0xAA11
#define PREAMBLE_JOINREQ    0xBB11
#define PREMABLE_JOINRESP   0xBB22
#define PREAMBLE_STATUS     0xCC11
#define PREAMBLE_CONTRL     0xCC22
#define PREAMBLE_STATUSACK  0xCC33


/* Message number defines */
#define COMMS_SYNC_MESSAGE       1
#define COMMS_JOINREQ_MESSAGE    2
#define COMMS_JOINRESP_MESSAGE   3
#define COMMS_STATUS_MESSAGE     4
#define COMMS_STATUSACK_MESSAGE  5
#define COMMS_CONTRL_MESSAGE     6
#define COMMS_EVNT_MESSAGE       7
#define COMMS_HIBERNATE_MESSAGE  8
#define COMMS_UNJOIN_MESSAGE     9



/* Network slot defines */
/* Server slot number is a ranked slot number, also used for sync message slot, changes with number of device */
#define COMMS_SERVER_SLOTNUM        1
#define COMMS_ACCESS_SLOTNUM        2
#define COMMS_BROADCAST_SLOTNUM     3
#define COMMS_SYNC_SLOTNUM          COMMS_SERVER_SLOTNUM
#define COMMS_MESSAGE_LENGTH        64


/* Generic message size defines */
#define COMMS_PREAMBLE_LENTH            2
#define COMMS_FIXED_HEADER_LENGTH       3
#define COMMS_CHECKSUM_SIZE             1
#define COMMS_MACADDR_SIZE              6
#define COMMS_SOURCE_MACADDR_SIZE       6
#define COMMS_DESTINATION_MACADDR_SIZE  6
#define COMMS_NETWORK_ID_SIZE           2
#define COMMS_SLOTNUM_SIZE              1
#define COMMS_PAYLOAD_LENGTH            20
#define COMMS_MESSAGE_TERMINATOR        "\rt"
#define COMMS_TERMINATOR_LENGTH         2


/* Sync Message size defines  */
#define COMMS_ACCESS_SLOT_SIZE  1
#define COMMS_SLOT_TIME_SIZE    2


/* JOINREQ and JOINRESP defines */
#define COMMS_JOIN_OPTONS_SIZE  1
#define COMMS_JOINREQ_PAYLOAD   11
#define COMMS_JOINRESP_PAYLOAD  12

/* STATUS, CONTRL and EVNT defines */
#define COMMS_DESTINATION_DEVICEID_SIZE 1


/* Server related defines */
#define COMMS_ACCESS_SLOT_SIZE     1
#define COMMS_SERVER_SLOT_TIME     5
#define COMMS_SLOT_TIME_SIZE       2
#define COMMS_SERVER_MAX_SLOTS     20
#define MAX_SLOT_TIME              1000


/* STATUS, CONTRL and EVNT defines */
#define COMMS_SOURCE_DEVICEID_SIZE      1
#define COMMS_DESTINATION_DEVICEID_SIZE 1


#endif /* NETWORK_PROTOCOL_CONFIGS_H_ */
