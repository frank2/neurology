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
   : split(true)
   , local(false)
{
}

Allocator::Allocator
(bool split)
   : split(split)
   , local(false)
{
}

Allocator::~Allocator
(void)
{
   /* unbind all allocation bindings */
   BindingMap::iterator iter = this->bindings.begin();

   while (iter != this->bindings.end())
   {
      if (iter->second.begin() != iter->second.end())
         this->unbind(*iter->second.begin());
      else
      {
         /* the binding set is empty, check if it's still pooled.
            if so, erase it. */
         if (this->isPooled(iter->first))
            /* ha ha fuck you Stoustrup here's a hack */
            this->unpool(Address(iter->first.label()));
         
         this->bindings.erase(iter->first);
      }

      /* unbinding may have misplaced the iterator-- start over from the beginning */
      iter = this->bindings.begin();
   }
   
   /* if there are any entries left in the memory pool, delete them */
   for (MemoryPool::iterator iter=this->pooledMemory.begin();
        iter != this->pooledMemory.end();
        ++iter)
   {
      this->unpool(Address(iter->first.label()));
   }
}

void
Allocator::allowSplitting
(void)
{
   this->split = true;
}

void
Allocator::denySplitting
(void)
{
   this->split = false;
}

bool
Allocator::splits
(void) const
{
   return this->split;
}

bool
Allocator::isLocal
(void) const
{
   return this->local;
}

bool
Allocator::isPooled
(const Address &address) const
{
   return this->pooledMemory.count(address) > 0;
}

bool
Allocator::isAssociated
(const Allocation &allocation) const
{
   return this->associations.count(const_cast<Allocation *>(&allocation)) > 0;
}

bool
Allocator::isBound
(const Allocation &allocation) const
{
   Address address;
   BindingMap::const_iterator bindIter;
   std::set<Allocation *>::const_iterator refIter;
   const BindingMap *bindMap;

   if (allocation.allocator != this)
      return false;

   if (this->associations.count(const_cast<Allocation *>(&allocation)) == 0)
      return false;
   
   address = this->associations.at(const_cast<Allocation *>(&allocation));

   if (this->bindings.count(address) == 0 && this->suballocations.count(address) == 0)
      return false;

   if (!this->hasParent(allocation))
      bindMap = &this->bindings;
   else
      bindMap = &this->suballocations;

   refIter = bindMap->at(address).find(const_cast<Allocation *>(&allocation));

   return refIter != bindMap->at(address).end();
}

bool
Allocator::hasAddress
(const Address &address) const
{
   if (this->pooledAddresses.hasLabel(address.label()))
      return true;

   if (this->bindings.count(address) > 0 || this->suballocations.count(address) > 0)
      return true;

   /* check all the address pools instead */
   for (AddressPoolMap::const_iterator iter=this->addressPools.begin();
        iter!=this->addressPools.end();
        ++iter)
   {
      if (iter->second->inRange(address.label()))
         return true;
   }

   return false;
}

bool
Allocator::willSplit
(const Address &address, SIZE_T size) const
{
   BindingMap::const_iterator bindIter;
   Allocation *foundAlloc;
   Address endAddr;

   /* can't split on an address that isn't there */
   if (!this->hasAddress(address))
      return false;

   bindIter = this->bindings.upper_bound(address);

   /* address doesn't appear to be here... odd. */
   if (bindIter == this->bindings.begin() || bindIter == this->bindings.end())
      return false;

   --bindIter;

   foundAlloc = *bindIter->second.begin();

   if (foundAlloc->inRange(address, size))
      return false;
   
   endAddr = foundAlloc->end();

   ++bindIter;
   
   /* return the determination of whether or not this has one or more splits */
   return bindIter != this->bindings.end() && endAddr == bindIter->first;
}

bool
Allocator::sharesPool
(const Allocation &left, const Allocation &right) const
{
   if (!this->isBound(left) || !this->isBound(right))
      return false;

   Allocation &leftRoot = this->root(left);
   Allocation &rightRoot = this->root(right);
   
   if (this->associations.count(&leftRoot) == 0 || this->associations.count(&rightRoot) == 0)
      return false;

   return this->associations.at(&leftRoot) == this->associations.at(&rightRoot);
}

