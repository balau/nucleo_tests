/*
 * Copyright (c) 2015 Francesco Balducci
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
#ifndef W5100_H
#define W5100_H

#include <stdint.h>
#include <stddef.h>

#define W5100_S0 0
#define W5100_S1 1
#define W5100_S2 2
#define W5100_S3 3
#define W5100_N_SOCKETS 4

#define W5100_SOCKET_REGS_SIZE 0x0100
#define W5100_S0_REGS_OFFSET   0x0400
#define W5100_S1_REGS_OFFSET   0x0500
#define W5100_S2_REGS_OFFSET   0x0600
#define W5100_S3_REGS_OFFSET   0x0700

#define W5100_TX_MEM_BASE 0x4000
#define W5100_TX_MEM_SIZE 0x2000
#define W5100_RX_MEM_BASE 0x6000
#define W5100_RX_MEM_SIZE 0x2000

/* Common registers */

#define W5100_MR     0x0000
#define W5100_GAR0   0x0001
#define W5100_GAR1   0x0002
#define W5100_GAR2   0x0003
#define W5100_GAR3   0x0004
#define W5100_SUBR0  0x0005
#define W5100_SUBR1  0x0006
#define W5100_SUBR2  0x0007
#define W5100_SUBR3  0x0008
#define W5100_SHAR0  0x0009
#define W5100_SHAR1  0x000A
#define W5100_SHAR2  0x000B
#define W5100_SHAR3  0x000C
#define W5100_SHAR4  0x000D
#define W5100_SHAR5  0x000E
#define W5100_SIPR0  0x000F
#define W5100_SIPR1  0x0010
#define W5100_SIPR2  0x0011
#define W5100_SIPR3  0x0012
#define W5100_IR     0x0015
#define W5100_IMR    0x0016
#define W5100_RTR0   0x0017
#define W5100_RTR1   0x0018
#define W5100_RCR    0x0019
#define W5100_RMSR   0x001A
#define W5100_TMSR   0x001B
#define W5100_PATR0  0x001C
#define W5100_PATR1  0x001D
#define W5100_PTIMER 0x0028
#define W5100_PMAGIC 0x0029
#define W5100_UIPR0  0x002A
#define W5100_UIPR1  0x002B
#define W5100_UIPR2  0x002C
#define W5100_UIPR3  0x002D
#define W5100_UPORT0 0x002E
#define W5100_UPORT1 0x002F

/* Socket registers */

/* Generic socket registers */
#define W5100_Sn_MR      0x0000
#define W5100_Sn_CR      0x0001
#define W5100_Sn_IR      0x0002
#define W5100_Sn_SR      0x0003
#define W5100_Sn_PORT0   0x0004
#define W5100_Sn_PORT1   0x0005
#define W5100_Sn_DHAR0   0x0006
#define W5100_Sn_DHAR1   0x0007
#define W5100_Sn_DHAR2   0x0008
#define W5100_Sn_DHAR3   0x0009
#define W5100_Sn_DHAR4   0x000A
#define W5100_Sn_DHAR5   0x000B
#define W5100_Sn_DIPR0   0x000C
#define W5100_Sn_DIPR1   0x000D
#define W5100_Sn_DIPR2   0x000E
#define W5100_Sn_DIPR3   0x000F
#define W5100_Sn_DPORT0  0x0010
#define W5100_Sn_DPORT1  0x0011
#define W5100_Sn_MSSR0   0x0012
#define W5100_Sn_MSSR1   0x0013
#define W5100_Sn_SPROTO  0x0014
#define W5100_Sn_TOS     0x0015
#define W5100_Sn_TTL     0x0016
#define W5100_Sn_TX_FSR0 0x0020
#define W5100_Sn_TX_FSR1 0x0021
#define W5100_Sn_TX_RD0  0x0022
#define W5100_Sn_TX_RD1  0x0023
#define W5100_Sn_TX_WR0  0x0024
#define W5100_Sn_TX_WR1  0x0025
#define W5100_Sn_RX_RSR0 0x0026
#define W5100_Sn_RX_RSR1 0x0027

