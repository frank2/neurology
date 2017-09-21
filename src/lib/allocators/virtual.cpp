#include <neurology/allocators/virtual.hpp>

using namespace Neurology;

VirtualAllocator VirtualAllocator::Instance(true);

Page::Exception::Exception
(VirtualAllocator *allocator, const LPWSTR message)
   : Allocator::Exception(allocator, message)
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
   : ~Allocator()
{
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
   while (Allocator::MemoryPool::iterator iter=this->pooledMemory.begin();
          iter!=this->pooledMemory.end();
          ++iter)
      this->unpool(iter->first);

   this->processHandle = handle;
   this->enumerate(void);
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
   return this->allocate(NULL, size, allocationType, protection);
}

Page &
VirtualAllocator::allocate
(Address address, SIZE_T size, Page::State allocationType, Page::State protection)
{
   Page &pageObj = this->pages[address];

   pageObj->allocator = this;
   this->allocate(&pageObj, address, size, allocationType, protection);

   if (pageObj->baseAddress() != address)
   {
      this->pages[pageObj->baseAddress()] = pageObj;
      address = pageObj->baseAddress();
      this->pages.erase(address);
   }

   return this->pages[address].reference();
}

void
VirtualAllocator::lock
(Page &page)
{
   this->throwIfNoPage(page);

   if (!VirtualLock(page->baseAddress().pointer(), page.size()))
      throw Win32Exception(EXCSTR(L"VirtualLock failed"));
}

void
VirtualAllocator::unlock
(Page &page)
{
   this->throwIfNoPage(page);

   if (!VirtualUnlock(page->baseAddress().pointer(), page.size()))
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
   Address address = NULL;
   Object<MEMORY_BASIC_INFORMATION> memoryInfo;
   SIZE_T size;
   bool foundEntries = false;

   /* this is intentional-- if size is zero we break the loop */
   while (size = this->query(address, memoryInfo.pointer(), memoryInfo.size()))
   {
      if (size != memoryInfo.size())
      {
         memoryInfo.reallocate<MEMORY_BASIC_INFORMATION>(size);
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

      /* this constructor should automatically bind */
      this->pages[address] = new Page(this, address, memoryInfo.pointer(), memoryInfo->RegionSize);

      address += memoryInfo->RegionSize;
   }

   if (!foundEntries)
      throw Win32Exception(EXCSTR(L"VirtualQuery failed"));
}

Address
VirtualAllocator::poolAddress
(SIZE_T size)
{
   return this->poolAddress(NULL, size, this->defaultAllocation, this->defaultProtection);
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

   if (resultAddress == NULL)
      throw Win32Error(EXCSTR(L"VirtualAlloc failed."));

   resultAddress = this->pooledAddresses.address(resultAddress.label());

   /* constructor should bind itself */
   this->pages[resultAddress] = Page(this, resultAddress);

   return resultAddress;
}

Address
VirtualAllocator::repoolAddress
(Address &address, SIZE_T newSize)
{
   Address newAddress;
   Data data;

   this->throwIfNotPooled(address);

   Page &page = this->pages[address];
   data = page.read();

   /* call freePage instead of deallocate/unbind/etc so that the unbinding process doesn't actually happen.
      what will happen instead is that the original page allocation will be overwritten with the new
      allocation information. */
   this->freePage(page);
   newAddress = this->poolAddress(address, newSize, page.memoryInfo->Type, page.memoryInfo->AllocationProtect);
   this->write(newAddress, data);

   return newAddress;
}
