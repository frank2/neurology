#include <neurology/handle.hpp>

HandleException::HandleException
(Handle &handle, const LPWSTR message)
   : NeurologyException(message)
{
   this->handle = handle;
}

NullHandleException::NullHandleException
(Handle &handle)
   : HandleException(handle, EXCSTR(L"Handle is null."))
{
}

HandleTypeMismatchException::HandleTypeMismatchException
(Handle &handle, Handle &otherHandle)
   : HandleException(handle)
{
   this->otherHandle = otherHandle;
}

Handle::Reference::Reference
(void)
   : Neurology::Reference()
{
   this->handleRef = NULL;
}

Handle::Reference::Reference
(HANDLE handle)
   : Neurology::Reference()
{
   this->allocate();
   *this->handleRef = handle;
}

void
Handle::Reference::operator=
(Handle::Reference &reference)
   : Neurology::Reference::operator=(reference)
{
   this->handleRef = reference.handleRef;
}

void
Handle::Reference::operator=
(HANDLE handle)
{
   this->setHandle(handle);
}

bool
Handle::Reference::isNull
(void) const
{
   return Neurology::Reference::isNull() || this->handleRef == NULL;
}

bool
Handle::Reference::hasNullHandle
(void) const
{
   this->throwIfNull();
   return *this->handleRef == NULL;
}

HANDLE
Handle::Reference::handle
(void) const
{
   this->throwIfNull();
   return *this->handleRef;
}

void
Handle::Reference::setHandle
(HANDLE handle)
{
   this->throwIfNull();
   *this->handleRef = handle;
}

void
Handle::Reference::close
(void)
{
   this->throwIfNull();
   
   if (*this->handleRef == NULL)
      return;
   
   if (!CloseHandle(*this->handleRef))
      throw Win32Exception(EXCSTR(L"CloseHandle() failed on supplied handle"));
   
   *this->handleRef = NULL;
}

void
Handle::Reference::allocate
(void)
{
   Neurology::Reference::allocate();

   if (this->handleRef != NULL)
      throw DoubleAllocationException(*this);

   this->handleRef = new HANDLE;
   *this->handleRef = NULL;
}

void
Handle::Reference::release
(void)
{
   Neurology::Reference::release();

   if (*this->handleRef != NULL)
      this->close();

   delete this->handleRef;
}

Handle::Handle
(void)
{
   this->objectType = SE_UNKNOWN_OBJECT_TYPE;
}

Handle::Handle
(HANDLE handle)
{
   this->reference = Handle::Reference(handle);
   this->objectType = SE_UNKNOWN_OBJECT_TYPE;
}

Handle::Handle
(HANDLE handle, SE_OBJECT_TYPE objectType)
{
   this->reference = Handle::Reference(handle);
   this->objectType = objectType;
}

Handle::Handle
(Handle &handle)
{
   *this = handle;
}

Handle::~Handle
(void)
{
   if (!this->reference.isNull())
      this->reference.deref();
}

void
Handle::operator=
(Handle &handle)
{
   this->reference = handle.reference;
   this->objectType = handle.objectType;
}

void
Handle::operator=
(HANDLE handle)
{
   this->setHandle(handle);
   this->objectType = SE_UNKNOWN_OBJECT_TYPE;
}

void
Handle::ref
(void)
{
   this->reference.throwIfNull();
   this->reference.ref();
}

void
Handle::deref
(void)
{
   this->reference.throwIfNull();
   this->reference.ref();
}

DWORD
Handle::refs
(void) const
{
   return this->reference.refs();
}

bool
Handle::isNull
(void) const
{
   return this->reference.isNull() || this->reference.hasNullHandle();
}

bool
Handle::hasNullHandle
(void) const
{
   this->reference.throwIfNull();
   return this->reference.hasNullHandle();
}

void
Handle::close
(void)
{
   this->reference.close();
}

HANDLE
Handle::handle
(void) const
{
   return this->reference.handle();
}

void
Handle::setHandle
(HANDLE handle)
{
   if (this->reference.isNull())
      this->reference = Handle::Reference(handle);
   else
      this->reference.setHandle(handle);

   this->objectType = SE_UNKNOWN_OBJECT_TYPE;
}

void
Handle::setHandle
(HANDLE handle, SE_OBJECT_TYPE objectType)
{
   this->setHandle(handle);
   this->objectType = objectType;
}

SE_OBJECT_TYPE
Handle::type
(void) const
{
   return this->objectType;
}

void
Handle::setType
(SE_OBJECT_TYPE type)
{
   this->objectType = type;
}
