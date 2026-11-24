/*
 * BSL entry / UART-on-TEST bench test, MSP430G2553 (see README.md)
 *
 * Software UART 9600 8E1 on the BSL pins: RX P1.5 (tied to TEST), TX P1.1.
 * The hardware UART (P1.1 RX / P1.2 TX) is on the wrong pins for this test.
 *
 * Frames as in ../fw: <type><hex payload>*<crc8 hex>\n
 *   host        MSP
 *   V           V<ver>
 *   P<1-8 B>    p<inverted bytes>
 *   L<mm><pp>   l<mm><pp>    LED 00 off, 01 on, 02 blink, toggle every pp * 10 ms
 *   S           s<uptime s:4><cause:2><ok:4><parity:4><framing:4><bad frame:4><overrun:2>
 *   J           j<8 x 4>     probe counters (see probe()), then cleared
 *   -           B<ver><cause><cal>  after reset (cal 01: factory DCO calibration used)
 *   -           T<uptime s:4>  once per second
 *   anything else: e01 bad format/CRC, e02 unknown command
 *
 * P1.0 LED (blinky). P1.6 is not used: TDI. P2.1 = TEST line (independent view),
 * P2.0 = wire from P1.7 (TDO).
 */
#include <msp430.h>
#include <stdint.h>

#ifndef FW_VERSION
#define FW_VERSION 1
#endif
#ifndef BLINK
#define BLINK 50            /* x 10 ms per toggle */
#endif

#define LED     BIT0
#define TXP     BIT1
#define RXP     BIT5
#define TDOP    BIT7
#define P2_TDO  BIT0
#define P2_TEST BIT1

/* BSLSKEY (0xFFDE) = 0: a wrong BSL password does not erase the flash */
__attribute__((section(".bslsignature"), used)) const uint16_t bslskey = 0x0000;

static unsigned bitt;                   /* SMCLK ticks per bit */
static uint8_t cause, cal;

/* ---- UART TX: TA0.0 output unit, CCR0 ---- */

#define TXN 64
static volatile uint8_t txbuf[TXN], txh, txt;
static unsigned txsh;
static volatile uint8_t txbits;

static uint8_t parity(uint8_t b)
{
    b ^= b >> 4;
    b ^= b >> 2;
    b ^= b >> 1;
    return b & 1;
}

__attribute__((interrupt(TIMER0_A0_VECTOR))) void ta0_ccr0(void)
{
    TA0CCR0 += bitt;
    if (!txbits) {
        if (txh == txt) {
            TA0CCTL0 = OUTMOD_0 | OUT;      /* idle high, interrupt off */
            return;
        }
        uint8_t b = txbuf[txt];
        txt = (txt + 1) & (TXN - 1);
        /* LSB first: start, 8 data, even parity, stop */
        txsh = ((unsigned)b << 1) | ((unsigned)parity(b) << 9) | (1u << 10);
        txbits = 11;
    }
    TA0CCTL0 = ((txsh & 1) ? OUTMOD_1 : OUTMOD_5) | CCIE;
    txsh >>= 1;
    txbits--;
}

static void putch(uint8_t c)
{
    uint8_t n = (txh + 1) & (TXN - 1);
    while (n == txt)
        ;
    txbuf[txh] = c;
    __disable_interrupt();
    txh = n;
    if (!(TA0CCTL0 & CCIE)) {
        txbits = 0;
        TA0CCR0 = TA0R + bitt;
        TA0CCTL0 = OUTMOD_1 | CCIE;         /* one stop-level bit first */
    }
    __enable_interrupt();
}

/* ---- UART RX: start edge on P1.5, samples from CCR1 ---- */

#define RXN 32
static volatile uint8_t rxbuf[RXN], rxh, rxt, e_ovr;
static volatile unsigned n_ok, e_par, e_frm;
static unsigned rxsh;
static uint8_t rxbits;

__attribute__((interrupt(PORT1_VECTOR))) void port1(void)
{
    unsigned now = TA0R;
    P1IFG &= ~RXP;
    if (P1IN & RXP)
        return;                             /* glitch */
    P1IE &= ~RXP;
    rxbits = 0;
    rxsh = 0;
    TA0CCR1 = now + bitt + bitt / 2 - 40;   /* middle of bit 0, minus ISR latency */
    TA0CCTL1 = CCIE;
}

