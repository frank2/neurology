#pragma once

#include <windows.h>

#include <map>
#include <set>

#include <neurology/exception.hpp>

namespace Neurology
{
   typedef std::size_t Label;
   typedef Label *Pointer;

   class Address;
   
   class AddressPool
   {
      friend Address;
      typedef std::map<Label, std::set<Pointer> > LabelMap;
      typedef std::map<Pointer, std::set<Address *> > PointerMap;
      
   public:
      static AddressPool Instance;
      
   protected:
      std::set<Pointer> pointers;
      std::set<Address *> addresses;
      LabelMap labels;
      PointerMap bindings;

   public:
      AddressPool(void);
      ~AddressPool(void);

      bool hasLabel(Label label);
      Address &null(void);
      Address &point(void *pointer);
      Address &point(Label label);
