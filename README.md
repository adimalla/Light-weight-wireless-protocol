# Light-weight-wireless-protocol

#### Light weight wireless messaging protocol for low power sensor networks based on slotted aloha access method.

**_Copyright (c) Aditya Mall, 2020_**
###### Please take prior permisson from Dr Jason.Losh and respective owners if you wish to use it in your Academic Project, if you happen to be a student at The University of Texas at Arlington.

## Info
This is a custom low bandwidth, low overhead light weight wireless protocol developed as a part of EE6314 project course work. The Initial design requirements were given by (Prof.) Dr.Jason Losh and later the protocol was modified to include a minimal API framework with single factor authentication and async state machine ruining in timer interrupt service routine which can interact with user application using shared buffers and post debug information on serial terminal. The protocol also implements extended features such as Quality of Service and Message Queuing.

The Protocol is based on an API framework which makes it portable to interface with any transceiver PHY module working on any serial protocol.

## Goal
* To implement a light weight wireless protocol for low power sensor devices in a low bandwidth network.
* To implement client and server state machines with API framework for portbality and ease of development.
* To support a framework for an application level protocol over the underlying wireless protocol.
* To implement a gateway state machine for protocol translation to standard network protocols like TCP, UDP etc.

## Description.

### Wireless Protocol Flow Diagram

<img src="https://github.com/adimalla/Light-weight-wireless-protocol/blob/master/docs/images/Selection_337.jpg" width="900" height="800" title="Layer Architecture">

### Protocol Messages
1. SYNC Message (Synchronization Message, Broadcast Message send by the server to synchronize client timers and give network
                 information)
2. JOINREQ Message (Join Request Message, send by the client to the server)
3. JOINRESP Message (Join Response Message, send by the Server to the client)
4. STATUS Message (Status Message, send by the client to the server)
5. CONTRL Message (Control Message, send by the server in response to status message, to the destination device)
6. STATACK Message (Status Acknowledgment message, Send by the server ins response to satus message, if quality of service is configured)

#### Connection Initiation Diagram
<img src="https://github.com/adimalla/Light-weight-wireless-protocol/blob/master/docs/images/Selection_338.jpg" width="800" height="800" title="Connection Iniation">

#### Data Communication Diagram
<img src="https://github.com/adimalla/Light-weight-wireless-protocol/blob/master/docs/images/Selection_339.jpg" width="900" height="900" title="Communication Diagram">


## Contact
For further queries please contact :- </br>

Aditya Mall (UTA MSEE)
</br>

email: aditya.mall1990@gmail.com.
