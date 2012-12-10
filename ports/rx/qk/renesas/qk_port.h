//////////////////////////////////////////////////////////////////////////////
// Product: QK/C++  port to Renesas RX, Renesas RX Compiler
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Oct 18, 2012
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
#ifndef qk_port_h
#define qk_port_h

                                                // QK interrupt entry and exit
#define QK_ISR_ENTRY() do { \
    ++QK_intNest_; \
    QF_INT_ENABLE(); \
} while (0)

#define QK_ISR_EXIT()  do { \
    QF_INT_DISABLE(); \
    --QK_intNest_; \
    if (QK_intNest_ == (uint8_t)0) { \
        uint8_t p = QK_schedPrio_(); \
        if (p != (uint8_t)0) { \
            set_psw((unsigned long)0); \
            QK_schedExt_(p); \
        } \
    } \
} while (0)

                     // saving and restoring the extended QK context (RX FPSW)
#define QK_EXT_SAVE(act_) \
    ((act_)->m_thread = (void *)get_fpsw())

#define QK_EXT_RESTORE(act_) \
    set_fpsw((unsigned long)(act_)->m_thread)

#include <machine.h>         // for intrinsics set_psw()/get_fpsw()/set_fpsw()
#include "qk.h"                    // QK platform-independent public interface

#endif                                                            // qk_port_h