/* S0 */
#define W5100_S0_MR      (W5100_S0_REGS_OFFSET + W5100_Sn_MR     )
#define W5100_S0_CR      (W5100_S0_REGS_OFFSET + W5100_Sn_CR     )
#define W5100_S0_IR      (W5100_S0_REGS_OFFSET + W5100_Sn_IR     )
#define W5100_S0_SR      (W5100_S0_REGS_OFFSET + W5100_Sn_SR     )
#define W5100_S0_PORT0   (W5100_S0_REGS_OFFSET + W5100_Sn_PORT0  )
#define W5100_S0_PORT1   (W5100_S0_REGS_OFFSET + W5100_Sn_PORT1  )
#define W5100_S0_DHAR0   (W5100_S0_REGS_OFFSET + W5100_Sn_DHAR0  )
#define W5100_S0_DHAR1   (W5100_S0_REGS_OFFSET + W5100_Sn_DHAR1  )
#define W5100_S0_DHAR2   (W5100_S0_REGS_OFFSET + W5100_Sn_DHAR2  )
#define W5100_S0_DHAR3   (W5100_S0_REGS_OFFSET + W5100_Sn_DHAR3  )
#define W5100_S0_DHAR4   (W5100_S0_REGS_OFFSET + W5100_Sn_DHAR4  )
#define W5100_S0_DHAR5   (W5100_S0_REGS_OFFSET + W5100_Sn_DHAR5  )
#define W5100_S0_DIPR0   (W5100_S0_REGS_OFFSET + W5100_Sn_DIPR0  )
#define W5100_S0_DIPR1   (W5100_S0_REGS_OFFSET + W5100_Sn_DIPR1  )
#define W5100_S0_DIPR2   (W5100_S0_REGS_OFFSET + W5100_Sn_DIPR2  )
#define W5100_S0_DIPR3   (W5100_S0_REGS_OFFSET + W5100_Sn_DIPR3  )
#define W5100_S0_DPORT0  (W5100_S0_REGS_OFFSET + W5100_Sn_DPORT0 )
#define W5100_S0_DPORT1  (W5100_S0_REGS_OFFSET + W5100_Sn_DPORT1 )
#define W5100_S0_MSSR0   (W5100_S0_REGS_OFFSET + W5100_Sn_MSSR0  )
#define W5100_S0_MSSR1   (W5100_S0_REGS_OFFSET + W5100_Sn_MSSR1  )
#define W5100_S0_SPROTO  (W5100_S0_REGS_OFFSET + W5100_Sn_SPROTO )
#define W5100_S0_TOS     (W5100_S0_REGS_OFFSET + W5100_Sn_TOS    )
#define W5100_S0_TTL     (W5100_S0_REGS_OFFSET + W5100_Sn_TTL    )
#define W5100_S0_TX_FSR0 (W5100_S0_REGS_OFFSET + W5100_Sn_TX_FSR0)
#define W5100_S0_TX_FSR1 (W5100_S0_REGS_OFFSET + W5100_Sn_TX_FSR1)
#define W5100_S0_TX_RD0  (W5100_S0_REGS_OFFSET + W5100_Sn_TX_RD0 )
#define W5100_S0_TX_RD1  (W5100_S0_REGS_OFFSET + W5100_Sn_TX_RD1 )
#define W5100_S0_TX_WR0  (W5100_S0_REGS_OFFSET + W5100_Sn_TX_WR0 )
#define W5100_S0_TX_WR1  (W5100_S0_REGS_OFFSET + W5100_Sn_TX_WR1 )
#define W5100_S0_RX_RSR0 (W5100_S0_REGS_OFFSET + W5100_Sn_RX_RSR0)
#define W5100_S0_RX_RSR1 (W5100_S0_REGS_OFFSET + W5100_Sn_RX_RSR1)

