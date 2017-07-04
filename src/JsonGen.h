#ifndef __JSON_GEN
#define __JSON_GEN

template <class T>
char * AddJsonAttr(char *pBuffer, u32 bufSize, const char *pName, const char *pFormat, T val)
{
   char szFormat[40];
   snprintf(szFormat, sizeof(szFormat), "\"%s\":\"%s\",", "%s", pFormat); 
   szFormat[39] = '\0';
   snprintf(pBuffer, (size_t)bufSize, szFormat, pName, val);
   pBuffer[bufSize-1] = '\0';
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
   pBuffer[bufSize-1] = '\0';
   return pBuffer;
}

inline
char * AddJsonYN(char *pBuffer, u32 bufSize, const char *pName, bool val)
{
   snprintf(pBuffer, (size_t)bufSize, "\"%s\":\"%s\"," , pName, val?"yes":"no");
   pBuffer[bufSize-1] = '\0';
   return pBuffer;
}

template <class T>
char * AddJsonBareAttr(char *pBuffer, u32 bufSize, const char *pName, const char *pFormat, T val)
{
   char szFormat[40] = {0};
   snprintf(szFormat, sizeof(szFormat), "\"%s\":%s,", "%s", pFormat); 
   szFormat[39] = '\0';
   snprintf(pBuffer, (size_t)bufSize, szFormat, pName, val);
   pBuffer[bufSize-1] = '\0';
   return pBuffer;
}

inline
char *AddJsonObjectStart(char *pBuffer, u32 bufSize, const char *pName)
{
    snprintf(pBuffer, (size_t)bufSize, "\"%s\": {" , pName);
    return pBuffer;
}

inline
char *AddJsonObjectEnd(char *pBuffer, u32 bufSize, bool last=false)
{
    snprintf(pBuffer, (size_t)bufSize, "}%c", last ? ' ' : ',');
    return pBuffer;
}

#endif