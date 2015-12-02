/*
 * Tlog JSON encoder stream run counter.
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

#ifndef _TLOG_RUN_H
#define _TLOG_RUN_H

/** Run counter */
struct tlog_run {
    uint8_t             valid_mark;     /**< Valid text record marker */
    uint8_t             invalid_mark;   /**< Invalid text record marker */

    size_t              txt_len;        /**< Text length in characters */
    size_t              txt_dig;        /**< Text length digit limit */

    size_t              bin_len;        /**< Binary length in bytes */
    size_t              bin_dig;        /**< Binary length digit limit */
};

extern bool tlog_run_add_txt(struct tlog_run *run,
                             size_t len,
                             size_t *prem)

extern bool tlog_run_add_bin(struct tlog_run *run,
                             size_t len,
                             size_t *prem)

extern bool tlog_run_flush(struct tlog_run *run);

#endif /* _TLOG_RUN_H */
