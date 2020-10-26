#include <errno.h>
#include <string.h>

char *strerror(int errnum)
{
	switch (errnum)
	{
	case ENOTBLK:
		return "Block device required";
	case ENODEV:
		return "No such device";
	case EBADF:
		return "Bad file descriptor";
	case EOVERFLOW:
		return "Value too large to be stored in data type";
	case ENOENT:
		return "No such file or directory";
	case ENOSPC:
		return "No space left on device";
	case EEXIST:
		return "File exists";
	case EROFS:
		return "Read-only file system";
	case EINVAL:
		return "Invalid argument";
	case ENOTDIR:
		return "Not a directory";
	case ENOMEM:
		return "Not enough memory";
	case ERANGE:
		return "Result too large";
	case EISDIR:
		return "Is a directory";
	case EPERM:
		return "Operation not permitted";
	case EIO:
		return "Input/output error";
	case ENOEXEC:
		return "Exec format error";
	case EACCES:
		return "Permission denied";
	case ESRCH:
		return "No such process";
	case ENOTTY:
		return "Not a tty";
	case ECHILD:
		return "No child processes";
	case ENOSYS:
		return "Function not implemented";
	case ENOTSUP:
		return "Operation not supported";
	case EBLOCKING:
		return "Operation is blocking";
	case EINTR:
		return "Interrupted function call";
	case ENOTEMPTY:
		return "Directory not empty";
	case EBUSY:
		return "Device or resource busy";
	case EPIPE:
		return "Broken pipe";
	case EILSEQ:
		return "Invalid byte sequence";
	case ELAKE:
		return "Sit by a lake";
	case EMFILE:
		return "Too many open files";
	case EAGAIN:
		return "Resource temporarily unavailable";
	case EEOF:
		return "End of file";
	case EBOUND:
		return "Out of bounds";
	case EINIT:
		return "Not initialized";
	case ENODRV:
		return "No such driver";
	case E2BIG:
		return "Argument list too long";
	case EFBIG:
		return "File too large";
	case EXDEV:
		return "Improper link";
	case ESPIPE:
		return "Cannot seek on stream";
	case ENAMETOOLONG:
		return "Filename too long";
	case ELOOP:
		return "Too many levels of symbolic links";
	case EMLINK:
		return "Too many links";
	case ENXIO:
		return "No such device or address";
	case EPROTONOSUPPORT:
		return "Protocol not supported";
	case EAFNOSUPPORT:
		return "Address family not supported";
	case ENOTSOCK:
		return "Not a socket";
	case EADDRINUSE:
		return "Address already in use";
	case ETIMEDOUT:
		return "Connection timed out";
	case ECONNREFUSED:
		return "Connection refused";
	case EDOM:
		return "Mathematics argument out of domain of function";
	case EINPROGRESS:
		return "Operation in progress";
	case EALREADY:
		return "Connection already in progress";
	case ESHUTDOWN:
		return "Cannot send after transport endpoint shutdown";
	case ECONNABORTED:
		return "Connection aborted";
	case ECONNRESET:
		return "Connection reset";
	case EADDRNOTAVAIL:
		return "Address not available";
	case EISCONN:
		return "Socket is connected";
	case EFAULT:
		return "Bad address";
	case EDESTADDRREQ:
		return "Destination address required";
	case EHOSTUNREACH:
		return "Host is unreachable";
	case EMSGSIZE:
		return "Message too long";
	case ENETDOWN:
		return "Network is down";
	case ENETRESET:
		return "Connection aborted by network";
	case ENETUNREACH:
		return "Network is unreachable";
	case ENOBUFS:
		return "No buffer space available";
	case ENOMSG:
		return "No message of the desired type";
	case ENOPROTOOPT:
		return "Protocol not available";
	case ENOTCONN:
		return "Socket is not connected";
	case EDEADLK:
		return "Resource deadlock avoided";
	case ENFILE:
		return "Too many open files in system";
	case EPROTOTYPE:
		return "Wrong protocol type for socket";
	case ENOLCK:
		return "No locks available";
	case ENOUSER:
		return "No such user";
	case ENOGROUP:
		return "No such group";
	case ESIGPENDING:
		return "Signal is already pending";
	case ESTALE:
		return "Stale file handle";
	case EBADMSG:
		return "Bad message";
	case ECANCELED:
		return "Operation canceled";
	case EDQUOT:
		return "Disk quota exceeded";
	case EIDRM:
		return "Identifier removed";
	case EMULTIHOP:
		return "Multihop attempted";
	case ENOLINK:
		return "Link has been severed";
	case ENOTRECOVERABLE:
		return "State not recoverable";
	case EOWNERDEAD:
		return "Previous owner died";
	case EPROTO:
		return "Protocol error";
	case ETXTBSY:
		return "Text file busy";
	case ENOMOUNT:
		return "No such mountpoint";
	default:
		return "Unknown error condition";
	}
}
