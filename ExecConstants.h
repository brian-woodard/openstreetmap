///////////////////////////////////////////////////////////////////////////////
// 
// File: ExecConstants.h
// Date:
// Revision:
// Creator: Brian Woodard
// License: (C) Copyright 2024 by Everus Engineering LLC. All Rights Reserved.
//
// Exec Constants
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>

#define KB(n)  (((uint64_t)(n)) << 10)
#define MB(n)  (((uint64_t)(n)) << 20)
#define GB(n)  (((uint64_t)(n)) << 30)
#define TB(n)  (((uint64_t)(n)) << 40)

#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))

const uint64_t MAX_PACKET          = KB(32); // How low should this be?
const uint64_t MAX_LOG_SIZE        = 4096;
const uint64_t MAX_CLIENT_LOG_SIZE = MB(8);
