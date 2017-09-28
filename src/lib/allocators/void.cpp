#include <neurology/allocators/void.hpp>

using namespace Neurology;

Allocator::Exception::Exception
(Allocator &allocator, const LPWSTR message)
   : Neurology::Exception(message)
   , allocator(allocator)
{
}

Allocator::ZeroSizeException::ZeroSizeException
(Allocator &allocator)
   : Allocator::Exception(allocator, EXCSTR(L"Cannot allocate or reallocate to a zero-sized object."))
{
}

Allocator::PoolAllocationException::PoolAllocationException
(Allocator &allocator)
   : Allocator::Exception(allocator, EXCSTR(L"Pooling of new address failed."))
{
}

Allocator::UnpooledAddressException::UnpooledAddressException
(Allocator &allocator, const Address &address)
   : Allocator::Exception(allocator, EXCSTR(L"The supplied address is not pooled by this allocator."))
   , address(address)
{
}

Allocator::BindingException::BindingException
(Allocator &allocator, Allocation &allocation, const LPWSTR message)
   : Allocator::Exception(allocator, message)
   , allocation(allocation)
{
}

Allocator::BoundAllocationException::BoundAllocationException
(Allocator &allocator, Allocation &allocation)
   : BindingException(allocator, allocation, EXCSTR(L"The allocation is already bound."))
{
}

Allocator::UnboundAllocationException::UnboundAllocationException
(Allocator &allocator, Allocation &allocation)
   : BindingException(allocator, allocation, EXCSTR(L"The allocation is not bound to the allocator."))
{
}

Allocator::UnmanagedAllocationException::UnmanagedAllocationException
(Allocator &allocator, Allocation &allocation)
   : Allocator::Exception(allocator, EXCSTR(L"The provided allocation is not being managed by the allocator."))
   , allocation(allocation)
{
}

Allocator::InsufficientSizeException::InsufficientSizeException
(Allocator &allocator, const SIZE_T size)
   : Allocator::Exception(allocator, EXCSTR(L"Supplied size not large enough for supplied type."))
   , size(size)
{
}

Allocator::VoidAllocatorException::VoidAllocatorException
(Allocator &allocator)
   : Allocator::Exception(allocator, EXCSTR(L"Allocator is void."))
{
}

Allocator::UnallocatedAddressException::UnallocatedAddressException
(Allocator &allocator, Address &address)
   : Allocator::Exception(allocator, EXCSTR(L"The provided address is not in range of any allocation in the allocator."))
   , address(address)
{
}

Allocator::SplitsExceededException::SplitsExceededException
(Allocator &allocator, Address &address, SIZE_T size)
   : Allocator::Exception(allocator, EXCSTR(L"Hit split terminator while data still present."))
   , address(address)
   , size(size)
{
}

Allocator::OrphanAllocationException::OrphanAllocationException
(Allocator &allocator, Allocation &allocation)
   : Allocator::Exception(allocator, EXCSTR(L"Allocation has no parent."))
   , allocation(allocation)
{
}

Allocator::PoolCollisionException::PoolCollisionException
(Allocator &allocator, Address &address)
   : Allocator::Exception(allocator, EXCSTR(L"A memory address was already pooled with the given address."))
   , address(address)
{
}

Allocator::AddressNotFoundException::AddressNotFoundException
(Allocator &allocator, Address &address)
   : Allocator::Exception(allocator, EXCSTR(L"The allocator has not allocated any such address."))
   , address(address)
{
}

Allocator::Allocator
(void)
   : local(false)
{
}

Allocator::~Allocator
(void)
{
   while (this->bindings.size() > 0)
   {
      /* unbind all allocation bindings */
      Allocator::BindingMap::iterator iter = this->bindings.begin();
      
      while (iter->second.size() > 0)
         this->unbind(*iter->second.begin());

      /* the binding set is now empty, check if it's still pooled.
         if so, erase it. */
      if (this->isPooled(iter->first))
         /* ha ha fuck you Stoustrup here's a hack */
         this->unpool(Address(iter->first.label()));
         
      this->bindings.erase(iter->first);
   }
   
   /* if there are any entries left in the memory pool, delete them */
   for (MemoryPool::iterator iter=this->pooledMemory.begin();
        iter != this->pooledMemory.end();
        ++iter)
   {
      this->unpool(Address(iter->first.label()));
   }
}

