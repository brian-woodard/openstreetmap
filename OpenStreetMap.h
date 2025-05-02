
#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>

#define OSM_IMAGE_CACHE_SIZE 1024
#define OSM_TILE_SIZE        256
#define SERVER_TIMEOUT       200 // cycles before trying server again

class COpenStreetMap
{
public:
   COpenStreetMap();
   ~COpenStreetMap();

   void Close();

   void Draw();

   void EnableSubframeBoundaries(bool Enable);

   bool Open(bool        WmtsEnabled,
             const char* WmtsUrl,
             bool        CacheEnabled,
             const char* CachePath);

   void SetCoverageRadiusScaleFactor(float ScaleFactor) { mCoverageRadiusScaleFactor = ScaleFactor; }

   void SetMapCenter(double MapCenterLat, double MapCenterLon);

   void SetMapRotation(double RotationClockwiseDeg);

   void SetMapScaleFactor(float ScaleFactor);

   void SetMapSize(int MapWidthPix, int MapHeightPix);

   void Update();

private:

   struct TTile
   {
      //CVOsmMapTile* Texture;
      double        Latitude;
      double        Longitude;
      int           ZoomLevel;
      int           TileX;
      int           TileY;
   };

   struct TCacheTag
   {
      int Zoom;
      int X;
      int Y;

      bool operator==(const TCacheTag& That)
      {
         return (this->Zoom == That.Zoom) &&
                (this->X == That.X) &&
                (this->Y == That.Y);
      }
   };

   typedef std::vector<TCacheTag> TSubframeCoverageList;

   //typedef CVCache<TTile, TCacheTag, OSM_IMAGE_CACHE_SIZE> TImageCache;

   std::string ConstructFilename(int Zoom, int X, int Y);

   void CoverageThread();

   //void CoverageImage(TSubframeCoverageList& SubframeCoverageList,
   //                   std::vector<TTile>&    DisplayListScratchpad,
   //                   std::vector<TTile>&    DisplayListTrashScratchpad,
   //                   TImageCache&           ImageCache);

   //void CreateNoDataTile();

   void EnableCache(bool Enable, const char* CachePath = nullptr);

   void EnableWmtsServer(bool Enable, const char* WmtsUrl = nullptr);

   double GetLatitudeFromTileY(int Y, int Zoom);

   double GetLongitudeFromTileX(int X, int Zoom);

   double GetMetersPerPixelEw(double Latitude, int Zoom);

   double GetMetersPerPixelNs(int Zoom);

   //void GetSubframeCoverageList(TSubframeCoverageList& SubframeCoverageList, double MapCenterLat, double MapCenterLon, int ZoomLevel, double CoverageRadiusPixels);

   int GetTileX(double Longitude, int Zoom);

   int GetTileY(double Latitude, int Zoom);

   void GetZoom();

   //CVOsmIf            mWmtsIf;
   std::thread        mCoverageThread;
   std::mutex         mMutex;
   std::vector<TTile> mDisplayList;
   std::vector<TTile> mDisplayListTrash;
   std::string        mCachePath;
   std::string        mWmtsUrl;
   TTile              mNoDataTile;
   double             mMapCenterLat;
   double             mMapCenterLon;
   double             mMapZoom;
   double             mMapScaleX;
   double             mMapScaleY;
   double             mMapBrightness;
   double             mMapRotation;
   double             mMetersPerPixNs;
   double             mMetersPerPixEw;
   float              mMapScaleFactor;
   float              mCoverageRadiusScaleFactor;
   int                mMapWidthPix;
   int                mMapHeightPix;
   int                mZoomLevel;
   int                mWmtsTimeout;
   bool               mTerminateCoverageThread;
   bool               mDrawSubframeBoundaries;
   bool               mCacheEnabled;
   bool               mWmtsEnabled;
   bool               mWmtsOnline;
   bool               mDrawUpdate;
};
