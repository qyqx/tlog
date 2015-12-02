/*
 * Tlog I/O buffer.
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

#include <errno.h>
#include <stdio.h>
#include <tlog/chunk.h>

/**
 * Advance meta time.
 *
 * @param dispatcher    The dispatcher to advance time for.
 * @param ts            The time to advance to, must be equal or greater than
 *                      the previously advanced to.
 *
 * @return True if there was space to record the advanced time, false
 *         otherwise.
 */
static bool
tlog_chunk_dispatcher_advance(struct tlog_dispatcher *dispatcher,
                              const struct timespec *ts)
{
    struct tlog_chunk *chunk = TLOG_CONTAINER_OF(dispatcher,
                                                 struct tlog_chunk,
                                                 dispatcher);
    assert(tlog_chunk_is_valid(chunk));
}

/**
 * Reserve space
 *
 * @param dispatcher    The dispatcher to reserve the space from.
 * @param len           The amount of space to reserve.
 *
 * @return True if there was enough space, false otherwise.
 */
static bool
tlog_chunk_dispatcher_reserve(struct tlog_dispatcher *dispatcher, size_t len)
{
    struct tlog_chunk *chunk = TLOG_CONTAINER_OF(dispatcher,
                                                 struct tlog_chunk,
                                                 dispatcher);
    assert(tlog_chunk_is_valid(chunk));
    if (len > chunk->rem)
        return false;
    chunk->rem -= len;
    return true;
}

/**
 * Print a record into the reserved space in the meta buffer.
 *
 * @param dispatcher    The dispatcher to print to.
 * @param fmt           Format string to print with.
 * @param ...           Format arguments to print with.
 */
static void
tlog_chunk_dispatcher_printf(struct tlog_dispatcher   *dispatcher,
                             const char               *fmt,
                             ...)
{
    struct tlog_chunk *chunk = TLOG_CONTAINER_OF(dispatcher,
                                                 struct tlog_chunk,
                                                 dispatcher);
    assert(tlog_chunk_is_valid(chunk));
}

tlog_grc
tlog_chunk_init(struct tlog_chunk *chunk, size_t size)
{
    tlog_grc grc;

    assert(chunk != NULL);
    assert(size >= TLOG_CHUNK_SIZE_MIN);

    memset(chunk, 0, sizeof(*chunk));

    chunk->size = size;
    chunk->rem = size;

    grc = tlog_meta_init(&chunk->meta, size);
    if (grc != TLOG_RC_OK)
        goto error;
    grc = tlog_stream_init(&chunk->input, size, '<', '[');
    if (grc != TLOG_RC_OK)
        goto error;
    grc = tlog_stream_init(&chunk->output, size, '>', ']');
    if (grc != TLOG_RC_OK)
        goto error;

    assert(tlog_chunk_is_valid(chunk));
    return TLOG_RC_OK;

error:
    tlog_chunk_cleanup(chunk);
    return grc;
}

bool
tlog_chunk_is_valid(const struct tlog_chunk *chunk)
{
    return chunk != NULL &&
           chunk->size >= TLOG_CHUNK_SIZE_MIN &&
           tlog_meta_is_valid(&chunk->meta) &&
           tlog_stream_is_valid(&chunk->input) &&
           tlog_stream_is_valid(&chunk->output) &&
           chunk->rem <= chunk->size &&
           ((chunk->meta.ptr - chunk->meta.buf) +
            chunk->input.txt_len + chunk->input.bin_len +
            chunk->output.txt_len + chunk->output.bin_len) <=
                (chunk->size - chunk->rem);
}

bool
tlog_chunk_is_pending(const struct tlog_chunk *chunk)
{
    assert(tlog_chunk_is_valid(chunk));
    return tlog_stream_is_pending(&chunk->input) ||
           tlog_stream_is_pending(&chunk->output);
}

bool
tlog_chunk_is_empty(const struct tlog_chunk *chunk)
{
    assert(tlog_chunk_is_valid(chunk));
    return chunk->rem >= chunk->size;
}

