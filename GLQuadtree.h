#ifndef __QU465AD_H__
#define __QU465AD_H__

#include <vector>
#include <OgreVector3.h>

#include "GLunit.h"
#include "GLlocation.h"
#include "GLFirewall.h"
#include "GLRangedUnit.h"

// If unit is in multiple nodes, it is placed in the parent of those nodes.
// Recommended max bin size = 4

namespace GAME
{

	/*#define LEAF   01
	#define BRANCH 10
	#define MIN_PER_QUADTREE 3
	#define MAX_PER_QUADTREE 6
	#define DEFAULT_LENGTH 1500*/

	#define LEAF   01
	#define BRANCH 10
	static const int MIN_PER_QUADTREE = 3;
	static const int MAX_PER_QUADTREE = 6;
	static const float DEFAULT_LENGTH = 1500.0f;

	struct UnitNodes{
		unsigned int radii;
		unsigned int controller;
		Ogre::Vector3 position;
		GAME::UnitController *unitController;

		UnitNodes(){};
		UnitNodes(const UnitNodes& copy)
		{
			radii = copy.radii;
			controller = copy.controller;
			position = copy.position;
			unitController = copy.unitController;
		};
		~UnitNodes() {};
	};

	struct LocationNodes{
		Ogre::Vector2 dimensions;
		Ogre::Vector3 position;

		bool isBase;
		GAME::Location *location;

		LocationNodes() : isBase(false) {};
		LocationNodes(const LocationNodes& copy) : isBase(false)
		{
			dimensions = copy.dimensions;
			position = copy.position;
			isBase = copy.isBase;
			location = copy.location;
		}
		~LocationNodes(){}
	};

	struct FirewallNodes{
		Ogre::Vector2 dimensions;
		Ogre::Vector3 position;
		GAME::Firewall *firewall;

		FirewallNodes() {};
		FirewallNodes(const FirewallNodes& copy)
		{
			dimensions = copy.dimensions;
			position = copy.position;
			firewall = copy.firewall;
		}
		~FirewallNodes(){}
	};

	class QuadtreeNode
	{
	public:
		QuadtreeNode *children;

		std::vector<UnitNodes> units;
		std::vector<LocationNodes> locations;
		std::vector<FirewallNodes> firewalls;
		//std::vector<ProjectileNodes> projectiles;

		void addChildren(int nodeType);

		QuadtreeNode();
		QuadtreeNode(int nodeType);
		QuadtreeNode(const QuadtreeNode& copy);
		~QuadtreeNode();
	};

	class Quadtree
	{
	public:
		float maxLength;
		float minLength;
		int treeSize;

		Quadtree();

		~Quadtree();

		void Build(float length, float fromX = 0, float fromZ = 0, QuadtreeNode *current = NULL);

		void Rebuild(float length, float fromX = 0, float fromZ = 0);

		// Places each Unitnode into 1 of the parent's children.
		void sortInformation(QuadtreeNode *current, float fromX, float fromZ, float halflength);

		QuadtreeNode root;
	private:
		
	};
}
#endif