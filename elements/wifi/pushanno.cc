/*
 * pushanno.{cc,hh} -- filters packets out with phy errors
 * John Bicket
 *
 * Copyright (c) 2004 Massachussrcrs Institute of Technology
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
#include <click/error.hh>
#include <click/args.hh>
#include <click/packet_anno.hh>
#include <click/straccum.hh>
#include "pushanno.hh"

CLICK_DECLS


PushAnno::PushAnno()
{
}

PushAnno::~PushAnno()
{
}

Packet *
PushAnno::simple_action(Packet *p_in)
{

  if (!p_in) {
    return 0;
  }

  p_in = p_in->push(Packet::anno_size);
  if (p_in) {
    memcpy((void *)p_in->data(), p_in->anno(), Packet::anno_size);
  }
  return p_in;
}

CLICK_ENDDECLS


EXPORT_ELEMENT(PushAnno)



