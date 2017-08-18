#pragma once

#include <windows.h>

#include <vector>

#include <neurology/exception.hpp>

namespace Neurology
{
   typedef std::vector<BYTE> MemoryData;
#define MemoryVar(var) MemoryData((LPBYTE)(&(var)), (LPBYTE)((&(var))+1))
#define MemoryPointer(ptr) MemoryData((LPBYTE)(ptr), (LPBYTE)((ptr)+1))
#define MemoryBlock(ptr, size) MemoryData((LPBYTE)(ptr), ((LPBYTE)(ptr))+size)

   class NullPointerException : public NeurologyException
   {
   public:
      NullPointerException(void);
      NullPointerException(NullPointerException &exception);
   };

   class BadPointerException : public NeurologyException
   {
   public:
      LPVOID address;
      SIZE_T size;

      BadPointerException(LPVOID address, SIZE_T size);
      BadPointerException(BadPointerException &exception);
   };

   class OffsetOutOfBoundsException : public NeurologyException
   {
   public:
      Memory *memory;
      SIZE_T offset, size;
      
      OffsetOutOfBoundsException(Memory *memory, SIZE_T offset, SIZE_T size);
      OffsetOutOfBoundsException(OffsetOutOfBoundsException &exception);
   };

   class Memory
   {
   protected:
      LPVOID *buffer;
      SIZE_T *size;
      LPDWORD refCount;

   public:
      Memory(void);
      Memory(LPVOID buffer, SIZE_T size);
      Memory(Memory &memory);
      ~Memory(void);

      SIZE_T size(void) const;
      virtual void free(void);
      MemoryData read(void);
      MemoryData read(SIZE_T size);
      virtual MemoryData read(SIZE_T offset, SIZE_T size);
      void write(Memory region);
      void write(MemoryData data);
      void write(SIZE_T offset, Memory region);
      virtual void write(SIZE_T offset, MemoryData data);
   };
}
