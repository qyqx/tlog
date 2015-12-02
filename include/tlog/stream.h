/*
 * Tlog I/O stream.
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

#ifndef _TLOG_STREAM_H
#define _TLOG_STREAM_H

#include <tlog/grc.h>
#include <tlog/utf8.h>
#include <tlog/trx.h>
#include <tlog/channel.h>

#define TLOG_STREAM_SIZE_MIN    32

/** I/O stream */
struct tlog_stream {
    struct tlog_channel    *channel;    /**< Channel */

    size_t                  size;       /**< Text/binary encoded buffer size */

    struct tlog_utf8        utf8;       /**< UTF-8 filter */

    uint8_t                *txt_buf;    /**< Encoded text buffer */
    size_t                  txt_len;    /**< Text output length in bytes */

    uint8_t                *bin_buf;    /**< Encoded binary buffer */
    size_t                  bin_len;    /**< Binary output length in bytes */
};

/** Stream transaction store */
TLOG_TRX_STORE_SIG(tlog_stream) {
    struct tlog_utf8    utf8;           /**< UTF-8 filter */
    size_t              txt_len;        /**< Text output length in bytes */
    size_t              bin_len;        /**< Binary output length in bytes */
};

/** Transfer transaction data of a stream */
static inline TLOG_TRX_XFR_SIG(tlog_stream)
{
    TLOG_TRX_XFR_VAR(utf8);
    TLOG_TRX_XFR_VAR(txt_len);
    TLOG_TRX_XFR_VAR(bin_len);
}

/**
 * Initialize a stream.
 *
 * @param stream    The stream to initialize.
 * @param channel   Channel to use.
 * @param size      Text/binary buffer size.
 *
 * @return Global return code.
 */
extern tlog_grc tlog_stream_init(struct tlog_stream *stream,
                                 struct tlog_channel *channel,
                                 size_t size);

/**
 * Check if a stream is valid.
 *
 * @param stream    The stream to check.
 *
 * @return True if the stream is valid, false otherwise.
 */
extern bool tlog_stream_is_valid(const struct tlog_stream *stream);

/**
 * Check if a stream has an incomplete character pending.
 *
 * @param stream    The stream to check.
 *
 * @return True if the stream has an incomplete character pending, false
 *         otherwise.
 */
extern bool tlog_stream_is_pending(const struct tlog_stream *stream);

/**
 * Check if a stream is empty (no data in buffers, except the possibly
 * pending incomplete character).
 *
 * @param stream    The stream to check.
 *
 * @return True if the stream is empty, false otherwise.
 */
extern bool tlog_stream_is_empty(const struct tlog_stream *stream);

/**
 * Write to a stream, accounting for any (potentially) used total remaining
 * space.
 *
 * @param stream    The stream to write to.
 * @param ts        The write timestamp.
 * @param pbuf      Location of/for the pointer to the data to write.
 * @param plen      Location of/for the length of the data to write.
 *
 * @return Number of input bytes written.
 */
extern size_t tlog_stream_write(struct tlog_stream *stream,
                                const struct timespec *ts,
                                const uint8_t **pbuf, size_t *plen);

/**
 * Flush a stream - write metadata record to reserved space and reset runs.
 *
 * @param stream    The stream to flush.
 */
extern void tlog_stream_flush(struct tlog_stream *stream);

/**
 * Cut a stream - write pending incomplete character to the buffers.
 *
 * @param stream    The stream to cut.
 * @param ts        The cut timestamp.
 *
 * @return True if incomplete character fit into the remaining space, false
 *         otherwise.
 */
extern bool tlog_stream_cut(struct tlog_stream *stream,
                            const struct timespec *ts);

/**
 * Empty buffers of a stream, but not pending incomplete characters.
 *
 * @param stream    The stream to empty.
 */
extern void tlog_stream_empty(struct tlog_stream *stream);

/**
 * Cleanup a stream. Can be called repeatedly.
 *
 * @param stream    The stream to cleanup.
 */
extern void tlog_stream_cleanup(struct tlog_stream *stream);

#endif /* _TLOG_STREAM_H */