bool
Allocator::isLocal
(void) const noexcept
{
   return this->local;
}

bool
Allocator::isPooled
(const Address &address) const noexcept
{
   return this->pooledMemory.count(address) > 0;
}

bool
Allocator::isBound
(const Allocation &allocation) const noexcept
{
   Address address;
   Allocator::BindingMap::const_iterator bindIter;
   Allocator::AllocationSet::const_iterator allocIter;

   if (allocation.allocator != this)
      return false;

   if (this->associations.count(&allocation) == 0)
      return false;

   address = this->associations.at(&allocation);

   if (this->bindings.count(address) == 0)
      return false;

   allocIter = this->bindings.at(address).find(const_cast<Allocation *>(&allocation));

   return allocIter != this->bindings.at(address).end();
}

bool
Allocator::hasAddress
(const Address &address) const noexcept
{
   if (this->pooledAddresses.hasLabel(address.label()))
      return true;

   if (this->bindings.count(address) > 0)
      return true;

   /* check the pool of every allocation we have */
   for (Allocator::AllocationSet::const_iterator iter=this->allocations.begin();
        iter!=this->allocations.end();
        ++iter)
   {
      if (!this->isPooled((**iter).address()) || (**iter).size() != this->pooledMemory.at((**iter).address()))
         continue;
            
      if ((**iter).pool.inRange(address.label()))
         return true;
   }

   return false;
}

bool
Allocator::sharesPool
(const Allocation &left, const Allocation &right) const noexcept
{
   if (!this->isBound(left) || !this->isBound(right))
      return false;

   const Allocation &leftRoot = this->root(left);
   const Allocation &rightRoot = this->root(right);
   Address leftAddress = this->associations.at(&leftRoot);
   Address rightAddress = this->associations.at(&rightRoot);
   
   if (!this->isPooled(leftAddress) || !this->isPooled(rightAddress))
      return false;

   return leftAddress == rightAddress;
}

bool
Allocator::hasParent
(const Allocation &allocation) const noexcept
{
   return allocation.hasParent();
}

bool
Allocator::hasChildren
(const Allocation &allocation) const noexcept
{
   return allocation.hasChildren();
}

bool
Allocator::isChild
(const Allocation &parent, const Allocation &child) const noexcept
{
   if (!this->hasParent(child))
      return false;

   return child.isChild(parent);
}

void
Allocator::throwIfNotPooled
(const Address &address) const
{
   if (!this->isPooled(address))
      throw UnpooledAddressException(const_cast<Allocator &>(*this), address);
}

void
Allocator::throwIfBound
(const Allocation &allocation) const
{
   if (this->isBound(allocation))
      throw BoundAllocationException(const_cast<Allocator &>(*this), const_cast<Allocation &>(allocation));
}

void
Allocator::throwIfNoAddress
(const Address &address) const
{
   if (!this->hasAddress(address))
      throw AddressNotFoundException(const_cast<Allocator &>(*this), const_cast<Address &>(address));
}

void
Allocator::throwIfNotBound
(const Allocation &allocation) const
{
   if (!this->isBound(allocation))
      throw UnboundAllocationException(const_cast<Allocator &>(*this), const_cast<Allocation &>(allocation));
}

void
Allocator::throwIfNoAllocation
(const Address &address) const
{
   if (!this->hasAddress(address))
      throw UnallocatedAddressException(const_cast<Allocator &>(*this), const_cast<Address &>(address));
}

void
Allocator::throwIfNoParent
(const Allocation &allocation) const
{
   if (!this->hasParent(allocation))
      throw OrphanAllocationException(const_cast<Allocator &>(*this), const_cast<Allocation &>(allocation));
}

const Address
Allocator::addressOf
(const Allocation &allocation) const noexcept
{
   if (!this->isBound(allocation))
      return Address(static_cast<Label>(0));

   return this->associations.at(&allocation);
}

Address
Allocator::address
(Allocation &allocation)
{
   return this->address(allocation, 0);
}

Address
Allocator::address
(Allocation &allocation, SIZE_T offset)
{
   this->throwIfNotBound(allocation);
   return allocation.address(offset);
}

Address
Allocator::newAddress
(Allocation &allocation)
{
   return this->newAddress(allocation, 0);
}

