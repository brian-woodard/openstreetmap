///////////////////////////////////////////////////////////////////////////////
// 
// File: ExecApi.h
// Date:
// Revision:
// Creator: Brian Woodard
// License: (C) Copyright 2024 by Everus Engineering LLC. All Rights Reserved.
//
// Exec API
//
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ExecMessageTypes.h"
#include "ExecGlobalTypes.h"

#include <cstdint>

void ExecApiInit(int ArgCount, char* Args[], const char* Config = nullptr, const char* Environment = nullptr);

const char* ExecApiGetEnvVar(const char* Name);

// The frame count and frame time calls require stats collection to be enabled,
// using the ExecApiStatsEnable(bool Enable) function
int ExecApiGetFrameCount(const char* Callback);
float ExecApiGetFrameTimeAvgMs(const char* Callback);
float ExecApiGetFrameTimeMaxMs(const char* Callback);
float ExecApiGetFrameTimeMinMs(const char* Callback);

int ExecApiGetIterationRate(const char* Callback);

void ExecApiLoadGlobals(const char* File, const char* Name, void* Address, int Size);
int ExecApiGetNumGlobals();
TVariableDisplay ExecApiGetGlobalData(uint16_t GlobalId, uint32_t VariableIndex, uint32_t Index);
TVariableValue ExecApiGetGlobalValue(TVariableDisplay Variable);
void ExecApiUpdateGlobal(TGlobalUpdateMessage* Global);
const char* ExecApiGetGlobalIndex(int Index, int* Size);
void ExecApiLoadGlobals(const char* File, const char* Name, void* Address, int Size);

size_t ExecApiLogBufferSize();
size_t ExecApiLogBuffer(uint8_t* Buffer, size_t Size);
void ExecApiLogInfo(const char* Format, ...);
void ExecApiLogMessage(const char* Format, ...);
void ExecApiLogWarning(const char* Format, ...);
void ExecApiLogError(const char* Format, ...);
void ExecApiLogDebug(const char* Format, ...);
void ExecApiLogDebugVerbose(const char* Format, ...);
void ExecApiLogTrace(const char* Format, ...);
void ExecApiLogBacktrace();
void ExecApiSetLogLevel(int LogLevel);
void ExecApiLogFile(const char* Filename);

void ExecApiMutexLock(const char* Mutex);
void ExecApiMutexUnlock(const char* Mutex);

void ExecApiShutdown();

bool ExecApiStatsEnabled();
void ExecApiStatsEnable(bool Enable);
void ExecApiStatsReset();

void ExecApiSwap(uint16_t* Data);
void ExecApiSwap(uint32_t* Data);
void ExecApiSwap(uint64_t* Data);

void ExecApiRun();
