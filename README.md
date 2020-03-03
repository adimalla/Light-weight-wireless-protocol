# Light-weight-wireless-protocol

#### (Updating docs) Light weight wireless messaging protocol for low power sensor networks based on slotted aloha access method.

**_Copyright (c) Aditya Mall, 2020_**
###### Please take Prior Persmisson from Dr Jason.Losh and respective owners if you wish to use it in your Academic Project at The University of Texas at Arlington.

## Info
This is a custom low bandwidth, low overhead light weight wireless protocol developed as a part of EE6314 project course work.
The Intitial design requirements were given by (Proff.) Dr.Jason Losh and later the protocol was modified to include a minimal API framework with authentication and Async State machine runing in timer interrupt service routine which can interract with user application using shared buffers and post debug information on serial terminal. The protocol also implements extended features such as Quality of Service and Message Queing.

The Protocol is based on an API framework which gives it protablibity to interface with any transceiver PHY module workiing on any serial protocol. 

## Goal
* To implement a light weight wireless protocol for low power sensor devices in a low bandwidth network.
* To implement client and server state machines with API framework for portbality and ease of development.
* To support a framework for an application level protocol over the underlying wireless protocol.
* To implement a gateway state machine for protocol translation to standard network protocols like TCP, UDP etc.

## Description.

### Wireless Protocol Flow Diagram

<img src="https://github.com/adimalla/Light-weight-wireless-protocol/blob/master/docs/images/Selection_337.jpg" width="900" height="800" title="CLI">

### Protocol Messages
1. SYNC Message (Synchronization Message, Broadcast Message send by the sever to synchronize client timers and give network
                 information)
2. JOINREQ Message (Join Request Message, send by the client to the server)
3. JOINRESP Message (Join Response Message, send by the Server to the client)
4. STATUS Message (Status Message, send by the client to the server)
5. CONTRL Message (Control Message, send by the server in response to status message, to the destination device)
6. STATACK Message (Status Acknowledgment message, Send by the server ins response to satus message, if quality of service is configured)

#### Connection Initiation Diagram
<img src="https://github.com/adimalla/Light-weight-wireless-protocol/blob/master/docs/images/Selection_338.jpg" width="800" height="800" title="CLI">


## Contact
For further queries please contact :- </br>

Aditya Mall (UTA MSEE)
</br>

email: aditya.mall1990@gmail.com.
