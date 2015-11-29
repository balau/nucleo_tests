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
#include <stdio.h>
#include <time.h>

int main(void)
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    printf("%lds %ldns\n", (long)t.tv_sec, (long)t.tv_nsec);

    t.tv_sec = 0;
    t.tv_nsec = 1000000;
    nanosleep(&t, NULL);
    
    clock_gettime(CLOCK_MONOTONIC, &t);
    printf("%lds %ldns\n", (long)t.tv_sec, (long)t.tv_nsec);

    while(1);
}


