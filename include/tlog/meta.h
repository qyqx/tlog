/*
 * Tlog JSON encoder metadata buffer.
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

#ifndef _TLOG_META_H
#define _TLOG_META_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <tlog/grc.h>
#include <tlog/trx.h>
#include <tlog/channel.h>

/** Minimum value of metadata buffer size */
#define TLOG_META_SIZE_MIN    32

/** Meta buffer */
struct tlog_meta {
    uint8_t                *buf;        /**< Buffer start */
    size_t                  size;       /**< Buffer size */
    uint8_t                *ptr;        /**< Writing position */

    struct timespec         first;      /**< First write timestamp */
    struct timespec         last;       /**< Last write timestamp */
    struct timespec         next;       /**< Next write timestamp */

    size_t                  delay_len;  /**< Next delay record length */

    bool                    written;    /**< Got a record written */

    struct tlog_channel     input;      /**< Input channel */
    struct tlog_channel     output;     /**< Output channel */
};

/** Meta transaction store */
TLOG_TRX_STORE_SIG(tlog_meta) {
    uint8_t            *ptr;        /**< Writing position */

    struct timespec     first;      /**< First write timestamp */
    struct timespec     last;       /**< Last write timestamp */
    struct timespec     next;       /**< Next write timestamp */

    size_t              delay_len;  /**< Next delay record length */

    bool                written;    /**< Got a record written */

    TLOG_TRX_STORE_SIG(tlog_channel)    input;  /**< Input channel */
    TLOG_TRX_STORE_SIG(tlog_channel)    output; /**< Output channel */
};

/** Transfer transaction data of a meta */
static inline TLOG_TRX_XFR_SIG(tlog_meta)
{
    TLOG_TRX_XFR_VAR(ptr);
    TLOG_TRX_XFR_VAR(first);
    TLOG_TRX_XFR_VAR(last);
    TLOG_TRX_XFR_VAR(next);
    TLOG_TRX_XFR_VAR(delay_len);
    TLOG_TRX_XFR_VAR(written);
    TLOG_TRX_XFR_OBJ(input);
    TLOG_TRX_XFR_OBJ(output);
}

/**
 * Initialize a meta.
 *
 * @param meta  The meta to initialize.
 * @param size  Size of meta buffer.
 *
 * @return Global return code.
 */
extern tlog_grc tlog_meta_init(struct tlog_meta *meta, size_t size);

/**
 * Check if a meta is valid.
 *
 * @param meta  The meta to check.
 *
 * @return True if the meta is valid, false otherwise.
 */
extern bool tlog_meta_is_valid(const struct tlog_meta *meta);

/**
 * Set timestamp for the next write and reserve total remaining space for it.
 *
 * @param meta  The meta to set timestamp for.
 * @param next  The timestamp of the next write, must be equal to or
 *              higher than the last timestamp set.
 * @param prem  Location of/for the total remaining output space.
 *
 * @return True if the future delay record with this timestamp will fit into
 *         the remaining output space.
 */
extern bool tlog_meta_set(struct tlog_meta *meta,
                          const struct timespec *next,
                          size_t *prem);

/**
 * Write a record to the end of the meta, prepended with a delay
 * record, if necessary. The delay record has space reserved for it by
 * tlog_meta_set and the supplied record should have space reserved for it
 * as well.
 *
 * @param meta  The meta to write the buffer to.
 * @param ptr   Pointer to the record to write.
 * @param len   Length of the record to write.
 */
extern void tlog_meta_write(struct tlog_meta *meta,
                            const uint8_t *ptr, size_t len);

/**
 * Empty a meta's contents.
 *
 * @param meta  The meta to empty.
 */
extern void tlog_meta_empty(struct tlog_meta *meta);

/**
 * Cleanup a meta (free any allocated data). Can be called repeatedly.
 *
 * @param meta  The meta to cleanup.
 */
extern void tlog_meta_cleanup(struct tlog_meta *meta);

#endif /* _TLOG_META_H */
