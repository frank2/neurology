#pragma once

#include <windows.h>

namespace Neurology
{
   class Process
   {
   public:
      static Process CurrentProcess;
      
   protected:
      DWORD pid;
      HANDLE handle;

   public:
      Process(void);
      Process(DWORD desiredAccess, BOOL inheritHandle, DWORD processID);
      Process(Process &process);

      LPVOID read(LPVOID baseAddress, SIZE_T size, SIZE_T *bytesRead);
   };
}
