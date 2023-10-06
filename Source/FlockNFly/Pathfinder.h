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

	virtual ~Pathfinder();

	/** Method for flow field pathfinding, and aalled each tick from grid class */
	void UpdateNodesFlowField();

	/** Method for A*, calculating path */
	bool FindPath(FVector &Start, FVector &End);
	//void FindPath();

	TArray<FVector> WayPoints;

	TArray<FVector> OldWayPoints;

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

	/** Sets cost to max for each node */
	void ResetFlowFieldNodeCosts();
	
	void SetDirectionInUnWalkableNode(FlockingNode* NeighbourNode); 

	bool bIsDirectionInUnWalkableNodesSet = false;

	//friend class FlockingHeap;
	//FlockingHeap* OpenSet = nullptr;
	
};

