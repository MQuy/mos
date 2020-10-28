#ifndef _LIBC_SOCKIOS_H
#define _LIBC_SOCKIOS_H 1

#define SIOCGIFDNSADDR 0x8900

/* Socket configuration controls. */
#define SIOCGIFNAME 0x8910	  /* get iface name		*/
#define SIOCSIFLINK 0x8911	  /* set iface channel		*/
#define SIOCGIFCONF 0x8912	  /* get iface list		*/
#define SIOCGIFFLAGS 0x8913	  /* get flags			*/
#define SIOCSIFFLAGS 0x8914	  /* set flags			*/
#define SIOCGIFADDR 0x8915	  /* get PA address		*/
#define SIOCSIFADDR 0x8916	  /* set PA address		*/
#define SIOCGIFDSTADDR 0x8917 /* get remote PA address	*/
#define SIOCSIFDSTADDR 0x8918 /* set remote PA address	*/
#define SIOCGIFBRDADDR 0x8919 /* get broadcast PA address	*/
#define SIOCSIFBRDADDR 0x891a /* set broadcast PA address	*/
#define SIOCGIFNETMASK 0x891b /* get network PA mask		*/
#define SIOCSIFNETMASK 0x891c /* set network PA mask		*/
#define SIOCGIFMETRIC 0x891d  /* get metric			*/
#define SIOCSIFMETRIC 0x891e  /* set metric			*/
#define SIOCGIFMEM 0x891f	  /* get memory address (BSD)	*/
#define SIOCSIFMEM 0x8920	  /* set memory address (BSD)	*/
#define SIOCGIFMTU 0x8921	  /* get MTU size			*/
#define SIOCSIFMTU 0x8922	  /* set MTU size			*/
#define SIOCSIFNAME 0x8923	  /* set interface name */
#define SIOCSIFHWADDR 0x8924  /* set hardware address 	*/
#define SIOCGIFENCAP 0x8925	  /* get/set encapsulations       */
#define SIOCSIFENCAP 0x8926
#define SIOCGIFHWADDR 0x8927 /* Get hardware address		*/
#define SIOCGIFSLAVE 0x8929	 /* Driver slaving support	*/
#define SIOCSIFSLAVE 0x8930
#define SIOCADDMULTI 0x8931 /* Multicast address lists	*/
#define SIOCDELMULTI 0x8932
#define SIOCGIFINDEX 0x8933		 /* name -> if_index mapping	*/
#define SIOGIFINDEX SIOCGIFINDEX /* misprint compatibility :-)	*/
#define SIOCSIFPFLAGS 0x8934	 /* set/get extended flags set	*/
#define SIOCGIFPFLAGS 0x8935
#define SIOCDIFADDR 0x8936		  /* delete PA address		*/
#define SIOCSIFHWBROADCAST 0x8937 /* set hardware broadcast addr	*/
#define SIOCGIFCOUNT 0x8938		  /* get number of devices */

#define SIOCGIFBR 0x8940 /* Bridging support		*/
#define SIOCSIFBR 0x8941 /* Set bridging options 	*/

#define SIOCGIFTXQLEN 0x8942 /* Get the tx queue length	*/
#define SIOCSIFTXQLEN 0x8943 /* Set the tx queue length 	*/

#define SIOCGIFDIVERT 0x8944 /* Frame diversion support */
#define SIOCSIFDIVERT 0x8945 /* Set frame diversion options */

#define SIOCETHTOOL 0x8946 /* Ethtool interface		*/

#define SIOCGMIIPHY 0x8947 /* Get address of MII PHY in use. */
#define SIOCGMIIREG 0x8948 /* Read MII PHY register.	*/
#define SIOCSMIIREG 0x8949 /* Write MII PHY register.	*/

#define SIOCWANDEV 0x894A /* get/set netdev parameters	*/

/* ARP cache control calls. */
/*  0x8950 - 0x8952  * obsolete calls, don't re-use */
#define SIOCDARP 0x8953 /* delete ARP table entry	*/
#define SIOCGARP 0x8954 /* get ARP table entry		*/
#define SIOCSARP 0x8955 /* set ARP table entry		*/

/* RARP cache control calls. */
#define SIOCDRARP 0x8960 /* delete RARP table entry	*/
#define SIOCGRARP 0x8961 /* get RARP table entry		*/
#define SIOCSRARP 0x8962 /* set RARP table entry		*/

#endif