/** A packet write status */
enum tlog_chunk_write_status {
    TLOG_CHUNK_WRITE_STATUS_IGNORED = -1,   /**< Write was ignored completely */
    TLOG_CHUNK_WRITE_STATUS_VOID,           /**< Write didn't fit at all */
    TLOG_CHUNK_WRITE_STATUS_INCOMPLETE,     /**< Write was incomplete */
    TLOG_CHUNK_WRITE_STATUS_COMPLETED,      /**< Write completed */
};

/**
 * Write a window packet payload to a chunk.
 *
 * @param chunk     The chunk to write to.
 * @param pkt       The packet to write the payload of.
 * @param ppos      Location of position in the packet the write should start
 *                  at (set to 0 on first write) / location for (opaque)
 *                  position in the packet the write ended at.
 *
 * @return Packet write status.
 */
static enum tlog_chunk_write_status
tlog_chunk_write_window(struct tlog_chunk *chunk,
                        const struct tlog_pkt *pkt,
                        size_t *ppos)
{
    unsigned short int width;
    unsigned short int height;
    /* Window string buffer (max: "=65535x65535") */
    uint8_t buf[16];
    int rc;
    size_t len;

    assert(tlog_chunk_is_valid(chunk));
    assert(tlog_pkt_is_valid(pkt));
    assert(pkt->type == TLOG_PKT_TYPE_WINDOW);
    assert(ppos != NULL);
    assert(*ppos <= 1);

    /* If there's nothing left to write */
    if (*ppos >= 1)
        return false;

    width = pkt->data.window.width;
    height = pkt->data.window.height;

    /* If window size is the same */
    if (chunk->got_window) {
        if (width == chunk->last_width && height == chunk->last_height)
            return TLOG_CHUNK_WRITE_STATUS_IGNORED;
    }

    rc = snprintf((char *)buf, sizeof(buf), "=%hux%hu", width, height);
    if (rc < 0) {
        assert(false);
        return TLOG_CHUNK_WRITE_STATUS_VOID;
    }

    len = (size_t)rc;
    if (len >= sizeof(buf)) {
        assert(false);
        return TLOG_CHUNK_WRITE_STATUS_VOID;
    }

    if (len > chunk->rem)
        return TLOG_CHUNK_WRITE_STATUS_VOID;

    tlog_stream_flush(&chunk->input, &chunk->meta);
    tlog_stream_flush(&chunk->output, &chunk->meta);

    chunk->rem -= len;
    tlog_meta_write(&chunk->meta, buf, len);

    chunk->got_window = true;
    chunk->last_width = width;
    chunk->last_height = height;
    *ppos = 1;
    return TLOG_CHUNK_WRITE_STATUS_COMPLETED;
}

/**
 * Write an I/O packet payload to a chunk.
 *
 * @param chunk     The chunk to write to.
 * @param pkt       The packet to write the payload of.
 * @param ppos      Location of position in the packet the write should start
 *                  at (set to 0 on first write) / location for (opaque)
 *                  position in the packet the write ended at.
 *
 * @return Packet write status.
 */
static enum tlog_chunk_write_status
tlog_chunk_write_io(struct tlog_chunk *chunk,
                    const struct tlog_pkt *pkt,
                    size_t *ppos)
{
    const uint8_t *buf;
    size_t len;
    size_t written;

    assert(tlog_chunk_is_valid(chunk));
    assert(tlog_pkt_is_valid(pkt));
    assert(pkt->type == TLOG_PKT_TYPE_IO);
    assert(ppos != NULL);
    assert(*ppos <= pkt->data.io.len);

    /* If there's nothing left to write */
    if (*ppos >= pkt->data.io.len)
        return TLOG_CHUNK_WRITE_STATUS_IGNORED;

    buf = pkt->data.io.buf + *ppos;
    len = pkt->data.io.len - *ppos;
    written = tlog_stream_write(pkt->data.io.output
                                    ? &chunk->output
                                    : &chunk->input,
                                &buf, &len,
                                &chunk->meta, &chunk->rem);
    *ppos += written;

    return len == 0 ? TLOG_CHUNK_WRITE_STATUS_COMPLETED
                    : (written != 0 ? TLOG_CHUNK_WRITE_STATUS_INCOMPLETE
                                    : TLOG_CHUNK_WRITE_STATUS_VOID);
}

