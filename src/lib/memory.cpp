#include <neurology/memory.hpp>

using namespace Neurology;

NullPointerException::NullPointerException
(void)
   : NeurologyException("memory pointer is null")
{
}

NullPointerException::NullPointerException
(NullPointerException &exception)
   : NeurologyException(exception)
{
}

BadPointerException::BadPointerException
(LPVOID address, SIZE_T size)
   : NeurologyException("provided pointer is bad")
{
   this->address = address;
   this->size = size;
}

BadPointerException::BadPointerException
(BadPointerException &exception)
   : NeurologyException(exception)
{
   this->address = exception.address;
   this->size = exception.size;
}

OffsetOutOfBoundsException::OffsetOutOfBoundsException
(Memory *memory, SIZE_T offset, SIZE_T size)
   : NeurologyException("offset and size are out of bounds in region")
{
   this->memory = memory;
   this->offset = offset;
   this->size = size;
}

OffsetOutOfBoundsException::OffsetOutOfBoundsException
(OffsetOutOfBoundsException &exception)
   : NeurologyException(exception)
{
   this->memory = exception.memory;
   this->offset = exception.offset;
   this->size = exception.size;
}
   
Memory::Memory
(void)
{
   this->buffer = NULL;
   this->size = NULL;
   this->refCount = NULL;
}

Memory::Memory
(LPVOID buffer, SIZE_T size)
{
   this->buffer = new LPVOID;
   this->size = new SIZE_T;
   this->refCount = new DWORD;

   *this->buffer = buffer;
   *this->size = size;
   *this->refCount = 1;
}

Memory::Memory
(Memory &memory)
{
   this->buffer = memory.buffer;
   this->size = memory.size;
   this->refCount = memory.refCount;

   ++*this->refCount;
}

Memory::~Memory
(void)
{
   this->free();
}

SIZE_T
Memory::size
(void) const
{
   if (this->size == NULL)
      return 0;
   
   return *this->size;
}

void
Memory::free
(void)
{
   if (this->buffer == NULL || *this->buffer == NULL)
      throw NullPointerException();

   --*this->refCount;

   if (*this->refCount > 0)
      return;

   delete this->buffer;
   delete this->size;
   delete this->refCount;

   this->buffer = NULL;
   this->size = NULL;
   this->refCount = NULL;
}

MemoryData
Memory::read
(void)
{
   if (this->size == NULL)
      throw NullPointerException();
   
   return this->read(*this->size);
}

MemoryData
Memory::read
(SIZE_T size)
{
   return this->read(0, size);
}

MemoryData
Memory::read
(SIZE_T offset, SIZE_T size)
{
   LPBYTE bufferPoint = NULL;
   MemoryData result;
   
   if (this->buffer == NULL || *this->buffer == NULL)
      throw NullPointerException();

   if (size == 0)
      return result;

   __try
   {
      bufferPoint = (LPBYTE)*this->buffer;
      bufferPoint += offset;
      result = MemoryBlock(bufferPoint, size);
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
      throw BadPointerException(bufferPoint, size);
   }

   return result;
}

void
Memory::write
(Memory region)
{
   this->write(region.read());
}

void
Memory::write
(MemoryData data)
{
   this->write(0, data);
}

void
Memory::write
(SIZE_T offset, Memory region)
{
   this->write(offset, region.read());
}

void
Memory::write
(SIZE_T offset, MemoryData data)
{
   LPBYTE bufferPoint;
   
   if (this->buffer == NULL || *this->buffer == NULL)
      throw NullPointerException();

   if (data.size() == 0)
      return;

   __try
   {
      bufferPoint = (LPBYTE)*this->buffer;
      bufferPoint += offset;
      CopyMemory(bufferPoint, data.data(), data.size());
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      throw BadPointerException(bufferPoint, data.size());
   }
}
