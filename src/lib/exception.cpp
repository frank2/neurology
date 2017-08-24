#include <neurology/exception.hpp>

using namespace Neurology;

NeurologyException::NeurologyException
(const LPWSTR message)
{
   this->explanation = message;
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
