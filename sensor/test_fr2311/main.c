/*
 * BSL entry / UART-on-TEST bench test, MSP430FR2311 LaunchPad (see README.md)
 *
 * Hardware UART UCA0 9600 8E1 on the BSL pins: RX P1.6 (TDI, tied to TEST), TX P1.7 (TDO).
 * Clock, UART and watchdog are set up as in ../fw/main.c.
 *
 * Frames as in ../fw: <type><hex payload>*<crc8 hex>\n
 *   host        MSP
 *   V           V<ver>
 *   P<1-8 B>    p<inverted bytes>
 *   L<mm><pp>   l<mm><pp>    red LED 00 off, 01 on, 02 blink, toggle every pp * 10 ms
 *   S           s<uptime s:4><cause:2><ok:4><parity:4><framing:4><bad frame:4><overrun:2><echo:4>
 *   W           w            then hangs until the watchdog resets it (~1 s)
 *   -           B<ver><cause><cause2>  after reset, first two SYSRSTIV values
 *   -           T<uptime s:4>  once per second
 *   anything else: e01 bad format/CRC, e02 unknown command
 *
 * echo: one wire only, own bytes that came back wrong or not at all.
 * P1.0 red LED (blink), P2.0 green LED (toggles with every good command).
 */
#include <msp430.h>
#include <stdint.h>

#ifndef FW_VERSION
#define FW_VERSION 1
#endif
#ifndef BLINK
#define BLINK 50            /* x 10 ms per toggle */
#endif

#ifndef ONEWIRE             /* 1: TX (diode) and RX share one line, half duplex */
#define ONEWIRE 0
#endif

#define LED     BIT0        /* P1 */
#define RXP     BIT6        /* P1 */
#define TXP     BIT7        /* P1 */
#define LED2    BIT0        /* P2 */

/* watchdog: ACLK = REFO 32 kHz / 2^15 = 1 s */
#define WDT_KICK()  (WDTCTL = WDTPW | WDTSSEL__ACLK | WDTCNTCL | WDTIS__32K)

static uint8_t cause, cause2;

/* ---- UART: ring buffers, interrupts ---- */

#define TXN 64
#define RXN 32
static volatile uint8_t txbuf[TXN], txh, txt;
static volatile uint8_t rxbuf[RXN], rxh, rxt, e_ovr;
static volatile unsigned n_ok, e_par, e_frm, e_echo;

#if ONEWIRE
static volatile uint8_t echo[4], eh, et;    /* own bytes on their way, at most 2 */
#endif

__attribute__((interrupt(EUSCI_A0_VECTOR))) void uca0(void)
{
    if (UCA0IFG & UCRXIFG) {
        unsigned st = UCA0STATW;            /* before RXBUF: reading that clears the flags */
        uint8_t c = UCA0RXBUF;
#if ONEWIRE
        if (eh != et) {                     /* own transmission */
            if ((st & UCRXERR) || c != echo[et & 3])
                e_echo++;
            et++;
            return;
        }
#endif
        if (st & UCOE)
            e_ovr++;
        if (st & (UCFE | UCBRK)) {
            e_frm++;
        } else if (st & UCPE) {
            e_par++;
        } else {
            uint8_t n = (rxh + 1) & (RXN - 1);
            n_ok++;
            if (n == rxt) {
                e_ovr++;
            } else {
                rxbuf[rxh] = c;
                rxh = n;
            }
        }
        return;
    }
    if ((UCA0IE & UCTXIE) && (UCA0IFG & UCTXIFG)) {
        uint8_t c;
        if (txh == txt) {
            UCA0IE &= ~UCTXIE;
            return;
        }
        c = txbuf[txt];
        txt = (txt + 1) & (TXN - 1);
#if ONEWIRE
        echo[eh++ & 3] = c;
#endif
        UCA0TXBUF = c;
    }
}

