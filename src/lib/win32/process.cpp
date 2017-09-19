#include <neurology/win32/process.hpp>

using namespace Neurology;

Process Process::CurrentProcess(Process::CurrentProcessHandle());

Process::Exception
(const Process &process, const LPWSTR message)
   : Neurology::Exception(process, message)
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
(Rights access, PID pid)
{
   this->open(access, pid);
}

Process::Process
(Rights access, BOOL inheritHandle, PID pid)
{
   this->open(access, inheritHandle, pid);
}

Process::Process
(Process &process)
{
   *this = process;
}

Process::Process
(const Process *process)
{
   *this = process;
}

Handle
Process::CurrentProcessHandle
(void)
{
   return Handle(GetCurrentProcess());
}

Process
Process::CurrentProcess
(void)
{
   return Process(PROCESS_ALL_ACCESS, Process::ProcessID(Process::CurrentProcessHandle()));
}

PID
Process::ProcessID
(Handle processHandle)
{
   return GetProcessId(*processHandle);
}

Process &
Process::operator=
(Process &process)
{
   this->handle = process.handle;
}

Process &
Process::operator=
(const Process *process)
{
   this->handle = &process->handle;
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

PID
Process::pid
(void) const
{
   return Process::ProcessID(this->handle);
}

void
Process::open
(PID pid)
{
   this->open(PROCESS_ALL_ACCESS, FALSE, pid);
}

void
Process::open
(Rights access, PID pid)
{
   this->open(access, FALSE, pid);
}

void
Process::open
(Rights access, BOOL inheritHandle, PID pid)
{
   HANDLE result = OpenProcess(access, inheritHandle, pid);

   if (result == NULL)
      throw Win32Error(EXCSTR(L"OpenProcess failed."));

   this->handle = result;
}

void
Process::close
(void)
{
   this->handle.close();
}
