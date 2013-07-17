#ifndef __SlowUnit_H__
#define __SlowUnit_H__

#include "GLunit.h"

namespace GAME
{
	class SlowUnitController : public UnitController
	{
	public:
		SlowUnitController();
		~SlowUnitController();

		void initialize();

		void update(Ogre::Real time);

		void buff(float amount);
		void unbuff(void);
		void slow(float amount);
		void unslow(void);
		
		UnitType getUnitType(void);

	private:

		bool HandleSleepState(Ogre::Real time);
		bool HandleCombatState(Ogre::Real time);
		bool HandleChaseState(Ogre::Real time);
		bool HandleFleeState(Ogre::Real time);
		bool HandlePathState(Ogre::Real time);
	};
}
#endif