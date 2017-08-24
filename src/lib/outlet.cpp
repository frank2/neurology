#include <neurology/outlet.hpp>

using namespace Neurology;

OutletException::OutletException
(const Outlet &outlet, const LPWSTR message)
   : outlet(outlet)
   , NeurologyException(message)
{
}

OutletException::OutletException
(OutletException &exception)
   : outlet(exception.outlet)
   , NeurologyException(exception)
{
}

NullSocketException::NullSocketException
(const Outlet &outlet)
   : OutletException(outlet, EXCSTR(L"The socket counter is null."))
{
}

DoubleAllocationException::DoubleAllocationException
(const Outlet &outlet)
   : OutletException(outlet, EXCSTR(L"The outlet was already allocated."))
{
}

ActiveOutletException::ActiveOutletException
(const Outlet &outlet, const Outlet &inactive)
   : OutletException(outlet, EXCSTR(L"A shallow Outlet is attempting to overwrite an active Outlet."))
   , inactive(inactive)
{
}

DepthException::DepthException
(const Outlet &outlet)
   : OutletException(outlet, EXCSTR(L"Non-const function called on shallow Outlet."))
{
}

Outlet::Outlet
(void)
{
   this->refCount = NULL;
   this->shallow = true;
}

Outlet::Outlet
(Outlet &outlet)
{
   *this = outlet;
}

Outlet::Outlet
(const Outlet &outlet)
{
   *this = outlet;
}

Outlet::~Outlet
(void)
{
   if (!this->isNull())
   {
      if (this->shallow)
         this->release();
      else
         this->unplug();
   }
}

void
Outlet::operator=
(Outlet &outlet)
{
   if (this->refCount != NULL)
      this->deref();
   
   this->refCount = outlet.refCount;

   if (this->refCount != NULL)
      this->ref();

   this->constOutlet = false;
}

void
Outlet::operator=
(const Outlet &outlet)
{
   if (!this->isNull() && !this->constOutlet && this->refs() > 1)
      throw SlaughteredOutletException(*this, outlet);
   else if (!outlet.isNull() && this->isNull())
      this->allocate();

   if (!outlet.isNull())
      *this->refCount = *outlet.refCount;
   else if (!this->isNull())
      *this->refCount = 0;
   
   this->constOutlet = true;
}

DWORD
Outlet::refs
(void) const
{
   if (this->refCount == NULL)
      return 0;

   return *this->refCount;
}

void
Outlet::ref
(void)
{
   this->throwIfConst();
   this->throwIfNull();
   ++*this->refCount;
}

void
Outlet::deref
(void)
{
   this->throwIfConst();
   this->throwIfNull();

   if (this->constOutlet)
      this->release();
   else
   {
      --*this->refCount;

      if (*this->refCount <= 0)
         this->release();
   }
}

bool
Outlet::isNull
(void) const
{
   return this->refCount == NULL;
}

bool
Outlet::isConst
(void) const
{
   return this->constOutlet;
}

void
Outlet::throwIfNull
(void) const
{
   if (this->isNull())
      throw NullOutletException(*this);
}

void
Outlet::throwIfConst
(void) const
{
   if (this->constOutlet)
      throw ConstMismatchException(*this);
}

void
Outlet::allocate
(void)
{
   if (this->refCount != NULL)
      throw DoubleAllocationException(*this);
   
   this->refCount = new DWORD;
   *this->refCount = 1;
}

void
Outlet::release
(void)
{
   this->throwIfNull();
   *this->refCount = 0;
   delete this->refCount;
}
