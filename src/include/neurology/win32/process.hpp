#pragma once

#include <windows.h>

#include <set>

#include <neurology/exception.hpp>
#include <neurology/handle.hpp>
#include <neurology/memory.hpp>

namespace Neurology
{
   class ProcessException : public NeurologyException
   {
   public:
      Process *process;

   public:
      ProcessException(Process *process, const LPWSTR message);
      ProcessException(ProcessException &exception);
   };

   class NullProcessException : public ProcessException
   {
   public:
      NullProcessException(void);
      NullProcessException(NullProcessException &exception);
   };

   class Page : public Memory
   {
   protected:
      Book *binding;
      
   public:
      Page(void);
      Page(Book *binding, LPVOID address, SIZE_T size, Memory::Mode mode);
      Page(Page &memory);
   };
   
   class Book
   {
   protected:
      Process *process;
      std::map<LPVOID, Page> pages;

   public:
      Book(void);
      Book(Process *process);
      Book(Book &book);
   };
   
   class Process
   {
   public:
      static Process Current;
      const static HANDLE CURRENT_PROCESS_HANDLE = (HANDLE)0xFFFFFFFF;

   protected:
      Book memory;

   public:
      Process(void);
      Process(HANDLE processHandle);
      Process(DWORD processID);
      Process(BOOL inheritHandle, DWORD processID);
      Process(DWORD desiredAccess, BOOL inheritHandle, DWORD processID);
      Process(Process &process);

      void openProcess(DWORD desiredAccess, BOOL inheritHandle, DWORD processID);
      DWORD pid(void);
      HANDLE getHandle(void);
      Data read(LPVOID address, SIZE_T size);
   };
}
