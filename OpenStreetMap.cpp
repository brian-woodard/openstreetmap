
#include <cmath>
#include <chrono>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include "OpenStreetMap.h"
#include "GlLineStrip.h"
#include "GlRect.h"
#include "ExecApi.h"

const double EQUATOR_CIRCUMFERENCE_M = 40075017.0;
const double DEGREES_TO_RADIANS      = M_PI / 180.0;
const double RADIANS_TO_DEGREES      = 180.0 / M_PI;
const double M_TO_DEG                = 1.0 / 111120.0;

COpenStreetMap::COpenStreetMap()
   : mShaderRect(nullptr),
     mShaderLine(nullptr),
     mNoDataTile{},
     mMapCenterLat(0.0),
     mMapCenterLon(0.0),
     mMapZoom(1.0),
     mMapScaleX(1.0),
     mMapScaleY(1.0),
     mMapBrightness(0.0),
     mMapRotation(0.0),
     mDegPerPixNs(0.0),
     mDegPerPixEw(0.0),
     mMetersPerPixNs(0.0),
     mMetersPerPixEw(0.0),
     mMapScaleFactor(1.0f),
     mCoverageRadiusScaleFactor(1.0f),
     mMapWidthPix(0),
     mMapHeightPix(0),
     mZoomLevel(0),
     mWmtsTimeout(0),
     mTerminateCoverageThread(false),
     mDrawSubframeBoundaries(false),
     mCacheEnabled(false),
     mWmtsEnabled(false),
     mWmtsOnline(false),
     mDrawUpdate(false)
{
}

COpenStreetMap::~COpenStreetMap()
{
   Close();
}

void COpenStreetMap::Close()
{
   if (mCoverageThread.joinable())
   {
      mTerminateCoverageThread = true;
      mCoverageThread.join();
   }

#if 0
   for (int i = 0; i < mDisplayListTrash.size(); i++)
   {
      if (mDisplayListTrash[i].Texture)
      {
         mDisplayListTrash[i].Texture->Delete();
         delete mDisplayListTrash[i].Texture;
      }
   }
   mDisplayListTrash.clear();
#endif
}

std::string COpenStreetMap::ConstructFilename(int Zoom, int X, int Y)
{
   std::string filename = mCachePath;

   filename += std::to_string(Zoom);
   filename += "_";
   filename += std::to_string(X);
   filename += "_";
   filename += std::to_string(Y);
   filename += ".png";

   return filename;
}

void COpenStreetMap::CoverageThread()
{
   TImageCache        image_cache;
   TTileList          tile_list;
   std::vector<TTile> display_list_trash_scratchpad;
   std::vector<TTile> display_list_scratchpad;
   double             map_center_lat;
   double             map_center_lon;
   double             coverage_radius_scale_factor;
   double             coverage_radius_pixels;
   double             scale_x;
   double             scale_y;
   double             map_zoom;
   int                zoom_level;
   int                window_width;
   int                window_height;

   // loop until terminated
   while (!mTerminateCoverageThread)
   {
      mMutex.lock();
      bool update = mDrawUpdate;
      mMutex.unlock();

      if (update)
      {
         // clear the display list and display list trash scratchpads
         display_list_scratchpad.clear();
         display_list_trash_scratchpad.clear();

         // snapshot things that need to be thread safe
         mMutex.lock();
         map_center_lat               = mMapCenterLat;
         map_center_lon               = mMapCenterLon;
         coverage_radius_scale_factor = mCoverageRadiusScaleFactor;
         scale_x                      = mMapScaleX;
         scale_y                      = mMapScaleY;
         map_zoom                     = mMapZoom;
         zoom_level                   = mZoomLevel;
         window_width                 = mMapWidthPix;
         window_height                = mMapHeightPix;
         mDrawUpdate                  = false;
         mMutex.unlock();

         // calculate the coverage radius in pixels as the greatest diagonal
         // from the map center of rotation offset to a corner of the window
         double coverage_radial_x = (window_width / 2.0) * coverage_radius_scale_factor * scale_x / map_zoom;
         double coverage_radial_y = (window_height / 2.0) * coverage_radius_scale_factor * scale_y / map_zoom;

         coverage_radius_pixels = sqrt((coverage_radial_x * coverage_radial_x) +
                                       (coverage_radial_y * coverage_radial_y)) / map_zoom;

         if (mWmtsTimeout > 0)
         {
            mWmtsTimeout--;
            if (mWmtsTimeout == 0)
            {
               mWmtsOnline = true;
            }
         }

         // get the tile list
         tile_list.clear();
         GetTileList(tile_list, map_center_lat, map_center_lon, zoom_level, coverage_radius_pixels);

         // update the image cache
         UpdateCache(tile_list,
                     display_list_scratchpad,
                     display_list_trash_scratchpad,
                     image_cache);

         // grab the mutex to make this chunk of code thread safe
         mMutex.lock();

         // copy the display list scratchpad to the display list to be drawn
         // in the Draw routine.  This is necessary to be in the correct drawing
         // context when the image is drawn.
         mDisplayList = display_list_scratchpad;

         // copy the display list trash scratchpad to the display list trash
         // to be emptied in the Draw routine.  This is necessary to be in
         // the correct drawing context when the image is deleted.
         for (int i = 0; i < display_list_trash_scratchpad.size(); i++)
         {
            mDisplayListTrash.push_back(display_list_trash_scratchpad[i]);
         }

         // release the mutex
         mMutex.unlock();
      }

      // take a break
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
   }

   // exiting .. grab the mutex to keep this chunk thread safe
   mMutex.lock();

   // put all of the images in the cache onto the display list trash to
   // be deleted by the Close function
   //for (int i = 0; i < image_cache.Size(); i++)
   //{
   //   TTile tile;
   //   if (image_cache.Peek(tile, i))
   //   {
   //      mDisplayListTrash.push_back(tile);
   //   }
   //}

   mMutex.unlock();
}

