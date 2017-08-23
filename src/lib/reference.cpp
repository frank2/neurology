#include <neurology/reference.hpp>

using namespace Neurology;

ReferenceException::ReferenceException
(const Reference &reference, const LPWSTR message)
   : reference(reference)
   , NeurologyException(message)
{
}

ReferenceException::ReferenceException
(ReferenceException &exception)
   : reference(exception.reference)
   , NeurologyException(exception)
{
}

NullReferenceException::NullReferenceException
(const Reference &reference)
   : ReferenceException(reference, EXCSTR(L"The reference counter is null."))
{
}

DoubleAllocationException::DoubleAllocationException
(const Reference &reference)
   : ReferenceException(reference, EXCSTR(L"The reference was already allocated."))
{
}

Reference::Reference
(void)
{
   this->refCount = NULL;
}

Reference::Reference
(Reference &reference)
{
   if (this->refCount != NULL)
      this->deref();
   
   this->refCount = reference.refCount;

   if (this->refCount != NULL)
      this->ref();
}

Reference::~Reference
(void)
{
   this->deref();
}

DWORD
Reference::refs
(void) const
{
   if (this->refCount == NULL)
      return 0;

   return *this->refCount;
}

void
Reference::ref
(void)
{
   if (this->refCount == NULL)
      throw NullReferenceException(*this);

   ++*this->refCount;
}

void
Reference::deref
(void)
{
   if (this->refCount == NULL)
      throw NullReferenceException(*this);

   --*this->refCount;

   if (*this->refCount <= 0)
      this->release();
}

bool
Reference::isNull
(void) const
{
   return this->refCount == NULL;
}

void
Reference::allocate
(void)
{
   if (this->refCount != NULL)
      throw DoubleAllocationException(*this);
   
   this->refCount = new DWORD;
   *this->refCount = 1;
}

void
Reference::release
(void)
{
   if (this->refCount == NULL)
      throw NullReferenceException(*this);

   *this->refCount = 0;
   delete this->refCount;
}
