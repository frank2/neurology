#pragma once

#include <windows.h>

#include <type_traits>
#include <vector>

#include <neurology/exception.hpp>

namespace Neurology
{
   class Circuit;
   class Capacitor;

   class CapacitorException : public NeurologyException
   {
   public:
      const Capacitor &capacitor;

      CapacitorException(const Capacitor &capacitor, const LPWSTR message);
      CapacitorException(CapacitorException &exception);
   };
   
   class CircuitException : public NeurologyException
   {
   public:
      const Circuit &circuit;
      
      CircuitException(const Circuit &circuit, const LPWSTR message);
      CircuitException(CircuitException &exception);
   };

   class NullCapacitorException : public CapacitorException
   {
   public:
      NullCapacitorException(const Capacitor &capacitor);
   };

   class OverloadedCapacitorException : public CapacitorException
   {
   public:
      OverloadedCapacitorException(const Capacitor &capacitor, const LPWSTR message);
   };

   class OpenCapacitorException : public OverloadedCapacitorException
   {
   public:
      OpenCapacitorException(const Capacitor &capacitor);
   };

   class ClosedCapacitorException : public OverloadedCapacitorException
   {
   public:
      ClosedCapacitorException(const Capacitor &capacitor);
   };

   class ShallowCapacitorException : public CapacitorException
   {
   public:
      ShallowCapacitorException(const Capacitor &capacitor);
   };

   class AssignmentMismatchException : public CapacitorException
   {
   public:
      AssignmentMismatchException(const Capacitor &capacitor);
   };
   
   class NullSocketException : public CircuitException
   {
   public:
      NullSocketException(const Circuit &circuit);
   };

   class ActiveCircuitException : public CircuitException
   {
   public:
      const Circuit &inactive;
      
      ActiveCircuitException(const Circuit &circuit, const Circuit &inactive);
   };

   class ShallowCircuitException : public CircuitException
   {
   public:
      ShallowCircuitException(const Circuit &circuit);
   };

   class SolderedCapacitorsException : public CircuitException
   {
   public:
      SolderedCapacitorsException(const Circuit &circuit);
   };

   class Capacitor
   {
   protected:
      LPVOID *energy;
      SIZE_T volts;

   private:
      bool shallow;

   public:
      Capacitor(void)
      {
         this->energy = NULL;
         this->volts = 0;
         this->shallow = false;
      }
      
      Capacitor(Capacitor &capacitor)
      {
         *this = capacitor;
      }
      
      Capacitor(const Capacitor &capacitor)
      {
         *this = capacitor;
      }

      ~Capacitor(void)
      {
         if (this->shallow)
            this->discharge();
      }

      static LPVOID Allocate(SIZE_T size)
      {
         return reinterpret_cast<LPVOID>(new BYTE[size]);
      }
      
      static void Deallocate(LPVOID energy)
      {
         delete[] energy;
      }

      void operator=(Capacitor &capacitor)
      {
         this->energy = capacitor.energy;
         this->volts = capacitor.volts;
         this->shallow = false;
      }

      void operator=(const Capacitor &capacitor)
      {
         if (capacitor.hasEnergy())
         {
            if (!this->hasEnergy())
               this->charge(capacitor.volts);
            else
               this->adjustVoltage(capacitor.volts);

            CopyMemory(*this->energy, *capacitor.energy, capacitor.size);
            this->shallow = true;
         }
         else
         {
            this->energy = NULL;
            this->volts = 0;
            this->shallow = false;
         }
      }

      template <class Type>
      void assign(Type value)
      {
         this->throwIfShallow();

         if (!this->hasEnergy())
            this->charge(sizeof(Type));
         else if (this->volts != sizeof(Type))
            throw BadVoltageException(*this);
         
         CopyMemory(*this->energy, &value, sizeof(Type));
      }

      template <class Type>
      void assign(Type *type)
      {
         this->throwIfShallow();

         if (!this->hasEnergy())
            this->charge(sizeof(Type));
         else if (this->volts != sizeof(Type))
            throw BadVoltageException(*this);

         CopyMemory(*this->energy, type, this->size);
      }

      template <class Type>
      void assign(Type *type, SIZE_T size)
      {
         this->throwIfShallow();

         if (!this->hasEnergy())
            this->charge(size);
         else if (this->volts != size)
            throw BadVoltageException(*this);

         this->assign<Type>(type);
      }

      template <class Type>
      void reassign(Type value)
      {
         if (!this->hasEnergy())
            this->charge(sizeof(Type));
         else
            this->adjustVoltage(sizeof(Type));

         this->assign<Type>(value);
      }

      template <class Type>
      void reassign(Type *type)
      {
         if (!this->hasEnergy())
            this->charge(sizeof(Type));
         else
            this->adjustVoltage(sizeof(Type));

         this->assign<Type>(type);
      }

      template <class Type>
      void reassign(Type *type, SIZE_T size)
      {
         if (!this->hasEnergy())
            this->charge(size);
         else
            this->adjustVoltage(size);

         this->assign<Type>(type, size);
      }

      template <class Type>
      Type *cast(void) const
      {
         this->throwIfNoEnergy();

         return reinterpret_cast<Type *>(*this->energy);
      }

