#pragma once

#include <deque>

template<typename TItem, typename TTag, int MAX_ITEMS>
class Cache
{
public:
   Cache();
   ~Cache();

   void Clear();

   bool Get(TItem& Item, const TTag Tag);

   bool Get(TItem& Item, int Index);

   bool GetBack(TItem& Item);

   bool GetFront(TItem& Item);

   bool IsFull();

   bool Peek(TItem& Item, int Index);

   bool PutFront(TItem Item, TTag Tag);

   int Size();

private:

   // defines a cache record
   struct TListItem
   {
      TItem item;
      TTag  tag;
   };

   // list that contains the items in the cache
   std::deque<TListItem> mList;
   int                   mMaxItems;
};

template<typename TItem, typename TTag, int MAX_ITEMS>
Cache<TItem, TTag, MAX_ITEMS>::Cache()
{
   mMaxItems = MAX_ITEMS;
}

template<typename TItem, typename TTag, int MAX_ITEMS>
Cache<TItem, TTag, MAX_ITEMS>::~Cache()
{
   Clear();
}


template<typename TItem, typename TTag, int MAX_ITEMS>
void Cache<TItem, TTag, MAX_ITEMS>::Clear()
{
   mList.clear();
}

template<typename TItem, typename TTag, int MAX_ITEMS>
bool Cache<TItem, TTag, MAX_ITEMS>::Get(TItem& Item, int Index)
{
   if (Index >= mList.size()) return false;
   
   Item = mList[Index].item;

   return true;
}

template<typename TItem, typename TTag, int MAX_ITEMS>
bool Cache<TItem, TTag, MAX_ITEMS>::Get(TItem& Item, const TTag Tag)
{
   TListItem list_item;

   // find the item in the list by comparing the tags.  If a match is found
   // move the item to the front of the list. 
   for (unsigned int i = 0; i < mList.size(); i++)
   {
      if (mList[i].tag == Tag)
      {
         list_item = mList[i];
         if (i != 0)
         {
            mList.erase(mList.begin() + i);
            mList.push_front(list_item);
         }
         Item = list_item.item;
         return true;
      }
   }

   // not on the list
   return false; 
}

template<typename TItem, typename TTag, int MAX_ITEMS>
bool Cache<TItem, TTag, MAX_ITEMS>::GetBack(TItem& Item)
{
   TListItem list_item;

   // check if the list is empty
   if (mList.empty()) return false;

   // get the item at the back of the list and then remove it from the list
   list_item = mList.back();
   mList.pop_back();
   Item = list_item.item;

   return true;
}

template<typename TItem, typename TTag, int MAX_ITEMS>
bool Cache<TItem, TTag, MAX_ITEMS>::GetFront(TItem& Item)
{
   TListItem list_item;

   // check if the list is empty
   if (mList.empty()) return false;

   // get the item at the front of the list and then remove it from the list
   list_item = mList.front();
   mList.pop_front();
   Item = list_item.item;

   return true;
}

template<typename TItem, typename TTag, int MAX_ITEMS>
bool Cache<TItem, TTag, MAX_ITEMS>::IsFull()
{
   return mList.size() >= mMaxItems;
}

template<typename TItem, typename TTag, int MAX_ITEMS>
bool Cache<TItem, TTag, MAX_ITEMS>::Peek(TItem& Item, int Index)
{
   // check the index
   if (Index >= mList.size()) return false;

   // copy the item out
   Item = mList[Index].item;

   return true;
}

template<typename TItem, typename TTag, int MAX_ITEMS>
bool Cache<TItem, TTag, MAX_ITEMS>::PutFront(TItem Item, TTag Tag)
{
   TListItem list_item;

   // check if the list is full
   if (IsFull()) return false;

   // put on the front of the list
   list_item.item = Item;
   list_item.tag  = Tag;
   mList.push_front(list_item);

   return true;
}

template<typename TItem, typename TTag, int MAX_ITEMS>
int Cache<TItem, TTag, MAX_ITEMS>::Size()
{
   // return the size of the cache
   return mList.size();
}
