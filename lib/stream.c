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

#include <errno.h>
#include <stdio.h>
#include <tlog/misc.h>
#include <tlog/stream.h>

void
tlog_stream_cleanup(struct tlog_stream *stream)
{
    assert(stream != NULL);
    free(stream->txt_buf);
    stream->txt_buf = NULL;
    free(stream->bin_buf);
    stream->bin_buf = NULL;
}

bool
tlog_stream_is_valid(const struct tlog_stream *stream)
{
    return stream != NULL &&
           tlog_channel_is_valid(stream->channel) &&
           stream->size >= TLOG_STREAM_SIZE_MIN &&
           tlog_utf8_is_valid(&stream->utf8) &&
           stream->txt_buf != NULL &&
           stream->bin_buf != NULL &&
           (stream->txt_len + stream->bin_len) <= stream->size;
}

bool
tlog_stream_is_pending(const struct tlog_stream *stream)
{
    assert(tlog_stream_is_valid(stream));
    assert(!tlog_utf8_is_ended(&stream->utf8));
    return tlog_utf8_is_started(&stream->utf8);
}

bool
tlog_stream_is_empty(const struct tlog_stream *stream)
{
    assert(tlog_stream_is_valid(stream));
    return stream->txt_len == 0 && stream->bin_len == 0;
}

tlog_grc
tlog_stream_init(struct tlog_stream *stream,
                 struct tlog_channel *channel,
                 size_t size)
{
    tlog_grc grc;
    assert(stream != NULL);
    assert(tlog_channel_is_valid(channel));
    assert(size >= TLOG_STREAM_SIZE_MIN);

    memset(stream, 0, sizeof(*stream));

    stream->size = size;
    stream->channel = channel;

    stream->txt_buf = malloc(size);
    if (stream->txt_buf == NULL) {
        grc = TLOG_GRC_ERRNO;
        goto error;
    }

    stream->bin_buf = malloc(size);
    if (stream->bin_buf == NULL) {
        grc = TLOG_GRC_ERRNO;
        goto error;
    }

    assert(tlog_stream_is_valid(stream));
    return TLOG_RC_OK;
error:
    tlog_stream_cleanup(stream);
    return grc;
}

/**
 * Print a byte to a buffer in decimal, as much as it fits.
 *
 * @param buf   The buffer pointer.
 * @param len   The buffer length.
 * @param b     The byte to print.
 *
 * @return Length of the number (to be) printed.
 */
size_t
tlog_stream_btoa(uint8_t *buf, size_t len, uint8_t b)
{
    size_t l = 0;
    if (b >= 100) {
        if (len > 0) {
            len--;
            *buf++ = '0' + b/100;
        }
        b %= 100;
        l++;
    }
    if (b >= 10) {
        if (len > 0) {
            len--;
            *buf++ = '0' + b/10;
        }
        b %= 10;
        l++;
    } else if (l > 0) {
        if (len > 0) {
            len--;
            *buf++ = '0';
        }
        l++;
    }
    if (len > 0)
        *buf = '0' + b;
    l++;
    return l;
}

#define ACC(_l) \
    do {                                            \
        if (!tlog_fork_account(fork, ts, ack, _l))  \
            return false;                           \
    } while (0)

#define ADV(_l) \
    do {                                    \
        if (!tlog_fork_reserve(fork, _l))   \
            return false;                   \
        olen += (_l);                       \
    } while (0)

/**
 * Encode an byte sequence into a JSON array buffer atomically.
 *
 * @param fork  The fork to use.
 * @param ts    The byte sequence timestamp.
 * @param ack   True if the sequence is valid.
 * @param optr  Output pointer.
 * @param polen Location of/for output byte counter.
 * @param ibuf  Input buffer.
 * @param ilen  Input length.
 *
 * @return True if both the bytes and the new counter did fit into the
 *         remaining output space.
 */
bool
tlog_stream_enc_bin(struct tlog_fork *fork, const struct timespec *ts, bool ack,
                    uint8_t *optr, size_t *polen,
                    const uint8_t *ibuf, size_t ilen)
{
    size_t olen;
    size_t l;

    assert(tlog_fork_is_valid(fork));
    assert(ts != NULL);
    assert(optr != NULL);
    assert(polen != NULL);
    assert(ibuf != NULL || ilen == 0);

    if (ilen == 0)
        return true;

    olen = *polen;

    for (; ilen > 0; ilen--) {
        ACC(1);
        if (olen > 0) {
            ADV(1);
            *optr++ = ',';
        }
        l = tlog_stream_btoa(NULL, 0, *ibuf);
        ADV(l);
        tlog_stream_btoa(optr, l, *ibuf++);
        optr += l;
    }

    *polen = olen;
    return true;
}

/**
 * Encode a character byte sequence into a JSON string buffer atomically.
 *
 * @param fork  The fork to use.
 * @param ts    The byte sequence timestamp.
 * @param ack   True if the sequence is valid.
 * @param optr  Output pointer.
 * @param polen Location of/for output byte counter.
 * @param ibuf  Input buffer.
 * @param ilen  Input length.
 *
 * @return True if both the character and the new counter did fit into the
 *         remaining output space.
 */
