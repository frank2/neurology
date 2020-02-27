#include <neurology/allocators/virtual.hpp>

using namespace Neurology;

Page::Page
(void)
   : Allocation()
   , ownedAllocation(false)
   , allocator(NULL)
{
   this->memoryInfo.construct();
}

Page::Page
(VirtualAllocator *allocator)
   : Allocation(allocator)
   , ownedAllocation(false)
   , allocator(allocator)
{
   this->memoryInfo.construct();
}

Page::Page
(VirtualAllocator *allocator, Address address)
   : Allocation(allocator)
   , ownedAllocation(false)
   , allocator(allocator)
{
   this->memoryInfo.construct();
   this->allocator->bind(this, address);
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

   return *this;
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

void
Page::release
(void)
{
   this->throwIfNotBound();
   this->allocator->unpool(this->address());
}

VirtualAllocator::Exception::Exception
(VirtualAllocator &allocator, const LPWSTR message)
   : Allocator::Exception(allocator, message)
   , allocator(allocator)
{
}

VirtualAllocator::NoSuchPageException::NoSuchPageException
(VirtualAllocator &allocator, Page &page)
   : VirtualAllocator::Exception(allocator, EXCSTR(L"No such page in the allocator."))
   , page(page)
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

VirtualAllocator::~VirtualAllocator
(void)
{
   PageObjectMap::iterator pageIter;

   while (this->pages.size() > 0)
   {
      pageIter = this->pages.begin();
      this->unpool(Address(pageIter->first.label()));
   }
}

bool
VirtualAllocator::hasPage
(Page &page) const noexcept
{
   PageObjectMap::const_iterator iter;

   iter = this->pages.find(page.address());

   if (iter == this->pages.end())
      return false;

   return iter->second == &page;
}

void
VirtualAllocator::throwIfNoPage
(Page &page) const
{
   if (!this->hasPage(page))
      throw NoSuchPageException(*const_cast<VirtualAllocator *>(this), page);
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
   this->local = false;
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
VirtualAllocator::pageOf
(Address address)
{
   PageObjectMap::iterator lowerBound;
   
   this->throwIfNoAddress(address);

   lowerBound = this->pages.lower_bound(address);

   if (lowerBound != this->pages.end() && lowerBound->second->address() == address)
      return *(lowerBound->second);

   --lowerBound;

   if (lowerBound == this->pages.end())
      throw Allocator::AddressNotFoundException(*this, address);

   lowerBound->second->throwIfNotInRange(address);

   return *lowerBound->second;
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

   if (!VirtualLock(page.address().pointer(), page.size()))
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

   memoryInfo.construct();

   /* this is intentional-- if size is zero we break the loop */
   while (size = this->query(address, memoryInfo.pointer(), memoryInfo.size()))
   {
      Address pooledAddress;
      
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
         this->pages[address]->query();
         address += memoryInfo->RegionSize;
         continue;
      }

      pooledAddress = this->pooledAddresses.address(address.label());
      this->pooledMemory[pooledAddress] = memoryInfo->RegionSize;
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

   if (this->isLocal())
      resultAddress = VirtualAlloc(address.pointer(), size, allocationType.mask, protection.mask);
   else
      resultAddress = VirtualAllocEx(*this->processHandle, address.pointer(), size, allocationType.mask, protection.mask);

   if (resultAddress == Address::Null())
      throw Win32Exception(EXCSTR(L"VirtualAlloc failed."));

   resultAddress = this->pooledAddresses.address(resultAddress.label());

   if (this->pages.count(resultAddress) == 0)
   {
      this->pooledMemory[resultAddress] = size;
      this->createPage(resultAddress, true);
   }
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

   newAddress = this->poolAddress(address, newSize, page.memoryInfo->Type, page.memoryInfo->AllocationProtect);
   this->write(newAddress, data);

   /* move the page to the new address, then erase the new one from the pages */
   if (newAddress != address)
   {
      Page *newPage = this->pages[newAddress];
      
      this->rebind(&page, newAddress);
      page.query();
            
      this->pages.erase(address);
      this->pages[newAddress] = &page;

      /* delete the new page object we allocated. after rebinding, the allocation should remain available,
         since there are now technically two allocations allocated to the address, thus preventing the new
         allocation from being deallocated */
      delete newPage;
   }

   return newAddress;
}

void
VirtualAllocator::unpoolAddress
(Address &address)
{
   this->throwIfNotPooled(address);

   Page &page = *this->pages[address];

   this->freePage(address);
   this->pages.erase(address);
}

void
VirtualAllocator::createPage
(Address &address, bool owned)
{
   this->pages[address] = new Page(this, address);
   this->pages[address]->ownedAllocation = owned;
}

void
VirtualAllocator::freePage
(Address &address)
{
   BOOL result;
   Page *pointer;
   Object<MEMORY_BASIC_INFORMATION> memoryInfo;
   SIZE_T memorySize;

   /* we query the page manually here instead of asking the page for its actual address because unbinding
      may have wiped out the pool. this is the same reason the function doesn't rely on querying the page itself
      for the address-- the page may have been unbound before it was freed, which assigns */
   this->throwIfNotPooled(address);
   memoryInfo.construct();
   memorySize = this->query(address, memoryInfo.pointer(), memoryInfo.size());

   if (memorySize == 0 && (this->isLocal() || !this->processHandle.isNull()))
      throw Win32Exception(EXCSTR(L"VirtualQuery/VirtualQueryEx failed."));

   if (memorySize != 0 && memorySize != memoryInfo.size())
   {
      memoryInfo.reallocate(memorySize);
      memorySize = this->query(address, memoryInfo.pointer(), memoryInfo.size());

      if (memorySize == 0)
         throw Win32Exception(EXCSTR(L"VirtualQuery/VirtualQueryEx failed."));
   }

   if (memorySize != 0 && address != memoryInfo->BaseAddress)
      throw Allocator::AddressNotFoundException(*this, address);
   
   pointer = this->pages[address];

   if (memorySize != 0 && pointer->ownedAllocation)
   {
      if (this->isLocal())
         result = VirtualFree(memoryInfo->BaseAddress, 0, MEM_RELEASE);
      else
         result = VirtualFreeEx(*this->processHandle, memoryInfo->BaseAddress, 0, MEM_RELEASE);
      
      if (result == FALSE)
         throw Win32Exception(EXCSTR(L"VirtualFree failed."));
   }

   pointer->memoryInfo.reset();

   this->pages.erase(address);
   this->pooledMemory.erase(address);
   delete pointer;
}

void
VirtualAllocator::allocate
(Allocation *allocation, SIZE_T size)
{
   Address newAddress = this->poolAddress(size);
   Page &page = *this->pages[newAddress];
   *allocation = page.slice(page.address(), size);
}

Data
VirtualAllocator::readAddress
(const Address &address, SIZE_T size) const
{
   Data data(size);
   SIZE_T bytesRead;
   LONG result;

   if (this->isLocal())
   {
      result = CopyData(data.data(), address.pointer(), size);

      if (result != 0)
         throw KernelFaultException(result, const_cast<Address &>(address), Address(data.data()), size);
   }
   else
   {
      result = ReadProcessMemory(*this->processHandle
                                 ,address.pointer()
                                 ,data.data()
                                 ,size
                                 ,&bytesRead);

      if (result == 0)
         throw Win32Exception(EXCSTR(L"ReadProcessMemory failed."));

      data.resize(bytesRead);
   }

   return data;
}

void
VirtualAllocator::writeAddress
(const Address &address, const Data data)
{
   SIZE_T bytesWritten;
   LONG result;

   if (this->isLocal())
   {
      result = CopyData(address.pointer()
                        ,reinterpret_cast<LPVOID>(
                           const_cast<LPBYTE>(data.data()))
                        ,data.size());

      if (result != 0)
         throw KernelFaultException(result
                                    ,const_cast<Address &>(address)
                                    ,Address(reinterpret_cast<LPVOID>(
                                                const_cast<LPBYTE>(data.data())))
                                    ,data.size());
   }
   else
   {
      result = WriteProcessMemory(*this->processHandle
                                  ,address.pointer()
                                  ,data.data()
                                  ,data.size()
                                  ,&bytesWritten);

      if (result == 0)
         throw Win32Exception(EXCSTR(L"ReadProcessMemory failed."));
   }
}

   
