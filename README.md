# Light-weight-wireless-protocol

#### (Updating docs) Light weight wireless messaging protocol for low power sensor networks based on slotted aloha access method.

**_Please take Prior Persmisson from Dr Jason.Losh and respective ownwers if you wish to use it in your Academic Project at The University of Texas at Arlington._**

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

<img src="https://github.com/adimalla/Light-weight-wireless-protocol/blob/master/docs/images/Selection_336.jpg" width="800" height="800" title="CLI">
<br/>



## Contact
