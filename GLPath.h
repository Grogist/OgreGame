#ifndef __MyPATH_H__
#define __MyPATH_H__

#include <deque>
#include <vector>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>

#include <OgreTimer.h>

namespace GAME
{
	// This is bad.
	struct pathVector
	{
		int x, z;
		pathVector() {};
		pathVector(int x, int z)
		{ this->x = x; this->z = z; };
		~pathVector() {};
	};
	
	struct Node
	{
		float f_score;
		float h_score;
		float g_score;

		pathVector position;

		Node(): f_score(0), h_score(0), g_score(0) {};
		~Node() {};
	};

	// Assuming that the world is always evenly spaced around 0. ie. world ranges from -x to x, and -z to z.
	class Path
	{
	public:
		Path();
		~Path();
		// wallList is the list of walls, MAP_WIDTH is WorldSizeX, MAP_HEIGHT is the WorldSizeZ,
		//  SIZE is the size of each square in the grid (ie. 10x10).
		void constructGrid(std::deque<Ogre::SceneNode *> wallList, int MAP_WIDTH, int MAP_HEIGHT, int SIZE=50);
		// Returns a path from start to goal.
		std::deque<Ogre::Vector3> constructPath(Ogre::Vector3 start, Ogre::Vector3 goal);

		// ARE MAKE PUBLIC TO COPE WITH COMMANDER MEMORY LEAK.
		// All open regions are represented as 1.
		// All closed regions are represented as 9.
		std::vector<std::vector<int>> grid;

		std::vector<std::vector<pathVector>> came_from_map;
		
	private:
		// Valid Location = true, Invalid Location = false.
		bool isValidLocation(pathVector location);
		int tieBreak(pathVector a, pathVector b);
		float heuristicCostEstimate(pathVector a, pathVector goal, pathVector start);
		std::deque<Ogre::Vector3> reconstructPath(Node *node, pathVector start);
		int cameFromDirection(pathVector to, pathVector from);
		// Insertion Sort. (Each list will already be mostly sorted.) Sorts by f_score.
		void sort(std::deque<Node> *list);
		// Returns the reconstructed path
		std::vector<Ogre::Vector3> reconstructPath(void);

		// Jump Point Search Functions
		std::deque<int> naturalNeighbours(int direction);
		std::deque<int> forcedNeighbours(pathVector current, int direction);
		std::deque<int> identifysuccesors(pathVector current, int direction, pathVector start, pathVector goal);
		pathVector jump(pathVector current, int direction, pathVector start, pathVector goal);		

		int blockSize;
		int HALFblockSize;
		int WIDTH;
		int HEIGHT;
		int HALFMAPWIDTH;
		int HALFMAPHEIGHT;

		Ogre::Timer timer;
	};
}
#endif