void COpenStreetMap::UpdateCache(TTileList&          TileList,
                                 std::vector<TTile>& DisplayListScratchpad,
                                 std::vector<TTile>& DisplayListTrashScratchpad,
                                 TImageCache&        ImageCache)
{
   TTile       tile;
   TTile       trash_tile;
   std::string png_filename;
   bool        got_file = false;

   static int prev_size = 0;
   if (ImageCache.Size() != prev_size)
   {
      prev_size = ImageCache.Size();
      ExecApiLogMessage("Cache size %d", prev_size);
   }

   // loop over the subframe coverage list
   for (int i = 0; i < TileList.size(); i++)
   {
      // check for terminate again to speed up exiting
      if (mTerminateCoverageThread) return;

      if (!ImageCache.Get(tile, TileList[i]))
      {
         // check if map source includes the local disk
         if (mCacheEnabled)
         {
            std::error_code err;

            // construct the file name
            png_filename = ConstructFilename(TileList[i].Zoom,
                                             TileList[i].X,
                                             TileList[i].Y);

            got_file = std::filesystem::exists(png_filename);
         }

         // check if map source includes the WMTS server and nothing has
         // been read in from the local png file
         if (mWmtsOnline && mWmtsEnabled && !got_file)
         {
            unsigned char* buffer;
            int            size;

            // get the png file from the WMTS server
            got_file = mWmtsIf.GetMapPngBuffer(TileList[i].Zoom,
                                              TileList[i].X,
                                              TileList[i].Y,
                                              &buffer,
                                              size);

            if (got_file)
            {
               // write the buffer out to a new png file if map source
               // includes the local png file
               if (mCacheEnabled)
               {
                  std::error_code err;
                  std::filesystem::create_directory(mCachePath, err);

                  std::ofstream png_file(png_filename, std::ios::out | std::ios::binary | std::ios::trunc);

                  png_file.write((char*)buffer, size);
               }
            }
            else
            {
               // bad png file from the tile server
               mWmtsOnline = false;
               mWmtsTimeout = SERVER_TIMEOUT;
            }
         }

         double ul_lat = GetLatitudeFromTileY(TileList[i].Y, TileList[i].Zoom);
         double ul_lon = GetLongitudeFromTileX(TileList[i].X, TileList[i].Zoom);
         double br_lat = GetLatitudeFromTileY(TileList[i].Y+1, TileList[i].Zoom);
         double br_lon = GetLongitudeFromTileX(TileList[i].X+1, TileList[i].Zoom);

         // allocate a new image
         tile.Texture   = nullptr;
         tile.Latitude  = (ul_lat + br_lat) / 2.0;
         tile.Longitude = (ul_lon + br_lon) / 2.0;
         tile.ZoomLevel = TileList[i].Zoom;
         tile.TileX     = TileList[i].X;
         tile.TileY     = TileList[i].Y;

         // check that the png file was retrieved
         if (got_file)
         {
            tile.Filename  = png_filename;
         }
         else
         {
            tile.Filename  = "no_data.png";
         }

         // check if the image cache is full
         if (ImageCache.IsFull())
         {
            // remove the oldest image from the cache
            ImageCache.GetBack(trash_tile);

            // move the old image onto the display list trash
            // scratchpad
            DisplayListTrashScratchpad.push_back(trash_tile);
         }

         // put the new image onto the cache
         ImageCache.PutFront(tile, TileList[i]);
      }

      // add the new image to the display list scratchpad
      DisplayListScratchpad.push_back(tile);
   }
}

