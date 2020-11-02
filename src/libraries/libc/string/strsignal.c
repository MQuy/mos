/*
 * Copyright (c) 2012, 2014 Jonas 'Sortie' Termansen.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * string/strsignal.c
 * Convert signal number to a string.
 */

#include <signal.h>
#include <string.h>

char* strsignal(int signum)
{
	switch (signum)
	{
	case SIGHUP:
		return "Hangup";
	case SIGINT:
		return "Interrupt";
	case SIGQUIT:
		return "Quit";
	case SIGILL:
		return "Invalid instruction";
	case SIGTRAP:
		return "Trace/breakpoint trap";
	case SIGABRT:
		return "Aborted";
	case SIGBUS:
		return "Bus Error";
	case SIGFPE:
		return "Floating point exception";
	case SIGKILL:
		return "Killed";
	case SIGUSR1:
		return "User defined signal 1";
	case SIGSEGV:
		return "Segmentation fault";
	case SIGUSR2:
		return "User defined signal 2";
	case SIGPIPE:
		return "Broken pipe";
	case SIGALRM:
		return "Alarm clock";
	case SIGTERM:
		return "Terminated";
	case SIGSYS:
		return "Bad system call";
	case SIGCHLD:
		return "Child exited";
	case SIGCONT:
		return "Continued";
	case SIGSTOP:
		return "Stopped (signal)";
	case SIGTSTP:
		return "Stopped";
	case SIGTTIN:
		return "Stopped (tty input)";
	case SIGTTOU:
		return "Stopped (tty output)";
	case SIGURG:
		return "Urgent I/O condition";
	case SIGXCPU:
		return "CPU time limit exceeded";
	case SIGXFSZ:
		return "File size limit exceeded";
	case SIGVTALRM:
		return "Virtual timer expired";
	case SIGPWR:
		return "Power Fail/Restart";
	case SIGWINCH:
		return "Window changed";
	default:
		break;
	}

	if (SIGRTMIN <= signum && signum <= SIGRTMAX)
		return "Real-time signal";

	return "Unknown signal value";
}
