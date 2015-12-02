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

#include <tlog/fork.h>
#include <assert.h>
#include <string.h>

void
tlog_fork_init(struct tlog_fork        *fork,
               const char              *mark_str,
               struct tlog_dispatcher  *dispatcher)
{
    assert(fork != NULL);
    assert(mark_str != NULL);
    assert(mark_str[0] != mark_str[1]);
    assert(tlog_dispatcher_is_valid(dispatcher));

    memset(fork, 0, sizeof(*fork));

    fork->mark_ack = mark_str[0];
    fork->mark_nak = mark_str[1];
    fork->dispatcher = dispatcher;
}

bool
tlog_fork_is_valid(const struct tlog_fork *fork)
{
    return fork != NULL &&
           fork->mark_ack != fork->mark_nak &&
           tlog_dispatcher_is_valid(fork->dispatcher) &&
           (fork->len == 0 || fork->len < fork->dig);
}

bool
tlog_fork_account(struct tlog_fork         *fork,
                  const struct timespec    *ts,
                  bool                      ack,
                  size_t                    len)
{
    size_t new_len;
    size_t new_dig;
    size_t req;

    assert(tlog_fork_is_valid(fork));
    assert(ts != NULL);

    if (len == 0)
        return true;

    /* Advance time, possibly flushing this fork */
    if (!fork->dispatcher->advance(fork->dispatcher, ts))
        return false;

    /* If the runs differ */
    if (fork->len > 0 && ack != fork->ack) {
        /* Flush the previous run */
        tlog_fork_flush(fork);
    }

    new_len = fork->len;
    new_dig = fork->dig;

    /* If this is the start of a run */
    if (new_len == 0) {
        new_dig = 10;
        /* Reserve space for the marker and single digit run */
        req = 2;
    } else {
        req = 0;
    }

    /*
     * Account for extra digits necessary to represent the counter.
     * Optimize for being called frequently with low values of len.
     */
    do {
        new_len++;
        if (new_len >= new_dig) {
            req++;
            new_dig *= 10;
        }
    } while (--len > 0);

    /* Reserve (more) space for the run record */
    if (!fork->dispatcher->reserve(fork->dispatcher, req))
        return false;

    fork->len = new_len;
    fork->dig = new_dig;
    fork->ack = ack;

    return true;
}

bool
tlog_fork_reserve(struct tlog_fork *fork,
                  size_t            len)
{
    assert(tlog_fork_is_valid(fork));
    return (len == 0) ? true
                      : fork->dispatcher->reserve(fork->dispatcher, len);
}

void
tlog_fork_flush(struct tlog_fork *fork)
{
    assert(tlog_fork_is_valid(fork));

    if (fork->len == 0)
        return;
    
    fork->dispatcher->printf(fork->dispatcher, "%c%zu",
                             (fork->ack ? fork->mark_ack : fork->mark_nak),
                             fork->len);
    fork->len = 0;
    fork->dig = 0;
}
