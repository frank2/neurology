#include <neurology/process.hpp>

Process Process::CurrentProcess;

OpenProcessException::OpenProcessException
(void)
   : Win32Exception("OpenProcess() failed")
{
}

OpenProcessException::OpenProcessException
(OpenProcessException &exception)
   : Win32Exception(exception)
{
}
    
Process::Process
(void)
{
   this->handle = INVALID_HANDLE_VALUE;
}

Process::Process
(HANDLE processHandle)
{
   this->handle = processHandle;
}

Process::Process
(DWORD desiredAccess, BOOL inheritHandle, DWORD processID)
{
   this->handle = OpenProcess(desiredAccess, inheritHandle, processID);

   if (this->handle == NULL)
      throw OpenProcessException();
}

Process::Process
(Process &process)
{
   this->handle = process.getHandle();
}

HANDLE
Process::getHandle
(void)
{
   return this->handle;
}

HeapAllocation
Process::read
(LPVOID address, SIZE_T size, SIZE_T *bytesRead)
{
}
