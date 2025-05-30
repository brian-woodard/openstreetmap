
#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <glm/glm.hpp>
#include "WmtsIf.h"
#include "Shader.h"
#include "Cache.h"
#include "Texture.h"

#define OSM_IMAGE_CACHE_SIZE 1024
#define OSM_TILE_SIZE        256
#define SERVER_TIMEOUT       200 // cycles before trying server again
#define MAX_ZOOM_LEVELS      21

class COpenStreetMap
{
public:
   COpenStreetMap();
   ~COpenStreetMap();

   void Close();

   void Draw();

   void EnableBorder(bool Enable) { mBorderEnabled = Enable; }
   void EnableClip(bool Enable) { mClipEnabled = Enable; }
   void EnableEasing(bool Enable);
   void EnableSubframeBoundaries(bool Enable);

   int GetCenterTileX() const { return mCenterTileX; }
   int GetCenterTileY() const { return mCenterTileY; }
   double GetMapZoom() const { return mMapZoom; }
   int GetZoomLevel() const { return mZoomLevel; }

   bool Open(bool        WmtsEnabled,
             const char* WmtsUrl,
             bool        CacheEnabled,
             const char* CachePath);

   void SetBorderColor(const glm::vec4& Color) { mBorderColor = Color; }

   void SetCoverageRadiusScaleFactor(float ScaleFactor) { mCoverageRadiusScaleFactor = ScaleFactor; }

   void SetMapCenter(double MapCenterLat, double MapCenterLon);

   void SetMapOffset(int MapOffsetX, int MapOffsetY);

   void SetMapRotation(double RotationClockwiseDeg);

   void SetMapScaleFactor(float ScaleFactor);

   void SetMapSize(int MapWidthPix, int MapHeightPix);

   void SetProjection(const glm::mat4& Projection) { mMapProjection = Projection; }

   void SetShaders(std::shared_ptr<CShader> ShaderRect, std::shared_ptr<CShader> ShaderLine)
   {
      mShaderRect = ShaderRect;
      mShaderLine = ShaderLine;
   }

   void SetWindowSize(int WinWidthPix, int WinHeightPix);

   void Update();

private:

   static const double mMapScale[MAX_ZOOM_LEVELS];

   struct TTile
   {
      std::shared_ptr<CTexture> Texture;
      std::string               Filename;
      double                    Latitude;
      double                    Longitude;
      int                       ZoomLevel;
      int                       TileX;
      int                       TileY;
      int                       Age;
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

   using TTileList = std::vector<TCacheTag>;
   using TImageCache = Cache<TTile, TCacheTag, OSM_IMAGE_CACHE_SIZE>;

   std::string ConstructFilename(int Zoom, int X, int Y);

   void CoverageThread();

   void UpdateCache(TTileList&          TileList,
                    std::vector<TTile>& DisplayListScratchpad,
                    std::vector<TTile>& DisplayListTrashScratchpad,
                    TImageCache&        ImageCache);

   void EnableCache(bool Enable, const char* CachePath = nullptr);

   void EnableWmtsServer(bool Enable, const char* WmtsUrl = nullptr);

   double GetLatitudeFromTileY(int Y, int Zoom);

   double GetLongitudeFromTileX(int X, int Zoom);

   double GetMetersPerPixelEw(double Latitude, int Zoom);

   double GetMetersPerPixelNs(int Zoom);

   void GetTileList(TTileList& TileList, double MapCenterLat, double MapCenterLon, int ZoomLevel, double ScaleX, double CoverageRadiusPixels);

   int GetTileX(double Longitude, int Zoom);

   int GetTileY(double Latitude, int Zoom);

   void GetZoom();

   CWmtsIf                  mWmtsIf;
   std::thread              mCoverageThread;
   std::mutex               mMutex;
   std::vector<TTile>       mDisplayList;
   std::vector<TTile>       mDisplayListEasing;
   std::vector<TTile>       mDisplayListTrash;
   std::string              mCachePath;
   std::string              mWmtsUrl;
   glm::mat4                mMapProjection;
   glm::vec4                mBorderColor;
   std::shared_ptr<CShader> mShaderRect;
   std::shared_ptr<CShader> mShaderLine;
   TTile                    mNoDataTile;
   double                   mMapCenterLat;
   double                   mMapCenterLon;
   double                   mMapZoom;
   double                   mMapScaleX;
   double                   mMapScaleY;
   double                   mMapBrightness;
   double                   mMapRotation;
   double                   mDegPerPixNs;
   double                   mDegPerPixEw;
   double                   mMetersPerPixNs;
   double                   mMetersPerPixEw;
   float                    mMapScaleFactor;
   float                    mCoverageRadiusScaleFactor;
   int                      mCenterTileX;
   int                      mCenterTileY;
   int                      mMapOffsetX;
   int                      mMapOffsetY;
   int                      mMapWidthPix;
   int                      mMapHeightPix;
   int                      mWinWidthPix;
   int                      mWinHeightPix;
   int                      mZoomLevel;
   int                      mWmtsTimeout;
   bool                     mTerminateCoverageThread;
   bool                     mDrawSubframeBoundaries;
   bool                     mEasingEnabled;
   bool                     mCacheEnabled;
   bool                     mWmtsEnabled;
   bool                     mWmtsOnline;
   bool                     mBorderEnabled;
   bool                     mClipEnabled;
};
