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

#### References

- [Brian Will's Unix terminals and shells](https://www.youtube.com/channel/UCseUQK4kC3x2x543nHtGpzw)
- [Killing a process and all of its descendants](http://morningcoffee.io/killing-a-process-and-all-of-its-descendants.html)
- [The TTY demystified](http://www.linusakesson.net/programming/tty/)
- [Using pseudo-terminals (pty) to control interactive programs](http://rachid.koucha.free.fr/tech_corner/pty_pdip.html)
- [General Terminal Interface](https://pubs.opengroup.org/onlinepubs/7908799/xbd/termios.html)
- [TTY under the hood](https://www.yabage.me/2016/07/08/tty-under-the-hood/)
- [TTYs / PTYs](https://forum.osdev.org/viewtopic.php?p=294636#p294636)
- [How do keyboard input and text ouput work](https://unix.stackexchange.com/a/116630/366870)
- [Character devices](https://www.win.tue.nl/~aeb/linux/lk/lk-11.html)

### Implementation

- [ ] Use `/dev/input/mice -> { x, y, state }`, `/dev/input/keyboard -> { key, state }` to distribute events to userland. Specified device files is easier to implement at the first step, we can later apply [linux way](https://www.kernel.org/doc/html/latest/input/input_uapi.html)
- [ ] Signal
- [ ] Map serial port terminals to `/dev/ttyS[N]` (COM1 -> 0, COM2 -> 2)
- [ ] PTY master/slave (to make it simplier, using one struct contains both)

#### Keyboard/Mice Event

#### Signals

![flow](https://i.imgur.com/4l6NaX9.png)

#### PTY
