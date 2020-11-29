#ifndef _LIBC_PATHS_H
#define _LIBC_PATHS_H 1

/* Default search path. */
#define _PATH_DEFPATH "/usr/bin:/bin"
/* All standard utilities path. */
#define _PATH_STDPATH \
	"/usr/bin:/bin:/usr/sbin:/sbin"

#define _PATH_BSHELL "/bin/sh"
#define _PATH_CONSOLE "/dev/console"
#define _PATH_CSHELL "/bin/csh"
#define _PATH_DEVDB "/var/run/dev.db"
#define _PATH_DEVNULL "/dev/null"
#define _PATH_DRUM "/dev/drum"
#define _PATH_GSHADOW "/etc/gshadow"
#define _PATH_KLOG "/proc/kmsg"
#define _PATH_KMEM "/dev/kmem"
#define _PATH_LASTLOG "/var/log/lastlog"
#define _PATH_MAILDIR "/var/mail"
#define _PATH_MAN "/usr/share/man"
#define _PATH_MEM "/dev/mem"
#define _PATH_MNTTAB "/etc/fstab"
#define _PATH_MOUNTED "/etc/mtab"
#define _PATH_NOLOGIN "/etc/nologin"
#define _PATH_PRESERVE "/var/lib"
#define _PATH_RWHODIR "/var/spool/rwho"
#define _PATH_SENDMAIL "/usr/sbin/sendmail"
#define _PATH_SHADOW "/etc/shadow"
#define _PATH_SHELLS "/etc/shells"
#define _PATH_TTY "/dev/tty"
#define _PATH_UNIX "/boot/vmlinux"
#define _PATH_UTMP "/var/run/utmp"
#define _PATH_VI "/usr/bin/vi"
#define _PATH_WTMP "/var/log/wtmp"

/* Provide trailing slash, since mostly used for building pathnames. */
#define _PATH_DEV "/dev/"
#define _PATH_TMP "/tmp/"
#define _PATH_VARDB "/var/lib/misc/"
#define _PATH_VARRUN "/var/run/"
#define _PATH_VARTMP "/var/tmp/"

#endif