void COpenStreetMap::Draw()
{
   static int prev_center_tile_x = 0;
   static int prev_center_tile_y = 0;
   static int prev_center_tile_zoom = 0;
   int    center_tile_x;
   int    center_tile_y;
   int    center_tile_zoom;
   double center_tile_pixels_x;
   double center_tile_pixels_y;
   double offset_pixels_x;
   double offset_pixels_y;

   // grab the mutex
   mMutex.lock();

   mDrawUpdate = true;

   if (mDisplayList.size())
   {
      // calculate the image offset in pixels from the map center of rotation
      // for the center tile
      center_tile_pixels_x = (mDisplayList[0].Longitude - mMapCenterLon) / mDegPerPixEw;
      center_tile_pixels_y = (mDisplayList[0].Latitude - mMapCenterLat) / mDegPerPixNs;
      center_tile_x        = mDisplayList[0].TileX;
      center_tile_y        = mDisplayList[0].TileY;
      center_tile_zoom     = mDisplayList[0].ZoomLevel;

      if (center_tile_x != prev_center_tile_x ||
          center_tile_y != prev_center_tile_y ||
          center_tile_zoom != prev_center_tile_zoom)
      {
         prev_center_tile_x = center_tile_x;
         prev_center_tile_y = center_tile_y;
         prev_center_tile_zoom = center_tile_zoom;
         ExecApiLogWarning("Center tile %d_%d_%d", center_tile_zoom, center_tile_x, center_tile_y);
      }
   }

   // loop through the display list
   for (auto& tile : mDisplayList)
   {
      if (!tile.Texture)
      {
         tile.Texture = GetOrCreateTexture(tile.Filename.c_str());
      }

      if (tile.ZoomLevel != mZoomLevel)
         continue;

      mMapScaleX = mMapZoom;
      mMapScaleY = mMapZoom;

      offset_pixels_x = (tile.TileX - center_tile_x) * OSM_TILE_SIZE * mMapScaleX;
      offset_pixels_y = -(tile.TileY - center_tile_y) * OSM_TILE_SIZE * mMapScaleY;

      glm::mat4 model(1.0f);

      model = glm::rotate(model, (float)(-mMapRotation * DEGREES_TO_RADIANS), glm::vec3(0.0f, 0.0f, 1.0f));
      model = glm::translate(model, glm::vec3(center_tile_pixels_x + offset_pixels_x,
                                              center_tile_pixels_y + offset_pixels_y,
                                              0.0f));
      model = glm::scale(model, glm::vec3(mMapScaleX, mMapScaleY, 0.0f));

      CGlRect tile_rect = CGlRect(mShaderRect, 0.0f, 0.0f, (float)OSM_TILE_SIZE, (float)OSM_TILE_SIZE);

      tile_rect.SetModelMatrix(model);
      tile_rect.SetTexture(tile.Texture);
      tile_rect.SetColor(glm::vec4(1.0f));
      tile_rect.Render(mMapProjection);

      // draw the subframe boundary
      if (mDrawSubframeBoundaries)
      {
         CGlLineStrip           linestrip = CGlLineStrip(mShaderLine, 0.0f, 0.0f, 0.0f, 0.0f);
         CGlRect                rect = CGlRect(mShaderRect, 0.0f, 0.0f, (float)OSM_TILE_SIZE, (float)OSM_TILE_SIZE);
         float                  half_size = (float)OSM_TILE_SIZE * 0.5f;
         std::vector<glm::vec3> points;

         points.push_back(glm::vec3(-half_size, -half_size, 0.0f));
         points.push_back(glm::vec3(-half_size,  half_size, 0.0f));
         points.push_back(glm::vec3( half_size,  half_size, 0.0f));
         points.push_back(glm::vec3( half_size, -half_size, 0.0f));
         points.push_back(glm::vec3(-half_size, -half_size, 0.0f));

         linestrip.SetLineWidth(3.0f);
         linestrip.SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
         linestrip.SetVertices(&points);
         linestrip.SetModelMatrix(model);
         linestrip.Render(mMapProjection);

         rect.SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 0.2f));
         rect.SetModelMatrix(model);
         rect.Render(mMapProjection);
      }
   }

   // Delete any tiles that have been evicted from the cache
   // TODO: Delete/remove textures from TextureMap and AvailableTextures
   mDisplayListTrash.clear();

   // release the mutex
   mMutex.unlock();

   if (mDrawSubframeBoundaries)
   {
      // draw viewport
      std::vector<glm::vec3> points;
      CGlLineStrip           viewport = CGlLineStrip(mShaderLine, 0.0f, 0.0f, 0.0f, 0.0f);
      CGlRect                rect = CGlRect(mShaderRect, 0.0f, 0.0f, mMapWidthPix, mMapHeightPix);

      points.push_back(glm::vec3(-(float)mMapWidthPix * 0.5f, -(float)mMapHeightPix * 0.5f, 0.0f));
      points.push_back(glm::vec3(-(float)mMapWidthPix * 0.5f,  (float)mMapHeightPix * 0.5f, 0.0f));
      points.push_back(glm::vec3( (float)mMapWidthPix * 0.5f,  (float)mMapHeightPix * 0.5f, 0.0f));
      points.push_back(glm::vec3( (float)mMapWidthPix * 0.5f, -(float)mMapHeightPix * 0.5f, 0.0f));
      points.push_back(glm::vec3(-(float)mMapWidthPix * 0.5f, -(float)mMapHeightPix * 0.5f, 0.0f));

      viewport.SetLineWidth(3.0f);
      viewport.SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
      viewport.SetVertices(&points);
      viewport.Render(mMapProjection);
   
      rect.SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 0.2f));
      rect.Render(mMapProjection);

      // draw coverage area
      CGlLineStrip coverage = CGlLineStrip(mShaderLine, 0.0f, 0.0f, 0.0f, 0.0f);
      float        coverage_radial_x;
      float        coverage_radial_y;
      float        coverage_radius_pixels;

      coverage_radial_x = ((float)mMapWidthPix * 0.5f) * mCoverageRadiusScaleFactor;
      coverage_radial_y = ((float)mMapHeightPix * 0.5f) * mCoverageRadiusScaleFactor;

      coverage_radius_pixels = sqrt(
            (coverage_radial_x * coverage_radial_x) +
            (coverage_radial_y * coverage_radial_y));

      points.clear();
      points.push_back(glm::vec3(-coverage_radius_pixels, -coverage_radius_pixels, 0.0f));
      points.push_back(glm::vec3(-coverage_radius_pixels,  coverage_radius_pixels, 0.0f));
      points.push_back(glm::vec3( coverage_radius_pixels,  coverage_radius_pixels, 0.0f));
      points.push_back(glm::vec3( coverage_radius_pixels, -coverage_radius_pixels, 0.0f));
      points.push_back(glm::vec3(-coverage_radius_pixels, -coverage_radius_pixels, 0.0f));

      coverage.SetLineWidth(3.0f);
      coverage.SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
      coverage.SetVertices(&points);
      coverage.SetModelMatrix(glm::mat4(1.0f));
      coverage.SetRotation(-mMapRotation * DEGREES_TO_RADIANS);
      coverage.Render(mMapProjection);
   }
}

