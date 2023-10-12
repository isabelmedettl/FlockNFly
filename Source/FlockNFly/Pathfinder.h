#pragma once

#include "CoreMinimal.h"

class FlockingHeap;
class AFlockNFlyCharacter;
class AFlockingGrid;
class FlockingNode;
/**
 * 
 */

class FLOCKNFLY_API Pathfinder
{
public:

	Pathfinder(APawn* PlayerActor, AFlockingGrid* MapGrid);
	
	/** Method for flow field pathfinding, and called each tick from grid class if grid is using flow field pathfinding.
	 * Updates flow field for navigation by calculating costs and directions for nodes based on the player's target location.
	 */
	void UpdateNodesFlowField();

	/** Method for A*, calculating path from a start location to target location
	 * Attempts to find a path from a given start location to an end location. Uses A* algorithm, and if a path is found, it stores the path's waypoints and returns true.
	 */
	bool FindPath(const FVector &Start, const FVector &End);

	/** Latest path calculated by FindPath*/
	TArray<FVector> WayPoints;

	/** Stores the latest viable path, if no path is found entity can follow the last known one until a path can be found*/
	TArray<FVector> OldWayPoints;

	/** Indicating if path has been found*/
	bool bHasPath = false;
	
	bool bIsOldPathStillValid = false;

	FVector TargetLocation = FVector::ZeroVector;
private:
	
	APawn* PlayerPawn;

	AFlockNFlyCharacter* PlayerCharacter;

	/** If target not moving, makes it to skip flowfield pathfinding */
	FlockingNode* OldTargetNode = nullptr;

	/** If entity not moving, makes it to skip a* pathfinding */
	FlockingNode* OldStartNode = nullptr;

	/** If target not moving, makes it to skip a* pathfinding */
	FlockingNode* OldEndNode = nullptr;

	/** Pointer to grid class*/
	AFlockingGrid* Grid;

	/** Resets flow field cost for all nodes in the grid to a maximum value (used for recalculating the flow field) */
	void ResetFlowFieldNodeCosts() const;

	/** Sets the direction for a node within a group of unwalkable nodes based on the lowest-cost path to a walkable node*/
	void SetDirectionInUnWalkableNode(FlockingNode* NeighbourNode) const; 

	bool bIsDirectionInUnWalkableNodesSet = false;
};

