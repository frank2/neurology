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
LocalAllocator::pool
(SIZE_T size)
{
   Address address = this->pooledAddresses.address(new BYTE[size]);
   this->pooledMemory[address] = size;

   return address;
}

void
LocalAllocator::repool
(Address &address, SIZE_T newSize)
{
   this->throwIfNotPooled(address);