bool
tlog_chunk_write(struct tlog_chunk *chunk,
                 const struct tlog_pkt *pkt,
                 size_t *ppos)
{
    tlog_trx trx = TLOG_TRX_INIT;
    TLOG_TRX_STORE_DECL(tlog_chunk);
    enum tlog_chunk_write_status status;

    assert(tlog_chunk_is_valid(chunk));
    assert(tlog_pkt_is_valid(pkt));
    assert(!tlog_pkt_is_void(pkt));
    assert(ppos != NULL);

    TLOG_TRX_BEGIN(&trx, tlog_chunk, chunk);

    /* Record the timestamp, if the delay fits */
    if (!tlog_meta_set(&chunk->meta, &pkt->timestamp, &chunk->rem)) {
        TLOG_TRX_ABORT(&trx, tlog_chunk, chunk);
        return false;
    }

    /* Write (a part of) the packet */
    switch (pkt->type) {
        case TLOG_PKT_TYPE_IO:
            status = tlog_chunk_write_io(chunk, pkt, ppos);
            break;
        case TLOG_PKT_TYPE_WINDOW:
            status = tlog_chunk_write_window(chunk, pkt, ppos);
            break;
        default:
            assert(false);
            status = TLOG_CHUNK_WRITE_STATUS_VOID;
            break;
    }

    /* If something was written */
    if (status >= TLOG_CHUNK_WRITE_STATUS_INCOMPLETE) {
        /* Accept all writes */
        TLOG_TRX_COMMIT(&trx);
        return status == TLOG_CHUNK_WRITE_STATUS_COMPLETED;
    } else {
        /* Revert the possible timestamp writing */
        TLOG_TRX_ABORT(&trx, tlog_chunk, chunk);
        /* Report "ignored" as "completed" */
        return status == TLOG_CHUNK_WRITE_STATUS_IGNORED;
    }
}

void
tlog_chunk_flush(struct tlog_chunk *chunk)
{
    assert(tlog_chunk_is_valid(chunk));
    tlog_stream_flush(&chunk->input, &chunk->meta);
    tlog_stream_flush(&chunk->output, &chunk->meta);
}

bool
tlog_chunk_cut(struct tlog_chunk *chunk)
{
    tlog_trx trx = TLOG_TRX_INIT;
    TLOG_TRX_STORE_DECL(tlog_chunk);

    assert(tlog_chunk_is_valid(chunk));

    TLOG_TRX_BEGIN(&trx, tlog_chunk, chunk);

    /* Cut the streams */
    if (!tlog_stream_cut(&chunk->input, &chunk->meta, &chunk->rem) ||
        !tlog_stream_cut(&chunk->output, &chunk->meta, &chunk->rem)) {
        TLOG_TRX_ABORT(&trx, tlog_chunk, chunk);
        return false;
    }

    TLOG_TRX_COMMIT(&trx);
    return true;
}

void
tlog_chunk_empty(struct tlog_chunk *chunk)
{
    assert(tlog_chunk_is_valid(chunk));
    chunk->rem = chunk->size;
    tlog_meta_empty(&chunk->meta);
    tlog_stream_empty(&chunk->input);
    tlog_stream_empty(&chunk->output);
    chunk->got_window = false;
    assert(tlog_chunk_is_valid(chunk));
}

void
tlog_chunk_cleanup(struct tlog_chunk *chunk)
{
    assert(chunk != NULL);
    tlog_meta_cleanup(&chunk->meta);
    tlog_stream_cleanup(&chunk->input);
    tlog_stream_cleanup(&chunk->output);
}
