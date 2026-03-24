#ifndef SDOT_KERNEL_ARGUMENTS_H
#define SDOT_KERNEL_ARGUMENTS_H

/*-------------------------------------------------------------------------
 * Copyright (c) 2025 Ainekko, Co.
 * SPDX-License-Identifier: Apache-2.0
 *-------------------------------------------------------------------------
 */

#include <cstdint>

struct KernelArguments {
  uint64_t numElements;
  float* x;
  float* y;
  float* res;
} __attribute__ ((packed));

#endif

