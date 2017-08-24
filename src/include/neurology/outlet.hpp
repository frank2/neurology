#pragma once

#include <windows.h>

#include <type_traits>

#include <neurology/exception.hpp>

namespace Neurology
{
   class Outlet;
   
   class OutletException : public NeurologyException
   {
   public:
      const Outlet &outlet;
      
      OutletException(const Outlet &outlet, const LPWSTR message);
      OutletException(OutletException &exception);
   };
   
   class NullSocketException : public OutletException
   {
   public:
      NullSocketException(const Outlet &outlet);
   };
   
   class DoubleAllocationException : public OutletException
   {
   public:
      DoubleAllocationException(const Outlet &outlet);
   };

   class ActiveOutletException : public OutletException
   {
   public:
      const Outlet &inactive;
      
      ActiveOutletException(const Outlet &outlet, const Outlet &inactive);
   };

   class DepthException : public OutletException
   {
   public:
      DepthException(const Outlet &outlet);
   };

   class Outlet;

   template <class Type>
   class Circuit
   {
   protected:
      Type *circuit;
      SIZE_T size;
      bool shallow;

   public:
      Circuit(void)
      {
         this->circuit = NULL;
         this->size = 0;
         this->shallow = false;
      }
      
      Circuit(Circuit &circuit)
      {
         *this = circuit;
      }
      
      Circuit(const Circuit &circuit)
      {
         *this = circuit;
      }

      Circuit(Type value)
      {
         this->size = sizeof(value);
         this->open();
         *this->circuit = value;
         this->shallow = false;
      }

      Circuit(Type *type, SIZE_T size)
      {
         this->size = size;
         this->open();
         CopyMemory(this->circuit, type, size);
         this->shallow = false;
      }

      void operator=(Circuit &circuit)
      {
         circuit.throwIfClosed();

         this->circuit = circuit.circuit;
         this->size = circuit.size;
         this->shallow = false;
      }

      void operator=(const Circuit &circuit)
      {
         circuit.throwIfClosed();

         this->size = circuit.size;

         if (this->isClosed())
            this->allocate();
         else
            this->adjust(circuit.size);

         CopyMemory(this->circuit, circuit.circuit, circuit.size);
         this->shallow = true;
      }

      void operator=(Type value)
      {
         this->throwIfShallow();
         this->throwIfClosed();

         *this->circuit = value;
      }

      void operator=(Type *type)
      {
         this->throwIfShallow();
         this->throwIfClosed();

         CopyMemory(this->circuit, circuit.circuit, this->size);
      }
         
      Type operator*(void) const
      {
         this->throwIfClosed();

         return *this->circuit;
      }

      bool isClosed(void) const
      {
         return this->circuit == NULL;
      }
      
      bool isShallow(void) const
      {
         return this->shallow;
      }
      
      void open(void)
      {
         this->throwIfOpen();

         this->circuit = reinterpret_cast<Type *>(new BYTE[this->size]);
      }
      
      void close(void);
      void adjust(SIZE_T size);
   };

   class Outlet
   {
   protected:
      bool shallow;
      LPDWORD sockets;
      
   public:
      Outlet(void);
      Outlet(Outlet &plug);
      Outlet(const Outlet &plug);
      ~Outlet(void);

      virtual void operator=(Outlet &plug);
      virtual void operator=(const Outlet &plug);

      DWORD sockets(void) const;
      virtual void plug(void);
      virtual void unplug(void);
      virtual bool isNull(void) const;
      bool isShallow(void) const;
      void throwIfNull(void) const;
      void throwIfShallow(void) const;

   protected:
      virtual void allocate(void);
      virtual void release(void);
   };

   template<class OutletType>
   class Plug
   {
      static_assert(std::is_base_of<Outlet, OutletType>::value, "template class does not derive Neurology::Outlet");
      
   protected:
      OutletType outlet;

   public:
      Plug(void)
      {
      }

      Plug(Plug &plug)
      {
         *this = plug;
      }

      Plug(const Plug &plug)
      {
         *this = plug;
      }

      virtual void operator=(Plug &plug)
      {
         this->outlet = plug.outlet;
      }

      virtual void operator=(const Plug &plug)
      {
         this->outlet = (const)plug.outlet;
      }

      DWORD sockets(void) const
      {
         return this->outlet.sockets();
      }
      
      void plug(void)
      {
         this->outlet.plug();
      }
      
      void unplug(void)
      {
         this->outlet.unplug();
      }
      
      bool isNull(void) const
      {
         return this->outlet.isNull();
      }
      
      bool isShallow(void) const
      {
         return this->outlet.isShallow();
      }
      
      void throwIfNull(void) const
      {
         this->outlet.throwIfNull();
      }
      
      void throwIfShallow(void) const
      {
         this->outlet.throwIfShallow();
      }
   };
}