__attribute__((interrupt(TIMER0_A1_VECTOR))) void ta0_ccr1(void)
{
    uint8_t in = P1IN & RXP;
    TA0CCTL1 &= ~CCIFG;
    TA0CCR1 += bitt;
    if (rxbits < 9) {
        if (in)
            rxsh |= 1u << rxbits;
        rxbits++;
        return;
    }
    TA0CCTL1 = 0;
    P1IFG &= ~RXP;
    P1IE |= RXP;
    if (!in) {
        e_frm++;                            /* break or noise */
    } else if (parity((uint8_t)rxsh) != (rxsh >> 8)) {
        e_par++;
    } else {
        uint8_t n = (rxh + 1) & (RXN - 1);
        n_ok++;
        if (n == rxt) {
            e_ovr++;
        } else {
            rxbuf[rxh] = (uint8_t)rxsh;
            rxh = n;
        }
    }
}

/* ---- 10 ms tick: TA1, SMCLK/8; 96 bit times = 10 ms at 9600 baud ---- */

static volatile uint8_t led_mode = 2, led_per = BLINK, sec_flag;
static volatile unsigned uptime;

__attribute__((interrupt(TIMER1_A0_VECTOR))) void ta1_ccr0(void)
{
    static uint8_t t, l;
    if (++t >= 100) {
        t = 0;
        uptime++;
        sec_flag = 1;
    }
    if (led_mode == 2) {
        if (++l >= led_per) {
            l = 0;
            P1OUT ^= LED;
        }
    } else if (led_mode) {
        P1OUT |= LED;
    } else {
        P1OUT &= ~LED;
    }
}

/* ---- frames ---- */

static uint8_t crc;

static uint8_t crc8(uint8_t c, uint8_t b)
{
    uint8_t i;
    for (i = 8; i; i--) {
        uint8_t mix = (c ^ b) & 1;
        c >>= 1;
        if (mix)
            c ^= 0x8C;
        b >>= 1;
    }
    return c;
}

static void fputch(uint8_t c)
{
    crc = crc8(crc, c);
    putch(c);
}

static const char hexd[] = "0123456789ABCDEF";

static void hex2(uint8_t v)
{
    fputch(hexd[v >> 4]);
    fputch(hexd[v & 15]);
}

static void hex4(unsigned v)
{
    hex2(v >> 8);
    hex2((uint8_t)v);
}

static void begin(uint8_t type)
{
    crc = 0;
    fputch(type);
}

static void end(void)
{
    uint8_t c = crc;
    putch('*');
    putch(hexd[c >> 4]);
    putch(hexd[c & 15]);
    putch('\n');
}

static void error(uint8_t code)
{
    begin('e');
    hex2(code);
    end();
}

/* ---- probe: do JTAG-shared pins still work while TEST is high? ----
 * [0] P1.5 != P2.1, TEST high  [1] samples  [2] P1.5 != P2.1, TEST low  [3] samples
 * [4] P1.7 read back wrong, TEST high  [5] samples  [6] wrong, TEST low  [7] samples */
static unsigned pc[8];

static void probe(void)
{
    uint8_t a, b, p1, i;
    P1OUT ^= TDOP;
    __delay_cycles(40);
    a = P2IN;
    p1 = P1IN;
    __delay_cycles(4);
    b = P2IN;
    if ((a ^ b) & P2_TEST)
        return;                             /* edge during the sample */
    i = (a & P2_TEST) ? 0 : 2;
    if (pc[i + 1] == 0xFFFF)
        return;
    pc[i + 1]++;
    if (!(p1 & RXP) != !(a & P2_TEST))
        pc[i]++;
    i += 4;
    pc[i + 1]++;
    if (!(P1OUT & TDOP) != !(b & P2_TDO))
        pc[i]++;
}

/* ---- commands ---- */

#define LINEN 24
static uint8_t line[LINEN], len;
static unsigned e_bad;

