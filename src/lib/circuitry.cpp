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
   this->solder();
   this->sockets = 0;
   this->shallow = false;
}

Outlet::Outlet
(Outlet &outlet)
{
   this->solder();
   *this = outlet;
}

Outlet::Outlet
(const Outlet &outlet)
{
   this->solder();
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
   if (this->isActive())
      this->unplug();

   if (this->circuitCount() != outlet.circuitCount())
      throw CircuitMappingException(*this, outlet);

   this->route(outlet);

   if (outlet.sockets != 
      this->plug();

   this->shallow = false;
}

void
Outlet::operator=
(const Outlet &outlet)
{
   if (this->isActive()
   if (!this->isNull() && !this->constOutlet && this->plugs() > 1)
      throw SlaughteredOutletException(*this, outlet);
   else if (!outlet.isNull() && this->isNull())
      this->allocate();

   if (!outlet.isNull())
      *this->plugCount = *outlet.plugCount;
   else if (!this->isNull())
      *this->plugCount = 0;
   
   this->constOutlet = true;
}

DWORD
Outlet::plugs
(void) const
{
   if (this->plugCount == NULL)
      return 0;

   return *this->plugCount;
}

void
Outlet::plug
(void)
{
   this->throwIfConst();
   this->throwIfNull();
   ++*this->plugCount;
}

void
Outlet::unplug
(void)
{
   this->throwIfConst();
   this->throwIfNull();

   if (this->constOutlet)
      this->release();
   else
   {
      --*this->plugCount;

      if (*this->plugCount <= 0)
         this->release();
   }
}

bool
Outlet::isNull
(void) const
{
   return this->plugCount == NULL;
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
   if (this->plugCount != NULL)
      throw DoubleAllocationException(*this);
   
   this->plugCount = new DWORD;
   *this->plugCount = 1;
}

void
Outlet::release
(void)
{
   this->throwIfNull();
   *this->plugCount = 0;
   delete this->plugCount;
}