/* S1 */
#define W5100_S1_MR      (W5100_S1_REGS_OFFSET + W5100_Sn_MR     )
#define W5100_S1_CR      (W5100_S1_REGS_OFFSET + W5100_Sn_CR     )
#define W5100_S1_IR      (W5100_S1_REGS_OFFSET + W5100_Sn_IR     )
#define W5100_S1_SR      (W5100_S1_REGS_OFFSET + W5100_Sn_SR     )
#define W5100_S1_PORT0   (W5100_S1_REGS_OFFSET + W5100_Sn_PORT0  )
#define W5100_S1_PORT1   (W5100_S1_REGS_OFFSET + W5100_Sn_PORT1  )
#define W5100_S1_DHAR0   (W5100_S1_REGS_OFFSET + W5100_Sn_DHAR0  )
#define W5100_S1_DHAR1   (W5100_S1_REGS_OFFSET + W5100_Sn_DHAR1  )
#define W5100_S1_DHAR2   (W5100_S1_REGS_OFFSET + W5100_Sn_DHAR2  )
#define W5100_S1_DHAR3   (W5100_S1_REGS_OFFSET + W5100_Sn_DHAR3  )
#define W5100_S1_DHAR4   (W5100_S1_REGS_OFFSET + W5100_Sn_DHAR4  )
#define W5100_S1_DHAR5   (W5100_S1_REGS_OFFSET + W5100_Sn_DHAR5  )
#define W5100_S1_DIPR0   (W5100_S1_REGS_OFFSET + W5100_Sn_DIPR0  )
#define W5100_S1_DIPR1   (W5100_S1_REGS_OFFSET + W5100_Sn_DIPR1  )
#define W5100_S1_DIPR2   (W5100_S1_REGS_OFFSET + W5100_Sn_DIPR2  )
#define W5100_S1_DIPR3   (W5100_S1_REGS_OFFSET + W5100_Sn_DIPR3  )
#define W5100_S1_DPORT0  (W5100_S1_REGS_OFFSET + W5100_Sn_DPORT0 )
#define W5100_S1_DPORT1  (W5100_S1_REGS_OFFSET + W5100_Sn_DPORT1 )
#define W5100_S1_MSSR0   (W5100_S1_REGS_OFFSET + W5100_Sn_MSSR0  )
#define W5100_S1_MSSR1   (W5100_S1_REGS_OFFSET + W5100_Sn_MSSR1  )
#define W5100_S1_SPROTO  (W5100_S1_REGS_OFFSET + W5100_Sn_SPROTO )
#define W5100_S1_TOS     (W5100_S1_REGS_OFFSET + W5100_Sn_TOS    )
#define W5100_S1_TTL     (W5100_S1_REGS_OFFSET + W5100_Sn_TTL    )
#define W5100_S1_TX_FSR0 (W5100_S1_REGS_OFFSET + W5100_Sn_TX_FSR0)
#define W5100_S1_TX_FSR1 (W5100_S1_REGS_OFFSET + W5100_Sn_TX_FSR1)
#define W5100_S1_TX_RD0  (W5100_S1_REGS_OFFSET + W5100_Sn_TX_RD0 )
#define W5100_S1_TX_RD1  (W5100_S1_REGS_OFFSET + W5100_Sn_TX_RD1 )
#define W5100_S1_TX_WR0  (W5100_S1_REGS_OFFSET + W5100_Sn_TX_WR0 )
#define W5100_S1_TX_WR1  (W5100_S1_REGS_OFFSET + W5100_Sn_TX_WR1 )
#define W5100_S1_RX_RSR0 (W5100_S1_REGS_OFFSET + W5100_Sn_RX_RSR0)
#define W5100_S1_RX_RSR1 (W5100_S1_REGS_OFFSET + W5100_Sn_RX_RSR1)

