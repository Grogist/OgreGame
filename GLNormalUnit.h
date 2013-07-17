#ifndef __NormalUnit_H__
#define __NormalUnit_H__

#include "GLunit.h"

namespace GAME
{
	class NormalUnitController : public UnitController
	{
	public:
		NormalUnitController();
		~NormalUnitController();

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