bool
Allocator::hasParent
(const Allocation &allocation) const
{
   return this->parents.count(const_cast<Allocation *>(&allocation)) > 0;
}

bool
Allocator::hasChildren
(const Allocation &allocation) const
{
   return this->children.count(const_cast<Allocation *>(&allocation)) > 0;
}

bool
Allocator::isChild
(const Allocation &parent, const Allocation &child) const
{
   if (!this->hasParent(child))
      return false;
   
   const std::set<Allocation *> &familyRef = this->children.at(const_cast<Allocation *>(&parent));
   return familyRef.find(const_cast<Allocation *>(&child)) != familyRef.end();
}

void
Allocator::throwIfNotPooled
(const Address &address) const
{
   if (!this->isPooled(address))
      throw UnpooledAddressException(const_cast<Allocator &>(*this), address);
}

void
Allocator::throwIfNotAssociated
(const Allocation &allocation) const
{
   if (!this->isAssociated(allocation))
      throw UnmanagedAllocationException(const_cast<Allocator &>(*this), const_cast<Allocation &>(allocation));
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
(const Allocation &allocation) const
{
   if (!this->isBound(allocation))
      return Address(static_cast<Label>(0));

   return const_cast<const Address &>(
      this->associations.at(const_cast<Allocation *>(
                               &allocation)));
}

Address
Allocator::address
(const Allocation &allocation)
{
   return this->address(allocation, 0);
}

Address
Allocator::address
(const Allocation &allocation, SIZE_T offset)
{
   AddressPool *pool;
   
   this->throwIfNotBound(allocation);
   pool = this->addressPools[const_cast<Allocation *>(&allocation)];
   return pool->address(pool->minimum() + offset);
}

Address
Allocator::newAddress
(const Allocation &allocation)
{
   return this->newAddress(allocation, 0);
}

Address
Allocator::newAddress
(const Allocation &allocation, SIZE_T offset)
{
   AddressPool *pool;
   
   this->throwIfNotBound(allocation);
   pool = this->addressPools[const_cast<Allocation *>(&allocation)];
   return pool->newAddress(pool->minimum() + offset);
}

SIZE_T
Allocator::bindCount
(const Address &address) const
{
   SIZE_T result = 0;

   if (this->bindings.count(address) > 0)
      result += this->bindings.at(address).size();

   if (this->suballocations.count(address) > 0)
      result += this->suballocations.at(address).size();

   return result;
}

SIZE_T
Allocator::querySize
(const Allocation &allocation) const
{
   Address baseAddress;

   /* if we're not even bound, you know for a fact that's a 0 */
   if (!this->isBound(allocation))
      return 0;

   /* if there's no address pool... welp, that's a 0 */
   if (this->addressPools.count(const_cast<Allocation *>(&allocation)) == 0)
      return 0;
   
   return this->addressPools.at(const_cast<Allocation *>(&allocation))->size();
}

Address
Allocator::pool
(SIZE_T size)
{
   Address address;

   if (size == 0)
      throw ZeroSizeException(*this);
   
   address = this->poolAddress(size);
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

      for (AllocationSet::iterator allocIter=allocations.begin();
           allocIter!=allocations.end();
           ++allocIter)
      {
         this->addressPools[*allocIter]->rebase(newAddress.label());
         this->addressPools[*allocIter]->setMax((newAddress+newSize).label());
         this->rebind(*allocIter, newAddress);
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
   BindingMap::const_iterator bindIter, subIter, allocIter;
   std::list<Allocation *> searchQueue;
   std::set<Allocation *> queueVisited;
   Allocation *possibleResult = NULL;

   subIter = this->suballocations.upper_bound(address);
   bindIter = this->bindings.upper_bound(address);

   if (this->suballocations.size() == 0 || subIter == this->suballocations.begin())
      subIter = this->suballocations.end();
   else
      --subIter;

   /* if the root allocations don't have this address, this address can't possibly exist */
   if (this->bindings.size() == 0 || bindIter == this->bindings.begin())
      throw AddressNotFoundException(const_cast<Allocator &>(*this), const_cast<Address &>(address));

   --bindIter;

   if (subIter == this->suballocations.end())
      allocIter = bindIter;
   else
      allocIter = subIter;

   for (AllocationSet::iterator iter=allocIter->second.begin();
        iter!=allocIter->second.end();
        ++iter)
      searchQueue.push_back(*iter);

   while (searchQueue.size() > 0)
   {
      Allocation *popped = searchQueue.front();
      
      searchQueue.pop_front();
      queueVisited.insert(popped);

      if (size == 0 && popped->inRange(address))
         return *popped;
      else if (popped->inRange(address, size))
      {
         if (size == popped->size())
            return *popped;
         else if (size < popped->size() && possibleResult == NULL)
            possibleResult = popped;
         else if (possibleResult != NULL && size < popped->size() < possibleResult->size())
            possibleResult = popped;
      }

      if (this->hasParent(popped))
      {
         const Allocation &parent = this->parent(*popped);

         if (queueVisited.find(const_cast<Allocation *>(&parent)) != queueVisited.end())
            searchQueue.push_back(const_cast<Allocation *>(&this->parent(*popped)));
      }
   }

   if (possibleResult != NULL)
      return *possibleResult;

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
(const Allocation &allocation, SIZE_T size)
{
   /* just call the protected reallocation function */
   this->reallocate(const_cast<Allocation *>(&allocation), size);
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

   if (this->willSplit(address, size))
      return this->splitRead(address, size);
   else
      return this->read(&this->find(address, size), address, size);
}

void
Allocator::write
(const Address &address, const Data data)
{
   this->throwIfNoAddress(address);

   if (this->willSplit(address, data.size()))
      return this->splitWrite(address, data);
   else
      return this->write(&this->find(address, data.size()), address, data);
}

Allocation &
Allocator::root
(const Allocation &allocation) const
{
   this->throwIfNotBound(allocation);
   
   if (!this->hasParent(allocation))
      return const_cast<Allocation &>(allocation);

   return this->root(this->parent(allocation));
}

Allocation &
Allocator::parent
(const Allocation &allocation)
{
   this->throwIfNoParent(allocation);

   return *this->parents[const_cast<Allocation *>(&allocation)];
}

const Allocation &
Allocator::parent
(const Allocation &allocation) const
{
   this->throwIfNoParent(allocation);

   return *this->parents.at(const_cast<Allocation *>(&allocation));
}

std::set<const Allocation *>
Allocator::getChildren
(const Allocation &allocation) const
{
   this->throwIfNoParent(allocation);

   if (!this->hasChildren(allocation))
      return std::set<const Allocation *>();

   return std::set<const Allocation *>(this->children.at(const_cast<Allocation *>(&allocation)).begin()
                                       ,this->children.at(const_cast<Allocation *>(&allocation)).end());
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
   if (!this->hasParent(*allocation))
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
   if (this->children.count(allocation) > 0 && allocation->end() >= paternalEnd)
   {
      std::set<Allocation *>::iterator childIter;
      std::vector<Allocation *> deadChildren;
      std::vector<Allocation *>::iterator deadIter;

      for (childIter=this->children.at(allocation).begin();
           childIter!=this->children.at(allocation).end();
           ++childIter)
      {
         if ((*childIter)->start() >= paternalEnd)
            deadChildren.push_back(*childIter);
         else if ((*childIter)->end() > paternalEnd)
            this->addressPools.at(*childIter)->setMax(paternalEnd);
      }

      for (deadIter=deadChildren.begin(); deadIter!=deadChildren.end(); ++deadIter)
         this->disownChild(*deadIter);
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
   std::intptr_t allocationSize;
   Address localAddress;
   BindingMap *bindMap;
   bool rootAlloc;

   this->throwIfBound(*allocation);
   
   rootAlloc = !this->hasParent(*allocation);

   if (rootAlloc)
   {
      bindMap = &this->bindings;
      this->throwIfNotPooled(address);
   }
   else
   {
      Allocation &parent = this->parent(*allocation);
      parent.throwIfNotInRange(address);
      bindMap = &this->suballocations;
   }

   this->throwIfBound(*allocation);

   if (!rootAlloc)
   {
      if (address.usesPool(this->addressPools.at(allocation)))
         localAddress = address;
      else
         localAddress = this->addressPools.at(allocation)->address(address.label());
   }
   else if (address.usesPool(&this->pooledAddresses))
      localAddress = address;
   else
      localAddress = this->pooledAddresses.address(address.label());

   (*bindMap)[localAddress].insert(allocation);
   allocation->allocator = this;

   /* we might be coming from spawn(), who handles creating new addresses for
      suballocations */
   if (this->isPooled(address) && this->addressPools.count(allocation) == 0)
      this->addressPools[allocation] = new AddressPool(localAddress.label()
                                                       ,localAddress.label() + this->pooledMemory[address]);

   this->associations[allocation] = localAddress;
}

void
Allocator::rebind
(Allocation *allocation, const Address &newAddress)
{
   Address oldAddress, localNewAddress;
   SIZE_T bindCount;
   std::intptr_t delta;
   BindingMap *bindMap;
   bool rootAlloc = !this->hasParent(*allocation);
   
   if (!this->isBound(*allocation))
      return this->bind(allocation, newAddress);

   this->throwIfNotAssociated(*allocation);

   if (rootAlloc)
   {
      bindMap = &this->bindings;
      this->throwIfNotPooled(newAddress);
   }
   else
   {
      Allocation &parent = this->parent(*allocation);
      parent.throwIfNotInRange(newAddress);
      bindMap = &this->suballocations;
   }

   if (this->hasParent(*allocation))
   {
      if (newAddress.usesPool(this->addressPools.at(allocation)))
         localNewAddress = newAddress;
      else
         localNewAddress = this->addressPools.at(allocation)->address(newAddress.label());
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
   
   (*bindMap)[localNewAddress].insert(allocation);
   (*bindMap)[oldAddress].erase(allocation);
   bindCount = (*bindMap)[oldAddress].size();

   if (this->addressPools.count(allocation) == 0)
      this->addressPools[allocation] = new AddressPool(localNewAddress.label()
                                                       ,localNewAddress.label() + allocation->size());
   else
      this->addressPools[allocation]->rebase(localNewAddress.label());

   this->associations[allocation] = localNewAddress;
   
   if (this->hasChildren(*allocation))
   {
      AllocationSet::iterator childIter;

      for (childIter = this->children.at(const_cast<Allocation *>(allocation)).begin();
           childIter != this->children.at(const_cast<Allocation *>(allocation)).end();
           ++childIter)
      {
         Address newAddress = Address(this->associations.at(*childIter).label() + delta);
         this->rebind(*childIter, newAddress);
      }
   }
   
   /* originally this moved the identifier of the old address... don't do that. it causes problems. */
   if (bindCount == 0)
   {
      bindMap->erase(oldAddress);

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
   BindingMap *bindMap;
   
   this->throwIfNotBound(*allocation);

   if (!this->hasParent(*allocation))
   {
      this->throwIfNotPooled(allocation->baseAddress());
      bindMap = &this->bindings;
   }
   else
   {
      this->disownChild(allocation);
      bindMap = &this->suballocations;
   }
   
   if (this->children.count(allocation) > 0)
   {
      std::set<Allocation *>::iterator childIter;

      while ((childIter=this->children.at(allocation).begin()) != this->children.at(allocation).end())
         this->unbind(*childIter);
   }

   boundAddress = this->associations[allocation];

   (*bindMap)[boundAddress].erase(allocation);
   this->associations.erase(allocation);

   if (this->addressPools.count(allocation))
   {
      delete this->addressPools[allocation];
      this->addressPools.erase(allocation);
   }
   
   if ((*bindMap)[boundAddress].size() != 0)
      return;
   
   bindMap->erase(boundAddress);
      
   if (this->isPooled(boundAddress))
      this->unpool(boundAddress);
}

void
Allocator::addChild
(Allocation *parent, Allocation *child)
{
   this->throwIfNotBound(*parent);

   this->parents[child] = parent;

   this->children[parent].insert(child);
}

void
Allocator::disownChild
(Allocation *child)
{
   Allocation *parent;
   
   this->throwIfNoParent(*child);

   parent = this->parents[child];

   this->children[parent].erase(child);

   if (this->children[parent].size() == 0)
      this->children.erase(parent);

   this->parents.erase(child);
}

Data
Allocator::splitRead
(const Address &startAddress, SIZE_T size) const
{
   BindingMap::const_iterator bindIter;
   Data result;
   Address addressIter;
   SIZE_T oldSize = size;
   
   this->throwIfNoAddress(startAddress);

   /* won't split, do a normal read */
   if (!this->willSplit(startAddress, size))
      this->read(startAddress, size);

   /* find the allocation tied to the address, then drop to the root */
   bindIter = this->bindings.lower_bound(startAddress);

   if (bindIter == this->bindings.end() || bindIter->first != startAddress)
      --bindIter;

   addressIter = startAddress;

   while (bindIter != this->bindings.end())
   {
      Allocation *boundAlloc = *bindIter->second.begin();
      Allocation *nextAlloc;
      Data stackData;
      bool inRange;

      inRange = boundAlloc->inRange(addressIter, size);

      if (inRange)
      {
         stackData = this->read(boundAlloc, addressIter, size);
         size = 0;
      }
      else
      {
         SIZE_T newSize;

         newSize = boundAlloc->end() - addressIter;
         stackData = this->read(boundAlloc, addressIter, newSize);
         size -= newSize;
      }

      result.resize(result.size() + stackData.size());
      MoveMemory(result.data() + result.size(), stackData.data(), stackData.size());

      if (size == 0)
         break;

      ++bindIter;

      if (bindIter == this->bindings.end())
         continue;

      nextAlloc = *bindIter->second.begin();

      /* we've reached a splitting boundary, break out */
      if (boundAlloc->end() != nextAlloc->start())
         break;

      addressIter = bindIter->first;
   }

   if (size != 0)
      throw SplitsExceededException(const_cast<Allocator &>(*this), const_cast<Address &>(startAddress), oldSize);

   return result;
}

void
Allocator::splitWrite
(const Address &destination, const Data data)
{
   BindingMap::iterator bindIter;
   Address addressIter;
   SIZE_T size = data.size();
   SIZE_T windowLeft, windowRight;
   
   this->throwIfNoAllocation(destination);

   /* won't split, do a normal read */
   if (!this->willSplit(destination, size))
      this->write(destination, data);

   /* find the lower bound of the address and wind it back one if it's not exact. */
   bindIter = this->bindings.lower_bound(destination);

   if (bindIter == this->bindings.end() || bindIter->first != destination)
      --bindIter;

   addressIter = destination;
   windowLeft = windowRight = 0;

   while (bindIter != this->bindings.end())
   {
      Allocation *boundAlloc = *bindIter->second.begin();
      Allocation *nextAlloc;
      Data dataSlice;
      bool inRange;

      inRange = boundAlloc->inRange(addressIter, size);

      if (inRange)
         windowRight = size;
      else
         windowRight += boundAlloc->end() - addressIter;
      
      dataSlice = Data(data.data()+windowLeft, data.data()+(windowRight - windowLeft));
      this->write(boundAlloc, addressIter, dataSlice);
      windowLeft = windowRight;

      if (windowLeft == data.size())
         break;

      ++bindIter;

      if (bindIter == this->bindings.end())
         continue;

      nextAlloc = *bindIter->second.begin();

      /* we've reached a splitting boundary, break out */
      if (boundAlloc->end() != nextAlloc->start())
         break;

      addressIter = bindIter->first;
   }

   if (windowLeft != data.size())
      throw SplitsExceededException(*this, const_cast<Address &>(destination), size);
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
(const Allocation *allocation, const Address &address, SIZE_T size)
{
   Allocation newAllocation;
   Address baseAddress;
   
   allocation->throwIfNotInRange(address, size);

   newAllocation = this->null();
   baseAddress = allocation->address(allocation->offset(address));
   this->addChild(const_cast<Allocation *>(allocation), &newAllocation);
   this->addressPools[&newAllocation] = new AddressPool(address.label()
                                                        ,address.label()+size);

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

Allocation::Allocation
(Allocator *allocator, Address &address, SIZE_T size)
{
   if (allocator == NULL)
      throw NoAllocatorException(*this);

   if (!address.isNull())
      allocator->throwIfNotPooled(&address);

   this->allocator = allocator;
}

Allocation::Allocation
(void)
   : allocator(NULL)
{
}

Allocation::Allocation
(Allocator *allocator)
{
   if (allocator == NULL)
      throw NoAllocatorException(*this);

   this->allocator = allocator;
}

Allocation::Allocation
(Allocation &allocation)
{
   if (allocation.isValid())
      this->copy(allocation);
}

Allocation::Allocation
(const Allocation *allocation)
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

   this->allocator = NULL;
}

void
Allocation::operator=
(Allocation &allocation)
{
   this->allocator = allocation.allocator;

   if (allocation.isValid())
      this->copy(allocation);
   else if (!this->isValid() && allocation.allocator != NULL)
      this->allocator = allocation.allocator;
   else
      this->throwIfInvalid();
}
   
void
Allocation::operator=
(const Allocation *allocation)
{
   this->allocator = allocation->allocator;
   
   if (allocation->isValid())
      this->clone(*allocation);
   else
      this->throwIfInvalid();
}

Address
Allocation::operator*
(void)
{
   return this->address();
}

const Address
Allocation::operator*
(void) const
{
   return this->address();
}

bool
Allocation::isNull
(void) const
{
   return this->allocator == NULL || !this->allocator->isAssociated(*this) || this->size() == 0;
}

bool
Allocation::isBound
(void) const
{
   return !this->isNull() && this->allocator->isBound(*this);
}

bool
Allocation::isValid
(void) const
{
   return this->isBound() && this->allocator->isPooled(this->allocator->addressOf(*this));
}

bool
Allocation::isLocal
(void) const
{
   return this->allocator != NULL && this->allocator->isLocal();
}

bool
Allocation::allocatedFrom
(const Allocator *allocator) const
{
   return this->allocator == allocator;
}

bool
Allocation::inRange
(SIZE_T offset) const
{
   Address check;
   
   if (this->isNull() || this->size() == 0)
      return false;

   /* C++ doesn't want to pass this to Address::operator+... because the address
      object is compatible with integers... fasjdlfajsdlkfajsd */
   return this->inRange(this->start() + static_cast<std::intptr_t>(offset));
}

bool
Allocation::inRange
(SIZE_T offset, SIZE_T size) const
{
   /* we subtract 1 because we want to check whether or not all the content within
      this allocation is readable/writable, which means the inRange check will fail
      on the end of the range provided. */
      
   return size != 0 && this->inRange(offset) && this->inRange(offset+size-1);
}

bool
Allocation::inRange
(const Address &address) const
{
   return address >= this->start() && address < this->end();
}

bool
Allocation::inRange
(const Address &address, SIZE_T size) const
{
   return size != 0 && this->inRange(address) && this->inRange(address + static_cast<std::intptr_t>(size - 1));
}

bool
Allocation::sharesPool
(const Allocation &allocation) const
{
   return this->isValid() && this->allocator->sharesPool(*this, allocation);
}

bool
Allocation::hasParent
(void) const
{
   return this->isValid() && this->allocator->hasParent(*this);
}

bool
Allocation::hasChildren
(void) const
{
   return this->isValid() && this->allocator->hasChildren(*this);
}

bool
Allocation::isChild
(const Allocation &parent) const
{
   return this->isValid() && this->allocator->isChild(parent, *this);
}

bool
Allocation::isParent
(const Allocation &child) const
{
   return this->isValid() && this->allocator->isChild(*this, child);
}

void
Allocation::throwIfNoAllocator
(void) const
{
   if (this->allocator == NULL)
      throw NoAllocatorException(*this);
}

void
Allocation::throwIfInvalid
(void) const
{
   if (this->isValid())
      return;

   if (this->allocator == NULL)
      throw NoAllocatorException(*this);
   if (this->isNull())
      throw DeadAllocationException(*this);
   if (this->size() == 0)
      throw ZeroSizeException(*this);

   this->allocator->throwIfNotPooled(this->baseAddress());
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

Address
Allocation::address
(void)
{
   this->throwIfInvalid();
   
   return this->allocator->address(*this);
}

const Address
Allocation::address
(void) const
{
   this->throwIfInvalid();

   return this->allocator->address(*this);
}

Address
Allocation::address
(SIZE_T offset)
{
   this->throwIfInvalid();
   
   return this->allocator->address(*this, offset);
}

const Address
Allocation::address
(SIZE_T offset) const
{
   this->throwIfInvalid();

   return this->allocator->address(*this, offset);
}

Address
Allocation::newAddress
(void)
{
   this->throwIfInvalid();
   
   return this->allocator->newAddress(*this);
}

const Address
Allocation::newAddress
(void) const
{
   this->throwIfInvalid();

   return this->allocator->newAddress(*this);
}

Address
Allocation::newAddress
(SIZE_T offset)
{
   this->throwIfInvalid();
   
   return this->allocator->newAddress(*this, offset);
}

const Address
Allocation::newAddress
(SIZE_T offset) const
{
   this->throwIfInvalid();

   return this->allocator->newAddress(*this, offset);
}

Address
Allocation::start
(void)
{
   return this->address();
}

const Address
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

const Address
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
(void) const
{
   this->throwIfNoAllocator();
   
   return this->allocator->querySize(*this);
}

void
Allocation::allocate
(SIZE_T size)
{
   this->throwIfNoAllocator();
   
   if (this->isValid())
      throw DoubleAllocationException(*this);

   this->allocator->allocate(this, size);
}

void
Allocation::reallocate
(SIZE_T size)
{
   if (!this->isValid())
      return this->allocate(size);

   /* repooling should automatically rebind this object */
   this->allocator->reallocate(this, size);
}

void
Allocation::deallocate
(void)
{
   this->throwIfInvalid();
   this->allocator->throwIfNotBound(*this);
   this->allocator->unbind(this);
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
   this->throwIfInvalid();
   this->throwIfNotInRange(address);
   
   if (this->allocator->willSplit(address, size))
      return this->allocator->splitRead(address, size);
   else
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
      this->write(this->address(offset), data);
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
   this->throwIfInvalid();
   this->throwIfNotInRange(destAddress);
   
   if (this->allocator->willSplit(destAddress, data.size()))
      this->allocator->splitWrite(destAddress, data);
   else
      this->allocator->write(this, destAddress, data);
}

void
Allocation::copy
(Allocation &allocation)
{
   allocation.throwIfInvalid();

   this->allocator = allocation.allocator;

   if (this->isBound())
      this->allocator->rebind(this, allocation.address());
   else
      this->allocator->bind(this, allocation.address());
}

void
Allocation::clone
(const Allocation &allocation)
{
   allocation.throwIfInvalid();

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
   this->throwIfInvalid();

   return this->allocator->spawn(this, address, size);
}

Allocation &
Allocation::root
(void) const
{
   this->throwIfInvalid();

   return this->allocator->root(*this);
}

Allocation &
Allocation::parent
(void)
{
   this->throwIfInvalid();

   return this->allocator->parent(*this);
}

const Allocation &
Allocation::parent
(void) const
{
   this->throwIfInvalid();

   return this->allocator->parent(*this);
}

std::set<const Allocation *>
Allocation::children
(void) const
{
   this->throwIfInvalid();

   return this->allocator->getChildren(*this);
}
