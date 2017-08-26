#pragma once

#include <windows.h>

#include <neurology/exception.hpp>
#include <neurology/memory.hpp>

namespace Neurology
{
   class NullHeapHandleException : public NeurologyException
   {
   public:
      NullHeapHandleException(void);
      NullHeapHandleException(HeapCreateException &exception);
   };
   
   class HeapCreateException : public Win32Exception
   {
   public:
      HeapCreateException(void);
      HeapCreateException(HeapCreateException &exception);
   };

   class HeapAllocException : public Win32Exception
   {
   public:
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
   
   class HeapAllocator
   {
   public:
      static HeapAllocator ProcessHeap;

   protected:
      HANDLE heapHandle;

   public:
      HeapAllocator(void);
      HeapAllocator(HANDLE heapHandle);
      HeapAllocator(DWORD options, SIZE_T initialSize, SIZE_T maximumSize);
      HeapAllocator(HeapAllocator &heap);

      static HeapAllocation Allocate(DWORD flags, SIZE_T size);

      HANDLE getHeapHandle(void);
      void setHeapHandle(HANDLE handle);
      
      HeapAllocation allocate(DWORD flags, SIZE_T size);
   };

   class HeapAllocation : public Memory
   {
   protected:
      HeapAllocator *heap;

   public:
      HeapAllocation(void);
      HeapAllocation(HeapAllocator *heap, LPVOID buffer, SIZE_T size);
      HeapAllocation(HeapAllocation &allocation);
      ~HeapAllocation(void);

      Heap *getHeap(void);

      void reallocate(DWORD flags, SIZE_T size);
      void free(void);
   };
}
