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
#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <time.h>
#include <netinet/in.h>

/*
 * timesync synchronizes CLOCK_REALTIME to a time synchronization source.
 * It uses clock_gettime to take current time, then decides if too much
 * time has passed since last synchronization and in case it executes it,
 * then uses clock_settime to apply the new time setting.
 *
 * With timesync_timespec the current time must be
 * passed as parameter. The current time after synchronization is written
 * back in the same parameter.
 *
 * timesync_now forces synchronization, and timesync_now_timespec also
 * returns the current time into the parameter.
 *
 * All functions return 1 when synchronization has been successful,
 * and -1 when an error occurred during synchronization. timesync and
 * timesync_timespec return 0 when it is not time to synchronize yet.
 */ 

extern
int timesync(void);

extern
int timesync_timespec(struct timespec *now);

extern
int timesync_now(void);

extern
int timesync_now_timespec(struct timespec *out);

/* Get time from a RFC868 server.
 * The time is written in the timespec structure pointed by ts parameter.
 * Returns 0 if successful, -1 if some error happened. 
 */
extern
int rfc868_gettime(struct timespec *ts);

/* Set the time server to retrieve time with RFC868 */
extern
void rfc868_timeserver_set(in_addr_t server);

/* Get the time server used to retrieve time with RFC868 */
extern
in_addr_t rfc868_timeserver_get(void);

/* Get time from a RFC4330 SNTP server.
 * The time is written in the timespec structure pointed by ts parameter.
 * Returns 0 if successful, -1 if some error happened. 
 */
extern
int sntp_gettime(struct timespec *ts);

/* Set the time server to retrieve time with RFC4330 */
extern
void sntp_timeserver_set(in_addr_t server);

/* Get the time server used to retrieve time with RFC4330 */
extern
in_addr_t sntp_timeserver_get(void);


#endif /* TIMESYNC_H */

