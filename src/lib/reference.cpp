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

SlaughteredReferenceException::SlaughteredReferenceException
(const Reference &reference, const Reference &suspect)
   : ReferenceException(reference, EXCSTR(L"A const Reference assignment nearly killed a well-used reference."))
   , suspect(suspect)
{
}

ConstMismatchException::ConstMismatchException
(const Reference &reference)
   : ReferenceException(reference, EXCSTR(L"Non-const function called on non-const reference that inherited a const reference."))
{
}

Reference::Reference
(void)
{
   this->refCount = NULL;
   this->constReference = false;
}

Reference::Reference
(Reference &reference)
{
   *this = reference;
}

Reference::Reference
(const Reference &reference)
{
   *this = reference;
}

Reference::~Reference
(void)
{
   if (!this->isNull())
   {
      if (this->constReference)
         this->release();
      else
         this->deref();
   }
}

void
Reference::operator=
(Reference &reference)
{
   if (this->refCount != NULL)
      this->deref();
   
   this->refCount = reference.refCount;

   if (this->refCount != NULL)
      this->ref();

   this->constReference = false;
}

void
Reference::operator=
(const Reference &reference)
{
   if (!this->isNull() && !this->constReference && this->refs() > 1)
      throw SlaughteredReferenceException(*this, reference);
   else if (!reference.isNull() && this->isNull())
      this->allocate();

   if (!reference.isNull())
      *this->refCount = *reference.refCount;
   else if (!this->isNull())
      *this->refCount = 0;
   
   this->constReference = true;
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
   this->throwIfConst();
   this->throwIfNull();
   ++*this->refCount;
}

void
Reference::deref
(void)
{
   this->throwIfConst();
   this->throwIfNull();

   if (this->constReference)
      this->release();
   else
   {
      --*this->refCount;

      if (*this->refCount <= 0)
         this->release();
   }
}

bool
Reference::isNull
(void) const
{
   return this->refCount == NULL;
}

bool
Reference::isConst
(void)
{
   return this->constReference;
}

void
Reference::throwIfNull
(void) const
{
   if (this->isNull())
      throw NullReferenceException(*this);
}

void
Reference::throwIfConst
(void) const
{
   if (this->constReference)
      throw ConstMismatchException(*this);
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
   this->throwIfNull();
   *this->refCount = 0;
   delete this->refCount;
}
