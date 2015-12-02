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

#ifndef _TLOG_CHANNEL_H
#define _TLOG_CHANNEL_H

#include <tlog/dispatcher.h>
#include <tlog/fork.h>

/** Channel */
struct tlog_channel {
    struct tlog_fork        txt;        /**< Text fork */
    struct tlog_fork        bin;        /**< Binary fork */
};

/**
 * Initialize a channel.
 *
 * @param channel       Channel to initialize.
 * @param mark_str      A four-character string with first pair of characters
 *                      being valid and invalid markers for text runs, and the
 *                      second - for binary runs.
 * @param dispatcher    Dispatcher.
 */
extern void tlog_channel_init(struct tlog_channel      *channel,
                              const char               *mark_str,
                              struct tlog_dispatcher   *dispatcher);

/**
 * Check if a channel is valid.
 *
 * @param channel   The channel to check.
 *
 * @return True if the channel is valid, false otherwise.
 */
extern bool tlog_channel_is_valid(const struct tlog_channel *channel);

/**
 * Flush a channel.
 *
 * @param channel   The channel to flush.
 */
extern void tlog_channel_flush(struct tlog_channel *channel);

#endif /* _TLOG_CHANNEL_H */
