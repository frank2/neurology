#include <neurology/handle.hpp>

HandleException::HandleException
(Handle *handle, const LPWSTR message)
   : NeurologyException(message)
{
   this->handle = handle;

   if (this->handle != NULL)
      this->handle->ref();
}

HandleException::HandleException
(HandleException &exception)
   : NeurologyException(exception)
{
   if (this->handle != NULL)
      this->handle->deref();

   this->handle = exception.handle;

   if (this->handle != NULL)
      this->handle->ref();
}

HandleException::~HandleException
(void)
{
   if (this->handle != NULL)
      this->handle->deref();
