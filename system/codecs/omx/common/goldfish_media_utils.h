/*
 * Copyright (C) 2018 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/types.h>
#include <stdint.h>

#ifndef GOLDFISH_COMMON_GOLDFISH_DEFS_H
#define GOLDFISH_COMMON_GOLDFISH_DEFS_H

enum class MediaCodecType : __u8 {
    VP8Codec = 0,
    VP9Codec = 1,
    H264Codec = 2,
    Max = 3,
};

enum class MediaOperation : __u8 {
    InitContext = 0,
    DestroyContext = 1,
    DecodeImage = 2,
    GetImage = 3,
    Flush = 4,
    Max = 5,
};

// This class will abstract away the knowledge required to send media codec data
// to the host. The implementation should only need the following information to
// properly send the data:
//   1) Which codec to use (MediaCodecType)
//   2) What operation to perform (MediaOperation)
//
// Example:
//   auto transport = GoldfishMediaTransport::getInstance();
//
class GoldfishMediaTransport {
protected:
    GoldfishMediaTransport() {}

public:
    virtual ~GoldfishMediaTransport() {}

    // Writes a parameter to send to the host. Each parameter will take up
    // 64-bits. |val| is the value of the parameter, and |num| is the parameter
    // number, starting from 0. If |val| is an address, wrap it around
    // offsetOf(), e.g., writeParam(offsetOf((uint64_t)ptr), 2);
    virtual void writeParam(__u64 val, unsigned int num) = 0;
    // Send the operation to perform to the host. At the time of this call, any
    // parameters that the host needs should have already been passed using
    // writeParam().
    virtual bool sendOperation(MediaCodecType codec, MediaOperation op) = 0;
    // Get the address for input. This is usually given the the codec context to
    // write data into for the host to process.
    virtual uint8_t* getInputAddr() const = 0;
    // Get the address for output. This is usually given to the codec context to
    // read data written there by the host.
    virtual uint8_t* getOutputAddr() const = 0;
    // Get the address for return data from the host. The guest codec
    // implementation will have knowledge of how the return data is laid out.
    virtual uint8_t* getReturnAddr() const = 0;
    // Get the offset of an address relative to the starting address of the
    // allocated memory region. Use this for passing pointers from the guest to
    // the host, as the guest address will be translated, thus the offset is the
    // only value of significance.
    virtual __u64 offsetOf(uint64_t addr) const = 0;

    static GoldfishMediaTransport* getInstance();
};

__u64 goldfish_create_media_metadata(MediaCodecType codecType,
                                     __u64 metadata);

#endif
