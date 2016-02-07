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

int timesync(void);

int timesync_timespec(struct timespec *now);

/* Get time from a RFC868 server.
 * The time is written in the timespec structure pointed by ts parameter.
 * Returns 0 if successful, -1 if some error happened. 
 */
int rfc868_gettime(struct timespec *ts);

/* Set the time server to retrieve time with RFC868 */
void rfc868_timeserver_set(in_addr_t server);

/* Get the time server used to retrieve time with RFC868 */
in_addr_t rfc868_timeserver_get(void);

#endif /* TIMESYNC_H */

