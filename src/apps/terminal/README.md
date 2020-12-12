- [Terminology](#terminology)
  - [Terminal](#terminal)
  - [Shell](#shell)
  - [Termios](#termios)
  - [X Window System](#x-window-system)
  - [Signals](#signals)
  - [References](#references)
- [Implementation](#implementation)
  - [Select syscall](#select-syscall)
  - [Keyboard/Mice Event](#keyboardmice-event)
  - [Signals](#signals-1)
  - [PTY](#pty)
  - [Terminal](#terminal-1)
  - [Terminal V2](#terminal-v2)
  - [ANSI escape code](#ansi-escape-code)

### Terminology

#### Terminal

Terimal is the text input/output environment (Console is the physical terminal). Examples of terminals below

1. Teletypewriter (TTY)

   ![Teletypewrite](https://i.imgur.com/IkmMw8s.jpg)

2. Video terminal

   ![Video terminal](https://i.imgur.com/4Hcq449.jpg)

3. TTY device consists UART driver, line discipline and TTY driver

   ![TTY device](https://i.imgur.com/5z7Esin.png)

- UART driver (`struct tty_driver`) manages the physical transmission of bytes, including parity checks and flow control
- Line discipline (`struct tty_lisc`) is the software module sits in between and is incharge of handling special characters, echoing ...
- TTY driver manages session with controling terminal

  TTY device has the corresponding character device file under `/dev/ttyX`
  ![TTY](https://i.imgur.com/rLDxnK2.png)

1. PTY device consists of a pair of device files: the master and slave. The slave behaves almost identically to a virual terimal, however, its input and output goes to the master

   ![PTY](https://i.imgur.com/WLnQ6JU.png)

In unix, tty master is created using `/dev/ptmx` (Unix98 PTY). It is not represented on the file system, only via file descriptor. tty slave is represented via `/dev/pts/N`

5. Terminal emulator is a program that opens a graphical window and let users interact with the shell

#### Shell

- [Shell](https://en.wikipedia.org/wiki/Shell_%28computing%29) (outermost layer around operating system) is the user interface for access to an operating system's servers, it can be either command-line interface (CLI) or graphical user interface (GUI)
- There are many command line shell like [Ubuntu's bash](<https://en.wikipedia.org/wiki/Bash_(Unix_shell)>), zsh, ash, ... all of them are just variant of [Bourne Shell](https://en.wikipedia.org/wiki/Bourne_shell). Command line shell reads from terminal and interprets what user enters as the command and executes that command
- How command process (each command is executed in separated process) works
  ![command](https://i.imgur.com/62LNw5c.png)
  ![pipe](https://i.imgur.com/i0TA5Ka.png)
- Command is executed in subshell, anychanges will not affect the parent shell
  ~[subshell](https://i.imgur.com/9UYFbb8.png)
- [Job](<https://en.wikipedia.org/wiki/Job_control_(Unix)>) is a shell's representation for a process group. Only one job in the session runs in the foreground (other jobs are in background) which freely read/write from controlling terminal. If processes run in the background want to read/write to controlling terminal, they have to send signal SIGTTIN (job control). Job control features are the suspending, resuming, terminating of all processes in the job/process group or sending signals to the job.
  ![job](https://i.imgur.com/2Xt0KhL.png)

#### Termios

termios describes a set of line settings which is used to implement line disclipline

- operate with standard Unix system call like `open`, `read`, `write` and `close`
- configure communication (line discipline, ...) with the help of termios functions and data structures

termios supports two different input modes (decide when data is available via `read`)

- Canonical mode: input is processed in units of lines. A line is delimited by newline (NL), end-of-file (EOF) or end-of-line (EOL)
- Non-canonical mode: input is not assembled into lines, the value of `MIN` and `TIME` are used to determine how to process the bytes received (or receiving signal) -> four scenarios
  - MIN > 0 and TIME > 0 -> MIN bytes received before inter-byte timer expires or inter-byte timer expires (inter-byte timer is started after first byte is received and reset on following received bytes)
  - MIN > 0 and TIME = 0 -> MIN bytes received
  - MIN = 0 and TIME > 0 -> a single byte is received or read timer expries (read timer is activated when calling `read`)
  - MIN = 0 and TIME = 0 -> currently available bytes (zero if no characters in buffer)

#### X Window System

In unix, [X Window System server](https://en.wikipedia.org/wiki/X_Window_System) (not a part of operating system) controls the screen and get input from mouse/keyboard. How it works is described in the following steps:

Keyboard -> Application

1. X Server subscribes to `/dev/input/eventN`, when user types something

   - irq1 (keyboard) is fired. For each readers, `open("/dev/input/eventN")`, adding event into their packets and mark it ready (if they are ready before)
   - X Server is waked up (at the loop which was waiting for upcoming events)
   - send keycodes to terminal emulator <- communicate with terminal emulator (active client) via message queue (canonical implementations are Unix domain socket, named pipes and shared memory)

2. terminal emulator converts keycode into [keysyms](https://en.wikipedia.org/wiki/X_Window_System_core_protocol#Mappings) -> writes to the master
3. line discipline is triggered by operating system to do things like line conversion, control key handling, job control, line buffering, input echo ...
4. data is copied to associated slave which is later read by a process

Application -> UI

1. application want to display something -> write tty slave
2. line discipline is triggered by operating system to do things like line ending conversion
3. tty master receives changes -> terminal emulator requests X window System to render

![X Window System server](https://i.imgur.com/ebh5ny8.png)

#### Signals

There are 3 kind of signals

- errors: program has done something invalid and cannot continue execution like division by zero, invalid memory addresses ...
- external events: I/O or other processes like arrival of input, expiration of timer, termination of child process ...
- explicit requests: use library function such as `kill`

There are 2 kind of behaviours

- synchronously: by a specific action in the program and is delivered during that action (errors or explicit requests by itself)
- asynchronously: by evnts outside the control of the program that receives them and arrive at unpredictable times during execution (external events and explicit requests)

There are 3 kind of actions which that signal is taken

- ignore the signal (discard any signal immediately)
- customized action
- default action (by default if we don't specify anything)

There are 2 basic strategies in signal handler functions

- note signal arrived by tweaking global variables
- termiate the program or transfer control like `setjmp/longjmp`

When a handler is invoked on a signal(on the same process stack), that kind of signal is automatically blocked until it returns but other kind of signals are not blocked and can arrive during handler execution. There are 2 kind of signal blockings

- `sigprocmask` while you are in critical section (whether in program or in handler)
- `sigaction(sa_mask)` while a particular signal handler runs

When a signal is generated (`raise` for itself, `kill` for general purpose). If it is blocked or the receiving process already has a pending signal of that kind, it is ignored (nothing will happen). Otherwise

- signal is appended to the receiving process signal pending queue
- the receiving process is moved to ready if currently blocked/suspended.

A signal is delivered when receiving process responds to that signal e.g. by invoking a signal handler. If that kind of signal is blocked, it remains `pending` and once unblocked, it will be delived immediately

![flow](https://i.imgur.com/4l6NaX9.png)

Signals are inherited by child processes (`fork`) but will be set back to default when executing (`exec`)

✍️ signal handler is executed in one of process's threads, we cannot predict which is chosen

#### References

- [Brian Will's Unix terminals and shells](https://www.youtube.com/channel/UCseUQK4kC3x2x543nHtGpzw)
- [The TTY demystified](http://www.linusakesson.net/programming/tty/)
- [Using pseudo-terminals (pty) to control interactive programs](http://rachid.koucha.free.fr/tech_corner/pty_pdip.html)
- [General Terminal Interface](https://pubs.opengroup.org/onlinepubs/7908799/xbd/termios.html)
- [TTY under the hood](https://www.yabage.me/2016/07/08/tty-under-the-hood/)
- [TTYs / PTYs](https://forum.osdev.org/viewtopic.php?p=294636#p294636)
- [How do keyboard input and text ouput work](https://unix.stackexchange.com/a/116630/366870)
- [Character devices](https://www.win.tue.nl/~aeb/linux/lk/lk-11.html)
- [Signals](https://www.gnu.org/software/libc/manual/html_node/Signal-Handling.html)
- [Implementation of signal hanlding](http://courses.cms.caltech.edu/cs124/lectures/CS124Lec15.pdf)
- [Bash](https://www.gnu.org/software/bash/manual/html_node/index.html)

### Implementation

- [x] Use `/dev/input/mouse -> { x, y, state }`, `/dev/input/keyboard -> { key, state }` to distribute events to userland. Specified device files is easier to implement at the first step, we can later apply [linux way](https://www.kernel.org/doc/html/latest/input/input_uapi.html)
- [x] Add mqueuefs and refactor message queue to link with file descriptor
- [x] Signal
- [x] PTY master/slave (to make it simplier, using one struct contains both)
- [ ] Map serial port terminals to `/dev/ttyS[N]` (COM1 -> 0, COM2 -> 2)

#### Select syscall

```c
struct poll_table {
  struct list_head list;
}

struct poll_table_entry {
  struct vfs_file *file;
  struct wait_queue_entry wait;
  struct list_head sibling;
}

struct wait_queue_entry {
  void *private;
  void *func;
  struct list_head sibling;
};

struct wait_queue_head {
  struct list_head list;
};

// mouse.c
struct wait_queue_head hwait;

static uint32_t mouse_poll(struct vfs_file *file, struct poll_table *ptable)
{
  poll_wait(file, hwait, ptable);
  // return possible POLLXXXs based on mouse_inode
}

// sched.c
void wake_up(struct wait_queue_head *hqueue) {
  for each entry in wait queue -> `entry->func(entry->private)`
}

// poll.c
struct pollfd {
  int fd;         /* file descriptor */
  short events;     /* requested events */
  short revents;    /* returned events */
};

int sys_poll(struct pollfd *fds, uint32_t nfds)
{
  1. create `poll_table`
  2. for each fd
    - get file from fd
    - call `file->poll(file, ptable)` to add wait queues and get a bit mask (which operations could be performed immediately without blokcing)
    - AND returned bit mask with `events` and assign to `revents`
  3. if there is at least one `pollfd` with valid `revents` (!= 0) -> step 5
  4. sleep -> later is wakeup -> go to step 2
  5. cleanup `poll_table` and return fds
}

int poll_wait(struct vfs_file *file, struct wait_queue_head *head, struct poll_table *ptable)
{
  1. Initialize `poll_table_entry` and `wait_queue_entry { private = current_thread, func = poll_wakeup }`
  2. Link `poll_table -> poll_table_entry -> wait_queue_entry` and `hwait -> wait_queue_entry`
}

void poll_wakeup(struct thread *t) {
  1. `t->state = THREAD_READY`
}
```

#### Keyboard/Mice Event

- create the new filesystem named `devfs` and mount at `/dev` (subs are located in memory)
- implement `wait_event(thread, cond)` (create `wait_queue_entry` and sleep until cond == true)

1. Mouse

```c
struct mouse_motion {
  int dx, dy;
  int buttons;
}

#define PACKET_QUEUE_LEN 16
// file->private_data = mouse_inode;
struct mouse_inode {
  bool ready;
  struct mouse_motion packets[PACKET_QUEUE_LEN]; // only store 16 latest mouse motions
  uint8 head, tail;
  struct list_head sibling;
  struct vfs_file *file;
}

struct vfs_file_operations mouse_fops = {
  .open = mouse_open,
}

#define MOUSE_MAJOR 13
struct chrdev_mouse mouse_dev = {
	.name = "mouse",
	.major = MOUSE_MAJOR,
	.f_ops = &mouse_fops
}

struct list_head nodelist;
struct wait_queue_head hwait;
```

- `mouse_init` to create `/dev/input/mouse` and register its major and one minor
- `open("/dev/input/mouse")` -> initializing a node and adding into `nodelist` (subscribe)
  `close(fd)` -> removing existing node from `nodelist` (unsubscribe)
  `read("/dev/input/mouse")`
  - if there is an packet in `node->packets`
    - get and remove that packet from list
    - `node->ready = false` if nothing left in `node->packets`
    - return that packet
  - if not `wait_event(current_thread, node->ready)`
- `irq_mouse_handler` is triggered (on 3rd time)
  - update `current_mouse_motion` with mouse changes (dx, dy and buttons)
  - for each node in `nodelist`
    - adding `motion` into `node->packets` (only 16 pending packets -> latest packet will override oldest packet)
    - set `node->ready = true`
  - `wake_up(hwait)`

2. Keyboard is the same as mouse

#### Signals

```c
#define NSIG 32

struct process {
  pid_t sid;
  pid_t gid;
  pid_t pid;

  struct sigaction sighand[NSIG];
}

struct thread {
  uint32_t pending;
  uint32_t blocked;
}

typedef void *__sighandler_t(int);
struct sigaction {
  __sighandler_t sa_handler;
  uint32_t sa_flags; // only support SA_RESTART
  sigset_t sa_mask;
}

struct process *process_fork(struct proces *parent) {
  1. assign `sid`, `gid` and `pid` with process's parent
  2. copy `sighand` from parent to child thread
}

int sys_sigaction(int signum, const struct sigaction *action, struct sigaction *old_action) {
  1. if signum is `SIGKILL` or `SIGSTOP` -> return `EINVAL`
  2. if old_action != NULL -> `memcpy(old_action, process->sighand[signum], sizeof(struct sigaction))`
  3. if action != NULL -> `memcpy(process->sighand[signum], action, sizeof(struct sigaction))`
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
  1. if oldset != NULL -> copy `current_thread->blocked` to `oldset`
  2. if `set` != NULL
    - if `set` is `SIGKILL` or `SIGSTOP` -> return
    - if how == `SIG_BLOCK` -> `current_thread->blocked` = `current_thread->blocked` ∪ `set` (union)
    - if how == `SIG_UNBLOCK` -> `current_thread->blocked` = `current_thread->blocked` - `set` (remove `set` from `blocked`)
    - if how = `SIG_SETMASK` -> `current_thread->blocked` = `set`
}

int sys_kill(pid_t pid, uint32_t signum) {
  1. if pid > 0
    - if `sig_ignored(signum)` -> return (fast check)
    - if signum is `SIGCONT`
      - remove any pending stop signals and resume thread (if not current)
      - `process->flags |= SIGNAL_CONTINUE` and unmark `SIGNAL_TERMIATED/SIGNAL_STOPED`
      - send `SIGCHLD` to parent
      - wake `parent->wait_chld`
    - if signum is a stop signal
      - remove pending SIGCONT signal
      - `process->flags |= SIGNAL_STOPED` and unmark `SIGNAL_TERMIATED/SIGNAL_CONTINUED`
      - send `SIGCHLD` to parent
      - wake `parent->wait_chld`
      - stop thread
    - otherwise, enable correspond bit in `thread->pending`
    - if signum is `SIGKILL` -> resume thread (if not current)
  2. if pid == 0 -> for each process in process group of calling process -> go through sub steps in step 1
  3. if pid == -1 -> for each process which calling process can send -> go through sub steps in step 1
  4. if pid < -1 -> send to process whose pid == -process->pid
}

void signal_handler(regs) {
  1. if there is no pending signals -> return
  2. if `regs->cs != 0x1b` (might not correct) or regs is not the first trap frame (return to userspace) -> return
  3. if `current_thread->signaling` -> return
  4. call `handle_signal(regs)`
}

void handle_signal(regs) {
  1. if `regs->int_no` == 0x7F (syscall is interrupted)
    - `is_syscall = true` only if `regs->eax` == `read/write` syscall
    - `regs->eax = -EINTR`
  2. copy `regs` into `thread->uregs` (interrupt doesn't not copy regs into `thread->uregs`)
  3. `current_thread->signaling = true`
  4. get the first pending signal (terminated signals are highest and `SIGCONT` is lowest)
  5. unmask correspond signum bit in `thread->pending`
  6. signal handler has not be `SIG_IGN`
  7. if signal handler is `SIG_DFL`
    - default action has to be terminate or coredump
    - `process->caused_signal = signum`
    - `process->flags |= SIGNAL_TERMINATED` and unmark `SIGNAL_STOPED/SIGNAL_CONTINUED`
    - unmark signal-handle
    - empty pending signals in process
    - kill the process (`exit`)
  8. otherwise, setup trap frame in thread user stack
    |_________________________| <- current user esp (A)
    | thread->uregs           |
    |-------------------------|
    | thread->blocked         |
    |-------------------------|
    | thread->signaling       |
    |-------------------------|
    | signum                  |
    |-------------------------|
    | sigreturn               | <- new user esp (B)
    |-------------------------|
    -`regs->eip` = signal handler address and `regs->useresp` = new user esp
    - enable correspond signum and `sa_mask` in `thread->blocked`
    - if `is_syscall == true` -> call `sigjump_usermode(thread->uregs)`
}

void sigreturn(struct interrupt_registers *regs) {
  1. restore our context `current_thread->uregs`, `current_thread->signaling`, `process->blocked` based on B
  2. move user esp back to A
  3. `memcpy(regs, current_thread->uregs)`
}

int32_t thread_page_fault(struct interrupt_registers *regs) {
  if fault address == sigreturn and from usermode -> call `sigreturn(regs)`
}

void schedule() {
  unlock_scheduler();
  1. if `current_thread` has pending signals
    - get `regs` from kernel stack
    - `handle_signal(regs)`
}

void do_exit(int32_t code) {
  1. for each `area` in `current_process->mm->mmap`
    - if `area` doesn't link to a file -> unmap `area`
    - unlink and free `area`
  2. for each `file` in `current_process->files->fd[xxx]`
    - release `file` if `file->f_count == 1`
    - remove from `current_process->files->fd[xxx]`
  3. for its thread
    - set state to `THREAD_TERMINATED`
    - unlink `sleep_timer`
    - unmap its user stack
  4. for each child process in `current_process->children`
    - send `SIGHUP` to them
    - assign a new parent process (usually `init`)
  5. send `SIGCHLD` to parent process
  6. if process is session leader with its controlling terminal
    - send `SIGHUP` to foreground process group
    - disassociate that terminal from the session
  7. `current_process->exit_code = code` and `current_process->flags |= SIGNAL_TERMINATED`
  8. wake `parent->wait_chldexit`
}

int do_wait(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
  1. add queue entry into `current_thread->wait`
  2. for each children `p`
    - if `p->pid` is not eligiable -> continue
    - if `options & WEXITED` -> break if `p->flags & SIGNAL_TERMINATED`
    - if `options & WSTOPPED` -> break if `p->flags & SIGNAL_STOPPED`
    - if `options & WCONTINUED` -> break if `p->flags & SIGNAL_CONTINUED`
  3. if there is no proces satisfies with the condition above -> go to step 2
  4. fill `infop` properties based on `p`
  5. remove entry above from `current_current->wait`
}
```

#### PTY

```c
// pty.c
#define PTY_MASTER_MAJOR 2
#define PTY_SLAVE_MAJOR	3
#define NR_PTY_MAX (1 << MINORBITS)

struct tty_driver *pty_master_driver, *pty_slave_driver;

struct tty_operations pty_ops = {
	.write = pty_write,
};

int pty_write(struct tty_struct * tty, const unsigned char *buf, int count) {
  1. get other side `to` via `tty->link`
  2. call `to->ldisc.receive_buff(to, buf, count)`
}


int pty_init() {
  1. init pty master and slave drivers with `NR_PTY_MAX` devices
  2. set default termio and `pty_ops`
  3. link pty master and slave via `driver->other`
  4. `tty_register_driver` for both pty master and slave
}

// tty.c
#define TTYAUX_MAJOR		5
#define N_TTY_BUF_SIZE 4096
#define NCCS 19

struct termios {
	tcflag_t c_iflag;		/* input mode flags */
	tcflag_t c_oflag;		/* output mode flags */
	tcflag_t c_cflag;		/* control mode flags */
	tcflag_t c_lflag;		/* local mode flags */
	cc_t c_line;			/* line discipline */
	cc_t c_cc[NCCS];		/* control characters */
};

/*	intr=^C		quit=^\		erase=del	kill=^U
	  eof=^D		vtime=\0	vmin=\1		sxtc=\0
	  start=^Q	stop=^S		susp=^Z		eol=\0
	  reprint=^R	discard=^U	werase=^W	lnext=^V
	  eol2=\0
*/
#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"

struct tty_driver {
	const char *name;
  struct char_device *cdev;
  int	major;		/* major device number */
	int	minor_start;	/* start of minor device number */
	int	minor_num;	/* number of *possible* devices */
  int num; /* number of devices allocated */
  struct tty_opereations *tops;
  struct list_head ttys;
  struct list_head sibling;
  struct tty_driver *other;
  struct termios init_termios;
}

struct tty_operations {
  int  (*open)(struct tty_struct * tty, struct file * filp);
	void (*close)(struct tty_struct * tty, struct file * filp);
	int  (*write)(struct tty_struct * tty, const unsigned char *buf, int count);
}

struct tty_struct {
  struct tty_driver *driver;
  struct tty_ldisc *ldisc;
  struct tty_struct *link;
  struct termios *termios;

  int pgrp;
	int session;

  struct wait_queue_head write_wait;
	struct wait_queue_head read_wait;
}

struct tty_ldisc {
  int num;
  struct list_head sibling;
  uint32_t column;
  char *read_buf;
  char *write_buf;

  int	(*open)(struct tty_struct *);
	void (*close)(struct tty_struct *);
  ssize_t	(*read)(struct tty_struct * tty, struct file * file, unsigned char * buf, size_t nr);
	ssize_t	(*write)(struct tty_struct * tty, struct file * file, const unsigned char * buf, size_t nr);
  void (*receive_buf)(struct tty_struct *, const unsigned char *cp, int count);
  unsigned int (*poll)(struct tty_struct *, struct file *, struct poll_table_struct *);
}

struct termios tty_std_termios = {
	.c_iflag = ICRNL | IXON,
	.c_oflag = OPOST | ONLCR,
	.c_cflag = B38400 | CS8 | CREAD | HUPCL,
	.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
		   ECHOCTL | ECHOKE | IEXTEN,
	.c_cc = INIT_C_CC
};

struct list_head tty_drivers;
struct list_head tty_ldiscs;

int tty_register_driver(struct tty_driver *driver) {
  1. init `driver->cdev` with `dev = MKDEV(driver->major, driver->minor_start)` and `fops = tty_fops`
  2. if `driver->flags` not contain `TTY_DRIVER_NO_DEVFS` -> create `/dev/{driver->name}{XXX}` (XXX minor_start -> minor_start + minor_num)
  2. add `driver` into `tty_drivers`
}

struct vfs_file_operations ptmx_fops = {
  .open = ptmx_open,
  .read = tty_read,
  .write = tty_write,
  .poll = tty_poll,
}

struct vfs_file_operations tty_fops = {
  .open = tty_open,
  .read = tty_read,
  .write = tty_write,
  .poll = tty_poll,
}

int tty_init() {
  1. call `pty_init()`
  2. create and register the new chardev `ptmx`
    `{ name = "/dev/ptmx", major = TTYAUX_MAJOR, baseminor = 2, minorct = 1, f_ops = ptmx_fops }`
  3. create `/dev/ptmx`
}

int init_dev(struct tty_driver *driver, int idx, struct tty_struct **tty) {
  1. init `tty` based on `driver`, `idx` and `name`
  2. `tty->terminos` = `driver->init_terminos`
  3. `tty->ldisc` = `tty_ldisc_N_TTY`
  3. call `tty->ldisc.open(tty)`
}

int ptmx_open(struct vfs_inode *inode, struct vfs_file *filp) {
  1. get a new `index`
  2. call `init_dev` for both master (`ttym`) and slave (`ttys`)
  3. link `ttym` and `ttys`
  5. mount `/dev/pts/xxx`
}

// n_tty.c
int ntty_open(struct tty_struct *tty) {
  1. allocate `tty->read_buf` with `N_TTY_BUF_SIZE`
}

int n_tty_write(struct tty_struct *tty, struct vfs_file *file, const unsigned char * buf, size_t nr) {
  1. allocate new buffer `pbuf`
  2. for each character in `buf`
    - if terminos->c_oflag-OPOST enabled
      - if `ONLCR` && ch == NL -> ch = CR-NL
      - if `OCRNL` && ch == CR -> ch = NL
    - otherwise, `*pbuf++ = ch`
  3. call `tty->driver->write(pbuf, nr)`
}

void n_tty_receive_buf(struct tty_struct *tty, const unsigned char *buf, int count) {
  2. for each character in `buf`
    - if `ch` == `INTR/QUIT/SUSP` and terminos->c_lflag-ISIG enabled
      - generate `SIGINT/SIGQUIT/SIGTSTP` to all processes in the foreground proces group controlling terminal
    - if `ch` == `ERASE/KILL` and terminos->c_lflag-ICANON enabled
      - erase the last character/delete the entire line only in the current line
      - echo if termios->c_lflag-ECHOE/ECHOK enabled and in canonical mode
    - if `ch` == `EOF` and terminos->c_lflag-ICANON enabled
      - write `tmp_buffer + '\0'` to read buffer and wake thread up
    - if `ch` == `NL/EOL` and terminos->c_lflag-ICANON enabled
      - if temrinos->c_iflag-INLCR is eanbled
      - write `tmp_buffer + ch` to read buffer and wake thread up
      - echo if termios->c_lflag-ECHONL is enabled and in canonical mode
    - if `ch` == `CR` and terminos->c_iflag-ICRNL, terminos->c_lflag-ICANON enabled and termios->c_iflag-IGNCR disabled
      - `CR` is translated into `NL`
    - if terminos->c_iflag-IUCLC enabled, upper to lower mapping is performed
    - otherwise, `*tmp_buf++ = ch` and echo if echo mode is enabled
  3. if terminos mode == non-canonical mode
    - wake thread up if read buffer >= MIN
}

int n_tty_read(struct tty_struct *tty, struct vfs_file *file, const unsigned char * buf, size_t nr) {
  1. if terminos mode == canonical mode
    - sleep till reader buffer contains either NL, EOL or EOF (`\0`)
  2. if terminos mode == non-canonical mode
    - sleep till MIN bytes is satisfied
}

int n_tty_poll(struct tty_struct * tty, struct file * file, poll_table *wait) {
  1. `poll_wait(file, &tty->read_wait, wait)`
  2. `poll_wait(file, &tty->write_wait, wait)`
  2. return possible POLLXXX based on tty
}

// char_dev.c
void chardev_init() {
  1. call `tty_init()`
}
```

#### Terminal

#define MAX_LINES 400

```c
// terminal.c
struct line {
  char content[256]; // better to use linked list for buf
  unsigned long msec;
  struct list_head sibling;
  unsigned int rowspan;
}

struct tab {
  struct list_head lines;
  struct line *scroll_line;
  unsigned int nth_rowline;
  unsigned int line_count;
  unsigned int row_count;
  unsigned int max_rows;
  unsigned int width, height;
  unsigned int cursor_row, cursor_column;
}

main() {
  1. create master/slave pty
  2. `fork`
  3. for child
    - close master
    - create a new session and set slave as the controlling temrinal
    - set default input/output/error (0, 1, 2 file descriptors) to slave
    - `exec` bash program
  4. for parent
    - close slave
    - init main ui window -> `draw`
    - enter evenloop <- use `poll` for key event from x11 and slave pty
    | from x11
      - convert key event (keycode) to ascii
      - write to slave pty
    | from slave pty
      - each character
        | is EOL/EOF/NL
          - create a new line -> append to `terminal->lines`
          - `row_count`
            - if `== max_rows` -> remove first line -> recalculate `line_count`, `row_count` and `scroll_line`
            - increase `line_count`, `row_count`
          - `scroll_line`
            | `< max - 20` -> the same
            | number of rows `scroll_line -> last line` == 20 -> increase
        | is `ARROW_RIGHT/ARROW_LEFT`
          - increase/decease `cursor_column` by one
        | is `ARROW_UP`
          | `nth_rowline` == 0 -> scroll_line is back to one line
          | decrease `nth_rowline` by one
        | is `ARROW_DOWN`
          | `nth_rowline` == `scroll_line->rowspan` -> scroll_line to the next line
          | increase `nth_rowline` by one
        | other
          - append to last line's content at `cursor_column` position
          - increase `cursor_column` by one
      - `draw`
}

draw() {
  1. set `nth_rowline = terminal->nth_rowline`
  2. for each line
    - from `nth_rowline -> line->rowspan`
      - jump to that segment in `line->content`
      - draw to ui window
    - `nth_rowline = 0`
  3. draw cursor
  4. draw bar
}

// shell.c
struct shell {
  char cwd[256];
  gid_t foreground_gid;
}

main() {
  1. create `/dev/shm/shell`, is assigned to initialized `shell`
  2. output `cwd` (like `➜  ~`)
  3. for each line reading from input (0-fd)
  4. parse to get command and args // proper bash grammar later https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_10
  5. command
    | builtin
      - is the same as external except instead of loading program, we call a function
      - `shell` is modified via openning `/shm/shell`
    | external
      - fork
        | for child
          - set in/out/error to slave pty and close other fds
          - new group as foreground group and child is group leader
          - `foreground_gid = child->pid`
          - `exec` program with args
        | for parent
          - wait for child to exit <- loop until success (failed means interrupt by signal)
          - output `cwd`
          - no foregroup ground and set `foreground_gid = shell->pid`
  7. go back to step 3
}
```

#### Terminal V2

```c++
struct terminal_style {
  unsigned int background, color;
  char bold : 1;
  char light : 1;
  char italic : 1;
  char underline : 1;
  char slow_blink : 1;
  char rapid_blink : 1;
  char reverse_video : 1;
  char strike_throught : 1;
  unsigned int references;
}

struct terminal_unit {
  unsigned char content;
  struct terminal_row *row;
  struct list_head sibling;
  struct terminal_style *style;
}

struct terminal_row {
  struct list_head units;
  unsigned short columns;

  struct terminal_group *group;
  struct terminal *terminal;
  struct list_head sibling;
}

struct terminal_group {
  struct terminal_row *rows;
  unsigned short number_of_rows;

  struct terminal *terminal;
  struct list_head sibling;
}

struct terminal_config {
  char vertical_padding : 4;
  char horizontal_padding : 4;
  char tabspan;
  unsigned short max_lines;
  short screen_columns;
  short screen_rows;
}

struct terminal {
  struct list_head groups;
  struct list_head rows;

  struct terminal_row *cursor_row;
  unsigned short cursor_unit_index;
  struct terminal_row *scroll_row;
  struct terminal_style *current_style;

  struct window *win;
  struct terminal_config config;
}

struct terminal *alloc_terminal(struct window *win) {
  1. allocate terminal style `style` with background (black) and color (white)
  2. allocate terminal `term` with window and empty groups, rows
    - init terminal config
    - set style as above
  3. allocate terminal group `group` with `term`
    - append it to `term->groups`
  4. allocate terminal row `row` with empty units and zero columns
    - append it to `term->rows`
    - append it to `group->rows`
}

void terminal_draw_cursor(struct terminal *term, int x, int y) {
  4. draw
}

void terminal_draw_unit(struct terminal_unit *unit, int *x, int y) {
  1. draw `unit->content` at x, y
  2. increase x with unit content width (check tab width based on terminal config tabspan)
}

void terminal_draw_row(struct terminal_row *row, int y) {
  1. for each unit in `row->units`
    - draw unit (accumulate x)
    - if cursor is row -> draw
}

void terminal_draw(struct terminal *term) {
  1. iterate `screen_rows` times from `term->scroll_row`
    - draw for each row
}
```

#### ANSI escape code

```c++
# escape_parser.c
enum {
 ASCI_SINGLE_SHIFT_TWO = 0,
 ASCI_SINGLE_SHIFT_THREE = 1,
 ASCI_DEVICE_CONTROL_STRING = 2,
 ASCI_CONTROL_SEQUENCE_INTRODUCER = 3,
};

struct asci_sequence {
  int type;
  char *data;
};

enum {
  ASCI_CURSOR_UP = 0,
  ASCI_CURSOR_DOWN = 1,
  ASCI_CURSOR_FORWARD = 2,
  ASCI_CURSOR_BACK = 3,
  ASCI_CURSOR_NEXTLINE = 4,
  ASCI_CURSOR_PREVLINE = 5,
  ASCI_CURSOR_HA = 6,
  ASCI_CURSOR_POSITION = 7,
  ASCI_ERASE = 8,
  ASCI_ERASE_LINE = 9,
  ASCI_SCROLL_UP = 10,
  ASCI_SCROLL_DOWN = 11,
  ASCI_HV_POSITION = 12,
  ASCI_SELECT_GRAPHIC_POSITION = 13,
};

struct asci_control {
  int type;
  char *data;
};

void convert_key_event_to_code(keycode, state) {
  1. hash table `ht_printable`, value is printable characters (ascii) and key
    - is unsigned integer
    - 3 top bits are CTRL|SHIFT|ALT
    - other bits are keycode (from X11)
  2. look up `ht_printable` -> return if found
  3. hash table `ht_sequences`, value is ansi sequences and key the same as `ht_printable`
  4. look up `ht_sequences` -> return if found
}

int parse_control_sequence(input, sequence) {
  1. control = sequence->data = calloc(1, sizeof(struct asci_control))
  2. iterate input from index 2
    - if ch == terminated code (A, B, C ...)
      - depend on which terminated code, we set `control->type` a corresponding value
      - copy that range into `control->data`
      - return 0
  3. return -1
}

int parse_asci_sequence_from_input(input, sequence) {
  1. if input[0] != '\33' -> step 3
  2. if input[1] == '['
    - sequence->type = ASCI_CONTROL_SEQUENCE_INTRODUCER
    - return parse_control_sequence(input, sequence)
  3. return -1
}

void handle_asci_control(tab, control) {
  1. if `control->type` == CUU
    - parse data
    - terminal_move_cursor_up(tab, n)
}

void handle_asci_sequence(tab, sequence) {
  1. if CSI -> handle_asci_control(tab, sequence->data)
  2. else raise
}
```
