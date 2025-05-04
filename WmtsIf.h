
#pragma once

#include <string>

class CWmtsIf
{
public:

   CWmtsIf();
   ~CWmtsIf();

   void Close();

   bool GetWmtsCapabilitiesXml(unsigned char** XmlFileBuffer, int& Size);

   bool GetMapPngBuffer(int Zoom,
                        int X,
                        int Y,
                        unsigned char** PngFileBuffer,
                        int& Size);

   bool Open(const char* WmtsUrl, int TimeoutSec);

private:

   void AppendToBuffer(void* Buffer, size_t Size);

   size_t CurlWriteFunction(void* Ptr, size_t Size, size_t Nmemb);

   static size_t RunCurlWriteFunction(
         void* Ptr, size_t Size, size_t Nmemb, void* Userdata);

   unsigned char* mCurlBuffer;
   std::string    mWmtsUrl;
   size_t         mSize;
   size_t         mSizeAllocated;
   int            mTimeoutMsec;
   bool           mIsOpen;
};
