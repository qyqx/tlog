/*
 * Tlog JSON encoder meta buffer.
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

#include <tlog/meta.h>
#include <tlog/misc.h>
#include <tlog/rc.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

tlog_grc
tlog_meta_init(struct tlog_meta *meta, size_t size)
{
    tlog_grc grc;
    assert(meta != NULL);
    assert(size >= TLOG_META_SIZE_MIN);

    memset(meta, 0, sizeof(*meta));

    meta->size = size;
    meta->buf = malloc(meta->size);
    if (meta->buf == NULL) {
        grc = TLOG_GRC_ERRNO;
        goto error;
    }
    meta->ptr = meta->buf;

    return TLOG_RC_OK;

error:
    tlog_meta_cleanup(meta);
    return grc;
}

bool
tlog_meta_is_valid(const struct tlog_meta *meta)
{
    return meta != NULL &&
           meta->size >= TLOG_META_SIZE_MIN &&
           meta->buf != NULL &&
           meta->ptr >= meta->buf &&
           (size_t)(meta->ptr - meta->buf) <= meta->size &&
           (!meta->written ||
            (tlog_timespec_cmp(&meta->first, &meta->last) <= 0 &&
             tlog_timespec_cmp(&meta->last, &meta->next) <= 0));
}

void
tlog_meta_cleanup(struct tlog_meta *meta)
{
    assert(meta != NULL);
    free(meta->buf);
    memset(meta, 0, sizeof(*meta));
}

/**
 * Format a record of a delay between two timestamps into a buffer.
 *
 * @param buf   The buffer to write the formatted record into.
 * @param len   The length of the buffer to write the formatted record into.
 * @param prev  The older timestamp.
 * @param next  The newer timestamp.
 *
 * @return How many characters the formatted record (would have) occupied.
 */
static size_t
tlog_meta_delay_format(uint8_t *buf, size_t len,
                       const struct timespec *prev,
                       const struct timespec *next)
{
    struct timespec delay;
    int rc;
    long sec;
    long msec;

    assert(buf != NULL || len == 0);
    assert(prev != NULL);
    assert(next != NULL);
    assert(tlog_timespec_cmp(prev, next) <= 0);

    tlog_timespec_sub(next, prev, &delay);
    sec = (long)delay.tv_sec;
    msec = delay.tv_nsec / 1000000;

    if (sec != 0) {
        rc = snprintf((char *)buf, len, "+%ld%03ld", sec, msec);
    } else if (msec != 0) {
        rc = snprintf((char *)buf, len, "+%ld", msec);
    } else {
        rc = 0;
    }
    assert(rc >= 0);

    return (size_t)rc;
}

bool
tlog_meta_set(struct tlog_meta *meta,
              const struct timespec *next,
              size_t *prem)
{
    assert(tlog_meta_is_valid(meta));
    assert(next != NULL);

    if (meta->written) {
        size_t new_delay_len;
        size_t delay_len_inc;

        assert(tlog_timespec_cmp(&meta->next, next) <= 0);

        new_delay_len = tlog_meta_delay_format(NULL, 0, &meta->last, next);
        assert(new_delay_len >= meta->delay_len);
        delay_len_inc = new_delay_len - meta->delay_len;
        if (delay_len_inc > *prem)
            return false;

        *prem -= delay_len_inc;
        meta->delay_len += delay_len_inc;
    }

    meta->next = *next;

    return true;
}

void
tlog_meta_write(struct tlog_meta *meta, const uint8_t *ptr, size_t len)
{
    assert(tlog_meta_is_valid(meta));
    assert(ptr != NULL || len == 0);
    assert((meta->ptr - meta->buf) +
           meta->delay_len + len <= meta->size);

    if (len == 0)
        return;

    if (meta->written) {
        meta->ptr += tlog_meta_delay_format(
                            meta->ptr,
                            meta->size - (meta->ptr - meta->buf),
                            &meta->last, &meta->next);
    } else {
        meta->first = meta->next;
        meta->written = true;
    }
    meta->last = meta->next;
    meta->delay_len = 0;

    memcpy(meta->ptr, ptr, len);
    meta->ptr += len;
}

void
tlog_meta_empty(struct tlog_meta *meta)
{
    assert(tlog_meta_is_valid(meta));
    meta->ptr = meta->buf;
    tlog_timespec_zero(&meta->first);
    tlog_timespec_zero(&meta->last);
    tlog_timespec_zero(&meta->next);
    meta->delay_len = 0;
    meta->written = false;
    assert(tlog_meta_is_valid(meta));
}
