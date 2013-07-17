#ifndef __TimerManager_H__
#define __TimerManager_H__

// Written using the TimerManager code snipped found at
// http://www.ogre3d.org/tikiwiki/TimerManager

#include <Ogre.h>

//#include "GLapplication.h"

namespace GAME
{
	class _Empty {};

	template <typename callbackClass>
	class TimerManager : public Ogre::FrameListener
	{
	public:
		TimerManager(bool _UseFrameStarted = true ) : UseFrameStarted(_UseFrameStarted) {}

		typedef bool (callbackClass::*MemberTimerHandler)( int Timer_ID, int index );
		typedef bool (*TimerCallback)( int Timer_ID ); 

		bool AddTimer( unsigned long Period, TimerCallback callback, int index,
			int Timer_ID = -1, bool Repeat = false, bool Play_Catchup = true )
		{
			typename TimerMap::iterator i;
			for( i = timers.begin(); i != timers.end(); ++i)
			{
				if(i->second.Timer_ID == Timer_ID && i->second.tcb == callback)
				{
					TimerInstance currentTimer = i->second;
					timers.erase(i);
					if( Period > 0)
					{
						timers.insert(std::make_pair(mTimer.getMilliseconds() + Period, currentTimer));
					}
					return false;
				}
			}
			TimerInstance t;
			t.mtH = NULL;
			t.thC = NULL;
			t.tcb = callback;
			t.Period = Period;
			t.Timer_ID = Timer_ID;
			t.index = index;
			t.Repeat = Repeat;
			t.Play_Catchup = Play_Catchp;
			unsigned long now = mTimer.getMilliseconds();
			timers.insert( std::make_pair(now+Period, t) );
			return true;
		}
		bool AddTimer( unsigned long Period, MemberTimerHandler callback, callbackClass* callbackObject, 
			int index, int Timer_ID = -1, bool Repeat = false, bool Play_Catchup = true )
		{
			typename TimerMap::iterator i;
			for( i = timers.begin(); i != timers.end(); ++i)
			{
				if(i->second.Timer_ID == Timer_ID && i->second.mtH == callback)
				{
					TimerInstance currentTimer = i->second;
					timers.erase(i);
					if( Period > 0)
					{
						timers.insert(std::make_pair(mTimer.getMilliseconds() + Period, currentTimer));
					}
					return false;
				}
			}

			TimerInstance t;
			t.mtH = callback;
			t.thC = callbackObject;
			t.tcb = NULL;
			t.Period = Period;
			t.Timer_ID = Timer_ID;
			t.index = index;
			t.Repeat = Repeat;
			t.Play_Catchup = Play_Catchup;
			unsigned long now = mTimer.getMilliseconds();
			timers.insert( std::make_pair(now+Period, t) );
			return true;
		}

		void StopTimer( int Timer_ID)
		{
			typename TimerMap::iterator i;
			for( i = timers.begin(); i != timers.end();)
			{
				if(i->second.Timer_ID == Timer_ID)
				{
					i = timers.erase(i);
					//i = timers.begin();
				}
				else
				{
					++i;
				}
			}
		}

		// Decrement all Timer_IDs that are greater than ID by 1.
		//  Used when killing units.
		void DecrementTimers(int ID)
		{
			if(ID >= 0)
			{
				typename TimerMap::iterator i;
				for( i = timers.begin(); i != timers.end(); ++i)
				{
					if(i->second.Timer_ID > ID)
					{
						i->second.index -= 1;
						i->second.Timer_ID -= 1;
					}
				}
			}
		}

		/*void AddToTimerPeriod( int Timer_ID, float duration )
		{
			typename TimerMap::iterator i;
			for( i = timers.begin(); i != timers.end(); ++i)
			{
				if(i->second.Timer_ID == Timer_ID)
				{
					i->second.Period += duration;
				}
			}
		}*/

	protected:

		Ogre::Timer mTimer;

		class TimerInstance
		{
		public:
			MemberTimerHandler mtH;
			callbackClass *thC;
			TimerCallback tcb;
			unsigned long Period;
			int Timer_ID;
			int index;
			bool Repeat;
			bool Play_Catchup;
			unsigned long Trigger(unsigned long behind)
			{
				bool callback_return = false;
				long temp_behind = (long)behind;
				while( temp_behind >= 0)
				{
					temp_behind -= Period;
					callback_return = Callback();
					if(!Repeat)
						return 0;
					if(!callback_return)
						return 0;
					if(Period == 0)
						return 0;
					if(!Play_Catchup)
						continue;
				}
				return (Period - (behind % Period));
			}
			bool Callback()
			{
				if( (mtH != NULL) && (thC != NULL) )
				{
					return (thC->*mtH)(Timer_ID, index);
				}
				else if( tcb != NULL)
				{
					return tcb(Timer_ID);
				}
				return false;
			}
		};

		virtual bool frameStarted(const Ogre::FrameEvent &e)
		{
			if(!UseFrameStarted)
				return true;
			while(Next());
			return true;
		}
		virtual bool frameEnded(const Ogre::FrameEvent &e)
		{
			if(UseFrameStarted)
				return true;
			while(Next());
			return true;
		}

		bool Next(void)
		{
			if(timers.empty())
				return false;
			unsigned long now = mTimer.getMilliseconds();
			unsigned long then = timers.begin()->first;

			if( now>=then )
			{
				TimerInstance currentTimer = timers.begin()->second;
				timers.erase(timers.begin());
				unsigned long next_trigger = currentTimer.Trigger(now - then);
				if( next_trigger > 0)
				{
					timers.insert(std::make_pair(now + next_trigger, currentTimer));
				}
				return true;
			}
			return false;
		}

		typedef std::multimap< int, TimerInstance > TimerMap;
		TimerMap timers;

		bool UseFrameStarted;
	};
}
#endif