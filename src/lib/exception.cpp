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