static void putch(uint8_t c)
{
    uint8_t n = (txh + 1) & (TXN - 1);
    while (n == txt)
        ;
    txbuf[txh] = c;
    __disable_interrupt();
    txh = n;
    UCA0IE |= UCTXIE;
    __enable_interrupt();
}

#if ONEWIRE
static uint8_t line_busy(void)              /* TX queued, or UART sending/receiving */
{
    return txh != txt || (UCA0STATW & UCBUSY) || (UCA0IFG & UCRXIFG);
}
#endif

/* ---- 10 ms tick: TB0, SMCLK/8 ---- */

static volatile uint8_t led_mode = 2, led_per = BLINK, sec_flag;
static volatile unsigned uptime, ticks;

__attribute__((interrupt(TIMER0_B0_VECTOR))) void tb0_ccr0(void)
{
    static uint8_t t, l;
    ticks++;
#if ONEWIRE
    if (eh != et && !line_busy()) {         /* own bytes that never came back */
        e_echo += (uint8_t)(eh - et);
        et = eh;
    }
#endif
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

#if ONEWIRE
/* Line free: own TX done, nothing being received, high for 2-3 characters.
 * Gives up after 100 ms (line stuck low). */
static void wait_idle(void)
{
    unsigned t0 = ticks, n = 0;
    while (n < 25 && (unsigned)(ticks - t0) < 10) {
        __delay_cycles(800);                /* 0.1 ms */
        n = (line_busy() || !(P1IN & RXP)) ? 0 : n + 1;
    }
}
#endif

static void begin(uint8_t type)
{
#if ONEWIRE
    wait_idle();
#endif
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
    P2OUT ^= LED2;
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
        hex4(e_echo);
        break;
    case 'W':
        begin('w');
        end();
        for (;;)                            /* no kick: watchdog reset */
            ;
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

/* ---- main ---- */

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    cause = SYSRSTIV;                       /* a read clears the highest pending one */
    cause2 = SYSRSTIV;
    while (SYSRSTIV)
        ;

    /* unused pins output low as in ../fw (P1.4/P1.5 = TCK/TMS too); P1.1 = button S1, pull-up */
    P1OUT = BIT1;
    P1REN = BIT1;
    P1DIR = (uint8_t)~(BIT1 | RXP);
    P1SEL0 = RXP | TXP;                     /* P1SEL = 01: UCA0 */
    P2OUT = 0;
    P2DIR = 0xFF;
    PM5CTL0 &= ~LOCKLPM5;

    /* DCO 8 MHz, FLL locked to REFO */
    __bis_SR_register(SCG0);
    CSCTL3 = SELREF__REFOCLK;
    CSCTL0 = 0;
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_3;
    CSCTL2 = FLLD_0 + 243;                  /* 32768 * 244 = 7.995 MHz */
    __delay_cycles(3);
    __bic_SR_register(SCG0);
    while (CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1))
        ;
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;
    WDT_KICK();

    /* 9600 8E1 from SMCLK 8 MHz: UCOS16, BR 52, BRF 1, BRS 0x49; keep bad characters and
     * breaks (counted) */
    UCA0CTLW0 = UCSWRST | UCSSEL__SMCLK | UCPEN | UCPAR | UCRXEIE | UCBRKIE;
    UCA0BRW = 52;
    UCA0MCTLW = (0x49 << 8) | UCBRF_1 | UCOS16;
    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE = UCRXIE;

    TB0CCR0 = 9999;                         /* 8 MHz / 8 / 10000 = 100 Hz */
    TB0CCTL0 = CCIE;
    TB0CTL = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR;
    __enable_interrupt();

    begin('B');
    hex2(FW_VERSION);
    hex2(cause);
    hex2(cause2);
    end();

    for (;;) {
        WDT_KICK();
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
                len = 0;                    /* BSL packets, noise */
            } else if (len < LINEN) {
                line[len++] = c;
            }
        }
    }
}