void COpenStreetMap::EnableCache(bool Enable, const char* CachePath)
{
   if (!CachePath || strlen(CachePath) == 0)
   {
      mCacheEnabled = false;
      mCachePath = "";
      return;
   }

   mCacheEnabled = Enable;
   mCachePath    = CachePath;

   // append '/' to the path if not already there
   if (mCachePath[mCachePath.length() - 1] != '/') {
      mCachePath += '/';
   }
}

void COpenStreetMap::EnableWmtsServer(bool Enable, const char* WmtsUrl)
{
   if (Enable && WmtsUrl)
   {
      mWmtsEnabled = Enable;
      mWmtsUrl  = WmtsUrl;
   }
   else
   {
      mWmtsEnabled = false;
      mWmtsUrl  = "";
   }
}

void COpenStreetMap::EnableSubframeBoundaries(bool Enable)
{
   mDrawSubframeBoundaries = Enable;
}

double COpenStreetMap::GetLatitudeFromTileY(int Y, int Zoom) 
{
   double n = M_PI - 2.0 * M_PI * (double)Y / (double)(1 << Zoom);
   return atan(0.5 * (exp(n) - exp(-n))) * RADIANS_TO_DEGREES;
}

double COpenStreetMap::GetLongitudeFromTileX(int X, int Zoom) 
{
   return (double)X / (double)(1 << Zoom) * 360.0 - 180.0;
}

