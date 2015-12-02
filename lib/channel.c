/*
 * Tlog JSON encoder channel.
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

#include <tlog/channel.h>
#include <assert.h>

/**
 * Check if a mark string contains only unique characters.
 *
 * @param mark_str  Mark string to check.
 *
 * @return True if the characters are unique, false otherwise.
 */
static bool
tlog_channel_mark_str_is_diverse(const char *mark_str)
{
    size_t i;
    size_t j;

    for (i = 0; i < 3; i++)
        for (j = i + 1; j < 4; j++)
            if (mark_str[i] == mark_str[j])
                return false;

    return true;
}

void
tlog_channel_init(struct tlog_channel      *channel,
                  const char               *mark_str,
                  struct tlog_dispatcher   *dispatcher)
{
    assert(channel != NULL);
    assert(mark_str != NULL);
    assert(tlog_channel_mark_str_is_diverse(mark_str));
    assert(tlog_dispatcher_is_valid(dispatcher));

    tlog_fork_init(&channel->txt, mark_str + 0, dispatcher);
    tlog_fork_init(&channel->bin, mark_str + 2, dispatcher);
}

bool
tlog_channel_is_valid(const struct tlog_channel *channel)
{
    return channel != NULL &&
           tlog_fork_is_valid(&channel->txt) &&
           tlog_fork_is_valid(&channel->bin);
}

void
tlog_channel_flush(struct tlog_channel *channel)
{
    assert(tlog_channel_is_valid(channel));
    tlog_fork_flush(&channel->txt);
    tlog_fork_flush(&channel->bin);
}
