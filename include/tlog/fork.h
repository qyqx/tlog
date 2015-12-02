/*
 * Tlog JSON encoder fork.
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

#ifndef _TLOG_FORK_H
#define _TLOG_FORK_H

#include <tlog/dispatcher.h>
#include <tlog/dispatcher.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

/** Fork */
struct tlog_fork {
    uint8_t                 mark_ack;   /**< Valid run marker */
    uint8_t                 mark_nak;   /**< Invalid run marker */

    struct tlog_dispatcher *dispatcher; /**< Dispatcher */

    size_t                  len;        /**< Run length */
    bool                    ack;        /**< Run is valid */
    size_t                  dig;        /**< Run length digit limit */
};

/**
 * Initialize a fork.
 *
 * @param fork          Fork to initialize.
 * @param mark_str      A two-character string with first character a valid
 *                      run marker, and the second - an invalid run marker.
 * @param dispatcher    Dispatcher.
 */
extern void tlog_fork_init(struct tlog_fork        *fork,
                           const char              *mark_str,
                           struct tlog_dispatcher  *dispatcher);

/**
 * Check if a fork is valid.
 *
 * @param fork  The fork to check.
 *
 * @return True if the fork is valid, false otherwise.
 */
extern bool tlog_fork_is_valid(const struct tlog_fork *fork);

/**
 * Account decoded data piece.
 *
 * @param fork  The fork to account for.
 * @param ts    The data piece timestamp.
 * @param ack   True if the data piece is valid.
 * @param len   Length of the data piece.
 *
 * @return True if there was enough space to account the data piece,
 *         false otherwise.
 */
extern bool tlog_fork_account(struct tlog_fork         *fork,
                              const struct timespec    *ts,
                              bool                      ack,
                              size_t                    len);

/**
 * Reserve space for encoded data piece.
 * 
 * @param fork  The fork to reserve for.
 * @param len   Length of the data piece.
 *
 * @return True if there was enought space, false otherwise.
 */
extern bool tlog_fork_reserve(struct tlog_fork *fork,
                              size_t            len);

/**
 * Flush a fork.
 *
 * @param fork  The fork to flush.
 */
extern void tlog_fork_flush(struct tlog_fork *fork);

#endif /* _TLOG_FORK_H */