/* S2 */
#define W5100_S2_MR      (W5100_S2_REGS_OFFSET + W5100_Sn_MR     )
#define W5100_S2_CR      (W5100_S2_REGS_OFFSET + W5100_Sn_CR     )
#define W5100_S2_IR      (W5100_S2_REGS_OFFSET + W5100_Sn_IR     )
#define W5100_S2_SR      (W5100_S2_REGS_OFFSET + W5100_Sn_SR     )
#define W5100_S2_PORT0   (W5100_S2_REGS_OFFSET + W5100_Sn_PORT0  )
#define W5100_S2_PORT1   (W5100_S2_REGS_OFFSET + W5100_Sn_PORT1  )
#define W5100_S2_DHAR0   (W5100_S2_REGS_OFFSET + W5100_Sn_DHAR0  )
#define W5100_S2_DHAR1   (W5100_S2_REGS_OFFSET + W5100_Sn_DHAR1  )
#define W5100_S2_DHAR2   (W5100_S2_REGS_OFFSET + W5100_Sn_DHAR2  )
#define W5100_S2_DHAR3   (W5100_S2_REGS_OFFSET + W5100_Sn_DHAR3  )
#define W5100_S2_DHAR4   (W5100_S2_REGS_OFFSET + W5100_Sn_DHAR4  )
#define W5100_S2_DHAR5   (W5100_S2_REGS_OFFSET + W5100_Sn_DHAR5  )
#define W5100_S2_DIPR0   (W5100_S2_REGS_OFFSET + W5100_Sn_DIPR0  )
#define W5100_S2_DIPR1   (W5100_S2_REGS_OFFSET + W5100_Sn_DIPR1  )
#define W5100_S2_DIPR2   (W5100_S2_REGS_OFFSET + W5100_Sn_DIPR2  )
#define W5100_S2_DIPR3   (W5100_S2_REGS_OFFSET + W5100_Sn_DIPR3  )
#define W5100_S2_DPORT0  (W5100_S2_REGS_OFFSET + W5100_Sn_DPORT0 )
#define W5100_S2_DPORT1  (W5100_S2_REGS_OFFSET + W5100_Sn_DPORT1 )
#define W5100_S2_MSSR0   (W5100_S2_REGS_OFFSET + W5100_Sn_MSSR0  )
#define W5100_S2_MSSR1   (W5100_S2_REGS_OFFSET + W5100_Sn_MSSR1  )
#define W5100_S2_SPROTO  (W5100_S2_REGS_OFFSET + W5100_Sn_SPROTO )
#define W5100_S2_TOS     (W5100_S2_REGS_OFFSET + W5100_Sn_TOS    )
#define W5100_S2_TTL     (W5100_S2_REGS_OFFSET + W5100_Sn_TTL    )
#define W5100_S2_TX_FSR0 (W5100_S2_REGS_OFFSET + W5100_Sn_TX_FSR0)
#define W5100_S2_TX_FSR1 (W5100_S2_REGS_OFFSET + W5100_Sn_TX_FSR1)
#define W5100_S2_TX_RD0  (W5100_S2_REGS_OFFSET + W5100_Sn_TX_RD0 )
#define W5100_S2_TX_RD1  (W5100_S2_REGS_OFFSET + W5100_Sn_TX_RD1 )
#define W5100_S2_TX_WR0  (W5100_S2_REGS_OFFSET + W5100_Sn_TX_WR0 )
#define W5100_S2_TX_WR1  (W5100_S2_REGS_OFFSET + W5100_Sn_TX_WR1 )
#define W5100_S2_RX_RSR0 (W5100_S2_REGS_OFFSET + W5100_Sn_RX_RSR0)
#define W5100_S2_RX_RSR1 (W5100_S2_REGS_OFFSET + W5100_Sn_RX_RSR1)