double COpenStreetMap::GetMetersPerPixelEw(double Latitude, int Zoom)
{
   return (GetMetersPerPixelNs(Zoom) / cos(Latitude * DEGREES_TO_RADIANS));
}

double COpenStreetMap::GetMetersPerPixelNs(int Zoom)
{
   return EQUATOR_CIRCUMFERENCE_M / (double)(1 << (Zoom + 8));
}

void COpenStreetMap::GetTileList(TTileList& TileList, double MapCenterLat, double MapCenterLon, int ZoomLevel, double CoverageRadiusPixels)
{
   TCacheTag tile;
   int       max_tiles = (1 << ZoomLevel);
   int       x = GetTileX(MapCenterLon, ZoomLevel);
   int       y = GetTileY(MapCenterLat, ZoomLevel);
   int       level = 0; // 0=1x1, 1=3x3, 2=5x5...
   bool      complete = false;

   if (ZoomLevel < 2)
   {
      // just add the center tile to the coverage list
      tile.Zoom = ZoomLevel;
      tile.X = x;
      tile.Y = y;

      TileList.push_back(tile);
   }
   else
   {
      while (!complete)
      {
         if (level == 0)
         {
            // add the center tile to the coverage list
            tile.Zoom = ZoomLevel;
            tile.X = x;
            tile.Y = y;

            TileList.push_back(tile);

            level++;
         }
         else
         {
            x++;
            y++;

            if (x >= max_tiles || y >= max_tiles)
               break;

            // move up by two times the "level" number of tiles
            for (int i = 0; i < (2*level); i++)
            {
               y--;

               if (y <= 0)
               {
                  complete = true;
                  break;
               }

               // add tile to the coverage list
               tile.X = x;
               tile.Y = y;
               TileList.push_back(tile);
            }

            if (complete)
               break;

            // move left by two times the "level" number of tiles
            for (int i = 0; i < (2*level); i++)
            {
               x--;

               if (x <= 0)
               {
                  complete = true;
                  break;
               }

               // add tile to the coverage list
               tile.X = x;
               tile.Y = y;
               TileList.push_back(tile);
            }

            if (complete)
               break;

            // move down by two times the "level" number of tiles
            for (int i = 0; i < (2*level); i++)
            {
               y++;

               if (y >= max_tiles)
               {
                  complete = true;
                  break;
               }

               // add tile to the coverage list
               tile.X = x;
               tile.Y = y;
               TileList.push_back(tile);
            }

            if (complete)
               break;

            // move right by two times the "level" number of tiles
            for (int i = 0; i < (2*level); i++)
            {
               x++;

               if (x >= max_tiles)
               {
                  complete = true;
                  break;
               }

               // add tile to the coverage list
               tile.X = x;
               tile.Y = y;
               TileList.push_back(tile);
            }

            if (complete)
               break;

            // check if level is enough to meet the coverage area
            double pixels_in_level = ((level * 2.0) + 1.0) * OSM_TILE_SIZE;

            if (pixels_in_level > CoverageRadiusPixels)
               complete = true;

            level++;
         }
      }
   }
}

int COpenStreetMap::GetTileX(double Longitude, int Zoom)
{
   return (int)(floor((Longitude + 180.0) / 360.0 * (1 << Zoom))); 
}

int COpenStreetMap::GetTileY(double Latitude, int Zoom)
{
   return (int)(floor((1.0 - asinh(tan(Latitude * DEGREES_TO_RADIANS)) / M_PI) / 2.0 * (1 << Zoom)));
}

