/*
* Copyright (C) 2017 Boris Altshul.
* All rights reserved.
*
* The software in this package is published under the terms of the BSD
* style license a copy of which has been included with this distribution in
* the LICENSE.txt file.
*/


#ifndef __COMMON_H
#define __COMMON_H

#include <vector>
#include <string>
#include <map>
#include <utility>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int64_t s64;
typedef uint64_t u64;

// default 
#define DBC_CHECK(v, message) assert(v)


#if 0
#define DBC_CHECK(v, message)  if(!v) {throw LibAirtException(message)  };
#endif



#define MAX_SET_SIZE 128

#define DBC_CHECK_VID(vid) DBC_CHECK(vid > 0 && vid < MAX_SET_SIZE, "VariableId is out of range")

typedef u32 InstanceId;
typedef unsigned int VarId;
typedef float ValueType;
// Value of variable within its domain definition
typedef u8 VarState;

template <class T>
char * AddJsonAttr(char *pBuffer, u32 bufSize, const char *pName, const char *pFormat, T val)
{
   char szFormat[40];
   snprintf(szFormat, sizeof(szFormat), "\"%s\":\"%s\",", "%s", pFormat);
   szFormat[39] = '\0';
   snprintf(pBuffer, (size_t)bufSize, szFormat, pName, val);
   pBuffer[bufSize - 1] = '\0';
   return pBuffer;
}

// no trailing comma
template <class T>
char * AddJsonAttrLast(char *pBuffer, u32 bufSize, const char *pName, const char *pFormat, T val)
{
   char szFormat[40];
   snprintf(szFormat, sizeof(szFormat), "\"%s\":\"%s\"", "%s", pFormat);
   szFormat[39] = '\0';
   snprintf(pBuffer, (size_t)bufSize, szFormat, pName, val);
   pBuffer[bufSize - 1] = '\0';
   return pBuffer;
}

inline
char * AddJsonYN(char *pBuffer, u32 bufSize, const char *pName, bool val)
{
   snprintf(pBuffer, (size_t)bufSize, "\"%s\":\"%s\",", pName, val ? "yes" : "no");
   pBuffer[bufSize - 1] = '\0';
   return pBuffer;
}

template <class T>
char * AddJsonBareAttr(char *pBuffer, u32 bufSize, const char *pName, const char *pFormat, T val)
{
   char szFormat[40] = { 0 };
   snprintf(szFormat, sizeof(szFormat), "\"%s\":%s,", "%s", pFormat);
   szFormat[39] = '\0';
   snprintf(pBuffer, (size_t)bufSize, szFormat, pName, val);
   pBuffer[bufSize - 1] = '\0';
   return pBuffer;
}

inline
char *AddJsonObjectStart(char *pBuffer, u32 bufSize, const char *pName)
{
   snprintf(pBuffer, (size_t)bufSize, "\"%s\": {", pName);
   return pBuffer;
}

inline
char *AddJsonObjectEnd(char *pBuffer, u32 bufSize, bool last = false)
{
   snprintf(pBuffer, (size_t)bufSize, "}%c", last ? ' ' : ',');
   return pBuffer;
}




#endif
