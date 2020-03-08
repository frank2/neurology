#include <neurology/win32/process.hpp>

using namespace Neurology;

Thread::Thread
(void)
   : handle(NULL_HANDLE)
   , process(NULL)
{
}

Thread::Thread
(Handle handle, Process *process)
   : handle(handle)
   , process(process)
{
}

Thread::Thread
(Thread &thread)
{
   *this = thread;
}

Thread &
Thread::operator=
(Thread &thread)
{
   this->handle = thread.handle;
   this->process = thread.process;
   return *this;
}

bool
Thread::isAlive
(void) const
{
   DWORD result;
   
   if (this->handle.isNull())
      return false;

   result = this->wait(0);

   return result == WAIT_TIMEOUT;
}

bool
Thread::isCurrentThread
(void) const
{
   return this->process->isCurrentProcess() && this->tid() == GetCurrentThreadId();
}

Handle &
Thread::getHandle
(void)
{
   return this->handle;
}

const Handle &
Thread::getHandle
(void) const
{
   return this->handle;
}

int
Thread::getPriority
(void) const
{
   int priority;

   priority = GetThreadPriority(*this->handle);

   if (priority == THREAD_PRIORITY_ERROR_RETURN)
      throw Win32Exception(EXCSTR(L"GetThreadPriority failed."));

   return priority;
}

void
Thread::setPriority
(int priority)
{
   if (!SetThreadPriority(*this->handle, priority))
      throw Win32Exception(EXCSTR(L"SetThreadPriority failed."));
}

TID
Thread::tid
(void) const
{
   TID tid;

   tid = GetThreadId(*this->handle);

   if (tid == 0)
      throw Win32Exception(EXCSTR(L"GetThreadId failed."));

   return tid;
}

void
Thread::suspend
(void)
{
   DWORD result;

   result = SuspendThread(*this->handle);

   if (result == (DWORD)-1)
      throw Win32Exception(EXCSTR(L"SuspendThread failed."));
}

void
Thread::resume
(void)
{
   DWORD result;

   result = ResumeThread(*this->handle);

   if (result == (DWORD)-1)
      throw Win32Exception(EXCSTR(L"ResumeThread failed."));
}

void
Thread::kill
(DWORD exitCode)
{
   if (!TerminateThread(*this->handle, exitCode))
      throw Win32Exception(EXCSTR(L"TerminateThread failed."));

   this->close();
}

void
Thread::close
(void)
{
   if (this->handle.isNull())
      return;
   
   this->handle.close();
}

DWORD
Thread::wait
(void) const
{
   return this->wait(INFINITE);
}

DWORD
Thread::wait
(DWORD timeout) const
{
   DWORD result;

   result = WaitForSingleObject(*this->handle, timeout);

   if (result == WAIT_FAILED)
      throw Win32Exception(EXCSTR(L"WaitForSingleObject failed."));

   return result;
}


DWORD
Thread::getExitCode
(void) const
{
   DWORD result;

   if (!GetExitCodeThread(*this->handle, &result))
      throw Win32Exception(EXCSTR(L"GetExitCodeThread failed."));

   return result;
}

Process::Exception::Exception
(const Process &process, const LPWSTR message)
   : Neurology::Exception(message)
   , process(process)
{
}

Process::RemoteProcessException::RemoteProcessException
(const Process &process)
   : Process::Exception(process, L"Operation cannot be performed on remote process.")
{
}

Process::Process
(void)
{
}

Process::Process
(Handle handle)
   : handle(handle)
{
}

Process::Process
(ProcessAccess access, PID pid)
{
   this->open(access, pid);
}

Process::Process
(ProcessAccess access, BOOL inheritHandle, PID pid)
{
   this->open(access, inheritHandle, pid);
}
 
Process::Process
(Process &process)
{
   *this = process;
}

Process
Process::Spawn
(std::wstring cmdLine)
{
   return Process::Spawn(cmdLine, 0);
}

Process
Process::Spawn
(std::wstring cmdLine, DWORD flags)
{
   STARTUPINFO startup;
   PROCESS_INFORMATION procInfo;
   WCHAR *tempBuffer;
   Process result;

   ZeroMemory(&startup, sizeof(STARTUPINFO));
   ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

   startup.cb = sizeof(STARTUPINFO);

   /* CreateProcess manipulates the buffer and std::basic_string.c_str
      returns a const buffer, so this is important */
   tempBuffer = new WCHAR[cmdLine.length()+1];
   CopyMemory(tempBuffer, cmdLine.c_str(), (cmdLine.length()+1)*sizeof(WCHAR));

   if (!CreateProcess(NULL
                      ,tempBuffer
                      ,NULL
                      ,NULL
                      ,FALSE
                      ,flags
                      ,NULL
                      ,NULL
                      ,&startup
                      ,&procInfo))
      throw Win32Exception(EXCSTR(L"CreateProcess failed."));

   result.handle = procInfo.hProcess;

   delete[] tempBuffer;

   return result;
}

Handle
Process::CurrentProcessHandle
(void)
{
   return Handle(GetCurrentProcess()).duplicate();
}

Process
Process::CurrentProcess
(void)
{
   return Process(Process::CurrentProcessHandle());
}

