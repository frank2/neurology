#include <neurology/exception.hpp>

using namespace Neurology;

Exception::Exception
(const LPWSTR message)
   : explanation(message)
{
}

Win32Exception::Win32Exception
(const LPWSTR message)
   : Exception(message)
{
   this->error = GetLastError();
}

Win32Exception::Win32Exception
(DWORD error, const LPWSTR message)
   : Exception(message)
{
   this->error = error;
}

NullPointerException::NullPointerException
(void)
   : Exception(EXCSTR(L"A pointer was null when it shouldn't have been."))
{
}

NullPointerException::NullPointerException
(const LPWSTR message)
   : Exception(message)
{
}
