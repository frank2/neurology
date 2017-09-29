#pragma once

#include <windows.h>

namespace Neurology
{
   class SecurityIdentifier
   {
   protected:
      PSID sid;

   public:
      SecurityIdentifier(void) : sid(NULL) {}
      SecurityIdentifier(PSID sid) : sid(sid) {}
      SecurityIdentifier(SecurityIdentifier &sid) : sid(sid.sid) {}
      SecurityIdentifier &operator=(SecurityIdentifier &sid) { this->setSID(sid.sid); return *this; }
      SecurityIdentifier &operator=(PSID sid) { this->setSID(sid); return *this; }
      PSID &operator*(void) { return this->getSID(); }
      const PSID &operator*(void) const { return this->getSID(); }

      PSID &getSID(void) { return this->sid; }
      const PSID &getSID(void) const { return this->sid; }
      void setSID(PSID sid) { this->sid = sid; }
   };
}
