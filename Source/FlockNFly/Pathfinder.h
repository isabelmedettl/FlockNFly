#pragma once

#include "CoreMinimal.h"

class AFlockNFlyCharacter;
class AFlockingGrid;
/**
 * 
 */

class FLOCKNFLY_API Pathfinder
{
public:

	Pathfinder(APawn* PlayerActor, AFlockingGrid* MapGrid);

	

	/** Method for flow field pathfinding, and aalled each tick from grid class */
	void UpdateNodesFlowField();

private:

	// ============ Flow field pathfinding ============= //
	APawn* PlayerPawn;

	AFlockNFlyCharacter* PlayerCharacter;

	FVector TargetLocation = FVector::ZeroVector;

	/** If target not moving, makes it to skip flowfield pathfinding */
	class FlockingNode* OldTargetNode = nullptr; 

	/** Pointer to grid class*/
	AFlockingGrid* Grid;

	/** Sets cost to max for each node */
	void ResetNodeCosts();
	
	void SetDirectionInUnWalkableNode(FlockingNode* NeighbourNode); 

	bool bIsDirectionInUnWalkableNodesSet = false; 
	
};
