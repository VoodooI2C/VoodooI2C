/*
 * Copyright (c) 2014 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Matthew Dillon <dillon@backplane.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *------------------------------------------------------------------------------
 * Copyright 2011 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ATMEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 */
/*
 * This file contains a mix of code taken from the atmel-maxtouch/obp-utils
 * github codebase and my own for testing the atmel touchscreen chipset in
 * the Acer C720P chromebook.
 *
 * This code is independent and is used by both kernel AND user mode programs.
 */

#include <sys/types.h>
#include "atmel_mxt.h"

/*!
 * @brief Information block checksum return function.
 * @return 24-bit checksum value in 32-bit integer.
 */
uint32_t
obp_convert_crc(mxt_raw_crc *crc)
{
    return ((crc->CRC_hi << 16u) | (crc->CRC));
}

/*!
 * @brief  Information Block Checksum algorithm.
 * @return Calculated Information Block Checksum.
 */
static
uint32_t
crc24_2byte(uint32_t crc, uint8_t firstbyte, uint8_t secondbyte)
{
    static const uint32_t CRCPOLY = 0x0080001B;
    uint32_t result;
    uint16_t data_word;
    
    data_word = (uint16_t)((uint16_t)(secondbyte << 8u) | firstbyte);
    result = ((crc << 1u) ^ (uint32_t)data_word);
    
    /* Check if 25th bit is set, and XOR the result to create
     * 24-bit checksum */
    if (result & 0x1000000) {
        result ^= CRCPOLY;
    }
    return result;
}

/*
 * Return 24-bit crc.
 */
uint32_t
obp_crc24(uint8_t *buf, size_t bytes)
{
    uint32_t crc;
    size_t i;
    
    crc = 0;
    for (i = 0; i < (bytes & ~1); i += 2) {
        crc = crc24_2byte(crc, buf[i], buf[i + 1]);
    }
    if (i < bytes)
        crc = crc24_2byte(crc, buf[i], 0);
    crc &= 0x00FFFFFF;
    
    return crc;
}