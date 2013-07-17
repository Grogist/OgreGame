#include "GLPath.h"

namespace GAME
{
	static const int directions = 8; // Can move in 8 directions
	// SW, W, NW, N, NE, E, SE, E
	static const int dx[directions] = {-1,-1,-1, 0, 1, 1, 1, 0};
	static const int dz[directions] = {-1, 0, 1, 1, 1, 0,-1,-1};

	Path::Path(){};
	Path::~Path(){ grid.clear(); came_from_map.clear(); };

	void Path::constructGrid(std::deque<Ogre::SceneNode *> wallList, int MAP_WIDTH, int MAP_HEIGHT, int SIZE)
	{
		blockSize = SIZE;
		HALFblockSize = blockSize/2;
		WIDTH = MAP_WIDTH/SIZE;
		HEIGHT = MAP_HEIGHT/SIZE;
		HALFMAPWIDTH = MAP_WIDTH / 2;
		HALFMAPHEIGHT = MAP_HEIGHT / 2;

		// Grid is only set if the proper size on the first call to constructGrid.
		if(grid.size() == 0)
		{
			grid.resize(WIDTH);
			came_from_map.resize(WIDTH);
			for(int i = 0; i < WIDTH; ++i)
			{
				grid[i].resize(HEIGHT);
				came_from_map[i].resize(HEIGHT);
			}
		}
		pathVector invalid;
		invalid.x=-1;
		invalid.z=-1;
		// Set all areas of grid to be open.
		for(int i = 0; i < WIDTH; ++i)
		{
			std::fill(grid[i].begin(), grid[i].end(), 1);
			std::fill(came_from_map[i].begin(), came_from_map[i].end(), invalid);
		}

		Ogre::Vector3 position, scale, boundingBoxMinimum, boundingBoxMaximum, minimumPosition, maximumPosition;

		for(unsigned int wallCounter = 0; wallCounter < wallList.size(); wallCounter++)
		{
			position = wallList.at(wallCounter)->getPosition();
			scale = wallList.at(wallCounter)->getScale();
			boundingBoxMinimum = scale * wallList.at(wallCounter)->getAttachedObject(0)->getBoundingBox().getMinimum();
			boundingBoxMaximum = scale * wallList.at(wallCounter)->getAttachedObject(0)->getBoundingBox().getMaximum();
			minimumPosition = position + boundingBoxMinimum;
			maximumPosition = position + boundingBoxMaximum;

			// Move minimumPosition and maximumPosition to grid[0][0] from world[0][0] (Units are allowed in -direction
			//  in world coordinates, this is not allowed in the grid.
			minimumPosition.x += MAP_WIDTH/2;
			maximumPosition.x += MAP_WIDTH/2;
			minimumPosition.z += MAP_HEIGHT/2;
			maximumPosition.z += MAP_HEIGHT/2;

			// Convert minimumPosition and maximumPositio to size of the grid.
			minimumPosition.x /= SIZE;
			maximumPosition.x /= SIZE;
			minimumPosition.z /= SIZE;
			maximumPosition.z /= SIZE;

			// x is width of grid, z is height of grid
			for(int xPosition = (int)minimumPosition.x; xPosition <= (int)maximumPosition.x; xPosition++)
			{
				for(int zPosition = (int)minimumPosition.z; zPosition <= (int)maximumPosition.z; zPosition++)
				{
					if(WIDTH <= xPosition || xPosition <= 0 || HEIGHT <= zPosition || zPosition <= 0)
					{
						Ogre::LogManager::getSingleton().logMessage("ERROR! WALL POSITION OUT OF RANGE!");
						continue;
					}
					grid[xPosition][zPosition] = 9;
				} //zPosition
			} //xPosition
		} //wallList
	}

	bool Path::isValidLocation(pathVector location)
	{
		if((unsigned int)location.x >= grid.size() || location.x < 0 || 
			(unsigned int)location.z >= grid[1].size() || location.z < 0)
			return false;
		else
			return true;
	}
	
