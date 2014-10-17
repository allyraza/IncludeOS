#ifndef CLASS_IP_STACK_HPP
#define CLASS_IP_STACK_HPP


#include <net/class_ethernet.hpp>
#include <net/class_arp.hpp>
#include <net/class_ip4.hpp>
#include <net/class_ip6.hpp>
#include <net/class_icmp.hpp>
#include <net/class_udp.hpp>

class IP_stack {
  
  /** Physical layer output */
  delegate<int(uint8_t*,int)> _physical_out;
  
  Ethernet _eth;
  Arp _arp;
  IP4 _ip4;
  IP6 _ip6;
  ICMP _icmp;
  UDP _udp;
  
public:
  
  /** Physical layer input. */
  inline int physical_in(uint8_t* data,int len){
    return _eth.physical_in(data,len);
  };
  
  /** Delegate physical layer output. 

      This rewires the whole stack downstream.
   */
  inline void set_physical_out(delegate<int(uint8_t*,int)> phys){
    _eth.set_physical_out(phys);
    
    auto eth_top(delegate<int(Ethernet::addr,Ethernet::ethertype,uint8_t*,int)>
                 ::from<Ethernet,&Ethernet::transmit>(_eth));
    //auto arp_top(
    // Temp: Routing arp directly to physical.
    _arp.set_linklayer_out(eth_top);
    
    auto arp_top(delegate<int(IP4::addr, IP4::addr, uint8_t*, uint32_t)>
                 ::from<Arp,&Arp::transmit>(_arp));
    
    _ip4.set_linklayer_out(arp_top);
    
    auto ip4_top(delegate<int(IP4::addr,IP4::addr,IP4::proto,uint8_t*,uint32_t)>
                 ::from<IP4,&IP4::transmit>(_ip4));
    _udp.set_network_out(ip4_top);
    
    // Maybe not necessary
    _physical_out = phys;
  };
  
  inline void udp_listen(uint16_t port, UDP::listener l)
  { _udp.listen(port,l); }

  inline int udp_send(IP4::addr sip,UDP::port sport,
                       IP4::addr dip,UDP::port dport,
                       uint8_t* data, int len)
  { return _udp.transmit(sip,sport,dip,dport,data,len); }

  
  inline IP4::addr& ip4() { return _arp.ip(); }
  
  // Don't know that we *want* copy construction....
  IP_stack(IP_stack& cpy):
    _eth(cpy._eth),_arp(cpy._arp),_ip4(cpy._ip4)
  {    
    printf("<IP Stack> Copy-constructing\n");
  }
  
  IP_stack() :
    _eth(Ethernet::addr({0x08,0x00,0x27,0x9D,0x86,0xE8})),
    _arp(Ethernet::addr({0x08,0x00,0x27,0x9D,0x86,0xE8}),
         IP4::addr({(uint8_t)192,(uint8_t)168,(uint8_t)0,(uint8_t)11})),
    _ip4(IP4::addr({(uint8_t)192,(uint8_t)168,(uint8_t)0,(uint8_t)11}))
  {
    
    printf("<IP Stack> constructing \n");
    /** Make delegates for bottom of layers */
    
    auto arp_bottom(delegate<int(uint8_t*,int)>::from<Arp,&Arp::bottom>(_arp));
    auto ip4_bottom(delegate<int(uint8_t*,int)>::from<IP4,&IP4::bottom>(_ip4));
    auto ip6_bottom(delegate<int(uint8_t*,int)>::from<IP6,&IP6::bottom>(_ip6));
    

    auto icmp_bottom(delegate<int(uint8_t*,int)>::from<ICMP,&ICMP::bottom>(_icmp));
    auto udp_bottom(delegate<int(uint8_t*,int)>::from<UDP,&UDP::bottom>(_udp));

    // Hook up layers on top of ethernet
    // Upstream:
    _eth.set_arp_handler(arp_bottom);
    _eth.set_ip4_handler(ip4_bottom);
    _eth.set_ip6_handler(ip6_bottom);
    
    _ip4.set_icmp_handler(icmp_bottom);
    _ip4.set_udp_handler(udp_bottom);
    
    
  }
  
};


#endif