Address
Allocator::newAddress
(Allocation &allocation, SIZE_T offset)
{
   this->throwIfNotBound(allocation);
   return allocation.address(offset);
}

SIZE_T
Allocator::bindCount
(const Address &address) const
{
   if (this->bindings.count(address) > 0)
      return this->bindings.at(address).size();

   return 0;
}

SIZE_T
Allocator::querySize
(const Allocation &allocation) const
{
   /* if we're not even bound, you know for a fact that's a 0 */
   if (!this->isBound(allocation))
      return 0;

   return allocation.size();
}

Address
Allocator::pool
(SIZE_T size)
{
   Address address;

   if (size == 0)
      throw ZeroSizeException(*this);
   
   address = this->poolAddress(size);

   if (!address.usesPool(&this->pooledAddresses))
      address = this->pooledAddresses.address(address.label());
   
   this->pooledMemory[address] = size;

   return address;
}

Address
Allocator::repool
(Address &address, SIZE_T newSize)
{
   Address newAddress, baseAddress;

   this->throwIfNotPooled(address);

   if (newSize == 0)
      throw ZeroSizeException(*this);

   if (address.usesPool(&this->pooledAddresses))
      baseAddress = address;
   else
      baseAddress = this->pooledAddresses.address(address.label());

   newAddress = this->repoolAddress(address, newSize);

   if (!newAddress.usesPool(&this->pooledAddresses))
      newAddress = this->pooledAddresses.address(newAddress.label());
   
   this->pooledMemory[newAddress] = newSize;

   /* if the address is the same, nothing needs to be done. */
   if (baseAddress == newAddress)
      return baseAddress;

   /* why did you give us a pool address that's already allocated but wasn't our
      original address? that's weird. */
   if (this->bindings.count(newAddress) > 0)
      throw PoolCollisionException(*this, newAddress);
   
   /* there are bindings to fix */
   if (this->bindings.count(baseAddress) > 0)
   {
      AllocationSet allocations(this->bindings[baseAddress]);

      for (AllocationSet::iterator allocIter=this->allocations.begin();
           allocIter!=this->allocations.end();
           ++allocIter)
      {
         this->rebind(*allocIter, newAddress);
         (**allocIter).pool.setMax((newAddress+newSize).label());
      }
   }

   this->pooledMemory.erase(baseAddress);

   return newAddress;
}

void
Allocator::unpool
(Address &address)
{
   Address localAddress = Address(address.label());
   
   this->throwIfNotPooled(address);

   while (this->bindings.count(localAddress) > 0)
      this->unbind(*this->bindings[localAddress].begin());

   /* unbind killed us, bail. */
   if (this->pooledMemory.count(localAddress) == 0)
      return;
   
   this->unpoolAddress(localAddress);
}

Allocation &
Allocator::find
(const Address &address) const
{
   return this->find(address, 0);
}

Allocation &
Allocator::find
(const Address &address, SIZE_T size) const
{
   Allocator::BindingMap::const_iterator bindIter, allocIter;
   std::list<const Allocation *> searchQueue;
   std::set<const Allocation *> queueVisited;
   const Allocation *possibleResult = NULL;

   bindIter = this->bindings.upper_bound(address);

   /* if the root allocations don't have this address, this address can't possibly exist */
   if (this->bindings.size() == 0 || bindIter == this->bindings.begin())
      throw AddressNotFoundException(const_cast<Allocator &>(*this), const_cast<Address &>(address));

   --bindIter;

   allocIter = bindIter;

   for (AllocationSet::iterator iter=allocIter->second.begin();
        iter!=allocIter->second.end();
        ++iter)
      searchQueue.push_back(*iter);

   while (searchQueue.size() > 0)
   {
      const Allocation *popped = searchQueue.front();
      
      searchQueue.pop_front();
      queueVisited.insert(popped);

      if (size == 0 && popped->inRange(address))
         return const_cast<Allocation &>(*popped);
      else if (popped->inRange(address, size))
      {
         if (size == popped->size())
            return const_cast<Allocation &>(*popped);
         else if (size < popped->size() && possibleResult == NULL)
            possibleResult = popped;
         else if (possibleResult != NULL && size < popped->size() && popped->size() < possibleResult->size())
            possibleResult = popped;
      }

      if (popped->hasParent())
      {
         const Allocation &parent = popped->getParent();

         if (queueVisited.find(&parent) != queueVisited.end())
            searchQueue.push_back(&popped->getParent());
      }
   }

   if (possibleResult != NULL)
      return const_cast<Allocation &>(*possibleResult);

   throw AddressNotFoundException(const_cast<Allocator &>(*this), const_cast<Address &>(address));
}