	std::deque<Ogre::Vector3> Path::constructPath(Ogre::Vector3 worldStart, Ogre::Vector3 worldGoal)
	{
		timer.reset();

		// Convert worldStart and worldGoal into grid position. Move each worldStart and worldGoal
		//  s.t. they are strictly positive.
		pathVector start;
		pathVector goal;
		start.x = (int)(worldStart.x + HALFMAPWIDTH) / blockSize;
		start.z = (int)(worldStart.z + HALFMAPHEIGHT) / blockSize;
		goal.x = (int)(worldGoal.x + HALFMAPWIDTH) / blockSize;
		goal.z = (int)(worldGoal.z + HALFMAPHEIGHT) / blockSize;
		
		// Check to see if start, and goal are in grid.
		if( !isValidLocation(start) || !isValidLocation(goal) )
		{
			Ogre::LogManager::getSingleton().logMessage("ERROR, INVALID PATH LOCATION");
			std::deque<Ogre::Vector3> empty;
			return empty;
		}
		
		// True at [x][z] means location is in the closedSet.
		// False at [x][z] means location is not in the closedSet.
		std::vector<std::vector<bool>> closedSet;
		std::deque<Node> openSet;
		// CAME_FROM

		closedSet.resize(WIDTH);		
		for(int i = 0; i < WIDTH; ++i)
		{
			closedSet[i].resize(HEIGHT);
			// Initially nothing is in the closedSet.
			std::fill(closedSet[i].begin(), closedSet[i].end(), false);
		}

		Node startNode;
		startNode.g_score = 0;
		startNode.h_score = heuristicCostEstimate(start, goal, start);
		startNode.f_score = startNode.g_score + startNode.h_score;
		startNode.position = start;
		openSet.push_back(startNode);

		// This is useful for finding an initial direction. came_from_map[start] is the only
		//  spot that directs to itself.
		came_from_map[start.x][start.z] = start;

		// while openSet is not Empty
		while(openSet.size())
		{
			// MAKE openSet UNSORTED, PICK SMALLEST FROM OPENSET, ON AVERAGE THIS WITH BE A FASTER
			// SOLUTION.
			Node current = openSet.front();

			if(current.position.x == goal.x && current.position.z == goal.z)
			{
				return reconstructPath(&current, start);
			}
			openSet.pop_front();
			closedSet[current.position.x][current.position.z] = true;
			
			// Invalid location, only possible if at the starting location
			int fromDirection = cameFromDirection(current.position, came_from_map[current.position.x][current.position.z]);
			std::deque<int> neighbourDirection = identifysuccesors(current.position, fromDirection, start, goal);

			// For each neighbourDirection of current
			while( neighbourDirection.size() )
			{
				int direction = neighbourDirection.back();
				neighbourDirection.pop_back();

				Node neighbour;

				neighbour.position = jump(current.position, direction, start, goal);

				if(neighbour.position.x == -1 && neighbour.position.z == -1) // INVALID
					continue;

				if(grid[neighbour.position.x][neighbour.position.z] == 9) // INACCESIBLE (SHOULD NOT NEED)
					continue;

				if(closedSet[neighbour.position.x][neighbour.position.z] == true)
					continue;

				if(!isValidLocation(neighbour.position)) // INVALID LOCATION ON GRID (SHOULD NOT NEED)
					continue;

				float tentative_g_score;
				// Neighbour is straight
				if(abs(dx[direction] + dz[direction]) == 1)
					tentative_g_score = current.g_score + 1.0f;
				// Neighbour is diagonal
				else
					tentative_g_score = current.g_score + 1.414f; // SQRT(2)

				// Checks for neighbour being in openSet
				bool isInOpen = false;
				unsigned int itr;
				for(itr = 0; itr < openSet.size(); itr++)
				{
					if( (openSet.at(itr).position.x == neighbour.position.x) &&
						(openSet.at(itr).position.z == neighbour.position.z) )
					{
						isInOpen = true;
						break;
					}
				}

				if(!isInOpen)
				{
					came_from_map[neighbour.position.x][neighbour.position.z] = current.position;
					neighbour.g_score = tentative_g_score;
					neighbour.h_score = heuristicCostEstimate(neighbour.position, goal, start);
					neighbour.f_score = neighbour.g_score + neighbour.h_score;
					openSet.push_front(neighbour);
					sort(&openSet);
				}
				else if(isInOpen && tentative_g_score < openSet.at(itr).g_score)
				{
					came_from_map[openSet.at(itr).position.x][openSet.at(itr).position.z] = current.position;
					openSet.at(itr).g_score = tentative_g_score;
					openSet.at(itr).f_score = openSet.at(itr).g_score + openSet.at(itr).h_score;
					sort(&openSet);
				}
			} // For Each Neighbour

		} // While openSet not Empty
		
		//FAILURE
		std::deque<Ogre::Vector3> failure;
			return failure;

	} // constructPath

