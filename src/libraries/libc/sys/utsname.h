#ifndef _LIBC_SYS_UTSNAME_H
#define _LIBC_SYS_UTSNAME_H

/* Length of the entries in `struct utsname' is 65.  */
#define _UTSNAME_LENGTH 65

/* Linux provides as additional information in the `struct utsname'
   the name of the current domain.  Define _UTSNAME_DOMAIN_LENGTH
   to a value != 0 to activate this entry.  */
#define _UTSNAME_DOMAIN_LENGTH _UTSNAME_LENGTH

#ifndef _UTSNAME_SYSNAME_LENGTH
#define _UTSNAME_SYSNAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_NODENAME_LENGTH
#define _UTSNAME_NODENAME_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_RELEASE_LENGTH
#define _UTSNAME_RELEASE_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_VERSION_LENGTH
#define _UTSNAME_VERSION_LENGTH _UTSNAME_LENGTH
#endif
#ifndef _UTSNAME_MACHINE_LENGTH
#define _UTSNAME_MACHINE_LENGTH _UTSNAME_LENGTH
#endif

/* Structure describing the system and machine.  */
struct utsname
{
	/* Name of the implementation of the operating system.  */
	char sysname[_UTSNAME_SYSNAME_LENGTH];

	/* Name of this node on the network.  */
	char nodename[_UTSNAME_NODENAME_LENGTH];

	/* Current release level of this implementation.  */
	char release[_UTSNAME_RELEASE_LENGTH];
	/* Current version level of this release.  */
	char version[_UTSNAME_VERSION_LENGTH];

	/* Name of the hardware type the system is running on.  */
	char machine[_UTSNAME_MACHINE_LENGTH];

	/* Name of the domain of this node on the network.  */
	char domainname[_UTSNAME_DOMAIN_LENGTH];
};

int uname(struct utsname *buf);

#endif
