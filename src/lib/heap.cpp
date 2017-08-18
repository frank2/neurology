#include <neurology/heap.hpp>

using namespace Neurology;

Heap Heap::ProcessHeap(GetProcessHeap());

HeapCreateException::HeapCreateException
(void)
   : NeurologyException("HeapCreate() failed")
{
   this->error = GetLastError();
}

HeapCreateException::HeapCreateException
(HeapCreateException &exception)
   : NeurologyException(exception)
{
   this->error = exception.error;
}

HeapAllocException::HeapAllocException
(void)
   : NeurologyException("HeapAlloc()/HeapReAlloc() failed")
{
   this->error = GetLastError();
}

HeapAllocException::HeapAllocException
(HeapAllocException &exception)
   : NeurologyException(exception)
{
   this->error = exception.error;
}

HeapUseAfterFreeException::HeapUseAfterFreeException
(void)
   : NeurologyException("heap allocation being used after calling free()")
{
}

HeapUseAfterFreeException::HeapUseAfterFreeException
(HeapUseAfterFreeException &exception)
   : NeurologyException(exception)
{
}

Heap::Heap
(HANDLE heapHandle)
{
   this->heapHandle = heapHandle;
}

Heap::Heap
(DWORD options, SIZE_T initialSize, SIZE_T maximumSize)
{
   this->heapHandle = HeapCreate(options, initialSize, maximumSize);

   if (this->heapHandle == NULL)
      throw HeapCreateException();
}

Heap::Heap
(Heap &heap)
{
   this->heapHandle = heap.getHeapHandle();
}

HeapAllocation
Heap::Allocate
(DWORD flags, SIZE_T size)
{
   return Heap::ProcessHeap.allocate(flags, size);
}

HANDLE
Heap::getHeapHandle
(void)
{
   return this->heapHandle;
}

void
Heap::setHeapHandle
(HANDLE handle)
{
   this->heapHandle = handle;
}

HeapAllocation
Heap::allocate
(DWORD flags, SIZE_T size)
{
   LPVOID allocation = HeapAlloc(this->heapHandle, flags, size);

   if (allocation == NULL)
      throw HeapAllocException();

   DWORD *refCount = (LPDWORD)HeapAlloc(this->heapHandle, HEAP_ZERO_MEMORY, sizeof(DWORD));

   if (refCount == NULL)
   {
      HeapFree(this->heapHandle, NULL, allocation);
      throw HeapAllocException();
   }

   LPVOID *allocationPointer = (LPVOID *)HeapAlloc(this->heapHandle, HEAP_ZERO_MEMORY, sizeof(LPVOID));

   if (allocationPointer == NULL)
   {
      HeapFree(this->heapHandle, NULL, allocation);
      HeapFree(this->heapHandle, NULL, refCount);
      throw HeapAllocException();
   }

   *allocationPointer = allocation;

   SIZE_T *sizePointer = (SIZE_T *)HeapAlloc(this->heapHandle, HEAP_ZERO_MEMORY, sizeof(SIZE_T));

   if (sizePointer == NULL)
   {
      HeapFree(this->heapHandle, NULL, allocation);
      HeapFree(this->heapHandle, NULL, refCount);
      HeapFree(this->heapHandle, NULL, allocationPointer);
      throw HeapAllocException();
   }

   *sizePointer = size;

   return HeapAllocation(this, refCount, allocationPointer, sizePointer);
}

HeapAllocation::HeapAllocation
(Heap *heap, DWORD *refCount, LPVOID *buffer, SIZE_T *size)
{
   this->heap = heap;
   this->refCount = refCount;
   this->buffer = buffer;
   this->size = size;

   ++*this->refCount;
}

HeapAllocation::HeapAllocation
(HeapAllocation &allocation)
{
   this->heap = allocation.getHeap();
   this->buffer = allocation.getBufferPointer();
   this->refCount = allocation.getRefCountPointer();
   this->size = allocation.getSizePointer();

   ++*this->refCount;
}

HeapAllocation::~HeapAllocation
(void)
{
   this->free();
}

Heap *
HeapAllocation::getHeap
(void)
{
   return this->heap;
}

LPVOID
HeapAllocation::getBuffer
(void)
{
   if (this->heap == NULL)
      throw HeapUseAfterFreeException();
   
   return *this->buffer;
}

LPVOID *
HeapAllocation::getBufferPointer
(void)
{
   return this->buffer;
}

SIZE_T
HeapAllocation::getSize
(void)
{
   if (this->heap == NULL)
      throw HeapUseAfterFreeException();
   
   return *this->size;
}

SIZE_T *
HeapAllocation::getSizePointer
(void)
{
   return this->size;
}

DWORD
HeapAllocation::getRefCount
(void)
{
   if (this->heap == NULL)
      throw HeapUseAfterFreeException();
   
   return *this->refCount;
}

DWORD *
HeapAllocation::getRefCountPointer
(void)
{
   return this->refCount;
}

void
HeapAllocation::reallocate
(DWORD flags, SIZE_T size)
{
   LPVOID newBuffer;
   
   if (this->heap == NULL)
      throw HeapUseAfterFreeException();
   
   newBuffer = HeapReAlloc(this->heap->getHeapHandle(), flags, this->buffer, size);

   if (newBuffer == NULL)
      throw HeapAllocException();

   *this->buffer = newBuffer;
   *this->size = size;
}

void
HeapAllocation::free
(void)
{
   --*this->refCount;

   if (*this->refCount != 0)
      return;
   
   HeapFree(this->heap->getHeapHandle(), NULL, this->refCount);
   HeapFree(this->heap->getHeapHandle(), NULL, *this->buffer);

   *this->buffer = NULL;

   HeapFree(this->heap->getHeapHandle(), NULL, this->buffer);
   HeapFree(this->heap->getHeapHandle(), NULL, this->size);

   this->buffer = NULL;
   this->refCount = NULL;
   this->size = NULL;
   this->heap = NULL;
}
