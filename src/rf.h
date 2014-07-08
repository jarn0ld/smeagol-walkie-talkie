#ifndef _RF_H_
#define _RF_H_

#include <stdint.h>
#include <msp430.h>

#define RF_CHANNEL 24
#define PAN_ID 0x1337

#define RF_TX_ADDR 0x0000
#define RF_RX_ADDR 0x0001

#ifdef TX
#define NODE_ADDR RF_TX_ADDR
#else
#define NODE_ADDR RF_RX_ADDR
#endif

#define PROTOCOL_OVERHEAD 11

#define CC2420_SNOP             0x00
#define CC2420_SXOSCON          0x01
#define CC2420_STXCAL           0x02
#define CC2420_SRXON            0x03
#define CC2420_STXON            0x04
#define CC2420_STXONCCA         0x05
#define CC2420_SRFOFF           0x06
#define CC2420_SXOSCOFF         0x07
#define CC2420_SFLUSHRX         0x08
#define CC2420_SFLUSHTX         0x09
#define CC2420_SACK             0x0A
#define CC2420_SACKPEND         0x0B
#define CC2420_SRXDEC           0x0C
#define CC2420_STXENC           0x0D
#define CC2420_SAES             0x0E

#define CC2420_MAIN             0x10
#define CC2420_MDMCTRL0         0x11
#define CC2420_MDMCTRL1         0x12
#define CC2420_RSSI             0x13
#define CC2420_SYNCWORD         0x14
#define CC2420_TXCTRL           0x15
#define CC2420_RXCTRL0          0x16
#define CC2420_RXCTRL1          0x17
#define CC2420_FSCTRL           0x18
#define CC2420_SECCTRL0         0x19
#define CC2420_SECCTRL1         0x1A
#define CC2420_BATTMON          0x1B
#define CC2420_IOCFG0           0x1C
#define CC2420_IOCFG1           0x1D
#define CC2420_MANFIDL          0x1E
#define CC2420_MANFIDH          0x1F
#define CC2420_FSMTC            0x20
#define CC2420_MANAND           0x21
#define CC2420_MANOR            0x22
#define CC2420_AGCCTRL          0x23
#define CC2420_AGCTST0          0x24
#define CC2420_AGCTST1          0x25
#define CC2420_AGCTST2          0x26
#define CC2420_FSTST0           0x27
#define CC2420_FSTST1           0x28
#define CC2420_FSTST2           0x29
#define CC2420_FSTST3           0x2A
#define CC2420_RXBPFTST         0x2B
#define CC2420_FSMSTATE         0x2C
#define CC2420_ADCTST           0x2D
#define CC2420_DACTST           0x2E
#define CC2420_TOPTST           0x2F
#define CC2420_RESERVED         0x30

#define CC2420_TXFIFO           0x3E
#define CC2420_RXFIFO           0x3F

#define CC2420RAM_TXFIFO	0x000
#define CC2420RAM_RXFIFO	0x080
#define CC2420RAM_KEY0		0x100
#define CC2420RAM_RXNONCE	0x110
#define CC2420RAM_SABUF		0x120
#define CC2420RAM_KEY1		0x130
#define CC2420RAM_TXNONCE	0x140
#define CC2420RAM_CBCSTATE	0x150
#define CC2420RAM_IEEEADDR	0x160
#define CC2420RAM_PANID		0x168
#define CC2420RAM_SHORTADDR	0x16A

#define ENABLE_FIFOP_INT() do { P1IFG &= ~0x01; P1IES &= ~0x01; P1IE |= 0x01; } while(0)
#define DISABLE_FIFOP_INT() do { P1IE &= ~0x01; } while(0)
#define CLEAR_FIFOP_INT() do { P1IFG &= ~0x01; } while(0)

#define FIFOP_IS_1() (P1IN & 0x01)
#define FIFO_IS_1() (P1IN & 0x08)
#define SFD_IS_1() (P4IN & 0x02)
#define CCA_IS_1() (P4IN & 0x10)

extern void init_rf(uint8_t channel, uint16_t pan_id, uint16_t addr, volatile uint16_t *flag);
extern volatile int16_t* get_samples_rf(void);
extern void send_rf_data(uint16_t addr, uint8_t *data, uint16_t size);

#endif