Allocation
Allocator::null
(void) 
{
   return Allocation(this);
}

Allocation
Allocator::null
(void) const
{
   return Allocation();
}

Allocation
Allocator::allocate
(SIZE_T size)
{
   Allocation newAllocation = this->null();
   this->allocate(&newAllocation, size);
   return newAllocation;
}

void
Allocator::reallocate
(Allocation &allocation, SIZE_T size)
{
   /* just call the protected reallocation function */
   this->reallocate(&allocation, size);
}

void
Allocator::deallocate
(Allocation &allocation)
{
   this->deallocate(&allocation);
}

Data
Allocator::read
(const Address &address, SIZE_T size) const
{
   this->throwIfNoAddress(address);
   return this->read(&this->find(address, size), address, size);
}

void
Allocator::write
(const Address &address, const Data data)
{
   this->throwIfNoAddress(address);
   return this->write(&this->find(address, data.size()), address, data);
}

Allocation &
Allocator::root
(Allocation &allocation) const
{
   this->throwIfNotBound(allocation);
   return allocation.root();
}

const Allocation &
Allocator::root
(const Allocation &allocation) const
{
   this->throwIfNotBound(allocation);
   return allocation.root();
}

Allocation &
Allocator::parent
(Allocation &allocation)
{
   this->throwIfNotBound(allocation);
   return allocation.getParent();
}

const Allocation &
Allocator::parent
(const Allocation &allocation) const
{
   this->throwIfNotBound(allocation);
   return allocation.getParent();
}

Allocator::AllocationSet
Allocator::children
(const Allocation &allocation) const
{
   this->throwIfNotBound(allocation);
   return allocation.getChildren();
}

void
Allocator::zeroAddress
(const Address &address, SIZE_T size)
{
   Data data(size);

   std::fill(data.begin(), data.end(), 0);
   this->writeAddress(address, data);
}

Address
Allocator::poolAddress
(SIZE_T size)
{
   throw VoidAllocatorException(*this);
}

Address
Allocator::repoolAddress
(Address &address, SIZE_T size)
{
   throw VoidAllocatorException(*this);
}

void
Allocator::unpoolAddress
(Address &address)
{
   throw VoidAllocatorException(*this);
}

void
Allocator::allocate
(Allocation *allocation, SIZE_T size)
{
   Address pooledAddress = this->pool(size);
   this->bind(allocation, pooledAddress);
}

void
Allocator::reallocate
(Allocation *allocation, SIZE_T size)
{
   std::intptr_t paternalDelta;
   Label paternalEnd;
   
   this->throwIfNotBound(*allocation);
   
   /* repooling should automatically rebind and resize the underlying allocations,
      but is only applicable to orphan allocations */
   if (!allocation->hasParent())
   {
      this->repool(this->associations.at(allocation), size);
      return;
   }

   if (size == 0)
      throw ZeroSizeException(*this);

   paternalDelta = size - allocation->size();
   paternalEnd = allocation->end().label() + paternalDelta;

   /* since we won't be doing any technical allocation now, we need to figure out
      which children are still alive and which children are not.

      * if old < new, children are fine and nothing needs to happen
      * if new < old, search children
      * calculate child->end().label() + delta
      * if it's less than child->start(), it has been deleted
      * if it's greater than child->start(), it has been resized */
   if (allocation->hasChildren() && allocation->end() > paternalEnd)
   {
      Allocator::AllocationSet::iterator childIter;
      std::vector<Allocation *> deadChildren;
      std::vector<Allocation *>::iterator deadIter;

      for (childIter=allocation->children.begin();
           childIter!=allocation->children.end();
           ++childIter)
      {
         if ((**childIter).start() >= paternalEnd)
            deadChildren.push_back(*childIter);
         else if ((**childIter).end() > paternalEnd)
            (**childIter).pool.setMax(paternalEnd);
      }

      for (deadIter=deadChildren.begin(); deadIter!=deadChildren.end(); ++deadIter)
      {
         /* try hosting the address in the parent's parent */
         if (allocation->hasParent() && allocation->getParent().hasParent())
            (**deadIter).setParent(allocation->getParent().getParent());
         else
            this->unbind(*deadIter);
      }
   }
}

