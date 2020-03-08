#pragma once

#include <windows.h>
#include <tlhelp32.h>

#include <string>
#include <vector>

#include <neurology/exception.hpp>
#include <neurology/win32/access.hpp>
#include <neurology/win32/handle.hpp>

namespace Neurology
{
   typedef DWORD PID;
   typedef DWORD TID;
   typedef PROCESSENTRY32 ProcessEntry;
   typedef std::vector<ProcessEntry> ProcessList;
   typedef THREADENTRY32 ThreadEntry;
   typedef std::vector<ThreadEntry> ThreadList;
   
   class Process;

   class Thread
   {
   protected:
      Handle handle;
      Process *process;

   public:
      Thread(void);
      Thread(Handle handle, Process *process);
      Thread(Thread &thread);

      Thread &operator=(Thread &thread);

      bool isAlive(void) const;
      bool isCurrentThread(void) const;

      Handle &getHandle(void);
      const Handle &getHandle(void) const;

      int getPriority(void) const;
      void setPriority(int priority);
      
      TID tid(void) const;
      
      void suspend(void);
      void resume(void);
      void kill(DWORD exitCode);
      void close(void);
      DWORD wait(void) const;
      DWORD wait(DWORD timeout) const;

      DWORD getExitCode(void) const;
   };

   class Process
   {
   public:
      class Exception : public Neurology::Exception
      {
      public:
         const Process &process;

         Exception(const Process &process, const LPWSTR message);
      };

      class RemoteProcessException : public Exception
      {
      public:
         RemoteProcessException(const Process &process);
      };

   protected:
      Handle handle;

   public:
      Process(void);
      Process(Handle handle);
      Process(ProcessAccess access, PID pid);
      Process(ProcessAccess access, BOOL inheritHandle, PID pid);
      Process(Process &process);

      static Process Spawn(std::wstring cmdLine);
      static Process Spawn(std::wstring cmdLine, DWORD flags);
      static Handle CurrentProcessHandle(void);
      static Process CurrentProcess(void);
      static ProcessList ProcessList(void);

      Process &operator=(Process &process);
      
      Handle &getHandle(void);
      const Handle &getHandle(void) const;

      bool isAlive(void) const;
      bool isCurrentProcess(void) const;

      PID pid(void) const;

      void open(PID pid);
      void open(ProcessAccess access);
      void open(ProcessAccess access, PID pid);
      void open(ProcessAccess access, BOOL inheritHandle, PID pid);
      void close(void);
      void kill(UINT exitCode);
      DWORD wait(void) const;
      DWORD wait(DWORD timeout) const;

      Thread createThread(LPTHREAD_START_ROUTINE start, LPVOID parameter);
      Thread createThread(LPTHREAD_START_ROUTINE start, LPVOID parameter, DWORD creationFlags);
      Thread createThread(LPTHREAD_START_ROUTINE start, LPVOID parameter, DWORD creationFlags
                          ,SIZE_T stackSize);

      Thread openThread(TID tid);
      Thread openThread(ThreadAccess access, TID tid);
      Thread openThread(ThreadAccess access, BOOL inheritHandle, TID tid);

      ThreadList threadList(void) const;
   };
}
