#include <neurology/allocators/local.hpp>

using namespace Neurology;

LocalAllocator LocalAllocator::Instance;

LocalAllocator::Exception::Exception
(LocalAllocator &allocator, const LPWSTR message)
   : Allocator::Exception(allocator, message)
{
}

LocalAllocator::KernelFaultException
(LocalAllocator &allocator, LONG status, Address source, Address destination, SIZE_T size)
   : Allocator::Exception(allocator, EXCSTR(L"Kernel fault occured during operation on addresses."))
   , status(status)
   , source(source)
   , destination(destination)
   , size(size)
{
}

LocalAllocator::LocalAllocator
(void)
   : Allocator()
{
}

Allocation
LocalAllocator::Allocate
(SIZE_T size)
{
   return LocalAllocator::Instance.allocate(size);
}

void
LocalAllocator::Deallocate
(Allocation &allocation)
{
   LocalAllocator::Instance.deallocate(allocation);
}

Address
LocalAllocator::poolAddress
(SIZE_T size)
{
   return this->pooledAddresses.address(new BYTE[size]);
}

Address
LocalAllocator::repoolAddress
(Address &address, SIZE_T newSize)
{
   Address newAddress;
   
   this->throwIfNotPooled(address);

   newAddress = this->pooledAddresses.address(new BYTE[size]);
   this->writeAddress(newAddress, this->readAddress(address, this->pooledMemory[address]));
   
   /* delete the old address, but don't unpool it-- that'll nuke all the allocations */
   if (newAddress != address)
      delete[] reinterpret_cast<LPBYTE>(address.label());

   return newAddress;
}
