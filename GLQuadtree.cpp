#include "GLQuadtree.h"
#include "GLapplication.h"
#include "GLPlayState.h"

namespace GAME
{
	QuadtreeNode::QuadtreeNode() : children(NULL) {}

	QuadtreeNode::~QuadtreeNode()
	{
		if (children)
		{
			delete[] children;
			children = 0;
		}
	}

	QuadtreeNode::QuadtreeNode(const QuadtreeNode& copy) : children(NULL)
	{
		children = copy.children;
		units = copy.units;
		locations = copy.locations;
	};

	void QuadtreeNode::addChildren(int nodeType)
	{
		if (nodeType != LEAF)
		{
			if(children)
			{
				delete[] children;
				children = 0;

			}
			children = new QuadtreeNode[4];
		}
	}

	Quadtree::Quadtree() : maxLength(DEFAULT_LENGTH), treeSize(0) {}

	Quadtree::~Quadtree() {}

	void Quadtree::Build(float length, float fromX, float fromZ, QuadtreeNode *current)
	{
		if(treeSize == 0)
		{
			current = &root;
			GAME::PlayState::getSingletonPtr()->getGame()->getQuadTreeInformation(&current->units, &current->locations, &current->firewalls);
			minLength = fromX;
			maxLength = fromX+length;
		}
		
		int capacity = current->units.size() + current->locations.size();

		// If capacity of parent is > 4, attempt to make a new branch.
		// If capacity of parent is <= 4, parent is a leaf.
		if ( capacity <= 4 )
		{
			current->addChildren(LEAF);// = new QuadtreeNode(LEAF);
			treeSize++;
		}
		else
		{
			treeSize++;

			float halfLength = length / 2;

			current->addChildren(BRANCH);// = new QuadtreeNode(BRANCH);			
			// ---------
			// | 1 | 3 |
			// ---------
			// | 0 | 2 |
			// ---------
			sortInformation(current, fromX, fromZ, halfLength);

			Build(halfLength, fromX, fromZ, &current->children[0]);
			Build(halfLength, fromX, fromZ+halfLength, &current->children[1]);
			Build(halfLength, fromX+halfLength, fromZ, &current->children[2]);
			Build(halfLength, fromX+halfLength, fromZ+halfLength, &current->children[3]);
		}
	}

	void Quadtree::Rebuild(float length, float fromX, float fromZ)
	{
		treeSize = 0;
		root.units.clear();
		root.locations.clear();
		root.firewalls.clear();
		//root.projectiles.clear();
		Build(length, fromX, fromZ);
	}

	// Max, Min are relative to unitNode. From, To are relavitve to QuadtreeNode.
	inline bool isBetween(float maxX, float maxZ, float minX, float minZ, float fromX, float fromZ, float toX, float toZ)
	{
		if( fromX <= minX && toX > maxX && fromZ <= maxZ && toZ > maxZ)
		{
			return true;
		}
		return false;
	}

	void Quadtree::sortInformation(QuadtreeNode *current, float fromX, float fromZ, float halflength)
	{
		float toX = fromX + halflength;
		float toZ = fromX + halflength;

		// Sorts Units
		int size = current->units.size();
		// Keeps track of where data current->data needs to be removed from. 
		// LOOK FOR BETTER SOLUTION.
		std::deque<int> locationsOfRemoval;
		for(int i = 0; i<size; i++)
		{
			int radius = current->units.at(i).radii;
			// maxSize, minSize are the maximum and minimum extend of a UnitNode
			//   with a certain radius. Each size is bounded such that it should
			//   not exceed the maximum dimensions of the game area.
			float maxSizeX = current->units.at(i).position.x + radius;
				if (maxSizeX > maxLength) maxSizeX = maxLength-1;
			float maxSizeZ = current->units.at(i).position.z + radius;
				if (maxSizeZ > maxLength) maxSizeZ = maxLength-1;
			float minSizeX = current->units.at(i).position.x - radius;
				if (minSizeX <= minLength) minSizeX = minLength;
			float minSizeZ = current->units.at(i).position.z - radius;
				if (minSizeZ <= minLength) minSizeZ = minLength;

			if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ, fromX, fromZ, toX, toZ) )
			{
				current->children[0].units.push_back(current->units.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX, fromZ+halflength, toX, toZ + halflength) )
			{
				current->children[1].units.push_back(current->units.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX+halflength, fromZ, toX+halflength, toZ) )
			{
				current->children[2].units.push_back(current->units.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if(isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX+halflength, fromZ+halflength, toX+halflength, toZ+halflength) )
			{
				current->children[3].units.push_back(current->units.at(i));
				locationsOfRemoval.push_back(i);
			}
		}
		for(int i=locationsOfRemoval.size()-1; i>=0; i--)
		{
			current->units.erase(current->units.begin() + locationsOfRemoval.at(i));
			//current->units.remove(locationsOfRemoval.at(i));
		}

