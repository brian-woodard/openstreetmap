#pragma once
#include <iostream>

// Enumeration for data types
enum class DataType
{
   INTEGER_1_TYPE,
   INTEGER_2_TYPE,
   INTEGER_4_TYPE,
   INTEGER_8_TYPE,
   UNSIGNED_INTEGER_1_TYPE,
   UNSIGNED_INTEGER_2_TYPE,
   UNSIGNED_INTEGER_4_TYPE,
   UNSIGNED_INTEGER_8_TYPE,
   BOOL_TYPE,
   FLOAT_TYPE,
   DOUBLE_TYPE,
   UNKNOWN_TYPE
};

enum class DisplayType
{
   Normal,
   Hex,
   String
};

// Struct to hold variable information
#pragma pack(1)
struct TVariableInfo
{
   DataType Type;
   uint32_t Offset;
   uint32_t VariableIndex;
   uint32_t Dimension;
};
#pragma pack()

struct TVariableDisplay
{
   std::string    Global;
   std::string    Name;
   void*          Address;
   uint16_t       GlobalId;
   DisplayType    Display;
   TVariableInfo  Info;
   TVariableValue Value;
   TVariableValue PrevValue;
   bool           PlotEnabled;
   bool           FirstPass;
};

struct TGlobalDefinition
{
   uint8_t* Buffer;
   size_t   Size;
};

using TGlobalKey = std::tuple<std::string, std::string, int>;

