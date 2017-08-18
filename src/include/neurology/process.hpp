#pragma once

#include <windows.h>

#include <neurology/exception.hpp>
#include <neurology/heap.hpp>

namespace Neurology
{
   class OpenProcessException : public Win32Exception
   {
   public:
      OpenProcessException(void);
      OpenProcessException(OpenProcessException &exception);
   }
   
   class Process
   {
   public:
      static Process CurrentProcess;
      const static DWORD INVALID_PID = 0xFFFFFFFF;
      
   protected:
      DWORD pid;
      HANDLE handle;

   public:
      Process(void);
      Process(HANDLE processHandle);
      Process(DWORD desiredAccess, BOOL inheritHandle, DWORD processID);
      Process(Process &process);

      HANDLE getHandle(void);
      HeapAllocation read(LPVOID address, SIZE_T size, SIZE_T *bytesRead);
   };
}
