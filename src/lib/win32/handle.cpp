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
(const Handle &handle)
{
   *this = handle;
}

Handle::~Handle
(void)
{
   if (*this->handle != NULL_HANDLE)
      this->close();
}

void
Handle::operator=
(const Handle &handle)
{
   this->handle = handle.handle;
}

bool
Handle::isNull
(void) const
{
   return *this->handle == NULL_HANDLE;
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
