#pragma once

#include <windows.h>

namespace Neurology
{
   class Memory
   {
   protected:
      Process *process;
      LPVOID buffer;
      SIZE_T size;

   public:
      Memory(void);
      Memory(LPVOID buffer, SIZE_T size);
      Memory(SIZE_T size, LPVOID base, DWORD allocationType, DWORD protect);
      Memory(Process *process, SIZE_T size, LPVOID base, DWORD allocationType, DWORD protect);

      LPVOID getBuffer(void);
      SIZE_T getSize(void) const;
      Process *getProcess(void);

      void setBuffer(LPVOID buffer, SIZE_t size);
      void setProcess(Process *process);

      void allocate(SIZE_T size, LPVOID base, DWORD allocationType, DWORD protect);
      void reallocate(SIZE_T size);
      void free(void);
   };
}
