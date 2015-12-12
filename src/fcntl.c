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
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include "file.h"

int fcntl(int fildes, int cmd, ...)
{
    va_list ap;
    struct fd *f;
    int ret;

    va_start(ap, cmd);
    
    f = file_struct_get(fildes);
    if (f == NULL)
    {
        errno = EBADF;
        ret = -1;
    }
    else
    {
        switch(cmd)
        {
            case F_GETFD:
                ret = f->descriptor_flags;
                break;
            case F_SETFD:
                f->descriptor_flags = va_arg(ap, int);
                ret = 0;
                break;
            case F_SETFL:
                f->status_flags = va_arg(ap, int);
                ret = 0;
                break;
            case F_GETFL:
                ret = f->status_flags;
                break;
            default:
                errno = ENOSYS; /* function not implemented */
                ret = -1;
                break;
        }
    }

    va_end(ap);

    return ret;
}



