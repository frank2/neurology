#include <neurology/dos.hpp>

using namespace Neurology;

InvalidDOSImageException::InvalidDOSImageException
(LPVOID offendingBuffer)
   : NeurologyException("the provided buffer is not a valid DOS image")
{
   this->offendingBuffer = offendingBuffer;
}

InvalidDOSImageException::InvalidDOSImageException
(InvalidDOSImageException &exception)
   : NeurologyException(exception)
{
   this->offendingBuffer = exception.offendingBuffer;
}

NullHeaderException::NullHeaderException
(void)
   : NeurologyException("DOS header is null")
{
}

NullHeaderException::NullHeaderException
(NullHeaderException &exception)
   : NeurologyException(exception)
{
}

DOSHeader::DOSHeader
(LPVOID buffer)
{
   if (buffer == NULL)
   {
      this->header = NULL;
   }
   else
   {
      this->header = (PIMAGE_DOS_HEADER)buffer;

      if (!this->isDOSImage())
         throw InvalidDOSImageException(buffer);
   }
}

WORD &
DOSHeader::e_magic
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_magic;
}

WORD &
DOSHeader::e_cblp
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_cblp;
}

WORD &
DOSHeader::e_cp
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_cp;
}

WORD &
DOSHeader::e_crlc
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_crlc;
}

WORD &
DOSHeader::e_cparhdr
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_cparhdr;
}

WORD &
DOSHeader::e_minalloc
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_minalloc;
}

WORD &
DOSHeader::e_maxalloc
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_maxalloc;
}

WORD &
DOSHeader::e_ss
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_ss;
}

WORD &
DOSHeader::e_csum
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_csum;
}

WORD &
DOSHeader::e_ip
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_ip;
}

WORD &
DOSHeader::e_cs
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_cs;
}

WORD &
DOSHeader::e_lfarlc
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_lfarlc;
}

WORD &
DOSHeader::e_ovno
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_ovno;
}

WORD 
(&DOSHeader::e_res
 (void))[4]
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_res;
}

WORD &
DOSHeader::e_oemid
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_oemid;
}

WORD &
DOSHeader::e_oeminfo
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_oeminfo;
}

WORD 
(&DOSHeader::e_res2
 (void))[10]
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_res2;
}

LONG &
DOSHeader::e_lfanew
(void)
{
   if (this->header == NULL)
      throw NullHeaderException();

   return this->header->e_lfanew;
}

bool
DOSHeader::isDOSImage
(void) const
{
   return this->header != NULL && this->header->e_magic == IMAGE_DOS_SIGNATURE;
}