bool
tlog_stream_enc_txt(struct tlog_fork *fork, const struct timespec *ts, bool ack,
                    uint8_t *optr, size_t *polen,
                    const uint8_t *ibuf, size_t ilen)
{
    uint8_t c;
    size_t olen;

    assert(tlog_fork_is_valid(fork));
    assert(ts != NULL);
    assert(optr != NULL);
    assert(polen != NULL);
    assert(ibuf != NULL || ilen == 0);

    if (ilen == 0)
        return true;

    olen = *polen;

    ACC(1);

    if (ilen > 1) {
        ADV(ilen);
        memcpy(optr, ibuf, ilen);
    } else {
        c = *ibuf;
        switch (c) {
#define ESC_CASE(_c, _e) \
        case _c:            \
            ADV(2);        \
            *optr++ = '\\'; \
            *optr = _e;   \
            break;
        ESC_CASE('"', c);
        ESC_CASE('\\', c);
        ESC_CASE('\b', 'b');
        ESC_CASE('\f', 'f');
        ESC_CASE('\n', 'n');
        ESC_CASE('\r', 'r');
        ESC_CASE('\t', 't');
#undef ESC
        default:
            if (c < 0x20 || c == 0x7f) {
                ADV(6);
                *optr++ = '\\';
                *optr++ = 'u';
                *optr++ = '0';
                *optr++ = '0';
                *optr++ = tlog_nibble_digit(c >> 4);
                *optr = tlog_nibble_digit(c & 0xf);
                break;
            } else {
                ADV(1);
                *optr = c;
            }
            break;
        }
    }

    *polen = olen;
    return true;
}

#undef ADV
#undef ACC

void
tlog_stream_flush(struct tlog_stream *stream)
{
    assert(tlog_stream_is_valid(stream));
    tlog_channel_flush(stream->channel);
}

/**
 * Atomically write a byte sequence to a stream, the sequence being either a
 * valid or an invalid UTF-8 character. Account for any (potentially) used
 * total remaining space.
 *
 * @param stream    The stream to write to.    
 * @param ts        The byte sequence timestamp.
 * @param valid     True if sequence is a valid character, false otherwise.
 * @param buf       Sequence buffer pointer.
 * @param len       Sequence buffer length.
 *
 * @return True if the sequence was written, false otherwise.
 */
static bool
tlog_stream_write_seq(struct tlog_stream *stream, const struct timespec *ts,
                      bool valid, const uint8_t *buf, size_t len)
{
    /* Unicode replacement character (u+fffd) */
    static const uint8_t repl_buf[] = {0xef, 0xbf, 0xbd};
    size_t txt_len;
    size_t bin_len;

    assert(tlog_stream_is_valid(stream));
    assert(ts != NULL);
    assert(buf != NULL || len == 0);

    if (len == 0)
        return true;
    
    txt_len = stream->txt_len;
    bin_len = stream->bin_len;

    if (valid) {
        /* Write the character to the text buffer */
        if (!tlog_stream_enc_txt(&stream->channel->txt, ts, true,
                                 stream->txt_buf + txt_len, &txt_len,
                                 buf, len))
            return false;
    } else {
        /* Write the replacement character to the text buffer */
        if (!tlog_stream_enc_txt(&stream->channel->txt, ts, false,
                                 stream->txt_buf + txt_len, &txt_len,
                                 repl_buf, sizeof(repl_buf)))
            return false;

        /* Write bytes to the binary buffer */
        if (!tlog_stream_enc_bin(&stream->channel->bin, ts, true,
                                 stream->bin_buf + bin_len, &bin_len,
                                 buf, len))
            return false;
    }

    stream->txt_len = txt_len;
    stream->bin_len = bin_len;
    return true;
}


size_t
tlog_stream_write(struct tlog_stream *stream, const struct timespec *ts,
                  const uint8_t **pbuf, size_t *plen)
{
    const uint8_t *buf;
    size_t len;
    struct tlog_utf8 *utf8;
    size_t written;

    assert(tlog_stream_is_valid(stream));
    assert(ts != NULL);
    assert(pbuf != NULL);
    assert(plen != NULL);
    assert(*pbuf != NULL || *plen == 0);

    buf = *pbuf;
    len = *plen;
    utf8 = &stream->utf8;
    assert(!tlog_utf8_is_ended(utf8));

    while (true) {
        do {
            /* If input is exhausted */
            if (len == 0) {
                /* Exit but leave the incomplete character buffered */
                goto exit;
            }
            /* If the byte was added */
            if (tlog_utf8_add(utf8, *buf)) {
                buf++;
                len--;
            }
        } while (!tlog_utf8_is_ended(utf8));

        /* If the first byte we encountered was invalid */
        if (tlog_utf8_is_empty(utf8)) {
            /* Write single input byte as invalid sequence and skip it */
            if (!tlog_stream_write_seq(stream, ts, false, buf, 1))
                break;
            buf++;
            len--;
        } else {
            /* If the (in)complete character doesn't fit into output */
            if (!tlog_stream_write_seq(stream, ts, tlog_utf8_is_complete(utf8),
                                       utf8->buf, utf8->len)) {
                /* Back up unwritten data */
                buf -= utf8->len;
                len += utf8->len;
                break;
            }
        }
        tlog_utf8_reset(utf8);
    }

    tlog_utf8_reset(utf8);
exit:
    written = (*plen - len);
    *pbuf = buf;
    *plen = len;
    return written;
}

bool
tlog_stream_cut(struct tlog_stream *stream, const struct timespec *ts)
{
    struct tlog_utf8 *utf8;
    assert(tlog_stream_is_valid(stream));

    utf8 = &stream->utf8;
    assert(!tlog_utf8_is_ended(utf8));

    if (tlog_stream_write_seq(stream, ts, false, utf8->buf, utf8->len)) {
        tlog_utf8_reset(utf8);
        return true;
    } else {
        return false;
    }
}

void
tlog_stream_empty(struct tlog_stream *stream)
{
    assert(tlog_stream_is_valid(stream));
    stream->txt_len = 0;
    stream->bin_len = 0;
}