void
Allocator::deallocate
(Allocation *allocation)
{
   this->unbind(allocation);
}

void
Allocator::bind
(Allocation *allocation, const Address &address)
{
   Address localAddress;
   bool rootAlloc;

   this->throwIfBound(*allocation);
   
   rootAlloc = !this->hasParent(*allocation);

   if (rootAlloc)
   {
      this->throwIfNotPooled(address);
      allocation->pool.setRange(address.label()
                                ,address.label() + this->pooledMemory.at(address));
   }
   else
   {
      Allocation &parent = allocation->getParent();
      parent.throwIfNotInRange(address);
   }

   if (!rootAlloc)
      localAddress = allocation->getParent().address(allocation->getParent().offset(address));
   else if (address.usesPool(&this->pooledAddresses))
      localAddress = address;
   else
      localAddress = this->pooledAddresses.address(address.label());

   this->bindings[localAddress].insert(allocation);
   this->associations[allocation] = localAddress;
   this->allocations.insert(allocation);
   
   allocation->allocator = this;
}

void
Allocator::rebind
(Allocation *allocation, const Address &newAddress)
{
   Address oldAddress, localNewAddress;
   SIZE_T bindCount;
   std::intptr_t delta;
   bool rootAlloc = !allocation->hasParent();
   
   if (!this->isBound(*allocation))
      return this->bind(allocation, newAddress);

   if (rootAlloc)
      this->throwIfNotPooled(newAddress);
   else
   {
      Allocation &parent = allocation->getParent();
      parent.throwIfNotInRange(newAddress);
   }

   if (!rootAlloc)
   {
      if (newAddress.usesPool(&allocation->getParent().pool))
         localNewAddress = newAddress;
      else
         localNewAddress = allocation->getParent().address(allocation->getParent().offset(newAddress));
   }
   else if (newAddress.usesPool(&this->pooledAddresses))
      localNewAddress = newAddress;
   else
      localNewAddress = this->pooledAddresses.address(newAddress.label());

   oldAddress = this->associations[allocation];
   delta = localNewAddress - oldAddress;

   /* if the addresses are the same, that's not an error, but there's no point in rebinding */
   if (delta == 0)
      return;
   
   this->bindings[localNewAddress].insert(allocation);
   this->bindings[oldAddress].erase(allocation);
   this->associations[allocation] = localNewAddress;

   allocation->pool.rebase(localNewAddress.label());
   bindCount = this->bindings[oldAddress].size();
   
   if (this->hasChildren(*allocation))
   {
      AllocationSet::iterator childIter;

      for (childIter = allocation->children.begin();
           childIter != allocation->children.end();
           ++childIter)
      {
         Address newAddress = Address(this->associations.at(*childIter).label() + delta);
         this->rebind(*childIter, newAddress);
      }
   }
   
   /* originally this moved the identifier of the old address... don't do that. it causes problems. */
   if (bindCount == 0)
   {
      this->bindings.erase(oldAddress);

      /* rebind may have been called by repool, which may have already unpooled the prior address.
         that or this might be a suballocation. */
      if (this->isPooled(oldAddress))
         this->unpool(oldAddress);
   }
}

void
Allocator::unbind
(Allocation *allocation)
{
   Address boundAddress;
   
   this->throwIfNotBound(*allocation);

   if (!allocation->hasParent())
      this->throwIfNotPooled(this->associations.at(allocation));
   else
      allocation->leaveParent();
   
   if (allocation->hasChildren())
   {
      Allocator::AllocationSet::iterator childIter;

      while (allocation->children.size() > 0)
         this->unbind(*allocation->children.begin());
   }

   boundAddress = this->associations[allocation];

   this->bindings[boundAddress].erase(allocation);
   this->associations.erase(allocation);
   this->allocations.erase(allocation);
   allocation->pool.setRange(0,0);
   
   if (this->bindings.count(boundAddress) > 0 && this->bindings[boundAddress].size() != 0)
      return;
   
   this->bindings.erase(boundAddress);
      
   if (this->isPooled(boundAddress))
      this->unpool(boundAddress);
}

Data
Allocator::read
(const Allocation *allocation, const Address &address, SIZE_T size) const
{
   allocation->throwIfNotInRange(address, size);
   return this->readAddress(address, size);
}