	int Path::tieBreak(pathVector a, pathVector b)
	{
		return 0;
	}

	float Path::heuristicCostEstimate(pathVector a, pathVector goal, pathVector start)
	{
		// Diagonal distance
		// theory.standford.edu/~amitp/GameProgramming/Heuristics.html
		float D = 1.414f;
		int diagonalCost = std::min(abs(a.x-goal.x), abs(a.z - goal.z));
		int straightCost = (abs(a.x-goal.x) + abs(a.z - goal.z));

		// Create a preference for straight lines.
		// Lines that are diagonal to  the start/goal vector are given
		//  an increased cost.
		/*int dx1 = a.x - goal.x;
		int dz1 = a.z - goal.z;
		int dx2 = start.x - goal.x;
		int dz2 = start.z - goal.z;
		int cross = std::abs(dx1*dz2 - dx2*dz1);*/

		//return (D * diagonalCost + (straightCost - 2*diagonalCost)) + cross*0.001f;
		return (D * diagonalCost + (straightCost - 2*diagonalCost));
	}

	// Insertion Sort
	void Path::sort(std::deque<Node> *list)
	{
		int j;
		unsigned int listSize = list->size();

		for (unsigned int i = 1; i < listSize; i++)
		{
			j = i;
			while (j>0 && (list->at(j-1).f_score > list->at(j).f_score))
			{
				Node tmpNode = list->at(j);
				list->at(j) = list->at(j-1);
				list->at(j-1) = tmpNode;
				j--;
			}
		}
	}

	std::deque<Ogre::Vector3> Path::reconstructPath(Node *node, pathVector start)
	{
		std::deque<Ogre::Vector3> path;
		path.push_front(Ogre::Vector3((float)node->position.x*blockSize + HALFblockSize - HALFMAPWIDTH, 0, 
			(float)node->position.z*blockSize + HALFblockSize - HALFMAPHEIGHT));
		
		pathVector next = came_from_map[node->position.x][node->position.z];

		while(next.x!=start.x || next.z!=start.z)
		{
			path.push_front(Ogre::Vector3( (float)next.x*blockSize + HALFblockSize - HALFMAPWIDTH, 0, 
				(float)next.z*blockSize + HALFblockSize - HALFMAPHEIGHT ));
			next = came_from_map[next.x][next.z];
		}

		char a[21];
		sprintf(a, "PATH TIMER: %d\n", timer.getMilliseconds());
		Ogre::LogManager::getSingleton().logMessage(Ogre::String(a));

		return path;
	}

	int Path::cameFromDirection(pathVector to, pathVector from)
	{
		//static const int dx[directions] = {-1,-1,-1, 0, 1, 1, 1, 0};
		//static const int dz[directions] = {-1, 0, 1, 1, 1, 0,-1,-1};

		if(to.x == from.x) // x = 0
		{
			if(to.z == from.z) // z = 0
				return -1; // Will only happen at the start location.
			else if(to.z > from.z) // z = 1
				return 3;
			else
				return 7; // z = -1
		}
		else if(to.x > from.x) // x = 1
		{
			if(to.z == from.z)
				return 5;
			else if(to.z > from.z)
				return 4;
			else
				return 6;
		}
		else // x = -1
		{
			if(to.z == from.z)
				return 1;
			else if(to.z > from.z)
				return 2;
			else
				return 0;
		}

	}

	inline bool isDiagonal(int direction)
	{
		if(direction%2 == 0)
			return true;
		return false;
	};

	std::deque<int> Path::naturalNeighbours(int direction)
	{
		std::deque<int> prunedNeighbours;
		prunedNeighbours.push_back(direction);

		if( isDiagonal(direction) )
		{
			prunedNeighbours.push_back( (direction+1)%directions );

			//prunedNeighbours.push_back( (direction-1)%directions );
			prunedNeighbours.push_back( (direction+7)%directions );
		}
		return prunedNeighbours;
	}