      template <class Type>
      Type resolve(void) const
      {
         return *this->cast<Type>();
      }

      SIZE_T voltage(void) const
      {
         return this->volts;
      }

      bool hasEnergy(void) const
      {
         return this->energy != NULL && *this->energy != NULL && this->volts > 0;
      }
      
      bool isShallow(void) const
      {
         return this->shallow;
      }

      void throwIfShallow(void) const
      {
         if (this->shallow)
            throw ShallowCapacitorException(*this);
      }

      void throwIfHasEnergy(void) const
      {
         if (this->hasEnergy())
            throw ChargedCapacitorException(*this);
      }

      void throwIfNoEnergy(void) const
      {
         if (!this->hasEnergy())
            throw DischargedCapacitorException(*this);
      }
      
      void charge(SIZE_T size)
      {
         this->throwIfHasEnergy();

         this->volts = size;

         if (this->energy == NULL)
            this->energy = new LPVOID;
         
         *this->energy = Capacitor::Allocate(this->volts);
      }
      
      void discharge(void)
      {
         this->throwIfNoEnergy();

         Capacitor::Deallocate(*this->energy);
         *this->energy = NULL;

         delete this->energy;
         this->energy = NULL;
      }
      
      void adjustVoltage(SIZE_T size)
      {
         LPVOID newEnergy;
         
         this->throwIfNoEnergy();
         this->throwIfShallow();
         
         newEnergy = Capacitor::Allocate(size);

         if (min(size, this->volts) != 0)
            CopyMemory(newEnergy, *this->energy, min(size, this->size));

         Capacitor::Deallocate(*this->energy);
         this->energy = newEnergy;
         this->volts = size;
      }
   };

   template <class Type>
   class TypedCapacitor : public Capacitor
   {
      TypedCapacitor(Type value)
         : Capacitor()
      {
         this->assign(value);
      }

      TypedCapacitor(Type *type)
         : Capacitor()
      {
         this->assign(type);
      }

      TypedCapacitor(Type *type, SIZE_T size)
         : Capacitor()
      {
         this->assign(type, size);
      }

      void operator=(Type value)
      {
         this->assign(value);
      }

      void operator=(Type *type)
      {
         this->assign(type);
      }

      Type operator*(void)
      {
         return this->resolve();
      }

      void assign(Type value)
      {
         this->assign<Type>(value);
      }

      void assign(Type *type)
      {
         this->assign<Type>(type);
      }

      void assign(Type *type, SIZE_T size)
      {
         this->assign<Type>(type, size);
      }

      void reassign(Type value)
      {
         this->reassign<Type>(value);
      }

      void reassign(Type *type)
      {
         this->reassign<Type>(type);
      }

      void reassign(Type *type, SIZE_T size)
      {
         this->reassign<Type>(type, size);
      }

      Type *cast(void) const
      {
         return this->cast<Type>();
      }

      Type resolve(void) const
      {
         return this->resolve<Type>();
      }
   };

   class MotherBoard
   {
   protected:
      TypedCapacitor<SIZE_T> circuits;

   private:
      std::vector<Capacitor *> capacitors;
      bool shallow;
      
   public:
      MotherBoard(void);
      MotherBoard(MotherBoard &board);
      MotherBoard(const MotherBoard &board);
      ~MotherBoard(void);

      virtual void operator=(MotherBoard &board);
      virtual void operator=(const MotherBoard &board);

      std::vector<const Capacitor *> getCapacitors(void) const;
      SIZE_T getCapacitorCount(void) const;
      SIZE_T getCircuits(void) const;
      virtual void plug(void);
      virtual void unplug(void);
      bool isDead(void) const;
      bool isWaiting(void) const;
      bool isActive(void) const;
      bool isShallow(void) const;
      void throwIfActive(void) const;
      void throwIfShallow(void) const;

   protected:
      virtual void solder(void);
      virtual void shutdown(void);

   private:
      void solder(Capacitor *capacitor);
   };

   template<class BoardType>
   class CircuitBoard
   {
      static_assert(std::is_base_of<MotherBoard, BoardType>::value, "template class does not derive Neurology::MotherBoard");
      
   protected:
      BoardType board;

   public:
      CircuitBoard(void)
      {
      }

      CircuitBoard(CircuitBoard &resistor)
      {
         *this = resistor;
      }

      CircuitBoard(const CircuitBoard &resistor)
      {
         *this = resistor;
      }

      virtual void operator=(CircuitBoard &resistor)
      {
         this->circuit = resistor.circuit;
      }

      virtual void operator=(const CircuitBoard &resistor)
      {
         this->circuit = (const)resistor.circuit;
      }

      DWORD resistors(void) const
      {
         return this->circuit.resistors();
      }
      
      void resistor(void)
      {
         this->circuit.resistor();
      }
      
      void unresistor(void)
      {
         this->circuit.unresistor();
      }
      
      bool isNull(void) const
      {
         return this->circuit.isNull();
      }
      
      bool isShallow(void) const
      {
         return this->circuit.isShallow();
      }
      
      void throwIfNull(void) const
      {
         this->circuit.throwIfNull();
      }
      
      void throwIfShallow(void) const
      {
         this->circuit.throwIfShallow();
      }
   };
}
