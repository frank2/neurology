#pragma once

#include <windows.h>

#include <neurology/exception.hpp>

namespace Neurology
{
   class HeapCreateException : public NeurologyException
   {
   public:
      DWORD error;

      HeapCreateException(void);
      HeapCreateException(HeapCreateException &exception);
   };

   class HeapAllocException : public NeurologyException
   {
   public:
      DWORD error;

      HeapAllocException(void);
      HeapAllocException(HeapAllocException &exception);
   };

   class HeapUseAfterFreeException : public NeurologyException
   {
   public:
      HeapUseAfterFreeException(void);
      HeapUseAfterFreeException(HeapUseAfterFreeException &exception);
   };
   
   class HeapAllocation;
   
   class Heap
   {
   public:
      static Heap ProcessHeap;

   protected:
      HANDLE heapHandle;

   public:
      Heap(HANDLE heapHandle);
      Heap(DWORD options, SIZE_T initialSize, SIZE_T maximumSize);
      Heap(Heap &heap);

      static HeapAllocation Allocate(DWORD flags, SIZE_T size);

      HANDLE getHeapHandle(void);
      void setHeapHandle(HANDLE handle);
      
      HeapAllocation allocate(DWORD flags, SIZE_T size);
   };

   class HeapAllocation
   {
   protected:
      Heap *heap;
      DWORD *refCount;
      LPVOID *buffer;
      SIZE_T *size;

   public:
      HeapAllocation(Heap *heap, DWORD *refCount, LPVOID *buffer, SIZE_T *size);
      HeapAllocation(HeapAllocation &allocation);
      ~HeapAllocation(void);

      Heap *getHeap(void);
      LPVOID getBuffer(void);
      LPVOID *getBufferPointer(void);
      SIZE_T getSize(void);
      SIZE_T *getSizePointer(void);
      DWORD getRefCount(void);
      DWORD *getRefCountPointer(void);

      void reallocate(DWORD flags, SIZE_T size);
      void free(void);
   };
}
