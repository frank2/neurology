#include <neurology/allocator/allocator.hpp>

using namespace Neurology;

Allocation::Exception::Exception
(const Allocation &allocation, const LPWSTR message)
   : Neurology::Exception(message)
   , allocation(allocation)
{
}

Allocation::NoAllocatorException::NoAllocatorException
(const Allocation &allocation)
   : Allocation::Exception(allocation, EXCSTR(L"Allocation must be tied to an Allocator."))
{
}

Allocation::DoubleAllocationException::DoubleAllocationException
(const Allocation &allocation)
   : Allocation::Exception(allocation, EXCSTR(L"Allocation has already been allocated."))
{
}

Allocation::DeadAllocationException::DeadAllocationException
(const Allocation &allocation)
   : Allocation::Exception(allocation, EXCSTR(L"Allocation is currently dead."))
{
}

Allocation::ZeroSizeException::ZeroSizeException
(const Allocation &allocation)
   : Allocation::Exception(allocation, EXCSTR(L"Size of allocation cannot be 0."))
{
}

Allocation::InsufficientSizeException::InsufficientSizeException
(const Allocation &allocation, const SIZE_T size)
   : Allocation::Exception(allocation, EXCSTR(L"Size larger than allocation."))
   , size(size)
{
}

Allocation::Allocation
(Allocator *allocator, LPVOID pointer, SIZE_T size)
{
   if (allocator == NULL)
      throw NoAllocatorException(*this);

   allocator->throwIfNotPooled(pointer);

   this->allocator = allocator;
   this->pointer = pointer;
   this->size = size;
}

Allocation::Allocation
(void)
   : allocator(NULL)
   , pointer(NULL)
   , size(0)
{
}

Allocation::Allocation
(Allocator *allocator)
   : pointer(NULL)
   , size(0)
{
   if (allocator == NULL)
      throw NoAllocatorException(*this);

   this->allocator = allocator;
}

Allocation::Allocation
(Allocation &allocation)
{
   *this = allocation;
}

Allocation::Allocation
(const Allocation &allocation)
{
   *this = allocation;
}

Allocation::~Allocation
(void)
{
   if (this->allocator == NULL)
      return;

   if (this->isBound())
      this->allocator->unbind(this);
}

void
Allocation::operator=
(Allocation &allocation)
{
   this->allocator = allocation.allocator;

   if (allocation.isValid())
   {
      if (this->isBound())
         this->allocator->rebind(this, allocation.pointer);
      else
         this->allocator->bind(this, allocation.pointer);
   }

   this->pointer = allocation.pointer;
   this->size = allocation.size;
}
   
void
Allocation::operator=
(const Allocation &allocation)
{
   this->allocator = allocation.allocator;
   
   if (allocation.isValid())
   {
      LPVOID newPointer;

      newPointer = this->allocator->pool(allocation.size);

      if (this->isBound())
         this->allocator->rebind(this, newPointer);
      else
         this->allocator->bind(this, newPointer);
      
      this->pointer = newPointer;
      this->size = allocation.size;
      
      CopyMemory(this->pointer, allocation.pointer, allocation.size);
   }
   else
   {
      this->pointer = allocation.pointer;
      this->size = allocation.size;
   }
}

LPVOID
Allocation::operator*
(void)
{
   return this->address();
}

const LPVOID
Allocation::operator*
(void) const
{
   return this->address();
}

bool
Allocation::isBound
(void) const
{
   return this->allocator != NULL && this->allocator->isBound(*this);
}

bool
Allocation::isValid
(void) const
{
   return this->allocator != NULL && this->pointer != NULL && this->size != 0 && this->allocator->isPooled(this->pointer);
}

bool
Allocation::inRange
(LPVOID address) const
{
   return this->pointer != NULL && this->size != 0 && address >= this->start() && address <= this->end();
}

bool
Allocation::inRange
(LPVOID address, SIZE_T size)
{
   return this->inRange(address) && this->inRange(static_cast<LPBYTE>(address)+size);
}

LPVOID
Allocation::address
(void)
{
   return this->pointer;
}

const LPVOID
Allocation::address
(void) const
{
   return const_cast<LPVOID>(this->pointer);
}

LPVOID
Allocation::address
(SIZE_T offset)
{
   this->throwIfInvalid();

   if (size > this->size)
      throw OffsetOutOfRangeException(*this, offset, 0);
   
   return static_cast<LPBYTE>(this->pointer)+offset;
}

const LPVOID
Allocation::address
(SIZE_T offset) const
{
   this->throwIfInvalid();

   if (size > this->size)
      throw OffsetOutOfRangeException(*this, offset, 0);
   
   return const_cast<LPBYTE>(this->pointer)+offset;
}

LPVOID
Allocation::start
(void)
{
   return this->address();
}

const LPVOID
Allocation::start
(void) const
{
   return this->address();
}

LPVOID
Allocation::end
(void)
{
   return this->address(this->size);
}

const LPVOID
Allocation::end
(void) const
{
   return this->address(this->size);
}

SIZE_T
Allocation::getSize
(void) const
{
   return this->size;
}

void
Allocation::allocate
(SIZE_T size)
{
   this->throwIfNoAllocator();
   
   if (this->pointer != NULL && this->allocator->isPooled(this->pointer))
      throw DoubleAllocationException(*this);

   this->pointer = this->allocator->pool(size);
   this->size = size;
   this->allocator->bind(this, this->pointer);
}

void
Allocation::reallocate
(SIZE_T size)
{
   if (!this->isValid())
      return this->allocate(size);

   /* repool should automatically rebind this object */
   this->pointer = this->allocator->repool(this->pointer, size);
   this->size = size;
}

void
Allocation::deallocate
(void)
{
   this->throwIfInvalid();

   if (this->isBound())
      this->allocator->unbind(this);
}

Data
Allocation::read
(void) const
{
   return this->read(this->size);
}

Data
Allocation::read
(SIZE_T size) const
{
   return this->read(0, size);
}

Data
Allocation::read
(SIZE_T offset, SIZE_T size) const
{
   try
   {
      return this->read(static_cast<LPBYTE>(this->pointer)+offset, size);
   }
   except (AddressOutOfRangeException &exception)
   {
      throw OffsetOutOfRangeException(*this, offset, size);
   }
}

Data
Allocation::read
(LPVOID address, SIZE_T size) const
{
   this->throwIfInvalid();
   
   if (!this->inRange(address, size))
      throw AddressOutOfRangeException(*this, address, size);

   __try
   {
      return Data(address, static_cast<LPBYTE>(address)+size);
   }
   __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION)
   {
      throw BadPointerException(*this, address, size);
   }
}

void
Allocation::write
(const Data data)
{
   this->write(0, data);
}

void
Allocation::write
(SIZE_T offset, const Data data)
{
   this->write(this->address(offset), data.data(), data.size());
}

void
Allocation::write
(LPVOID pointer, SIZE_T size)
{
   this->write(0, pointer, size);
}

void
Allocation::write
(SIZE_T offset, LPVOID pointer, SIZE_T size)
{
   try
   {
      this->write(this-address(offset), pointer, size);
   }
   catch (AddressOutOfRangeException &exception)
   {
      throw OffsetOutOfRangeException(*this, offset, size);
   }
}

void
Allocation::write
(LPVOID address, LPVOID pointer, SIZE_T size)
{
   this->throwIfInvalid();

   if (!this->inRange(address, size))
      throw AddressOutOfRangeException(*this, address, size);

   __try
   {
      CopyMemory(address, pointer, size);
   }
   __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION)
   {
      throw BadPointerException(*this, address, size);
   }
}
