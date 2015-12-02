/*
 * Tlog JSON encoder dispatcher interface.
 *
 * Copyright (C) 2015 Red Hat
 *
 * This file is part of tlog.
 *
 * Tlog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tlog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _TLOG_DISPATCHER_H
#define _TLOG_DISPATCHER_H

#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

/** Forward declaration of dispatcher */
struct tlog_dispatcher;

/**
 * Time-advancing function prototype.
 *
 * @param dispatcher    The dispatcher to advance time for.
 * @param ts            The time to advance to, must be equal or greater than
 *                      the previously advanced to.
 *
 * @return True if there was space to record the advanced time, false
 *         otherwise.
 */
typedef bool (*tlog_dispatcher_advance_fn)(struct tlog_dispatcher *dispatcher,
                                           const struct timespec *ts);
/**
 * Space-reservation function prototype.
 *
 * @param dispatcher    The dispatcher to reserve the space from.
 * @param len           The amount of space to reserve.
 *
 * @return True if there was enough space, false otherwise.
 */
typedef bool (*tlog_dispatcher_reserve_fn)(struct tlog_dispatcher *dispatcher,
                                           size_t len);
/**
 * Printing function prototype. Prints into the reserved space.
 *
 * @param dispatcher    The dispatcher to print to.
 * @param fmt           Format string to print with.
 * @param ...           Format arguments to print with.
 */
typedef void (*tlog_dispatcher_printf_fn)(struct tlog_dispatcher   *dispatcher,
                                          const char               *fmt,
                                          ...);

/** Fork */
struct tlog_dispatcher {
    tlog_dispatcher_advance_fn  advance;    /**< Time-advancing function */
    tlog_dispatcher_reserve_fn  reserve;    /**< Space-reservation function */
    tlog_dispatcher_printf_fn   printf;     /**< Printing function */
};

/**
 * Check if a dispatcher is valid.
 *
 * @param dispatcher    The dispatcher to check.
 *
 * @return True if the dispatcher is valid, false otherwise.
 */
static inline bool
tlog_dispatcher_is_valid(const struct tlog_dispatcher *dispatcher)
{
    return dispatcher != NULL &&
           dispatcher->advance != NULL &&
           dispatcher->reserve != NULL &&
           dispatcher->printf != NULL;
}

#endif /* _TLOG_DISPATCHER_H */
