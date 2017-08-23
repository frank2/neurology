#include <neurology/reference.hpp>

using namespace Neurology;

ReferenceException::ReferenceException
(const LPWSTR message)
   : NeurologyException(message)
{
}

NullReferenceException::NullReferenceException
(void)
   : ReferenceException(EXCSTR(L"The reference counter is null."))
{
}

DoubleAllocationException::DoubleAllocationException
(void)
   : ReferenceException(EXCSTR(L"The reference was already allocated."))
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
(void)
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
      throw NullReferenceException();

   ++*this->refCount;
}

void
Reference::deref
(void)
{
   if (this->refCount == NULL)
      throw NullReferenceException();

   --*this->refCount;

   if (*this->refCount <= 0)
      this->release();
}

bool
Reference::isNull
(void)
{
   return this->refCount == NULL;
}

void
Reference::allocate
(void)
{
   if (this->refCount != NULL)
      throw DoubleAllocationException();
   
   this->refCount = new DWORD;
   *this->refCount = 1;
}

void
Reference::release
(void)
{
   if (this->refCount == NULL)
      throw NullReferenceException();

   *this->refCount = 0;
   delete this->refCount;
}