/* S3 */
#define W5100_S3_MR      (W5100_S3_REGS_OFFSET + W5100_Sn_MR     )
#define W5100_S3_CR      (W5100_S3_REGS_OFFSET + W5100_Sn_CR     )
#define W5100_S3_IR      (W5100_S3_REGS_OFFSET + W5100_Sn_IR     )
#define W5100_S3_SR      (W5100_S3_REGS_OFFSET + W5100_Sn_SR     )
#define W5100_S3_PORT0   (W5100_S3_REGS_OFFSET + W5100_Sn_PORT0  )
#define W5100_S3_PORT1   (W5100_S3_REGS_OFFSET + W5100_Sn_PORT1  )
#define W5100_S3_DHAR0   (W5100_S3_REGS_OFFSET + W5100_Sn_DHAR0  )
#define W5100_S3_DHAR1   (W5100_S3_REGS_OFFSET + W5100_Sn_DHAR1  )
#define W5100_S3_DHAR2   (W5100_S3_REGS_OFFSET + W5100_Sn_DHAR2  )
#define W5100_S3_DHAR3   (W5100_S3_REGS_OFFSET + W5100_Sn_DHAR3  )
#define W5100_S3_DHAR4   (W5100_S3_REGS_OFFSET + W5100_Sn_DHAR4  )
#define W5100_S3_DHAR5   (W5100_S3_REGS_OFFSET + W5100_Sn_DHAR5  )
#define W5100_S3_DIPR0   (W5100_S3_REGS_OFFSET + W5100_Sn_DIPR0  )
#define W5100_S3_DIPR1   (W5100_S3_REGS_OFFSET + W5100_Sn_DIPR1  )
#define W5100_S3_DIPR2   (W5100_S3_REGS_OFFSET + W5100_Sn_DIPR2  )
#define W5100_S3_DIPR3   (W5100_S3_REGS_OFFSET + W5100_Sn_DIPR3  )
#define W5100_S3_DPORT0  (W5100_S3_REGS_OFFSET + W5100_Sn_DPORT0 )
#define W5100_S3_DPORT1  (W5100_S3_REGS_OFFSET + W5100_Sn_DPORT1 )
#define W5100_S3_MSSR0   (W5100_S3_REGS_OFFSET + W5100_Sn_MSSR0  )
#define W5100_S3_MSSR1   (W5100_S3_REGS_OFFSET + W5100_Sn_MSSR1  )
#define W5100_S3_SPROTO  (W5100_S3_REGS_OFFSET + W5100_Sn_SPROTO )
#define W5100_S3_TOS     (W5100_S3_REGS_OFFSET + W5100_Sn_TOS    )
#define W5100_S3_TTL     (W5100_S3_REGS_OFFSET + W5100_Sn_TTL    )
#define W5100_S3_TX_FSR0 (W5100_S3_REGS_OFFSET + W5100_Sn_TX_FSR0)
#define W5100_S3_TX_FSR1 (W5100_S3_REGS_OFFSET + W5100_Sn_TX_FSR1)
#define W5100_S3_TX_RD0  (W5100_S3_REGS_OFFSET + W5100_Sn_TX_RD0 )
#define W5100_S3_TX_RD1  (W5100_S3_REGS_OFFSET + W5100_Sn_TX_RD1 )
#define W5100_S3_TX_WR0  (W5100_S3_REGS_OFFSET + W5100_Sn_TX_WR0 )
#define W5100_S3_TX_WR1  (W5100_S3_REGS_OFFSET + W5100_Sn_TX_WR1 )
#define W5100_S3_RX_RSR0 (W5100_S3_REGS_OFFSET + W5100_Sn_RX_RSR0)
#define W5100_S3_RX_RSR1 (W5100_S3_REGS_OFFSET + W5100_Sn_RX_RSR1)


/* Multiple-byte registers */
#define W5100_GAR        W5100_GAR0
#define W5100_GAR_SIZE   4
#define W5100_SUBR       W5100_SUBR0
#define W5100_SUBR_SIZE  4
#define W5100_SHAR       W5100_SHAR0
#define W5100_SHAR_SIZE  6
#define W5100_SIPR       W5100_SIPR0
#define W5100_SIPR_SIZE  4
#define W5100_RTR        W5100_RTR0
#define W5100_RTR_SIZE   2
#define W5100_PATR       W5100_PATR0
#define W5100_PATR_SIZE  2
#define W5100_UIPR       W5100_UIPR0
#define W5100_UIPR_SIZE  4
#define W5100_UPORT      W5100_UPORT0
#define W5100_UPORT_SIZE 4

/* Generic socket registers */
#define W5100_Sn_PORT        W5100_Sn_PORT0
#define W5100_Sn_PORT_SIZE   2
#define W5100_Sn_DHAR        W5100_Sn_DHAR0
#define W5100_Sn_DHAR_SIZE   6
#define W5100_Sn_DIPR        W5100_Sn_DIPR0
#define W5100_Sn_DIPR_SIZE   4
#define W5100_Sn_DPORT       W5100_Sn_DPORT0
#define W5100_Sn_DPORT_SIZE  2
#define W5100_Sn_MSSR        W5100_Sn_MSSR0
#define W5100_Sn_MSSR_SIZE   2
#define W5100_Sn_TX_FSR      W5100_Sn_TX_FSR0
#define W5100_Sn_TX_FSR_SIZE 2
#define W5100_Sn_TX_RD       W5100_Sn_TX_RD0
#define W5100_Sn_TX_RD_SIZE  2
#define W5100_Sn_TX_WR       W5100_Sn_TX_WR0
#define W5100_Sn_TX_WR_SIZE  2
#define W5100_Sn_RX_RSR      W5100_Sn_RX_RSR0
#define W5100_Sn_RX_RSR_SIZE 2
//TODO

/* Modes */
#define W5100_MODE_IND      0x01
#define W5100_MODE_AI       0x02
#define W5100_MODE_PPPOE    0x08
#define W5100_MODE_PB       0x10
#define W5100_MODE_RST      0x80

