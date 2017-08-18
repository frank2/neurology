#include <neurology/image.hpp>

Image::Image
(void)
{
   this->buffer = NULL;
   this->size = 0;
   this->setProcess(&Process::CurrentProcess);
}

Image::Image
(LPVOID buffer, SIZE_T size)
{
   this->setBuffer(buffer, size);
   this->setProcess(&Process::CurrentProcess);
}

Image::Image
(SIZE_T size, LPVOID base, DWORD allocationType, DWORD protect)
{
   this->setProcess(&Process::CurrentProcess);
   this->allocate(size, base, allocationType, protect);
}

Image::Image
(Process *process, SIZE_T size, LPVOID base, DWORD allocationType, DWORD protect)
{
   this->setProcess(process);
   this->allocate(size, base, allocationType, protect);
}

LPVOID
Image::getBuffer
(void)
{
   LPVOID newBuffer;
   
   if (*this->process == Process::CurrentProcess)
      return this->buffer;

   
