smeagol-walkie-talkie
=====================

Welcome to Audio Group of SMEAGOL Project 2014 the Small, Embedded, Advanced and Generic Objects Laboratory at RWTH Aachen University. 

## Introduction ##

The goal of this project is to stream voice over a radio link. This includes voice recording, voice transmission and playback.
As radio nodes telosb´s needed to be used. This nodes are pretty power full battery powered devices including a micro-controller, a 2.4GHz radio transceiver and some sensors. 

## Hardware ##

Besides the two telosb´s some hardware was required to condition the microphone signal and to amplifie the output signal so that it can be used to drive earphones or a speaker.

### Microphone Amplifier ###
![alt text](/images/mic_amp/schematic.png "Schematic of Microphone Amplifier")
To generate electrical signals out out of sound waves a microphone is needed. For our purposes a electret microphone is right choice because of its small dimensions, its small price and
its good usability. However, the electrical signal generated by the microphone is ways to small for the telosb`s ADC. Therefore, the signal needs to be amplified to match up with the dynamic range
of the ADC. In order to do so we used an OpAmp as an amplifier in negative feedback configuration.  However before amplification the microphone needs to be biased through resistor R1. After amplification it is important to introduce a band limitation in order to satisfy Nyquist's theorem.
Therefore, a low pass is formed by R4 and C4 with a cut cuff below 4kHz.

#### Things to Notice ####
As this board circuit will be mounted directly beneath the transmitting node it is very important to be aware of the radio interference. Therefore, a proper ground plane is a must have, signal lines should be as short as possible and all supply pins should be closely decoupled.   


### Audio Amplifier ###
![alt text](/images/audio_amp/schematic.png "Schematic of Audio Amplifier")

## Software ##
The program reads the ADC input (10 kHz and 12 bit) and send it over the radio using the CC2420 (RF chip). Afterwards, the sent samples are outputted at the DAC on the receiver seide.

Data flow of the samples:

Sender: ADC->DMA->Buffer->CC2420

Receiver: CC2420->Buffer->DMA->DAC



The code consists of 5 main components:

adc.c and dac.c:

- handles the initialization of the ADC/DAC using a timer

- sampling rate: 10 kHz, 12 bit

dma.c:

- configures the DMA Controller to interact with the ADC/DAC.

- includes the interrupts that are triggered, when a sample buffer was completely filled or outputted

rf.c:

- handles the initialization of the SPI interface, which is required to communite with the CC2420

- initializes the CC2420 (configuring registers)
		 - handles the sending and receiving of packets

main.c:

- puts all components together
