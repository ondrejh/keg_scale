/*
 * keg20 sensor bridge - MSP430FR2100 / FR2111 (see ../README.md)
 *
 * Reads HX711 (channel A, gain 128) and DS18B20, streams ASCII frames over
 * UCA0 (P1.7 TXD), 9600 8E1. No requests, no interrupts.
 *
 *   <type><hex payload>*<crc8 hex>\n
 *   V<ver>      version, once after reset
 *   H<6 hex>    HX711 raw 24-bit two's complement, MSB first (~10/s)
 *   h<err>      HX711 error   01 not ready 500 ms, 02 DOUT low after 25th pulse
 *   T<4 hex>    DS18B20 raw int16, 1/16 C (~1/s)
 *   t<err>      DS18B20 error 01 bus stuck low, 02 no presence, 03 scratchpad CRC
 *
 * CRC: Dallas/Maxim CRC-8 (reflected poly 0x8C, init 0) over the bytes before '*'.
 *
 * Pins: P1.0 HX711 DOUT (in), P1.1 HX711 SCK (out), P1.2 DS18B20 DQ (open drain,
 * external 4k7 pull-up), P1.6 UCA0RXD (unused, BSL), P1.7 UCA0TXD.
 *
 * Size: ~0.85 KB - does not fit the FR2000 (384 B code), use FR2100 (896 B) or FR2111.
 * Native 16-bit types and shared helpers keep it small.
 */
#include <msp430.h>
#include <stdint.h>

#define FW_VERSION  0x01

#define HX_DOUT     BIT0
#define HX_SCK      BIT1
#define DQ          BIT2

#define F_CPU       8000000UL
/* busy wait, 3 cycles per loop (dec 1 + jnz 2) + ~10 cycles call overhead */
#define US(x)       dly((unsigned)((x) * (F_CPU / 1000000UL) / 3))
#define NOINLINE    __attribute__((noinline))

#define TICK_US         10000   /* main loop tick (plus work time) */
#define HX_TIMEOUT      50      /* ticks without DOUT low -> h01 */
#define DS_PERIOD       100     /* ticks per DS18B20 cycle (~1 s) */
#define DS_READ_TICK    80      /* read 800 ms after convert (12 bit = 750 ms) */

/* watchdog: ACLK = REFO 32 kHz / 2^15 = 1 s */
#define WDT_KICK()  (WDTCTL = WDTPW | WDTSSEL__ACLK | WDTCNTCL | WDTIS__32K)

/* .noinit: no bss init code gets linked (crt_bss + memset); set in main */
static unsigned crc __attribute__((section(".noinit")));
static uint8_t buf[9] __attribute__((section(".noinit")));

static NOINLINE void dly(unsigned n)
{
    __asm__ volatile("1: dec %0\n\tjnz 1b" : "+r"(n));
}

static NOINLINE void crc_add(unsigned b)
{
    for (unsigned i = 8; i; i--) {
        unsigned mix = (crc ^ b) & 1;
        crc >>= 1;
        if (mix)
            crc ^= 0x8C;
        b >>= 1;
    }
}

/* ---- frame output ---- */

static NOINLINE void put(unsigned c)
{
    while (!(UCA0IFG & UCTXIFG))
        ;
    UCA0TXBUF = c;
    crc_add(c);
}

static NOINLINE void hex(unsigned b)
{
    for (unsigned i = 2; i; i--) {
        unsigned n = (b >> 4) & 0x0F;
        put(n < 10 ? '0' + n : 'A' - 10 + n);
        b <<= 4;
    }
}

/* <type><n bytes of buf in hex>*<crc>\n */
static NOINLINE void frame(unsigned type, unsigned n)
{
    crc = 0;
    put(type);
    for (uint8_t *p = buf; n; n--)
        hex(*p++);
    unsigned c = crc;
    put('*');
    hex(c);
    put('\n');
}

static void frame_err(unsigned type, unsigned err)
{
    buf[0] = err;
    frame(type, 1);
}

/* ---- HX711 ---- */

/* One SCK pulse, DOUT sampled while SCK is high. At 8 MHz the high phase is
 * ~1 us (>= 0.2 us, << 50 us power-down limit). */
static NOINLINE unsigned hx_clk(void)
{
    P1OUT |= HX_SCK;
    __no_operation();
    unsigned d = P1IN & HX_DOUT;
    P1OUT &= ~HX_SCK;
    return d;
}

