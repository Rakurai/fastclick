/*
 * disorder.{cc,hh} -- reorder packets randomly
 * Tom Barbette
 *
 * Copyright (c) 2015 University of Liege
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "disorder.hh"
#include <click/glue.hh>
#include <click/error.hh>
#include <click/args.hh>
CLICK_DECLS

Disorder::Disorder()
{
  _count = 0;
  _mindelay = 0;
  _maxdelay = 16;
  _timeout = 50;
}

Disorder::~Disorder()
{
}

int
Disorder::configure(Vector<String> &conf, ErrorHandler *errh)
{

    if (Args(conf, this, errh)
        .read_p("MINDELAY", _mindelay)
        .read_p("MAXDELAY", _maxdelay)
        .read_p("TIMEOUT", _timeout)
        .complete() < 0)
        return -1;
    _delay.resize(_maxdelay);
    for (int i = 0; i < _maxdelay; i++) {
        _delay[i].time = 0;
    }

    _timer = new Timer(this);
    _timer->initialize(this,true);
}

void Disorder::cleanup() {
    for (int i = 0; i < _maxdelay; i++) {
        if (_delay[i].time != 0)
           _delay[i].p->kill();
    }

}

void
Disorder::run_timer(Timer *timer)
{
    push(0,NULL);
}


void
Disorder::push(int,Packet *p_in)
{
    _timer->unschedule();
    _count++;


    int empty_index = -1;

    //Send delayed packets
    for (int i = 0; i < _maxdelay; i++) {
        if (_delay[i].time == _count) {
            output(0).push(this->_delay[i].p);
            this->_delay[i].time = 0;
        } else if (empty_index == -1 && _delay[i].time == 0) {
            empty_index = i;
        }
    }

    _timer->schedule_after(Timestamp::make_usec(_timeout));

    if (p_in == NULL)
        return;

    int delay = rand()%(_maxdelay - _mindelay + 1) + _mindelay;

    if (delay == 0) {
        output(0).push(p_in);
    } else {
        if (empty_index == -1) {
            click_chatter("BUG in %s ! No place for packet !", name().c_str());
            output(0).push(p_in);
        } else {
            this->_delay[empty_index].p = p_in;
            this->_delay[empty_index].time = _count + delay;
        }
    }
}

CLICK_ENDDECLS
EXPORT_ELEMENT(Disorder)
