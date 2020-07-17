# UART Driver for STM32F4

The microcontroller used is STM32F427ZITx.
This contains receiving data with UART and DMA with application size being 1 byte.


## Abbreviations
- DMA: Direct Memory Access Controller in STM32
- UART: Universal Asynchronous Receiver Transmitter
- USART: Universal Synchronous Asynchronous Receiver Transmitter
- TX: Transmit
- RX: Receive

## General about UART

UART in STM32 allows customers to configure it using different transmit(`TX`)/receive(`RX`) modes:
-   DMA is used to transfer data from USART RX data register to user memory on hardware level. No application interaction is needed at this point except processing received data by application once necessary
-   P: Transfer from USART peripheral to memory is done on hardware level without CPU interaction
-   P: Can work very easily with operating systems
-   P: Optimized for highest baudrates  `> 1Mbps`  and low-power applications
-   P: In case of big bursts of data, increasing data buffer size can improve functionality
-   C: Number of bytes to transfer must be known in advance by DMA hardware
-   C: If communication fails, DMA may not notify application about all bytes transferred

## General about DMA

DMA in STM32 can be configured in  `normal`  or  `circular`  mode. For each mode,  _DMA_  requires number of  _elements_  to transfer before its events (half-transfer complete, transfer complete) are triggered.

-   _Normal mode_: DMA starts with data transfer, once it transfers all elements, it stops and sets enable bit to  `0`.
    -   Application is using this mode when transmitting data
-   _Circular mode_: DMA starts with transfer, once it transfers all elements (as written in corresponding length register), it starts from beginning of memory and transfers more
    -   Applicaton is using this mode when receiving data

While transfer is active,  `2`  (among others) interrupts may be triggered:

-   _Half-Transfer complete  `HT`_: Triggers when DMA transfers half count of elements
-   _Transfer-Complete  `TC`_: Triggers when DMA transfers all elements

> When DMA operates in  _circular_  mode, these interrupts are triggered periodically

> Number of elements to transfer by DMA hardware must be written to relevant DMA register before start of transfer

## Combine UART + DMA for Data Reception


Now it is time to understand which features to use to receive data with UART and DMA to offload CPU. As for the sake of this example, we use memory buffer array of  `20`  bytes. DMA will transfer data received from UART to this buffer.
-   Application writes  `20`  to relevant DMA register for data length
-   Application writes memory & peripheral addresses to relevant DMA registers
-   Application sets DMA direction to  _peripheral-to-memory_  mode
-   Application enables DMA & UART in reception mode. Receive can not start & DMA will wait UART to receive first character and transmit it to array. This is done for every received byte.

## Application
- The Python program will send one number (between 0 and 100) to the MCU via FTDI chip.
- CRC16 CCITT check occurs whenever data is sent to the MCU. If successfull, `Received` is printed.
- This number is then pushed into the memory buffer, which is a FIFO queue implemented using DMA.
-  The number is dequeued and checked for being a multiple of 4 and/or 7 and the following output is printed.
   ```
|  Multiple    | Output		          |  
|:------------:|:------------------:|
| 	  4  	     |     Rightbot		    |
|  	  7 	     |  		Labs 	        |
|  4 and 7 	   |  Rightbot Pvt Ltd  |
| Not 4 or 7   | 	_inputnumber_	    |
- The LED lights up for 10s with its duty cycle equal to the number sent
- If another number is sent during the 10s, it gets enqueued to the Rx Buffer and will get dequeued after the LED stops glowing.
