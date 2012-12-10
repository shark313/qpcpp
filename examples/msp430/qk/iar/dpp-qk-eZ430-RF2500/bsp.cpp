//////////////////////////////////////////////////////////////////////////////
// Product: BSP for DPP application on eZ430-RF2500, IAR compiler
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Oct 09, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

#define BSP_MCK       8000000
#define BSP_SMCLK     8000000
#define BSP_ACLK      12000

#define LED0_on()     (P1OUT |= (uint8_t)BIT0)
#define LED0_off()    (P1OUT &= (uint8_t)~BIT0)

#define LED1_on()     (P1OUT |= (uint8_t)BIT1)
#define LED1_off()    (P1OUT &= (uint8_t)~BIT1)

#ifdef Q_SPY
    QSTimeCtr QS_tickTime_;
    static uint8_t const l_timerA_ISR = 0;

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QS_USER
    };
#endif

//............................................................................
#pragma vector = TIMERA0_VECTOR
__interrupt void timerA_ISR(void) {
    QK_ISR_ENTRY();                        // inform QK about entering the ISR

#ifdef NDEBUG
    __low_power_mode_off_on_exit();
#endif

#ifdef Q_SPY
    TACTL &= ~TAIFG;                       // clear the interrupt pending flag
    QS_tickTime_ +=
       (((BSP_SMCLK / 8) + BSP_TICKS_PER_SEC/2) / BSP_TICKS_PER_SEC) + 1;
#endif

    QF::TICK(&l_timerA_ISR);

    QK_ISR_EXIT();                          // inform QK about exiting the ISR
}
//............................................................................
void BSP_init(void) {
    WDTCTL   = (WDTPW | WDTHOLD);                                  // Stop WDT

       // configure the Basic Clock Module */
    DCOCTL   = CALDCO_8MHZ;                                 // Set DCO to 8MHz
    BCSCTL1  = CALBC1_8MHZ;

    TACTL    = (ID_3 | TASSEL_2 | MC_1);          // SMCLK, /8 divider, upmode
    TACCR0   = (((BSP_SMCLK / 8) + BSP_TICKS_PER_SEC/2) / BSP_TICKS_PER_SEC);

    P1DIR   |= (BIT0 | BIT1);                  // P1.0 and P1.1 outputs (LEDs)

    if (QS_INIT((void *)0) == 0) {       // initialize the QS software tracing
        Q_ERROR();
    }
}
//............................................................................
void QF::onStartup(void) {
    TACCTL0 = CCIE;                          // Timer_A CCR0 interrupt enabled
    __enable_interrupt();            // make sure that interrupts are unlocked
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {
#ifdef Q_SPY
    if (((IFG2 & UCA0TXIFG)) != 0) {

        QF_INT_DISABLE();
        uint16_t b = QS::getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {
            UCA0TXBUF = (uint8_t)b;            // stick the byte to the TX BUF
        }
    }
#elif defined NDEBUG
    __low_power_mode_1();               // Enter LPM1; also UNLOCKS interrupts
#endif
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    (void)file;                                      // avoid compiler warning
    (void)line;                                      // avoid compiler warning
    QF_INT_DISABLE();                // make sure that interrupts are disabled
    for (;;) {
    }
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    if (n == 0) {                                              // for Philo[0]
        if (stat[0] == 'e') {                          // ON when Philo eating
            LED0_on();
        }
        else {
            LED0_off();
        }
    }
    else if (n == 1) {                                         // for Philo[1]
        if (stat[0] == 'e') {                          // ON when Philo eating
            LED1_on();
        }
        else {
            LED1_off();
        }
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}

//----------------------------------------------------------------------------
#ifdef Q_SPY
//............................................................................
uint8_t QS::onStartup(void const *arg) {
    static uint8_t qsBuf[256];                       // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

       // configure USART0 */
    P3SEL   |= BIT4;                                      // P3.4 = USART0 TXD
    UCA0CTL1 = UCSSEL_2;                                              // SMCLK
    UCA0BR0  = 52;                                           // 9600 from 8MHz
    UCA0BR1  = UCBRS0 | UCOS16;
    UCA0MCTL = UCBRS_2;
    UCA0CTL1 &= ~UCSWRST;                     // initialize USCI state machine

                                                    // setup the QS filters...
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_dummyD);

    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
    QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
//    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return (uint8_t)1;                                       // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {              // invoked with interrupts locked
    if ((TACTL & TAIFG) == 0) {                      // interrupt not pending?
        return QS_tickTime_ + TAR;
    }
    else {         // the rollover occured, but the timerA_ISR did not run yet
        return QS_tickTime_
           + (((BSP_SMCLK / 8) + BSP_TICKS_PER_SEC/2) / BSP_TICKS_PER_SEC) + 1
           + TAR;
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    while ((b = getByte()) != QS_EOD) {       // next QS trace byte available?
        while ((IFG2 & UCA0TXIFG) == 0) {                     // TX not ready?
        }
        UCA0TXBUF = (uint8_t)b;                // stick the byte to the TX BUF
    }
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------
