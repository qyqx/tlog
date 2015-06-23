/*
 * Tlog tlog_stream_enc_bin function test.
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

#include <stdio.h>
#include "tlog_stream_enc_test.h"

#include "tlog_stream.c"

int
main(void)
{
    bool passed = true;

#define TEST(_name_token, _struct_init_args...) \
    passed = passed &&                                                  \
             tlog_stream_enc_test(#_name_token,                         \
                                  (struct tlog_stream_enc_test)         \
                                       {.func = tlog_stream_enc_bin,    \
                                        ##_struct_init_args})

    /* No input producing no output */
    TEST(zero,          .idig_in    = 10,
                        .idig_out   = 10,
                        .fit_out    = true);

    /* Output for one byte input */
    TEST(one,           .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .obuf_out   = "255",
                        .orem_in    = 3,
                        .olen_out   = 3,
                        .irun_out   = 1,
                        .idig_in    = 10,
                        .idig_out   = 10,
                        .fit_out    = true);

    /* One byte input, output short of one byte */
    TEST(one_out_one,   .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .obuf_out   = "25",
                        .orem_in    = 2,
                        .orem_out   = 2,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /* One byte input, output short of two bytes */
    TEST(one_out_two,   .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .obuf_out   = "2",
                        .orem_in    = 1,
                        .orem_out   = 1,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /* One byte input, output short of three (all) bytes */
    TEST(one_out_three, .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /* Output for two bytes input */
    TEST(two,           .ibuf_in    = {0x01, 0x02},
                        .ilen_in    = 2,
                        .obuf_out   = "1,2",
                        .orem_in    = 3,
                        .olen_out   = 3,
                        .irun_out   = 2,
                        .idig_in    = 10,
                        .idig_out   = 10,
                        .fit_out    = true);

    /* Two byte input, output short of one byte */
    TEST(two_out_one,   .ibuf_in    = {0xfe, 0xff},
                        .ilen_in    = 2,
                        .obuf_out   = "254,25",
                        .orem_in    = 6,
                        .orem_out   = 6,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /* Two byte input, output short of two bytes */
    TEST(two_out_two,   .ibuf_in    = {0xfe, 0xff},
                        .ilen_in    = 2,
                        .obuf_out   = "254,2",
                        .orem_in    = 5,
                        .orem_out   = 5,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /*
     * Two byte input, output short of three bytes
     * (no space for second input byte in the output at all)
     */
    TEST(two_out_three, .ibuf_in    = {0xfe, 0xff},
                        .ilen_in    = 2,
                        .obuf_out   = "254,",
                        .orem_in    = 4,
                        .orem_out   = 4,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /*
     * Two byte input, output short of four bytes (no space for comma and
     * second input byte in the output)
     */
    TEST(two_out_four,  .ibuf_in    = {0xfe, 0xff},
                        .ilen_in    = 2,
                        .obuf_out   = "254",
                        .orem_in    = 3,
                        .orem_out   = 3,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /*
     * Two byte input, output short of five bytes
     * (no space even for the first complete byte)
     */
    TEST(two_out_five,  .ibuf_in    = {0xfe, 0xff},
                        .ilen_in    = 2,
                        .obuf_out   = "25",
                        .orem_in    = 2,
                        .orem_out   = 2,
                        .idig_in    = 10,
                        .idig_out   = 10);

    /* Output for three bytes input */
    TEST(three,         .ibuf_in    = {0x01, 0x02, 0x03},
                        .ilen_in    = 3,
                        .obuf_out   = "1,2,3",
                        .orem_in    = 5,
                        .olen_out   = 5,
                        .irun_out   = 3,
                        .idig_in    = 10,
                        .idig_out   = 10,
                        .fit_out    = true);

    /* Non-zero input olen */
    TEST(non_zero_olen, .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .obuf_out   = ",255",
                        .orem_in    = 4,
                        .olen_in    = 100,
                        .olen_out   = 104,
                        .irun_out   = 1,
                        .idig_in    = 10,
                        .idig_out   = 10,
                        .fit_out    = true);

    /* Non-zero input irun */
    TEST(non_zero_irun, .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .obuf_out   = "255",
                        .orem_in    = 3,
                        .olen_out   = 3,
                        .irun_in    = 1,
                        .irun_out   = 2,
                        .idig_in    = 10,
                        .idig_out   = 10,
                        .fit_out    = true);

    /* Rolling over to second digit */
    TEST(second_digit,  .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .obuf_out   = "255",
                        .orem_in    = 4,
                        .olen_out   = 3,
                        .irun_in    = 9,
                        .irun_out   = 10,
                        .idig_in    = 10,
                        .idig_out   = 100,
                        .fit_out    = true);

    /* Rolling over to third digit */
    TEST(third_digit,   .ibuf_in    = {0xff},
                        .ilen_in    = 1,
                        .obuf_out   = "255",
                        .orem_in    = 4,
                        .olen_out   = 3,
                        .irun_in    = 99,
                        .irun_out   = 100,
                        .idig_in    = 100,
                        .idig_out   = 1000,
                        .fit_out    = true);

    return !passed;
}