/* Socket modes */
#define W5100_SOCK_MODE_CLOSED   0x00
#define W5100_SOCK_MODE_TCP      0x01
#define W5100_SOCK_MODE_UDP      0x02
#define W5100_SOCK_MODE_IPRAW    0x03
#define W5100_SOCK_MODE_MACRAW   0x04
#define W5100_SOCK_MODE_PPPOE    0x05
#define W5100_SOCK_MODE_ND       0x20
#define W5100_SOCK_MODE_MC       0x20
#define W5100_SOCK_MODE_MULTI    0x80

/* Commands */
#define W5100_CMD_OPEN      0x01
#define W5100_CMD_LISTEN    0x02
#define W5100_CMD_CONNECT   0x04
#define W5100_CMD_DISCON    0x08
#define W5100_CMD_CLOSE     0x10
#define W5100_CMD_SEND      0x20
#define W5100_CMD_SEND_MAC  0x21
#define W5100_CMD_SEND_KEEP 0x22
#define W5100_CMD_RECV      0x40

/* Socket states */
#define W5100_SOCK_CLOSED       0x00
#define W5100_SOCK_INIT         0x13
#define W5100_SOCK_LISTEN       0x14
#define W5100_SOCK_ESTABLISHED  0x17
#define W5100_SOCK_CLOSE_WAIT   0x1C
#define W5100_SOCK_UDP          0x22
#define W5100_SOCK_IPRAW        0x32
#define W5100_SOCK_MACRAW       0x42
#define W5100_SOCK_PPPOE        0x5F
#define W5100_SOCK_SYNSENT      0x15
#define W5100_SOCK_SYNRECV      0x16
#define W5100_SOCK_FIN_WAIT     0x18
#define W5100_SOCK_CLOSING      0x1A
#define W5100_SOCK_TIME_WAIT    0x1B
#define W5100_SOCK_LAST_ACK     0x1D
#define W5100_SOCK_ARP          0x11
#define W5100_SOCK_ARP1         0x11
#define W5100_SOCK_ARP2         0x21
#define W5100_SOCK_ARP3         0x31

/* Interrupts */
#define W5100_S0_INT        0x01
#define W5100_S1_INT        0x02
#define W5100_S2_INT        0x04
#define W5100_S3_INT        0x08
#define W5100_INT_PPPOE     0x20
#define W5100_INT_UNREACH   0x40
#define W5100_INT_CONFLICT  0x80

/* Socket interrupts */
#define W5100_INT_CON       0x01
#define W5100_INT_DISCON    0x02
#define W5100_INT_RECV      0x04
#define W5100_INT_TIMEOUT   0x08
#define W5100_INT_SEND_OK   0x10

extern
void w5100_read_mem(uint16_t addr, void *buf, size_t n);

extern
void w5100_write_mem(uint16_t addr, const void *buf, size_t n);

extern 
uint8_t w5100_read_reg(uint16_t reg);

extern 
uint16_t w5100_read_reg2(uint16_t reg);

extern 
void w5100_write_reg(uint16_t reg, uint8_t val);

extern 
void w5100_write_reg2(uint16_t reg, uint16_t val);

static inline
uint16_t w5100_sock_reg_get(uint16_t sn_reg, int socket)
{
    return
          W5100_S0_REGS_OFFSET
        + sn_reg
        + (socket * W5100_SOCKET_REGS_SIZE);
}

#define w5100_read_regx(reg, buf) w5100_read_mem((reg), (buf), reg ## _SIZE)

#define w5100_write_regx(reg, buf) w5100_write_mem((reg), (buf), reg ## _SIZE)

#define w5100_read_sock_regx(sn_reg, socket, buf) \
    w5100_read_mem( \
            w5100_sock_reg_get((sn_reg), (socket)), \
            (buf), \
            sn_reg ## _SIZE)

#define w5100_write_sock_regx(sn_reg, socket, buf) \
    w5100_write_mem( \
            w5100_sock_reg_get((sn_reg), (socket)), \
            (buf), \
            sn_reg ## _SIZE)

#define w5100_read_sock_reg(sn_reg, socket) \
    w5100_read_reg(w5100_sock_reg_get((sn_reg), (socket)))

#define w5100_write_sock_reg(sn_reg, socket, val) \
    w5100_write_reg(w5100_sock_reg_get((sn_reg), (socket)), (val))

extern
void w5100_init(void);

#endif /* W5100_H */

