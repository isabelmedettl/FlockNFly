// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlockingNode.h"
#include "GameFramework/Actor.h"
#include "FlockingGrid.generated.h"

class Pathfinder;

UCLASS()
class FLOCKNFLY_API AFlockingGrid : public AActor
{
	GENERATED_BODY()

public:
	
	// Sets default values for this actor's properties
	AFlockingGrid();

	virtual ~AFlockingGrid() override;

	/** bool to start flow field pathfinding. If false, A* will be run instead */
	UPROPERTY(EditAnywhere)
	bool bUseFlowFillAlgorithm = true;

	virtual void Tick(float DeltaSeconds) override;

	/** Returns the direction of node in specified location
	 * @param WorldLocation of the node to check
	 * @return Direction of node
	 */
	FVector GetDirectionBasedOnWorldLocation(const FVector WorldLocation){ return GetNodeFromWorldLocation(WorldLocation)->GetDirection(); }
	
	/** Returns node on specified location
	 * @param WorldLocation of the node
	 * @return pointer to Node in location
	 */
	FlockingNode* GetNodeFromWorldLocation(const FVector WorldLocation);

	/** Does what the method is called*/
	FVector GetGridBottomLeftLocation() const { return GridBottomLeftLocation; }

	/** Does what the method is called*/
	FVector GetGridSize() const { return GridSize; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	// ============ General grid variables and funcs ============= //
	
	FlockingNode* Nodes; 

	/** Radius for each node, can be altered to smaller size for more accurate pathfinding but more performance expensive */
	UPROPERTY(EditAnywhere)
	float NodeRadius = 50.f;
	
	float NodeDiameter;  

	/** The size of the grid */
	UPROPERTY(EditAnywhere)
	FVector GridSize = FVector(100, 100, 100);

	/** Array size x*/
	int GridLengthX;
	/** Array size x*/
	int GridLengthY;
	/** Array size x*/
	int GridLengthZ;
	
	FVector GridBottomLeftLocation; 

	/** Checks if a node is walkable (actor checks overlaps) */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> OverlapCheckActorClass;

	friend class Pathfinder;
	Pathfinder* FlockingPathfinder = nullptr;

	UPROPERTY(EditAnywhere)
	bool bDebug = true; 
	
	void OnDebugDraw();

	// ========== Flow field pathfinding ======= //
	void CreateGrid();

	void AddToArray(const int IndexX, const int IndexY, const int IndexZ, const FlockingNode Node);

	FlockingNode* GetNodeFromGrid(const int IndexX, const int IndexY, const int IndexZ) const;

	TArray<FlockingNode*> GetNeighbours(FlockingNode* Node) const;

	int GetIndex(const int IndexX, const int IndexY, const int IndexZ) const; 

	
	

};
