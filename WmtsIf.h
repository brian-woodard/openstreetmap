
#pragma once

#include <string>
#include <vector>

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

   size_t CurlWriteFunction(void* Ptr, size_t Size, size_t Nmemb);

   static size_t RunCurlWriteFunction(
         void* Ptr, size_t Size, size_t Nmemb, void* Userdata);

   std::vector<unsigned char> mCurlBuffer;
   std::string                mWmtsUrl;
   int                        mTimeoutMsec;
   bool                       mIsOpen;
};