		// Sorts Locations
		size = current->locations.size();
		// Keeps track of where data current->data needs to be removed from. 
		// LOOK FOR BETTER SOLUTION.
		locationsOfRemoval.clear();
		for(int i = 0; i<size; i++)
		{
			Ogre::Vector2 dimensions = current->locations.at(i).dimensions;
			// maxSize, minSize are the maximum and minimum extend of a UnitNode
			//   with a certain radius. Each size is bounded such that it should
			//   not exceed the maximum dimensions of the game area.
			float maxSizeX = current->locations.at(i).position.x + dimensions.x;
				if (maxSizeX > maxLength) maxSizeX = maxLength-1;
			float maxSizeZ = current->locations.at(i).position.z + dimensions.y;
				if (maxSizeZ > maxLength) maxSizeZ = maxLength-1;
			float minSizeX = current->locations.at(i).position.x - dimensions.x;
				if (minSizeX <= minLength) minSizeX = minLength;
			float minSizeZ = current->locations.at(i).position.z - dimensions.y;
				if (minSizeZ <= minLength) minSizeZ = minLength;

			if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ, fromX, fromZ, toX, toZ) )
			{
				current->children[0].locations.push_back(current->locations.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX, fromZ+halflength, toX, toZ + halflength) )
			{
				current->children[1].locations.push_back(current->locations.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX+halflength, fromZ, toX+halflength, toZ) )
			{
				current->children[2].locations.push_back(current->locations.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if(isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX+halflength, fromZ+halflength, toX+halflength, toZ+halflength) )
			{
				current->children[3].locations.push_back(current->locations.at(i));
				locationsOfRemoval.push_back(i);
			}
		}
		for(int i=locationsOfRemoval.size()-1; i>=0; i--)
		{
			current->locations.erase(current->locations.begin() + locationsOfRemoval.at(i));
			//current->units.remove(locationsOfRemoval.at(i)); 
		}

		// Sorts Firewalls
		size = current->firewalls.size();
		// Keeps track of where data current->data needs to be removed from. 
		// LOOK FOR BETTER SOLUTION.
		locationsOfRemoval.clear();
		for(int i = 0; i<size; i++)
		{
			Ogre::Vector2 dimensions = current->firewalls.at(i).dimensions;
			// maxSize, minSize are the maximum and minimum extend of a UnitNode
			//   with a certain radius. Each size is bounded such that it should
			//   not exceed the maximum dimensions of the game area.
			float maxSizeX = current->firewalls.at(i).position.x + dimensions.x;
				if (maxSizeX > maxLength) maxSizeX = maxLength-1;
			float maxSizeZ = current->firewalls.at(i).position.z + dimensions.y;
				if (maxSizeZ > maxLength) maxSizeZ = maxLength-1;
			float minSizeX = current->firewalls.at(i).position.x - dimensions.x;
				if (minSizeX <= minLength) minSizeX = minLength;
			float minSizeZ = current->firewalls.at(i).position.z - dimensions.y;
				if (minSizeZ <= minLength) minSizeZ = minLength;

			if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ, fromX, fromZ, toX, toZ) )
			{
				current->children[0].firewalls.push_back(current->firewalls.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX, fromZ+halflength, toX, toZ + halflength) )
			{
				current->children[1].firewalls.push_back(current->firewalls.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if( isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX+halflength, fromZ, toX+halflength, toZ) )
			{
				current->children[2].firewalls.push_back(current->firewalls.at(i));
				locationsOfRemoval.push_back(i);
			}
			else if(isBetween(maxSizeX, maxSizeZ, minSizeX, minSizeZ,
				fromX+halflength, fromZ+halflength, toX+halflength, toZ+halflength) )
			{
				current->children[3].firewalls.push_back(current->firewalls.at(i));
				locationsOfRemoval.push_back(i);
			}
		}
		for(int i=locationsOfRemoval.size()-1; i>=0; i--)
		{
			current->firewalls.erase(current->firewalls.begin() + locationsOfRemoval.at(i));
			//current->units.remove(locationsOfRemoval.at(i)); 
		}
	}
}