void
Allocator::write
(const Allocation *allocation, const Address &destination, const Data data)
{
   allocation->throwIfNotInRange(destination, data.size());
   return this->writeAddress(destination, data);
}

Data
Allocator::readAddress
(const Address &address, SIZE_T size) const
{
   throw VoidAllocatorException(const_cast<Allocator &>(*this));
}

void
Allocator::writeAddress
(const Address &destination, const Data data)
{
   throw VoidAllocatorException(*this);
}

Allocation
Allocator::spawn
(Allocation *allocation, const Address &address, SIZE_T size)
{
   Allocation newAllocation;
   Address baseAddress;
   
   allocation->throwIfNotInRange(address, size);

   newAllocation = this->null();
   baseAddress = allocation->address(allocation->offset(address));

   newAllocation.setParent(*allocation);
   newAllocation.pool.setRange(address.label(), address.label()+size);
   this->bind(&newAllocation, baseAddress);

   return newAllocation;
}

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

Allocation::AddressOutOfRangeException::AddressOutOfRangeException
(const Allocation &allocation, const Address address, const SIZE_T size)
   : Allocation::Exception(allocation, EXCSTR(L"Provided address and size are out of range of the allocation."))
   , address(address)
   , size(size)
{
}

Allocation::OffsetOutOfRangeException::OffsetOutOfRangeException
(const Allocation &allocation, const SIZE_T offset, const SIZE_T size)
   : Allocation::Exception(allocation, EXCSTR(L"Provided address and size are out of range of the allocation."))
   , offset(offset)
   , size(size)
{
}

Allocation::OrphanAllocationException::OrphanAllocationException
(const Allocation &allocation)
   : Allocation::Exception(allocation, EXCSTR(L"Allocation has no parent."))
{
}

Allocation::Allocation
(void)
   : allocator(NULL)
   , parent(NULL)
{
   this->pool.setRange(0,0);
}

Allocation::Allocation
(Allocator *allocator)
   : allocator(allocator)
   , parent(NULL)
{
   if (this->allocator == NULL)
      throw NoAllocatorException(*this);

   this->pool.setRange(0,0);
}

Allocation::Allocation
(Allocation &allocation)
   : allocator(allocation.allocator)
   , parent(NULL)
{
   if (allocation.isBound())
      this->copy(allocation);
}

Allocation::~Allocation
(void)
{
   if (this->allocator == NULL)
      return;

   if (this->isBound())
      this->allocator->unbind(this);

   this->allocator = NULL;
}

void
Allocation::operator=
(Allocation &allocation)
{
   this->allocator = allocation.allocator;

   if (allocation.isBound())
      this->copy(allocation);
   else if (!this->isBound() && allocation.allocator != NULL)
      this->allocator = allocation.allocator;
   else
      this->throwIfNotBound();
}

void
Allocation::operator=
(const Allocation *allocation)
{
   this->allocator = allocation->allocator;

   if (allocation->isBound())
      this->clone(*allocation);
   else if (!this->isBound() && allocation->allocator != NULL)
      this->allocator = allocation->allocator;
   else
      this->throwIfNotBound();
}

bool
Allocation::isNull
(void) const noexcept
{
   return this->allocator == NULL || this->size() == 0;
}

bool
Allocation::isBound
(void) const noexcept
{
   return !this->isNull() && this->allocator->isBound(*this);
}

bool
Allocation::isLocal
(void) const noexcept
{
   return this->allocator != NULL && this->allocator->isLocal();
}

bool
Allocation::allocatedFrom
(const Allocator *allocator) const noexcept
{
   return this->allocator == allocator;
}

bool
Allocation::inRange
(SIZE_T offset) const noexcept
{
   Address check;
   
   if (this->isNull() || this->size() == 0)
      return false;

   return this->inRange(this->start() + offset);
}

bool
Allocation::inRange
(SIZE_T offset, SIZE_T size) const noexcept
{
   /* we subtract 1 because we want to check whether or not all the content within
      this allocation is readable/writable, which means the inRange check will fail
      on the end of the range provided. */
      
   return size != 0 && this->inRange(offset) && this->inRange(offset+size-1);
}

bool
Allocation::inRange
(const Address &address) const noexcept
{
   return address >= this->start() && address < this->end();
}

