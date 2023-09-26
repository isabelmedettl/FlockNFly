#pragma once

#include "CoreMinimal.h"

class FLOCKNFLY_API FlockingNode
{
public:
	/** Constructor with parameters*/
	FlockingNode(const bool bIsWalkable, const FVector WorldCoord, const int GridX, const int GridY, const int GridZ) : GridX(GridX), GridY(GridY), GridZ(GridZ), bWalkable(bIsWalkable), WorldCoordinate(WorldCoord)
	{}
	/** Empty constructor*/
	FlockingNode() {}

	/** Sets nodes direction based on Targetnode
	 * @param TargetNode node to point towards
	 */
	void SetDirection(const FlockingNode* TargetNode)
	{
		Direction = TargetNode->WorldCoordinate - WorldCoordinate;
		Direction.Normalize();
	}

	/** Gets nodes current direction, aka the direction of where entity should move toward when at current node
	 * @return FVector Direction of node
	 */
	FVector GetDirection() const { return Direction; }

	
	bool IsWalkable() const { return bWalkable; }

	FVector GetWorldCoordinate() const { return WorldCoordinate; }

	int GetCost() const { return Cost; }

	void SetCost(const int NewCost) { Cost = NewCost; }

	int GridX = -1;
	int GridY = -1;
	int GridZ = -1; 
	
private:
	/** Nodes current direction, aka the direction of where entity should move toward when at current node*/
	FVector Direction = FVector::Zero();

	/** If the node is walkable */
	bool bWalkable = false;

	/** Nodes location in world space*/
	FVector WorldCoordinate;

	/** Cost to get to this node */
	int Cost = INT_MAX;
	
};
