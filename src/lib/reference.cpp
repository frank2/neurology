#include <neurology/reference.hpp>

using namespace Neurology;

VoidReference::Exception::Exception
(const VoidReference &reference, const LPWSTR message)
   : Neurology::Exception(message)
   , reference(reference)
{
}

VoidReference::VoidReferenceStateException::VoidReferenceStateException
(const VoidReference &reference, const LPWSTR message)
   : VoidReference::Exception(reference, message)
{
}

VoidReference::OpenVoidReferenceException::OpenVoidReferenceException
(const VoidReference &reference)
   : VoidReference::VoidReferenceStateException(reference, EXCSTR(L"The reference is currently open."))
{
}

VoidReference::ClosedVoidReferenceException::ClosedVoidReferenceException
(const VoidReference &reference)
   : VoidReference::VoidReferenceStateException(reference, EXCSTR(L"The reference is currently closed."))
{
}

VoidReference::ShallowVoidReferenceException::ShallowVoidReferenceException
(const VoidReference &reference)
   : VoidReference::Exception(reference, EXCSTR(L"The reference is shallow."))
{
}

VoidReference::SizeMismatchException::SizeMismatchException
(const VoidReference &reference, SIZE_T size)
   : VoidReference::Exception(reference, EXCSTR(L"Supplied size does not match reference size."))
   , size(size)
{
}

VoidReference::VoidReference
(void)
   : data(NULL)
   , size(0)
   , shallow(false)
{
}

VoidReference::VoidReference
(VoidReference &reference)
{
   *this = reference;
}

VoidReference::VoidReference
(const VoidReference &reference)
{
   *this = reference;
}

VoidReference::~VoidReference
(void)
{
   if (this->shallow)
      this->deallocate();
}

LPVOID
VoidReference::Allocate
(SIZE_T size)
{
   BYTE *allocation = new BYTE[size];

   ZeroMemory(allocation, size);
   
   return reinterpret_cast<LPVOID>(allocation);
}

void
VoidReference::Deallocate
(LPVOID data)
{
   delete[] data;
}

void
VoidReference::operator=
(VoidReference &reference)
{
   if (reference.isShallow())
      *this = const_cast<VoidReference &>(reference);
   else
   {
      this->data = reference.data;
      this->size = reference.size;
      this->shallow = false;
   }
}

void
VoidReference::operator=
(const VoidReference &reference)
{
   reference.throwIfNoData();

   if (!this->hasData)
      this->allocate(size);
   else if (this->size != reference.size)
      this->resize(reference.size);
         
   this->assign(*reference.data, reference.size);
   this->shallow = true;
}

void
VoidReference::assign
(const LPVOID pointer, SIZE_T size)
{
   if (!this->hasData())
      this->allocate(size);
   else if (this->size != size)
      throw VoidReference::SizeMismatchException(*this, size);
   
   CopyMemory(*this->data, pointer, size);
}

void
VoidReference::reassign
(const LPVOID pointer, SIZE_T size)
{
   this->throwIfShallow();
   
   if (this->hasData() && this->size != size)
      this->resize(size);

   this->assign(pointer, size);
}

LPVOID
VoidReference::deref
(void)
{
   this->throwIfNoData();

   return *this->data;
}

const LPVOID
VoidReference::deref
(void) const
{
   this->throwIfNoData();

   return const_cast<LPVOID>(*this->data);
}

SIZE_T
VoidReference::getSize
(void) const
{
   return this->size;
}

bool
VoidReference::hasData
(void) const
{
   return this->data != NULL && *this->data != NULL && this->size != 0;
}

bool
VoidReference::isShallow
(void) const
{
   return this->shallow;
}

void
VoidReference::throwIfShallow
(void) const
{
   if (this->isShallow())
      throw VoidReference::ShallowVoidReferenceException(*this);
}

void
VoidReference::throwIfHasData
(void) const
{
   if (this->hasData())
      throw VoidReference::OpenVoidReferenceException(*this);
}

void
VoidReference::throwIfNoData
(void) const
{
   if (!this->hasData())
      throw VoidReference::ClosedVoidReferenceException(*this);
}

void
VoidReference::allocate
(SIZE_T size)
{
   this->throwIfHasData();

   this->size = size;

   if (this->data == NULL)
      this->data = new LPVOID;
         
   *this->data = VoidReference::Allocate(this->size);
}
      
void
VoidReference::deallocate
(void)
{
   this->throwIfNoData();

   VoidReference::Deallocate(*this->data);
   *this->data = NULL;

   delete this->data;
   this->data = NULL;
}

void
VoidReference::resize
(SIZE_T size)
{
   LPVOID newData;
         
   this->throwIfShallow();
   this->throwIfNoData();

   if (size != 0)
   {
      newData = VoidReference::Allocate(size);
      CopyMemory(newData, *this->data, min(size, this->size));
   }
   else
      newData = NULL;

   VoidReference::Deallocate(*this->data);
   *this->data = newData;
   this->size = size;
}