	std::deque<int> Path::forcedNeighbours(pathVector current, int direction)
	{
		Ogre::Timer forced;
		forced.reset();
		std::deque<int> prunedNeighbours;
		pathVector tmp;
		if( isDiagonal(direction) )
		{
			tmp.x = current.x + dx[(direction+3)%directions];
			tmp.z = current.z + dz[(direction+3)%directions];
			if( isValidLocation(tmp) && grid[tmp.x][tmp.z] == 9 )
			{
				prunedNeighbours.push_back( (direction+2)%directions );
			}
			//tmp.x = current.x + dx[(direction-3)%directions];
			//tmp.z = current.z + dz[(direction-3)%directions];
			tmp.x = current.x + dx[(direction+5)%directions];
			tmp.z = current.z + dz[(direction+5)%directions];
			if( isValidLocation(tmp) && grid[tmp.x][tmp.z] == 9 )
			{
				//prunedNeighbours.push_back( (direction-2)%directions );
				prunedNeighbours.push_back( (direction+6)%directions );
			}
		}
		else
		{
			tmp.x = current.x + dx[(direction+2)%directions];
			tmp.z = current.z + dz[(direction+2)%directions];
			if( isValidLocation(tmp) && grid[tmp.x][tmp.z] == 9 )
			{
				prunedNeighbours.push_back( (direction+1)%directions );
			}
			//tmp.x = current.x + dx[(direction-2)%directions];
			//tmp.z = current.x + dz[(direction-2)%directions];
			tmp.x = current.x + dx[(direction+6)%directions];
			tmp.z = current.z + dz[(direction+6)%directions];
			if( isValidLocation(tmp) && grid[tmp.x][tmp.z] == 9 )
			{
				//prunedNeighbours.push_back( (direction-1)%directions );
				prunedNeighbours.push_back( (direction+7)%directions );
			}
		}

		if(forced.getMicroseconds()>500)
		{char a[23];
		sprintf(a, "FORCED:%d",forced.getMicroseconds());
		Ogre::LogManager::getSingleton().logMessage(a);}

		return prunedNeighbours;
	}

	std::deque<int> Path::identifysuccesors(pathVector current, int direction, pathVector start, pathVector goal)
	{
		std::deque<int> neighbours;
		unsigned int i;

		if(direction == -1) // At Start Location
		{
			for(i = 0; i<directions; i++)
			{	neighbours.push_back(i);	}
			 return neighbours;
		}

		std::deque<int> tmp;
		tmp = naturalNeighbours(direction);

		for(i = 0; i<tmp.size(); i++)
		{
			neighbours.push_back(tmp.at(i));
		}
		tmp = forcedNeighbours(current, direction);

		for(i = 0; i<tmp.size(); i++)
		{
			neighbours.push_back(tmp.at(i));
		}
		return neighbours;
	}

	pathVector Path::jump(pathVector current, int direction, pathVector start, pathVector goal)
	{
		Ogre::Timer jumpTimer;
		jumpTimer.reset();

		pathVector n;
		n.x = current.x + dx[direction];
		n.z = current.z + dz[direction];
		if(!isValidLocation(n) || grid[n.x][n.z] == 9)
		{
			// return an invalid position
			return pathVector(-1,-1);
		}
		if(n.x == goal.x && n.z == goal.z)
		{
			return n;
		}
		if( forcedNeighbours(n, direction).size() )
		{
			return n;
		}
		if(isDiagonal(direction))
		{
			pathVector next = jump(n, (direction+1)%directions, start, goal);
			// A valid jump point is found.
			if(next.x!=-1)
			{	
				return n;
			}
			if(jumpTimer.getMilliseconds() > 5)
//			return pathVector(-1,-1);
			{
				char a[21];
				sprintf(a,"JUMP TIME a:%d", jumpTimer.getMilliseconds());
				Ogre::LogManager::getSingleton().logMessage(a);
			}
			//next = jump(n, (direction-1)%directions, start, goal);
			next = jump(n, (direction+7)%directions, start, goal);
			if(next.x!=-1)
			{
				return n;
			}
			if(jumpTimer.getMilliseconds() > 5)
//			return pathVector(-1,-1);
			{
				char a[21];
				sprintf(a,"JUMP TIME b:%d", jumpTimer.getMilliseconds());
				Ogre::LogManager::getSingleton().logMessage(a);
			}
		}

		return jump(n, direction, start, goal);
	}
}