static int unhex(uint8_t c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static void execute(void)
{
    uint8_t d[8], n, i, c = 0;
    /* <type><hex>*<crc> */
    if (len < 4 || line[len - 3] != '*')
        goto bad;
    for (i = 0; i < len - 3; i++)
        c = crc8(c, line[i]);
    {
        int hi = unhex(line[len - 2]), lo = unhex(line[len - 1]);
        if (hi < 0 || lo < 0 || c != ((hi << 4) | lo))
            goto bad;
    }
    n = len - 4;
    if ((n & 1) || n > 16)
        goto bad;
    n >>= 1;
    for (i = 0; i < n; i++) {
        int hi = unhex(line[1 + 2 * i]), lo = unhex(line[2 + 2 * i]);
        if (hi < 0 || lo < 0)
            goto bad;
        d[i] = (hi << 4) | lo;
    }
    switch (line[0]) {
    case 'V':
        begin('V');
        hex2(FW_VERSION);
        break;
    case 'P':
        begin('p');
        for (i = 0; i < n; i++)
            hex2(~d[i]);
        break;
    case 'L':
        if (n != 2 || d[0] > 2 || !d[1])
            goto bad;
        led_mode = d[0];
        led_per = d[1];
        begin('l');
        hex2(d[0]);
        hex2(d[1]);
        break;
    case 'S':
        begin('s');
        hex4(uptime);
        hex2(cause);
        hex4(n_ok);
        hex4(e_par);
        hex4(e_frm);
        hex4(e_bad);
        hex2(e_ovr);
        break;
    case 'J':
        begin('j');
        for (i = 0; i < 8; i++) {
            hex4(pc[i]);
            pc[i] = 0;
        }
        break;
    default:
        error(2);
        return;
    }
    end();
    return;
bad:
    e_bad++;
    error(1);
}

/* ---- start-up ---- */

/* No factory calibration: measure the low pulse of 0x80 (start + 7 zero bits = 8 bits).
 * LED flickers until two pulses in a row agree within ~3 %. */
static unsigned autobaud(void)
{
    unsigned prev = 0;
    for (;;) {
        unsigned t0, w;
        while (!(P1IN & RXP))
            ;
        while (P1IN & RXP) {
            if (TA0CTL & TAIFG) {
                static uint8_t k;
                TA0CTL &= ~TAIFG;
                if (++k >= 6) {
                    k = 0;
                    P1OUT ^= LED;
                }
            }
        }
        t0 = TA0R;
        TA0CTL &= ~TAIFG;
        while (!(P1IN & RXP))
            ;
        w = TA0R - t0;
        if (w > 2000 && w < 16000) {
            unsigned diff = (w > prev) ? w - prev : prev - w;
            if (diff < (w >> 5))
                return (w + 4) >> 3;
            prev = w;
        } else {
            prev = 0;
        }
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    cause = IFG1 & (WDTIFG | PORIFG | RSTIFG | NMIIFG | OFIFG);
    IFG1 &= ~(WDTIFG | PORIFG | RSTIFG | NMIIFG);

    P1OUT = TXP;
    P1SEL = TXP;                            /* P1.1 = TA0.0 output */
    P1SEL2 = 0;
    P1DIR = LED | TXP | TDOP;
    P1REN = BIT2 | BIT3 | BIT4 | BIT6;      /* unused inputs: pull-down ... */
    P1OUT |= BIT4;                          /* ... TCK pull-up */
    P2OUT = 0;
    P2SEL = P2SEL2 = 0;
    P2DIR = 0;
    P2REN = (uint8_t)~(P2_TEST | P2_TDO);

    cal = (CALBC1_8MHZ != 0xFF && CALDCO_8MHZ != 0xFF);
    DCOCTL = 0;
    if (cal) {
        BCSCTL1 = CALBC1_8MHZ;
        DCOCTL = CALDCO_8MHZ;
    } else {
        BCSCTL1 = XT2OFF | 13;              /* ~8 MHz, uncalibrated */
        DCOCTL = 0x60;
    }
    BCSCTL2 = 0;

    TA0CCTL0 = OUTMOD_0 | OUT;
    TA0CTL = TASSEL_2 | MC_2 | TACLR;
    bitt = cal ? 833 : autobaud();

    TA1CCR0 = 12 * bitt - 1;
    TA1CCTL0 = CCIE;
    TA1CTL = TASSEL_2 | ID_3 | MC_1 | TACLR;

    P1IES |= RXP;
    P1IFG = 0;
    P1IE = RXP;
    __enable_interrupt();

    begin('B');
    hex2(FW_VERSION);
    hex2(cause);
    hex2(cal);
    end();

    for (;;) {
        if (sec_flag) {
            sec_flag = 0;
            begin('T');
            hex4(uptime);
            end();
        }
        while (rxt != rxh) {
            uint8_t c = rxbuf[rxt];
            rxt = (rxt + 1) & (RXN - 1);
            if (c == '\n') {
                if (len)
                    execute();
                len = 0;
            } else if (c < ' ' || c >= 0x7F) {
                len = 0;                    /* sync bytes, noise */
            } else if (len < LINEN) {
                line[len++] = c;
            }
        }
        probe();
    }
}
