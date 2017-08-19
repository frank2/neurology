#include <neurology/process.hpp>

Process Process::CurrentProcess;

NullProcessException::NullProcessException
(void)
   : NeurologyException("the process handle is null")
{
}

NullProcessException::NullProcessException
(NullProcessException &exception)
   : NeurologyException(exception)
{
}

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
   this->handle = NULL;
}

Process::Process
(HANDLE processHandle)
{
   this->handle = processHandle;
}

Process::Process
(DWORD desiredAccess, BOOL inheritHandle, DWORD processID)
{
   this->openProcess(desiredAccess, inheritHandle, processID);
}

Process::Process
(Process &process)
{
   this->handle = process.handle;
}

DWORD
Process::pid
(void)
{
   if (this->handle == NULL)
      throw NullProcessException();
   
   return GetProcessId(this->handle);
}

void
Process::openProcess
(DWORD desiredAccess, BOOL inheritHandle, DWORD processID)
{
   this->handle = OpenProcess(desiredAccess, inheritHandle, processID);

   if (this->handle == NULL)
      throw OpenProcessException();
}

HANDLE
Process::getHandle
(void)
{
   return this->handle;
}

Data
Process::read
(LPVOID address, SIZE_T size, SIZE_T *bytesRead)
{
}
