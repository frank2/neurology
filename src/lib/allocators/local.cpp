#include <neurology/allocators/local.hpp>

using namespace Neurology;

LocalAllocator LocalAllocator::Instance;

LONG
Neurology::CopyData
(LPVOID destination, const LPVOID source, SIZE_T size)
{
   __try
   {
      MoveMemory(destination, source, size);
      return 0; // STATUS_SUCCESS
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      return GetExceptionCode();
   }
}

LocalAllocator::Exception::Exception
(LocalAllocator &allocator, const LPWSTR message)
   : Allocator::Exception(allocator, message)
{
}

LocalAllocator::KernelFaultException::KernelFaultException
(LocalAllocator &allocator, LONG status, Address &source, Address &destination, SIZE_T size)
   : LocalAllocator::Exception(allocator, EXCSTR(L"Kernel fault occured during operation on addresses."))
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
   this->split = false;
}

Address
LocalAllocator::poolAddress
(SIZE_T size)
{
   Address newAddress = this->pooledAddresses.address(new BYTE[size]);
   this->pooledMemory[newAddress] = size;

#ifndef _DEBUG
   this->zeroAddress(newAddress, size);
#endif

   return newAddress;
}

Address
LocalAllocator::repoolAddress
(Address &address, SIZE_T newSize)
{
   Address newAddress;
   
   this->throwIfNotPooled(address);

   newAddress = this->poolAddress(newSize);
   this->writeAddress(newAddress
                      ,this->readAddress(address
                                         ,min(this->pooledMemory[address]
                                              ,newSize)));
   
   /* delete the old address, but don't unpool it-- that'll nuke all the allocations */
   if (newAddress != address)
      this->unpoolAddress(address);

   return newAddress;
}

void
LocalAllocator::unpoolAddress
(Address &address)
{
   this->throwIfNotPooled(address);

#ifndef _DEBUG
   this->zeroAddress(address, this->pooledMemory[address]);
#endif

   this->pooledMemory.erase(address);

   delete[] reinterpret_cast<LPVOID>(address.label());
}

Data
LocalAllocator::readAddress
(const Address &address, SIZE_T size) const
{
   Data result;
   LONG status;

   result.resize(size);
   status = CopyData(result.data(), reinterpret_cast<LPVOID>(address.label()), size);

   if (status != 0)
      throw KernelFaultException(const_cast<LocalAllocator &>(*this)
                                 ,status
                                 ,const_cast<Address &>(address)
                                 ,Address(result.data())
                                 ,size);

   return result;
}

void
LocalAllocator::writeAddress
(const Address &destination, const Data data)
{
   LONG status;

   status = CopyData(reinterpret_cast<LPVOID>(destination.label())
                     ,static_cast<LPVOID>(
                        const_cast<LPBYTE>(data.data()))
                     ,data.size());

   if (status != 0)
      throw KernelFaultException(*this
                                 ,status
                                 ,Address(
                                    static_cast<LPVOID>(
                                       const_cast<LPBYTE>(data.data())))
                                 ,const_cast<Address &>(destination)
                                 ,data.size());
}

void
LocalAllocator::zeroAddress
(const Address &address, SIZE_T size)
{
   Data data(size);

   std::fill(data.begin(), data.end(), 0);
   this->writeAddress(address, data);
}

Allocation
Neurology::nrlMalloc
(SIZE_T size)
{
   return LocalAllocator::Instance.allocate(size);
}

void
Neurology::nrlRealloc
(Allocation &allocation, SIZE_T size)
{
   LocalAllocator::Instance.reallocate(allocation, size);
}

void
Neurology::nrlFree
(Allocation &allocation)
{
   LocalAllocator::Instance.deallocate(allocation);
}
