//////////////////////////////////////////////////////////////////////////////
// Model: dpp.qm
// File:  ./dpp.h
//
// This file has been generated automatically by QP Modeler (QM).
// DO NOT EDIT THIS FILE MANUALLY.
//
// Please visit www.state-machine.com/qm for more information.
//////////////////////////////////////////////////////////////////////////////
#ifndef dpp_h
#define dpp_h

using namespace QP;

enum DPPSignals {
   EAT_SIG = Q_USER_SIG,  // published by Table to let a philosopher eat
   DONE_SIG,              // published by Philosopher when done eating
   TERMINATE_SIG,         // published by BSP to terminate the application
   MAX_PUB_SIG,           // the last published signal

   HUNGRY_SIG,            // posted direclty to Table from hungry Philosopher
   MAX_SIG                // the last signal
};

// @(/1/0) ...................................................................
class TableEvt : public QP::QEvt {
public:
    uint8_t philoNum;
};

enum { N_PHILO = 5 };     // number of philosophers

extern QActive * const AO_Philo[N_PHILO];
extern QActive * const AO_Table;

#endif // dpp_h
