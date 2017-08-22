#pragma once

#include <windows.h>

#include <map>

#include <neurology/exception.hpp>
#include <neurology/memory.hpp>

namespace Neurology
{
   class NullProcessException : public NeurologyException
   {
   public:
      NullProcessException(void);
      NullProcessException(NullProcessException &exception);
   };

   class ProcessMemory;
   
   class Process
   {
   public:
      static Process CurrentProcess;
      const static DWORD CURRENT_PROCESS_HANDLE = 0xFFFFFFFF;

   protected:
      HANDLE handle;

   public:
      Process(void);
      Process(HANDLE processHandle);
      Process(DWORD desiredAccess, BOOL inheritHandle, DWORD processID);
      Process(Process &process);

      DWORD pid(void);
      void openProcess(DWORD desiredAccess, BOOL inheritHandle, DWORD processID);
      HANDLE getHandle(void);
      Data read(LPVOID address, SIZE_T size);
   };

   class ProcessMemory;
   
   class ProcessMemoryManager : public Memory
   {
   protected:
      Process *process;
      std::map<Address, ProcessMemory> memory;
   };

   class ProcessMemory : public Memory
   {
   protected:
      ProcessMemoryManager *manager;

   public:
      ProcessMemory(void);
      ProcessMemory(Process *process, LPVOID address, SIZE_T size);
      ProcessMemory(ProcessMemory &memory);
   };
}