void COpenStreetMap::GetZoom()
{
   if (mMapScaleFactor >= 500000000.0)
   {
      mZoomLevel = 0;
      mMapZoom = 500000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 250000000.0)
   {
      mZoomLevel = 1;
      mMapZoom = 250000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 150000000.0)
   {
      mZoomLevel = 2;
      mMapZoom = 150000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 70000000.0)
   {
      mZoomLevel = 3;
      mMapZoom = 70000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 35000000.0)
   {
      mZoomLevel = 4;
      mMapZoom = 35000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 15000000.0)
   {
      mZoomLevel = 5;
      mMapZoom = 15000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 10000000.0)
   {
      mZoomLevel = 6;
      mMapZoom = 10000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 4000000.0)
   {
      mZoomLevel = 7;
      mMapZoom = 4000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 2000000.0)
   {
      mZoomLevel = 8;
      mMapZoom = 2000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 1000000.0)
   {
      mZoomLevel = 9;
      mMapZoom = 1000000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 500000.0)
   {
      mZoomLevel = 10;
      mMapZoom = 500000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 250000.0)
   {
      mZoomLevel = 11;
      mMapZoom = 250000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 150000.0)
   {
      mZoomLevel = 12;
      mMapZoom = 150000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 70000.0)
   {
      mZoomLevel = 13;
      mMapZoom = 70000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 35000.0)
   {
      mZoomLevel = 14;
      mMapZoom = 35000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 15000.0)
   {
      mZoomLevel = 15;
      mMapZoom = 15000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 8000.0)
   {
      mZoomLevel = 16;
      mMapZoom = 8000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 4000.0)
   {
      mZoomLevel = 17;
      mMapZoom = 4000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 2000.0)
   {
      mZoomLevel = 18;
      mMapZoom = 2000.0 / mMapScaleFactor;
   }
   else if (mMapScaleFactor >= 1000.0)
   {
      mZoomLevel = 19;
      mMapZoom = 1000.0 / mMapScaleFactor;
   }
   else
   {
      mZoomLevel = 20;
      mMapZoom = 1000.0 / mMapScaleFactor;
   }
}

bool COpenStreetMap::Open(bool        WmtsEnabled,
                          const char* WmtsUrl,
                          bool        CacheEnabled,
                          const char* CachePath)
{
   if (mCoverageThread.joinable())
   {
      return false;
   }

   EnableWmtsServer(WmtsEnabled, WmtsUrl);
   EnableCache(CacheEnabled, CachePath);

   // Open the WMTS interface
   if (mWmtsEnabled)
   {
      std::string    capabilities_filename;
      unsigned char* buffer;
      int            size;
      bool           got_capabilities = false;

      // Get the wmts capabilities
      if (mWmtsIf.Open(mWmtsUrl.c_str(), 10))
         got_capabilities = mWmtsIf.GetWmtsCapabilitiesXml(&buffer, size);

      // write the wms capabilities to the file
      if (CachePath && got_capabilities && size)
      {
         capabilities_filename = CachePath;
         capabilities_filename += "/osm_wmts_capabilities.xml";

         std::ofstream wmts_capabilities_file(
               capabilities_filename,
               std::ios::out | std::ios::binary | std::ios::trunc);

         wmts_capabilities_file.write((char*)buffer, size);

         mWmtsOnline = true;
      }
      else
      {
         ExecApiLogWarning("Failed to connect with WMTS server");
         mWmtsTimeout = SERVER_TIMEOUT;
         mWmtsOnline = false;
      }
   }

   // kick off the coverage thread
   if (!mCoverageThread.joinable())
      mCoverageThread = std::thread(&COpenStreetMap::CoverageThread, this);

   return true;
}

void COpenStreetMap::SetMapCenter(double MapCenterLat, double MapCenterLon)
{
   mMutex.lock();
   mMapCenterLat = MapCenterLat;
   mMapCenterLon = MapCenterLon;
   mMutex.unlock();
}

void COpenStreetMap::SetMapRotation(double RotationClockwiseDeg)
{
   mMapRotation = -RotationClockwiseDeg;
}

void COpenStreetMap::SetMapScaleFactor(float ScaleFactor)
{
   mMutex.lock();
   mMapScaleFactor = ScaleFactor;
   mMutex.unlock();
}

void COpenStreetMap::SetMapSize(int MapWidthPix, int MapHeightPix)
{
   mMutex.lock();
   mMapWidthPix  = MapWidthPix;
   mMapHeightPix = MapHeightPix;
   mMutex.unlock();
}

void COpenStreetMap::Update()
{
   mMutex.lock();
   GetZoom();
   mMutex.unlock();

   // get the meters per pixel in the east-west and north-south directions
   mMetersPerPixEw = GetMetersPerPixelEw(mMapCenterLat, mZoomLevel);
   mMetersPerPixNs = GetMetersPerPixelNs(mZoomLevel);

   mDegPerPixEw = mMetersPerPixEw * M_TO_DEG;
   mDegPerPixNs = mMetersPerPixNs * M_TO_DEG;
}
