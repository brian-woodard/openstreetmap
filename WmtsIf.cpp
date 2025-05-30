
#include <string.h>
#include <curl/curl.h>
#include "WmtsIf.h"

CWmtsIf::CWmtsIf()
   : mCurlBuffer(),
     mWmtsUrl(""),
     mTimeoutMsec(500),
     mIsOpen(false)
{
}

CWmtsIf::~CWmtsIf()
{
   Close();
}

void CWmtsIf::Close()
{
   if (!mIsOpen) return;

   // cleanup curl
   curl_global_cleanup();

   mIsOpen = false;
}

size_t CWmtsIf::CurlWriteFunction(void* Ptr, size_t Size, size_t Nmemb)
{
   int            data_size;                                                    
   unsigned char* data_array;                                                   
                                                                                
   // calculate the data size                                                   
   data_size = Size * Nmemb;                                                    
                                                                                
   // insert the data at the end of the buffer                                  
   data_array = (unsigned char*) Ptr;                                           
   mCurlBuffer.insert(mCurlBuffer.end(), &data_array[0], &data_array[data_size]);
                                                                                
   return data_size;
}

bool CWmtsIf::GetWmtsCapabilitiesXml(unsigned char** XmlFileBuffer, int& Size)
{
   std::string wmts_cmd;

   // check if open
   if (!mIsOpen) return false;

   // clear the curl buffer
   mCurlBuffer.clear();

   // construct the wmts command
   wmts_cmd = mWmtsUrl + "/styles/basic-preview/wmts.xml";

   // initialize a curl connection
   CURL* curl = curl_easy_init();

   if (!curl)
      return false;

   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,0L);
   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,0L);
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RunCurlWriteFunction);
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
   curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, mTimeoutMsec);
   curl_easy_setopt(curl, CURLOPT_URL, wmts_cmd.c_str());
   curl_easy_perform(curl);
   curl_easy_cleanup(curl);

   *XmlFileBuffer = mCurlBuffer.data();
   Size = mCurlBuffer.size();

   return true;
}

bool CWmtsIf::GetMapPngBuffer(int Zoom,
                              int X,
                              int Y,
                              unsigned char** PngFileBuffer,
                              int& Size)
{
   std::string wmts_cmd;

   // check if open
   if (!mIsOpen) return false;

   // clear the curl buffer
   mCurlBuffer.clear();

   // construct the wms command
   wmts_cmd = mWmtsUrl + "/styles/basic-preview/256/" +
              std::to_string(Zoom) + "/" +
              std::to_string(X) + "/" +
              std::to_string(Y) + ".png";

   // initialize a curl connection
   CURL* curl = curl_easy_init();

   if (!curl)
      return false;

   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,0L);
   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,0L);
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RunCurlWriteFunction);
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
   curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, mTimeoutMsec);
   curl_easy_setopt(curl, CURLOPT_URL, wmts_cmd.c_str());
   curl_easy_perform(curl);
   curl_easy_cleanup(curl);

   // check for png file validity
   if (mCurlBuffer.size() == 0)
      return false;

   if ((mCurlBuffer[1] != 'P') ||
       (mCurlBuffer[2] != 'N') ||
       (mCurlBuffer[3] != 'G'))
      return false;

   // output the curl buffer to the png file buffer
   *PngFileBuffer = mCurlBuffer.data();
   Size = mCurlBuffer.size();

   return true;
}

bool CWmtsIf::Open(const char* WmtsUrl, int TimeoutSec)
{
   mWmtsUrl = WmtsUrl;

   //  initialize curl
   if (curl_global_init(CURL_GLOBAL_ALL)) return false;

   // set to open
   mIsOpen = true;

   return true;
}


size_t CWmtsIf::RunCurlWriteFunction(void* Ptr, size_t Size, size_t Nmemb, void* Userdata)
{
   // Userdata points to this instance
   if (!Userdata) return CURLE_WRITE_ERROR;

   CWmtsIf* this_ptr = (CWmtsIf*) Userdata;

   // call this instance's curl write function
   return this_ptr->CurlWriteFunction(Ptr, Size, Nmemb);
}