/* Call when DOUT is low (conversion ready). */
static NOINLINE void hx_read(void)
{
    for (unsigned i = 0; i < 3; i++) {
        unsigned b = 0;
        for (unsigned j = 8; j; j--)
            b = (b << 1) | hx_clk();
        buf[i] = b;
    }
    hx_clk();       /* 25th pulse: next conversion channel A, gain 128; DOUT goes high */
    if (P1IN & HX_DOUT)
        frame('H', 3);
    else            /* nothing clocked out - unpowered HX711, the divider holds DOUT low */
        frame_err('h', 0x02);
}

/* ---- 1-wire (DQ open drain: P1OUT.DQ stays 0, drive low by P1DIR) ---- */

/* 0 = presence, 1 = bus stuck low, 2 = no presence */
static NOINLINE unsigned ow_reset(void)
{
    if (!(P1IN & DQ))
        return 1;
    P1DIR |= DQ;
    US(480);
    P1DIR &= ~DQ;
    US(70);
    unsigned p = P1IN & DQ;
    US(410);
    return p ? 2 : 0;
}

/* Write/read one byte LSB first; write 0xFF to read. Slot: low ~2 us, release
 * for a 1, sample at ~10 us (< 15 us), slot ~60 us, then recovery. */
static NOINLINE unsigned ow_byte(unsigned b)
{
    for (unsigned i = 8; i; i--) {
        P1DIR |= DQ;
        __no_operation();
        if (b & 1)
            P1DIR &= ~DQ;
        US(7);
        unsigned r = P1IN & DQ;
        US(50);
        P1DIR &= ~DQ;
        US(3);
        b >>= 1;
        if (r)
            b |= 0x80;
    }
    return b;
}

/* reset + skip ROM + command (single sensor on the bus); 0 = ok, else error */
static NOINLINE unsigned ds_cmd(unsigned cmd)
{
    unsigned e = ow_reset();
    if (!e) {
        ow_byte(0xCC);
        ow_byte(cmd);
    }
    return e;
}

static NOINLINE void ds_read(void)
{
    unsigned e = ds_cmd(0xBE);   /* read scratchpad */

    if (!e) {
        /* CRC over all 9 bytes including the CRC byte is 0 */
        crc = 0;
        for (unsigned i = 0; i < 9; i++)
            crc_add(buf[8 - i] = ow_byte(0xFF));    /* stored reversed: buf[7] MSB, buf[8] LSB */
        if (crc)
            e = 3;
    }
    if (e) {
        frame_err('t', e);
    } else {
        buf[0] = buf[7];        /* temperature MSB */
        buf[1] = buf[8];        /* temperature LSB */
        frame('T', 2);
    }
}

/* ---- init ---- */

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    /* unused pins: output low; DOUT, DQ, RXD inputs; P1.6/P1.7 = UCA0 (P1SEL = 01) */
    P1OUT = 0;
    P1DIR = (uint8_t)~(HX_DOUT | DQ | BIT6);
    P1SEL0 = BIT6 | BIT7;
    P2OUT = 0;
    P2DIR = 0xFF;
    PM5CTL0 &= ~LOCKLPM5;

    /* DCO 8 MHz, FLL locked to REFO (TI example msp430fr211x_CS_03) */
    __bis_SR_register(SCG0);
    CSCTL3 = SELREF__REFOCLK;
    CSCTL0 = 0;
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_3;
    CSCTL2 = FLLD_0 + 243;              /* 32768 * 244 = 7.995 MHz */
    __delay_cycles(3);
    __bic_SR_register(SCG0);
    while (CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1))
        ;
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;
    WDT_KICK();

    /* UART 9600 8E1 from SMCLK 8 MHz: UCOS16, BR 52, BRF 1, BRS 0x49 */
    UCA0CTLW0 = UCSWRST | UCSSEL__SMCLK | UCPEN | UCPAR;
    UCA0BRW = 52;
    UCA0MCTLW = (0x49 << 8) | UCBRF_1 | UCOS16;
    UCA0CTLW0 &= ~UCSWRST;

    /* HX711 reset: SCK high > 60 us = power down, low = power up */
    P1OUT |= HX_SCK;
    US(100);
    P1OUT &= ~HX_SCK;

    frame_err('V', FW_VERSION);

    unsigned hx_wait = 0, ds_tick = 0;

    for (;;) {
        WDT_KICK();

        if (!(P1IN & HX_DOUT)) {
            hx_read();
            hx_wait = 0;
        } else if (++hx_wait >= HX_TIMEOUT) {
            frame_err('h', 0x01);
            hx_wait = 0;
        }

        if (ds_tick == 0)
            ds_cmd(0x44);       /* convert T; errors are reported by the read */
        else if (ds_tick == DS_READ_TICK)
            ds_read();
        if (++ds_tick >= DS_PERIOD)
            ds_tick = 0;

        US(TICK_US);
    }
}