bool
Allocation::inRange
(const Address &address, SIZE_T size) const noexcept
{
   /* create a new address object to check the range, because adding the address object might throw
      an exception */
   return size != 0 && this->inRange(address) && this->inRange(Address(address.label()+size-1));
}

bool
Allocation::hasParent
(void) const noexcept
{
   return this->parent != NULL;
}

bool
Allocation::hasChildren
(void) const noexcept
{
   return this->children.size() > 0;
}

bool
Allocation::isChild
(const Allocation &parent) const noexcept
{
   return this->isBound() && parent.children.find(const_cast<Allocation *>(this)) != parent.children.end();
}

bool
Allocation::isParent
(const Allocation &child) const noexcept
{
   return this->isBound() && child.parent == this;
}

void
Allocation::throwIfNoAllocator
(void) const
{
   if (this->allocator == NULL)
      throw NoAllocatorException(*this);
}

void
Allocation::throwIfNotBound
(void) const
{
   if (this->isBound())
      return;

   if (this->allocator == NULL)
      throw NoAllocatorException(*this);
   if (this->isNull())
      throw DeadAllocationException(*this);

   this->allocator->throwIfNotBound(*this);
}

void
Allocation::throwIfNotInRange
(SIZE_T offset) const
{
   try
   {
      this->throwIfNotInRange(this->address(offset));
   }
   catch (AddressOutOfRangeException &exception)
   {
      throw OffsetOutOfRangeException(*this, offset, exception.size);
   }
}

void
Allocation::throwIfNotInRange
(SIZE_T offset, SIZE_T size) const
{
   try
   {
      this->throwIfNotInRange(this->address(offset), size);
   }
   catch (AddressOutOfRangeException &exception)
   {
      throw OffsetOutOfRangeException(*this, offset, exception.size);
   }
}

void
Allocation::throwIfNotInRange
(const Address &address) const
{
   if (!this->inRange(address))
      throw AddressOutOfRangeException(*this, address, 0);
}

void
Allocation::throwIfNotInRange
(const Address &address, SIZE_T size) const
{
   if (!this->inRange(address, size))
      throw AddressOutOfRangeException(*this, address, size);
}

void
Allocation::throwIfNoParent
(void) const
{
   if (!this->hasParent())
      throw OrphanAllocationException(*this);
}

Address
Allocation::address
(void)
{
   this->throwIfNotBound();
   
   return this->pool.address(this->pool.minimum());
}

Address
Allocation::address
(void) const
{
   this->throwIfNotBound();

   return Address(this->pool.minimum());
}

Address
Allocation::address
(SIZE_T offset)
{
   this->throwIfNotBound();
   return this->address() + offset;
}

Address
Allocation::address
(SIZE_T offset) const
{
   Label newLabel;
   
   this->throwIfNotBound();

   newLabel = this->pool.minimum() + offset;

   this->pool.throwIfNotInRange(newLabel);

   return this->address() + offset;
}

Address
Allocation::newAddress
(void)
{
   this->throwIfNotBound();
   
   return this->pool.newAddress(this->pool.minimum());
}

Address
Allocation::newAddress
(void) const
{
   this->throwIfNotBound();

   return AddressPool::Instance.newAddress(this->pool.minimum());
}

Address
Allocation::newAddress
(SIZE_T offset)
{
   this->throwIfNotBound();
   
   return this->pool.newAddress(this->pool.minimum() + offset);
}

Address
Allocation::newAddress
(SIZE_T offset) const
{
   Label newLabel;
   
   this->throwIfNotBound();

   newLabel = this->pool.minimum() + offset;

   this->pool.throwIfNotInRange(newLabel);

   return AddressPool::Instance.newAddress(newLabel);
}

Address
Allocation::start
(void)
{
   return this->address();
}

Address
Allocation::start
(void) const
{
   return this->address();
}

Address
Allocation::end
(void)
{
   return this->address(this->size());
}

Address
Allocation::end
(void) const
{
   return this->address(this->size());
}

const Address
Allocation::baseAddress
(void) const
{
   this->throwIfNoAllocator();
   this->allocator->throwIfNotBound(*this);
   
   return this->allocator->addressOf(*this);
}

SIZE_T
Allocation::offset
(const Address &address) const
{
   this->throwIfNotInRange(address);

   return address - this->baseAddress();
}

