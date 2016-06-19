/*
 * Copyright (c) 2016 Francesco Balducci
 *
 * This file is part of nucleo_tests.
 *
 *    nucleo_tests is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    nucleo_tests is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with nucleo_tests.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SYS_TERMIOS_H
#define SYS_TERMIOS_H

typedef unsigned int cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 11

#define VEOF    0
#define VEOL    1
#define VERASE  2
#define VINTR   3
#define VKILL   4
#define VMIN    5
#define VQUIT   6
#define VSTART  7
#define VSTOP   8
#define VSUSP   9
#define VTIME  10

#define BRKINT 0x0001 /* Signal interrupt on break. */
#define ICRNL  0x0002 /* Map CR to NL on input. */
#define IGNBRK 0x0004 /* Ignore break condition. */
#define IGNCR  0x0008 /* Ignore CR. */
#define IGNPAR 0x0010 /* Ignore characters with parity errors. */
#define INLCR  0x0020 /* Map NL to CR on input. */
#define INPCK  0x0040 /* Enable input parity check. */
#define ISTRIP 0x0080 /* Strip character. */
#define IXANY  0x0100 /* Enable any character to restart output. */
#define IXOFF  0x0200 /* Enable start/stop input control. */
#define IXON   0x0400 /* Enable start/stop output control. */
#define PARMRK 0x0800 /* Mark parity errors. */

#define OPOST  0x0001 /* Post-process output. */
#define ONLCR  0x0002 /* Map NL to CR-NL on output. */
#define OCRNL  0x0004 /* Map CR to NL on output. */
#define ONOCR  0x0008 /* No CR output at column 0. */
#define ONLRET 0x0010 /* NL performs CR function. */
#define OFDEL  0x0020 /* Fill is DEL. */
#define OFILL  0x0040 /* Use fill characters for delay. */
#define NLDLY  0x0080 /* Select newline delays: */
#define NL0    0x0000 /* Newline type 0. */
#define NL1    0x0080 /* Newline type 1. */
#define CRDLY  0x0300 /* Select carriage-return delays: */
#define CR0    0x0000 /* Carriage-return delay type 0. */
#define CR1    0x0100 /* Carriage-return delay type 1. */
#define CR2    0x0200 /* Carriage-return delay type 2. */
#define CR3    0x0300 /* Carriage-return delay type 3. */
#define TABDLY 0x0C00 /* Select horizontal-tab delays: */
#define TAB0   0x0000 /* Horizontal-tab delay type 0. */
#define TAB1   0x0400 /* Horizontal-tab delay type 1. */
#define TAB2   0x0800 /* Horizontal-tab delay type 2. */
#define TAB3   0x0C00 /* Expand tabs to spaces. */
#define BSDLY  0x1000 /* Select backspace delays: */
#define BS0    0x0000 /* Backspace-delay type 0. */
#define BS1    0x1000 /* Backspace-delay type 1. */
#define VTDLY  0x2000 /* Select vertical-tab delays: */
#define VT0    0x0000 /* Vertical-tab delay type 0. */
#define VT1    0x2000 /* Vertical-tab delay type 1. */
#define FFDLY  0x4000 /* Select form-feed delays: */
#define FF0    0x0000 /* Form-feed delay type 0. */
#define FF1    0x4000 /* Form-feed delay type 1. */

#define B1         1    
#define B50       50   
#define B75       75   
#define B110     110  
#define B134     134  
#define B150     150  
#define B200     200  
#define B300     300  
#define B600     600  
#define B1200   1200 
#define B1800   1800 
#define B2400   2400 
#define B4800   4800 
#define B9600   9600 
#define B19200 19200
#define B38400 38400

#define CSIZE  0x0003 /* Character size: */
#define CS5    0x0000 /* 5 bits */
#define CS6    0x0001 /* 6 bits */
#define CS7    0x0002 /* 7 bits */
#define CS8    0x0003 /* 8 bits */
#define CSTOPB 0x0004 /* Send two stop bits, else one. */
#define CREAD  0x0008 /* Enable receiver. */
#define PARENB 0x0010 /* Parity enable. */
#define PARODD 0x0020 /* Odd parity, else even. */
#define HUPCL  0x0040 /* Hang up on last close. */
#define CLOCAL 0x0080 /* Ignore modem status lines. */

#define ECHO   0x0001 /* Enable echo. */
#define ECHOE  0x0002 /* Echo erase character as error-correcting backspace. */
#define ECHOK  0x0004 /* Echo KILL. */
#define ECHONL 0x0008 /* Echo NL. */
#define ICANON 0x0010 /* Canonical input (erase and kill processing). */
#define IEXTEN 0x0020 /* Enable extended input character processing. */
#define ISIG   0x0040 /* Enable signals. */
#define NOFLSH 0x0080 /* Disable flush after interrupt or quit. */
#define TOSTOP 0x0100 /* Send SIGTTOU for background output. */

#define TCSANOW   0 /* Change attributes immediately. */
#define TCSADRAIN 1 /* Change attributes when output has drained. */
#define TCSAFLUSH 2 /* Change attributes when output has drained; also flush pending input. */

#define TCIFLUSH  0 /* Flush pending input. */
#define TCIOFLUSH 1 /* Flush both pending input and untransmitted output. */
#define TCOFLUSH  2 /* Flush untransmitted output. */

#define TCIOFF 0 /* Transmit a STOP character, intended to suspend input data. */
#define TCION  1 /* Transmit a START character, intended to restart input data. */
#define TCOOFF 2 /* Suspend output. */
#define TCOON  3 /* Restart output. */

struct termios
{
    tcflag_t c_iflag;    /* Input modes. */
    tcflag_t c_oflag;    /* Output modes. */
    tcflag_t c_cflag;    /* Control modes. */
    tcflag_t c_lflag;    /* Local modes. */
    cc_t     c_cc[NCCS]; /* Control characters. */
};

speed_t cfgetispeed(const struct termios *);
speed_t cfgetospeed(const struct termios *);
int     cfsetispeed(struct termios *, speed_t);
int     cfsetospeed(struct termios *, speed_t);
int     tcdrain(int);
int     tcflow(int, int);
int     tcflush(int, int);
int     tcgetattr(int, struct termios *);
pid_t   tcgetsid(int);
int     tcsendbreak(int, int);
int     tcsetattr(int, int, const struct termios *);

#endif /* SYS_TERMIOS_H */

