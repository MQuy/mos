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

### Implementation

- [ ] Use `/dev/input/mouse -> { x, y, state }`, `/dev/input/keyboard -> { key, state }` to distribute events to userland. Specified device files is easier to implement at the first step, we can later apply [linux way](https://www.kernel.org/doc/html/latest/input/input_uapi.html)
- [ ] Signal
- [ ] Map serial port terminals to `/dev/ttyS[N]` (COM1 -> 0, COM2 -> 2)
- [ ] PTY master/slave (to make it simplier, using one struct contains both)

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

int sys_poll(struct pollfd *fds, nfds_t nfds)
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

- create the new filesystem named `devtmpfs` and mount at `/dev` (subs are located in memory)
- implement `wait_event(thread, cond)` (sleep until cond == true)

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

- `mouse_mount` to open `/dev/input/mouse` and register corresponding its major (no minor due to only one mouse supported)
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
  struct tty_struct *tty;

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

int sys_kill(pid_t pid, uint32_t signum) {
  1. if pid > 0
    - if pid == `current_process->pid` and `current_thread` is not in signal-handle state
      - if signum is not blocked -> enable correspond bit in `current_thread->pending`
      - call `signal_handle(current_thread)`
    - otherwise, if signum is currently blocked or in pending in all threads of receiving process -> return
      - pick receiving thread which doesn't block or has pending `signum`
      - enable correspond bit in `thread->pending`
      - if receiving thread is suspended -> move to ready state
  2. if pid == 0 -> for each process in process group of calling process -> go through sub steps in step 1
  3. if pid == -1 -> for each process which calling process can send -> go through sub steps in step 1
  4. if pid < -1 -> send to process whose pid == -process->pid
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

void signal_handle(struct thread *t) {
  1. if there is no pending signals
    - if thread is in signal-handle state
      - copy `thread->uregs` into thread kernel stack
      - umark thread from signal-handle state
    - return
  2. `regs` = the first trap frame on thread kernel stack
  2. if thread not in signal-handle state
    - if `regs->int_no` == 0x7F (syscall is interrupted)
      - `is_syscall_return = true` only if `regs->eax` == `read/write` syscall
      - `regs->eax = -EINTR`
    - copy `regs` into `thread->uregs` (interrupt doesn't not copy regs into `thread->uregs`)
  3. mark thread in signal-handle state
  4. get the first pending signal
  5. remove correspond signum bit in `process->pending`
  6. if signal handler is default
    - enable correspond signum and `sa_mask` bits in `process->blocked`
    - call it directly
    - reset `process->blocked`
    - go to step 1
  7. otherwise, setup trap frame in thread user stack
  |_________________________| <- current user esp (A)
  | thread->uregs           |
  |-------------------------|
  | process->blocked        |
  |-------------------------|
  | signum                  |
  |-------------------------|
  | sigreturn               | <- new user esp (B)
  |-------------------------|
  8. `regs->eip` = signal handler address and `regs->useresp` = new user esp
  9. enable correspond signum and `sa_mask` in `process->blocked`
  10. if `is_syscall_return` == true -> call `sigjump_usermode(thread->uregs)`
}

void sigreturn(struct interrupt_registers *regs) {
  1. restore our context `current_thread->uregs`, `process->blocked` based on B
  2. move user esp back to A
  3. call `signal_handle(current_thread)`
}

int32_t thread_page_fault(struct interrupt_registers *regs) {
  if fault address == sigreturn and from usermode -> call `sigreturn(regs)`
}

void schedule() {
  unlock_scheduler();
  1. if `current_thread` is not in signal-handle state
    -> `signal_handle(current_thread)`
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
  struct termios termios;

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
  1. allocate and init `tty` based on `driver`, `idx` and `name`
  2. assign `tty->terminos` with `driver->init_terminos`
  3. call `tty->ldisc.open(tty)`
}

int ptmx_open(struct vfs_inode *inode, struct vfs_file *filp) {
  1. get a new `index`
  2. call `init_dev` for both master (`ttym`) and slave (`ttys`)
  4. link `ttym` and `ttys`
  4. create `/dev/ttys`
}

// n_tty.c
int n_tty_write(struct tty_struct *tty, struct vfs_file *file, const unsigned char * buf, size_t nr) {
  1. allocate new buffer `pbuf`
  2. for each character in `buf`
    - if `ch` == `SUSP` and terminos->c_lflag-ISIG enabled
      - generate `SIGTSTP` to all processes in the foreground proces group controlling terminal
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
  3. if terminos mode == non-canonical mode and read buffer >= MIN
    - wake thread up
}

int n_tty_read(struct tty_struct *tty, struct vfs_file *file, const unsigned char * buf, size_t nr) {
  1. if terminos mode == canonical mode
    - sleep till reader buffer contains either NL, EOL or EOF (`\0`)
  2. if terminos mode == non-canonical mode
    - sleep till MIN bytes is satisfied
}

int n_tty_poll(struct tty_struct * tty, struct file * file, poll_table *wait) {
  1. `poll_wait(file, &tty->read_wait, wait)`
  2. return possible POLLXXX based on tty
}

// char_dev.c
void chardev_init() {
  1. init chardev list
  2. mount devfs at `/dev`
  3. call `tty_init()`
}
```