SIZE_T
Allocation::size
(void) const noexcept
{
   return this->pool.size();
}

void
Allocation::allocate
(SIZE_T size)
{
   this->throwIfNoAllocator();
   
   if (this->isBound())
      throw DoubleAllocationException(*this);

   this->allocator->allocate(this, size);
}

void
Allocation::reallocate
(SIZE_T size)
{
   if (!this->isBound())
      return this->allocate(size);

   this->allocator->reallocate(this, size);
}

void
Allocation::deallocate
(void)
{
   this->throwIfNotBound();
   this->allocator->throwIfNotBound(*this);
   this->allocator->unbind(this);
}

void
Allocation::zeroFill
(void)
{
   this->throwIfNotBound();
   this->allocator->zeroAddress(this->address(), this->size());
}

Data
Allocation::read
(void) const
{
   return this->read(this->size());
}

Data
Allocation::read
(SIZE_T size) const
{
   return this->read(static_cast<SIZE_T>(0), size);
}

Data
Allocation::read
(SIZE_T offset, SIZE_T size) const
{
   try
   {
      return this->read(this->address(offset), size);
   }
   catch (AddressOutOfRangeException &exception)
   {
      UNUSED(exception);
      throw OffsetOutOfRangeException(*this, offset, size);
   }
}

Data
Allocation::read
(const Address &address, SIZE_T size) const
{
   this->throwIfNotBound();
   this->throwIfNotInRange(address, size);
   return this->allocator->read(this, address, size);
}

void
Allocation::write
(const Data data)
{
   this->write(static_cast<SIZE_T>(0), data);
}

void
Allocation::write
(SIZE_T offset, const Data data)
{
   try
   {
      /* explicitly create an address object so that it doesn't cast the resulting address as an integer...
         fucking C++ */
      Address offsetAddr = this->address(offset);
      this->write(offsetAddr, data);
   }
   catch (AddressOutOfRangeException &exception)
   {
      UNUSED(exception);
      throw OffsetOutOfRangeException(*this, offset, data.size());
   }
}

void
Allocation::write
(Address &destAddress, const Data data)
{
   this->throwIfNotBound();
   this->throwIfNotInRange(destAddress, data.size());
   this->allocator->write(this, destAddress, data);
}

void
Allocation::copy
(Allocation &allocation)
{
   allocation.throwIfNotBound();
   
   this->allocator = allocation.allocator;

   if (allocation.hasParent())
      this->setParent(allocation.getParent());

   /* don't copy the children though-- those belong to the other allocation, not us */

   if (this->isBound())
      this->allocator->rebind(this, allocation.address());
   else
      this->allocator->bind(this, allocation.address());

   this->pool.setRange(allocation.pool.range());
}

void
Allocation::clone
(const Allocation &allocation)
{
   allocation.throwIfNotBound();

   if (this->allocator == NULL)
      this->allocator = allocation.allocator;

   if (this->size() != allocation.size())
      this->reallocate(allocation.size());

   this->write(allocation.read());
}

Allocation 
Allocation::slice
(const Address &address, SIZE_T size)
{
   this->throwIfNotBound();

   return this->allocator->spawn(this, address, size);
}

Allocation &
Allocation::root
(void) noexcept
{
   if (this->hasParent())
      return this->parent->root();

   return *this;
}

const Allocation &
Allocation::root
(void) const noexcept
{
   if (this->hasParent())
      return this->parent->root();

   return *this;
}

Allocation &
Allocation::getParent
(void)
{
   this->throwIfNoParent();

   return *this->parent;
}

const Allocation &
Allocation::getParent
(void) const
{
   this->throwIfNoParent();

   return *this->parent;
}

Allocator::AllocationSet
Allocation::getChildren
(void) const
{
   if (this->children.size() == 0)
      return Allocator::AllocationSet();
   
   return Allocator::AllocationSet(this->children);
}

void
Allocation::setParent
(Allocation &allocation)
{
   if (this->parent != NULL)
      this->leaveParent();

   this->parent = &allocation;
   this->parent->children.insert(this);
}

void
Allocation::disownChild
(Allocation &allocation)
{
   if (this->children.find(&allocation) == this->children.end())
      return;

   allocation.parent = NULL;
   this->children.erase(&allocation);
}

void
Allocation::leaveParent
(void)
{
   this->throwIfNoParent();

   this->parent->disownChild(*this);
}
