#include <neurology/allocators/virtual.hpp>

using namespace Neurology;

VirtualAllocator VirtualAllocator::Instance;

Page::Page
(void)
   : Allocation()
   , ownedAllocation(false)
   , allocator(NULL)
{
}

Page::Page
(VirtualAllocator *allocator)
   : Allocation(allocator)
   , ownedAllocation(false)
   , allocator(allocator)
{
}

Page::Page
(VirtualAllocator *allocator, Address address)
   : Allocation(allocator)
   , ownedAllocation(false)
   , allocator(allocator)
{
   this->allocator->bind(this, address);
   this->query();
}

Page::Page
(Page &page)
   : Allocation(page)
{
   *this = page;
}

Page &
Page::operator=
(Page &page)
{
   Allocation::operator=(page);

   this->ownedAllocation = page.ownedAllocation;
   this->allocator = page.allocator;
   this->memoryInfo = page.memoryInfo;
}

void
Page::query
(void)
{
   Label baseLabel;
   
   this->throwIfNotBound();

   this->allocator->query(*this);
   baseLabel = reinterpret_cast<Label>(this->memoryInfo->BaseAddress);

   if (baseLabel != this->pool.minimum())
      this->pool.rebase(baseLabel);
   
   if (this->memoryInfo->RegionSize != this->pool.size())
      this->pool.setMax(baseLabel+this->memoryInfo->RegionSize);
}

Address
Page::allocationBase
(void)
{
   this->query();
   return this->memoryInfo->AllocationBase;
}

Page::Protection
Page::allocationProtect
(void)
{
   this->query();
   return this->memoryInfo->AllocationProtect;
}

Page::State
Page::state
(void)
{
   this->query();
   return this->memoryInfo->State;
}

Page::Protection
Page::protection
(void)
{
   this->query();
   return this->memoryInfo->Protect;
}

Page::State
Page::type
(void)
{
   this->query();
   return this->memoryInfo->Type;
}

VirtualAllocator::Exception::Exception
(VirtualAllocator &allocator, const LPWSTR message)
   : Allocator::Exception(allocator, message)
   , allocator(allocator)
{
}

VirtualAllocator::VirtualAllocator
(void)
   : Allocator()
   , defaultAllocation(MEM_RESERVE | MEM_COMMIT)
   , defaultProtection(PAGE_READWRITE)
{
   this->local = true;
}

VirtualAllocator::VirtualAllocator
(Handle &processHandle)
   : Allocator()
   , defaultAllocation(MEM_RESERVE | MEM_COMMIT)
   , defaultProtection(PAGE_READWRITE)
{
   this->setProcessHandle(processHandle);
}

void
VirtualAllocator::setProcessHandle
(Handle &handle)
{
   /* if the handle is not null, we assume the process we're being given is valid and contains the proper
      rights-- it should be the job of the caller to verify whether or not the handle has the rights it needs
      else it throw a Win32 error */
   if (handle.isNull())
      return;

   /* remove all pages in the allocator-- if we're declaring this another process's virtual allocator, then
      all underlying allocations become invalid. */
   for (Allocator::MemoryPool::iterator iter=this->pooledMemory.begin();
        iter!=this->pooledMemory.end();
        ++iter)
      this->unpool(Address(iter->first.label()));

   this->processHandle = handle;
   this->enumerate();
}

void
VirtualAllocator::setDefaultAllocation
(Page::State state)
{
   this->defaultAllocation = state;
}

void
VirtualAllocator::setDefaultProtection
(Page::State state)
{
   this->defaultProtection = state;
}

Page &
VirtualAllocator::allocate
(SIZE_T size, Page::State allocationType, Page::State protection)
{
   return this->allocate(Address::Null(), size, allocationType, protection);
}

Page &
VirtualAllocator::allocate
(Address address, SIZE_T size, Page::State allocationType, Page::State protection)
{
   Address newAddress = this->poolAddress(address, size, allocationType, protection);

   /* this creates and binds a pool, so find it and return it. */
   return *this->pages[newAddress];
}

void
VirtualAllocator::lock
(Page &page)
{
   this->throwIfNoPage(page);

   if (!VirtualLock(page.baseAddress().pointer(), page.size()))
      throw Win32Exception(EXCSTR(L"VirtualLock failed"));
}

void
VirtualAllocator::unlock
(Page &page)
{
   this->throwIfNoPage(page);

   if (!VirtualUnlock(page.baseAddress().pointer(), page.size()))
      throw Win32Exception(EXCSTR(L"VirtualUnlock failed"));
}

void
VirtualAllocator::protect
(Page &page, Page::Protection protection)
{
   DWORD oldProtect;
   
   this->throwIfNoPage(page);

   if (this->isLocal())
   {
      if (!VirtualProtect(page.address().pointer()
                          ,page.size()
                          ,protection.mask
                          ,&oldProtect))
      {
         throw Win32Exception(EXCSTR(L"VirtualProtect failed."));
      }
   }
   else
   {
      if (!VirtualProtectEx(*this->processHandle
                            ,page.address().pointer()
                            ,page.size()
                            ,protection.mask
                            ,&oldProtect))
      {
         throw Win32Exception(EXCSTR(L"VirtualProtectEx failed."));
      }
   }
}

