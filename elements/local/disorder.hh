#ifndef CLICK_Disorder_HH
#define CLICK_Disorder_HH
#include <click/element.hh>
#include <click/glue.hh>
#include <click/vector.hh>
#include <click/timer.hh>
CLICK_DECLS

/*
 * =c
 * Disorder([MINDELAY] [MAXDELAY] [TIMEOUT])
 * =s
 * re-order packets
 *
 * =d
 *
 * MINDELAY
 *
 * Minimum number of packets to let pass before transmitting a packet. Default 0.
 *
 *
 * MAXDELAY
 *
 * Maximum number of packets to let pass before transmitting a packet. Default 16.
 *
 * TIMEOUT
 *
 * Time after which we simulate the passage of a packet, if not real packet passed
 *
 */

class Disorder : public Element {
public:
  Disorder() CLICK_COLD;
  ~Disorder() CLICK_COLD;

  const char *class_name() const		{ return "Disorder"; }
  const char *port_count() const		{ return PORTS_1_1; }
  int configure(Vector<String> &conf, ErrorHandler *errh) CLICK_COLD;
  void cleanup();
  void run_timer(Timer *timer);
  void push(int,Packet *);

private:
  int _mindelay;
  int _maxdelay;
  int _timeout;

  Timer* _timer;

  int _count;
  struct DelayPacket{
      Packet* p;
      int time;
  };
  Vector<DelayPacket> _delay;
};

CLICK_ENDDECLS
#endif
