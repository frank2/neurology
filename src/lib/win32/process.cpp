#include <neurology/win32/process.hpp>

using namespace Neurology;

Process::Exception::Exception
(const Process &process, const LPWSTR message)
   : Neurology::Exception(message)
   , process(process)
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
(AccessMask access, PID pid)
{
   this->open(access, pid);
}

Process::Process
(AccessMask access, BOOL inheritHandle, PID pid)
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

   result = WaitForSingleObject(*this->handle, 0);

   return result == WAIT_TIMEOUT;
}

PID
Process::pid
(void) const
{
   return GetProcessId(*this->handle);
}

void
Process::open
(PID pid)
{
   this->open(PROCESS_ALL_ACCESS, FALSE, pid);
}

void
Process::open
(AccessMask access)
{
   this->open(access, FALSE, this->pid());
}

void
Process::open
(AccessMask access, PID pid)
{
   this->open(access, FALSE, pid);
}

void
Process::open
(AccessMask access, BOOL inheritHandle, PID pid)
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
