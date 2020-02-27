#include <neurology/win32/handle.hpp>

using namespace Neurology;

Handle::Exception::Exception
(Handle &handle, const LPWSTR message)
   : Neurology::Exception(message)
   , handle(handle)
{
}

Handle::NullHandleException::NullHandleException
(Handle &handle)
   : Handle::Exception(handle, EXCSTR(L"The handle is null."))
{
}

Handle::Handle
(void)
   : handle(NULL_HANDLE)
{
}

Handle::Handle
(HANDLE handle)
   : handle(handle)
{
}

Handle::Handle
(Handle &handle)
{
   *this = handle;
}

Handle::~Handle
(void)
{
   if (this->handle.references() == 1)
      this->close();
}

Handle &
Handle::operator=
(HANDLE handle)
{
   this->handle = handle;
   return *this;
}

Handle &
Handle::operator=
(Handle &handle)
{
   this->handle = handle.handle;
   return *this;
}

bool
Handle::operator==
(const Handle &handle)
{
   return *this->handle == *handle.handle;
}

bool
Handle::operator==
(HANDLE handle)
{
   return *this->handle == handle;
}

bool
Handle::operator!=
(const Handle &handle)
{
   return *this->handle != *handle.handle;
}

bool
Handle::operator!=
(HANDLE handle)
{
   return *this->handle == handle;
}

HANDLE &
Handle::operator*
(void)
{
   return *this->handle;
}

const HANDLE &
Handle::operator*
(void) const
{
   return *this->handle;
}

bool
Handle::isNull
(void) const
{
   return *this->handle == NULL_HANDLE;
}

bool
Handle::isValid
(void) const
{
   return !this->isNull() && *this->handle != INVALID_HANDLE_VALUE;
}

void
Handle::throwIfNull
(void) const
{
   if (this->isNull())
      throw NullHandleException(*const_cast<Handle *>(this));
}

HANDLE
Handle::getHandle
(void) 
{
   return *this->handle;
}

HANDLE
Handle::getHandle
(void) const
{
   return *this->handle;
}

void
Handle::setHandle
(HANDLE handle)
{
   this->handle = handle;
}

Handle
Handle::duplicate
(void) 
{
   return this->duplicate(GetCurrentProcess()
                          ,GetCurrentProcess()
                          ,NULL
                          ,FALSE
                          ,DUPLICATE_SAME_ACCESS);
}

Handle
Handle::duplicate
(DWORD access, BOOL inheritHandle, DWORD options)
{
   return this->duplicate(GetCurrentProcess()
                          ,GetCurrentProcess()
                          ,access
                          ,inheritHandle
                          ,options);
}

Handle
Handle::duplicate
(Handle sourceProcess, Handle destProcess)
{
   return this->duplicate(sourceProcess
                          ,destProcess
                          ,NULL
                          ,FALSE
                          ,DUPLICATE_SAME_ACCESS);
}

Handle
Handle::duplicate
(Handle sourceProcess, Handle destProcess, DWORD access, BOOL inheritHandle, DWORD options)
{
   Handle result;
   HANDLE targetHandle;
   
   if (!DuplicateHandle(*sourceProcess
                        ,*this->handle
                        ,*destProcess
                        ,&targetHandle
                        ,access
                        ,inheritHandle
                        ,options))
      throw Win32Exception(EXCSTR(L"DuplicateHandle failed."));

   result = targetHandle;

   return result;
}

void
Handle::close
(void)
{
   if (*this->handle != NULL)
   {
      CloseHandle(*this->handle);
      this->handle = NULL_HANDLE;
   }
}
