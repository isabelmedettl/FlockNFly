// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlockingNode.h"
#include "GameFramework/Actor.h"
#include "FlockingGrid.generated.h"

class AFlockNFlyCharacter;
class Pathfinder;

UCLASS()
class FLOCKNFLY_API AFlockingGrid : public AActor
{
	GENERATED_BODY()

public:

	TArray<FVector> RequestPath(FVector &Start, FVector &End);

	/** Timer handle to debug unwalkable nodes and grid, a lot of stuff in begin play can sometimes make nodes not being created fast enough*/
	FTimerHandle DebugDrawTimerHandle;

	/** Timer handel to start pathfinding, making sure grid and nodes are instantiated before calculations start*/
	FTimerHandle StartPathfindingTimerHandle;

	/** Method for switching bool to start pathfinding*/
	void OnStartPathfinding();

	/** bool to start pathfinding */
	bool bCanStartPathfinding = false;

	bool bAllNodesAdded = false;
	
	// Sets default values for this actor's properties
	AFlockingGrid();

	virtual ~AFlockingGrid() override;

	/** bool to start flow field pathfinding. If false, A* will be run instead. Set from brain depending on editor bool sets*/
	bool bUseFlowFillAlgorithm = false;

	bool bUseAStarAlgorithm = false;

	virtual void Tick(float DeltaSeconds) override;

	/** Returns the direction of node in specified location
	 * @param WorldLocation of the node to check
	 * @return Direction of node
	 */
	FVector GetDirectionBasedOnWorldLocation(const FVector WorldLocation) const { return GetNodeFromWorldLocation(WorldLocation)->GetDirection(); }
	
	/** Returns node on specified location
	 * @param NodeWorldLocation of the node
	 * @return pointer to Node in location
	 */
	FlockingNode* GetNodeFromWorldLocation(const FVector &NodeWorldLocation) const;

	/** Does what the method is called*/
	FVector GetGridBottomLeftLocation() const { return GridBottomLeftLocation; }

	/** Does what the method is called*/
	FVector GetGridSize() const { return GridSize; }

	FVector TargetLocation = FVector::ZeroVector;

	FVector StartLocation = FVector::ZeroVector;

	void OnDebugPathDraw(TArray<FVector> PathWaypoints);

	/** Gets the max size of grid */
	int GetGridMaxSize() const
	{
		return GridLengthX * GridLengthY * GridLengthZ;
	}

	void OnUpdatedPathFound();

	bool bHasUpdatedPath = false;

	void OnNoNeedUpdate();

	float GetNodeRadius() const;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	//TArray<FVector> PathWaypoints = TArray<FVector>();

	UPROPERTY(VisibleAnywhere, Category="A*")
	float MovementPenalty = 400.f;
	

	AFlockNFlyCharacter* PlayerCharacter;

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
	int GridLengthX = 0;
	/** Array size x*/
	int GridLengthY = 0;
	/** Array size x*/
	int GridLengthZ = 0;
	
	FVector GridBottomLeftLocation = FVector::ZeroVector; 

	/** Checks if a node is walkable (actor checks overlaps) */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> OverlapCheckActorClass;

	friend class Pathfinder;
	Pathfinder* FlockingPathfinder = nullptr;

	UPROPERTY(EditAnywhere)
	bool bDebug = true; 
	
	void OnDebugDraw() const;
	
	void CreateGrid();

	void AddToArray(const int IndexX, const int IndexY, const int IndexZ, const FlockingNode Node);

	FlockingNode* GetNodeFromGrid(const int IndexX, const int IndexY, const int IndexZ) const;

	TArray<FlockingNode*> GetNeighbours(FlockingNode* Node) const;

	int GetIndex(const int IndexX, const int IndexY, const int IndexZ) const; 

	
	

};
