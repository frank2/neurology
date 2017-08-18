#include <neurology/exception.hpp>

using namespace Neurology;

NeurologyException::NeurologyException
(char *message)
   : std::exception()
{
   this->explanation = message;
}

NeurologyException::NeurologyException
(NeurologyException &exception)
{
   this->explanation = exception.what();
}

const char *
NeurologyException::what
(void) const
{
   return this->explanation;
}

Win32Exception::Win32Exception
(const char *message)
   : NeurologyException(message)
{
   this->error = GetLastError();
}

Win32Exception::Win32Exception
(Win32Exception &exception)
{
   this->error = exception.error;
}
