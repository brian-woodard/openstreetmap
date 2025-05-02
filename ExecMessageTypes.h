///////////////////////////////////////////////////////////////////////////////
// 
// File: ExecMessageTypes.h
// Date:
// Revision:
// Creator: Jason Stinnett
// License: (C) Copyright 2024 by Everus Engineering LLC. All Rights Reserved.
//
// Exec Message types contains all the data structures transferred between
// frontend and backend
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <cstddef>
#include "ExecConstants.h"

enum class Command : uint32_t
{
   NONE,
   SHUTDOWN,
   STATS_ENABLE,
   STATS_DATA,
   GLOBALS_REQUEST,
   GLOBALS_RESPONSE,
   GLOBAL_ADD,
   GLOBAL_UPDATE,
   GLOBAL_REMOVE,
   GLOBAL_REMOVE_ALL,
   LOG_SETTINGS,
   LOG_DATA,
   HEARTBEAT,
   THREAD_DEFINITION_REQUEST,
   THREAD_DEFINITION_RESPONSE,
   CALLBACK_ENABLE,
   STATS_RESET,
   RUN,
   PAUSE,
   CONTINUE,

   NUM_COMMANDS
};

// Defined in ExecMessageTypes.cpp
extern const char* CommandStr[(int)Command::NUM_COMMANDS + 1];

#pragma pack(1)
struct TMessageHeader
{
   Command CommandId;  // Command identifier (e.g., shutdown = 1, etc.)
   uint16_t Size;
};

struct TLogSettingsMessage
{
   Command  CommandId;
   uint16_t Size;
   uint16_t LogLevel;
   bool     EnableDate;
   bool     EnableTimestamp;
};

struct THeartbeat
{
   Command  CommandId;
   uint16_t Size;
   uint16_t Mode;
};

// variable sized PDU up to MAX_LOG_SIZE + header
struct TServerLogMessage
{
   Command  CommandId;
   uint16_t Size;
   char     Message[MAX_LOG_SIZE];
};

struct TGlobalDefinitionMessage
{
   Command  CommandId;
   uint16_t Size;
   uint16_t Id;
   uint16_t MessageNumber;
   uint16_t NumMessages;
   uint16_t Checksum;
   uint64_t Address;
   uint32_t GlobalSize;
   // variable info from here on out, defined by ExecGlobal.h
};

union TVariableValue
{
   int64_t  i64_value;
   int32_t  i32_value;
   int16_t  i16_value;
   int8_t   i8_value;
   uint64_t u64_value;
   uint32_t u32_value;
   uint16_t u16_value;
   uint8_t  u8_value;
   double   f64_value;
   float    f32_value;
   bool     bool_value;
};

struct TGlobalLocation
{
   uint16_t GlobalId;
   uint32_t VariableIndex;
   uint32_t Index;
};

struct TGlobalAddRemoveMessage
{
   Command  CommandId;
   uint16_t Size;
   uint16_t NumMessages;
   TGlobalLocation* Global;
};

struct TGlobalUpdateMessage
{
   Command        CommandId;
   uint16_t       Size;
   uint16_t       GlobalId;
   uint32_t       VariableIndex;
   uint32_t       Index;
   TVariableValue Value;
};

struct TThreadDefinitionMessage
{
   Command  CommandId;
   uint16_t Size;
   uint16_t NumThreads;
   uint16_t MaxCpus;
   int32_t  Pid;
   uint16_t VersionMajor;
   uint16_t VersionMinor;
   uint16_t VersionBuild;
   uint16_t VersionMonth;
   uint16_t VersionDay;
   uint16_t VersionYear;
   // variable info from here on out
};

struct TStatsEnableMessage
{
   Command  CommandId;
   uint16_t Size;
   uint8_t  Enable;
   uint8_t  RefreshRate;
};

struct TStatsThread
{
   int      Frames;
   uint32_t Overruns;
   float    FrameTimeAvgMs;
   float    FrameTimeMaxMs;
   float    FrameTimeMinMs;
};

struct TStatsCallback
{
   int      Frames;
   float    FrameTimeAvgMs;
   float    FrameTimeMaxMs;
   float    FrameTimeMinMs;
};

struct TStatsUpdateMessage
{
   Command  CommandId;
   uint16_t Size;
   uint16_t NumThreads;
   uint8_t  RefreshRate;
   // variable info from here on out
   // made up of TStatsThread and TStatsCallback
};

struct TCallbackEnableMessage
{
   Command  CommandId;
   uint16_t Size;
   uint16_t ThreadIndex;
   uint16_t ModeIndex;
   uint16_t CallbackIndex;
   uint16_t Enable;
};
#pragma pack()