SIZE_T
VirtualAllocator::query
(Page &page)
{
   this->throwIfNoPage(page);
   
   SIZE_T result = this->query(page.address(), page.memoryInfo.pointer(), page.memoryInfo.size());

   if (result != page.memoryInfo.size())
   {
      page.memoryInfo.reallocate(result);
      result = this->query(page.address(), page.memoryInfo.pointer(), page.memoryInfo.size());

      if (result == 0)
         throw Win32Exception(EXCSTR(L"VirtualQuery/VirtualQueryEx failed."));
   }

   return result;
}

SIZE_T
VirtualAllocator::query
(Address address, PMEMORY_BASIC_INFORMATION buffer, SIZE_T length)
{
   SIZE_T result;
   
   if (this->isLocal())
   {
      result = VirtualQuery(address.pointer()
                            ,buffer
                            ,length);
   }
   else
   {
      result = VirtualQueryEx(*this->processHandle
                              ,address.pointer()
                              ,buffer
                              ,length);
   }

   return result;
}

void
VirtualAllocator::enumerate
(void)
{
   Address address = Label(NULL);
   Object<MEMORY_BASIC_INFORMATION> memoryInfo;
   SIZE_T size;
   bool foundEntries = false;

   /* this is intentional-- if size is zero we break the loop */
   while (size = this->query(address, memoryInfo.pointer(), memoryInfo.size()))
   {
      if (size != memoryInfo.size())
      {
         memoryInfo.reallocate(size);
         continue;
      }

      foundEntries = true;

      address = memoryInfo->BaseAddress;

      /* we already appear to have this page-- skip it */
      if (this->pages.find(address) != this->pages.end())
      {
         address += memoryInfo->RegionSize;
         continue;
      }

      this->createPage(address, false);
      address += memoryInfo->RegionSize;
   }

   if (!foundEntries)
      throw Win32Exception(EXCSTR(L"VirtualQuery failed"));
}

Address
VirtualAllocator::poolAddress
(SIZE_T size)
{
   return this->poolAddress(Address::Null(), size, this->defaultAllocation, this->defaultProtection);
}

Address
VirtualAllocator::poolAddress
(Address address, SIZE_T size, Page::State allocationType, Page::State protection)
{
   Address resultAddress;
   std::pair<PageObjectSet::iterator,bool> insertion;

   if (this->isLocal())
      resultAddress = VirtualAlloc(address.pointer(), size, allocationType.mask, protection.mask);
   else
      resultAddress = VirtualAllocEx(*this->processHandle, address.pointer(), size, allocationType.mask, protection.mask);

   if (resultAddress == Address::Null())
      throw Win32Exception(EXCSTR(L"VirtualAlloc failed."));

   resultAddress = this->pooledAddresses.address(resultAddress.label());

   if (this->pages.count(resultAddress) == 0)
      this->createPage(resultAddress, true);
   else
      this->pages[resultAddress]->query();

   return resultAddress;
}

Address
VirtualAllocator::repoolAddress
(Address &address, SIZE_T newSize)
{
   Address newAddress;
   Data data;

   this->throwIfNotPooled(address);

   Page &page = *this->pages[address];
   data = page.read(min(page.size(), newSize));

   /* call freePage instead of deallocate/unbind/etc so that the unbinding process doesn't actually happen.
      what will happen instead is that the original page allocation will be overwritten with the new
      allocation information. */
   newAddress = this->poolAddress(address, newSize, page.memoryInfo->Type, page.memoryInfo->AllocationProtect);
   this->write(newAddress, data);

   /* move the page to the new address, then erase it from the pages */
   if (newAddress != address)
   {
      PageObjectSet::iterator pageIter;
      Page &newPage = *this->pages[newAddress];
      
      this->rebind(&page, newAddress);
      page.query();
      
      this->pages.erase(address);
      this->pages[newAddress] = &page;
      
      pageIter = this->pageObjects.find(newPage);

      if (pageIter != this->pageObjects.end)
         this->pageObjects.erase(*pageIter);
   }

   return newAddress;
}

void
VirtualAllocator::unpoolAddress
(Address &address)
{
   this->throwIfNotPooled(address);

   Page &page = *this->pages[address];

   this->freePage(page);
   this->pages.erase(address);

   if (this->pageObjects.find(page) != this->pageObjects.end())
      this->pageObjects.erase(page);
}

void
VirtualAllocator::createPage
(Address &address, bool owned)
{
   std::pair<PageObjectSet::iterator, bool> insertion;

   insertion = this->pageObjects.insert(Page(this, address));
   this->pages[address] = &(*insertion.first);
}

void
VirtualAllocator::freePage
(Page &page)
{
   BOOL result;
   Page::State releaseState;
   
   this->throwIfNotPooled(page.address());

   if (page.state().commit && page.state().reserve)
      releaseState.release = 1;
   else
      releaseState.decommit = 1;

   if (this->isLocal())
      result = VirtualFree(page.address().pointer(), page.size(), releaseState);
   else
      result = VirtualFreeEx(*this->processHandle, page.address().pointer(), page.size(), releaseState);

   page.memoryInfo.reset();

   if (result == FALSE)
      throw Win32Exception(EXCSTR(L"VirtualFree failed."));
}

void
VirtualAllocator::allocate
(Allocation *allocation, SIZE_T size)
{
   Address newAddress = this->poolAddress(size);
   Page &page = *this->pages[newAddress];
   *allocation = page.slice(page.address(), size);
}
