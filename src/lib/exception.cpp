#include <neurology/exception.hpp>

using namespace Neurology;

NeurologyException::NeurologyException
(const LPWSTR message)
{
   this->explanation = message;
}

NeurologyException::NeurologyException
(NeurologyException &exception)
{
   this->explanation = exception.explanation;
}

Win32Exception::Win32Exception
(const LPWSTR message)
   : NeurologyException(message)
{
   this->error = GetLastError();
}

Win32Exception::Win32Exception
(DWORD error, const LPWSTR message)
   : NeurologyException(message)
{
   this->error = error;
}

Win32Exception::Win32Exception
(Win32Exception &exception)
   : NeurologyException(exception)
{
   this->error = exception.error;
}

NullPointerException::NullPointerException
(void)
   : NeurologyException(EXCSTR(L"A pointer was null when it shouldn't have been."))
{
}

NullPointerException::NullPointerException
(const LPWSTR message)
   : NeurologyException(message)
{
}

NullPointerException::NullPointerException
(NullPointerException &exception)
   : NeurologyException(exception)
{
}