ProcessList
Process::ProcessList
(void)
{
   HANDLE snapshot;
   ProcessEntry entry;
   ::ProcessList result;

   snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

   if (snapshot == INVALID_HANDLE_VALUE)
      throw Win32Exception(EXCSTR(L"CreateToolhelp32Snapshot failed."));

   entry.dwSize = sizeof(PROCESSENTRY32);

   if (!Process32First(snapshot, &entry))
   {
      CloseHandle(snapshot);
      throw Win32Exception(EXCSTR(L"Process32First failed."));
   }

   do
   {
      result.push_back(entry);
   } while (Process32Next(snapshot, &entry));

   return result;
}

Process &
Process::operator=
(Process &process)
{
   this->handle = process.handle;
   return *this;
}

Handle &
Process::getHandle
(void)
{
   return this->handle;
}

const Handle &
Process::getHandle
(void) const
{
   return this->handle;
}

bool
Process::isAlive
(void) const
{
   DWORD result;
   
   if (this->handle.isNull())
      return false;

   result = this->wait(0);

   return result == WAIT_TIMEOUT;
}

bool
Process::isCurrentProcess
(void) const
{
   if (this->handle.isNull())
      return false;

   return this->pid() == GetProcessId(GetCurrentProcess());
}

PID
Process::pid
(void) const
{
   PID pid;
   
   pid = GetProcessId(*this->handle);

   if (pid == 0)
      throw Win32Exception(EXCSTR(L"GetProcessId failed."));

   return pid;
}

void
Process::open
(PID pid)
{
   this->open(PROCESS_ALL_ACCESS, FALSE, pid);
}

void
Process::open
(ProcessAccess access)
{
   this->open(access, FALSE, this->pid());
}

void
Process::open
(ProcessAccess access, PID pid)
{
   this->open(access, FALSE, pid);
}

void
Process::open
(ProcessAccess access, BOOL inheritHandle, PID pid)
{
   HANDLE result = OpenProcess(access, inheritHandle, pid);

   if (result == NULL)
      throw Win32Exception(EXCSTR(L"OpenProcess failed."));

   this->handle = result;
}

void
Process::close
(void)
{
   this->handle.close();
}

void
Process::kill
(UINT exitCode)
{
   if (!TerminateProcess(*this->handle
                         ,exitCode))
      throw Win32Exception(EXCSTR(L"TerminateProcess failed."));

   this->close();
}

DWORD
Process::wait
(void) const
{
   return this->wait(INFINITE);
}

DWORD
Process::wait
(DWORD timeout) const
{
   DWORD result;

   result = WaitForSingleObject(*this->handle, timeout);

   if (result == WAIT_FAILED)
      throw Win32Exception(EXCSTR(L"WaitForSingleObject failed."));

   return result;
}

Thread
Process::createThread
(LPTHREAD_START_ROUTINE start, LPVOID parameter)
{
   return this->createThread(start, parameter, 0);
}

Thread
Process::createThread
(LPTHREAD_START_ROUTINE start, LPVOID parameter, DWORD creationFlags)
{
   return this->createThread(start, parameter, creationFlags, 8192);
}

Thread
Process::createThread
(LPTHREAD_START_ROUTINE start, LPVOID parameter, DWORD creationFlags, SIZE_T stackSize)
{
   HANDLE result;
   Handle threadHandle;
   DWORD tid;

   if (this->isCurrentProcess())
      result = CreateThread(NULL
                            ,stackSize
                            ,start
                            ,parameter
                            ,creationFlags
                            ,&tid);
   else
      result = CreateRemoteThread(*this->handle
                                  ,NULL
                                  ,stackSize
                                  ,start
                                  ,parameter
                                  ,creationFlags
                                  ,&tid);

   if (result == NULL)
      throw Win32Exception(EXCSTR(L"CreateThread/CreateRemoteThread failed."));

   threadHandle = result;

   return Thread(threadHandle, this);
}

Thread
Process::openThread
(TID tid)
{
   return this->openThread(THREAD_ALL_ACCESS, tid);
}

Thread
Process::openThread
(ThreadAccess access, TID tid)
{
   return this->openThread(access, FALSE, tid);
}

Thread
Process::openThread
(ThreadAccess access, BOOL inheritHandle, TID tid)
{
   HANDLE result;
   Handle threadHandle;

   if (!this->isCurrentProcess())
      throw RemoteProcessException(*this);

   result = ::OpenThread(access.mask, inheritHandle, tid);

   if (result == NULL)
      throw Win32Exception(EXCSTR(L"OpenThread failed."));

   threadHandle = result;

   return Thread(threadHandle, this);
}

ThreadList
Process::threadList
(void) const
{
   HANDLE snapshot;
   ThreadEntry entry;
   PID pid;
   ThreadList result;

   snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

   if (snapshot == INVALID_HANDLE_VALUE)
      throw Win32Exception(EXCSTR(L"CreateToolhelp32Snapshot failed."));

   entry.dwSize = sizeof(THREADENTRY32);

   if (!Thread32First(snapshot, &entry))
   {
      CloseHandle(snapshot);
      throw Win32Exception(EXCSTR(L"Thread32First failed."));
   }

   pid = this->pid();
   
   do
   {
      if (entry.th32OwnerProcessID != pid)
         continue;

      result.push_back(entry);
   } while (Thread32Next(snapshot, &entry));

   return result;
}
