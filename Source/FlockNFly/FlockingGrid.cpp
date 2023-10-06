/// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingGrid.h"

#include "FlockNFlyCharacter.h"
#include "Pathfinder.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/Reverse.h"


// Sets default values
AFlockingGrid::AFlockingGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

AFlockingGrid::~AFlockingGrid()
{
	delete [] Nodes;
	delete FlockingPathfinder;
}

// Called when the game starts or when spawned
void AFlockingGrid::BeginPlay()
{
	Super::BeginPlay();

	//SetActorTickEnabled(false);

	NodeDiameter = NodeRadius * 2;
	
	CreateGrid();

	FlockingPathfinder = new Pathfinder(UGameplayStatics::GetPlayerPawn(this, 0), this);

	PlayerCharacter = Cast<AFlockNFlyCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));

	if(bDebug)
	{
		GetWorldTimerManager().SetTimer(DebugDrawTimerHandle, this, &AFlockingGrid::OnDebugDraw, 0.1f, false, -1);
	}
	ensure (PlayerCharacter != nullptr);

	GetWorldTimerManager().SetTimer(StartPathfindingTimerHandle, this, &AFlockingGrid::OnStartPathfinding, 0.1f, false, 0.1f);

}

TArray<FVector> AFlockingGrid::RequestPath( FVector &Start,  FVector &End)
{
	TArray<FVector> NewPath;
	if (FlockingPathfinder->FindPath(Start, End))
	{
		NewPath = FlockingPathfinder->WayPoints;
	}
	else
	{
		NewPath = FlockingPathfinder->OldWayPoints;
	}
	
	Algo::Reverse(NewPath);
	if (bDebug) OnDebugPathDraw(NewPath);
	return NewPath;
	/*
	NewPath = FlockingPathfinder->FindPath(Start, End);
	if (NewPath.Num() != 0)
	{
		Algo::Reverse(NewPath);
		if (bDebug)
		{
			OnDebugPathDraw(NewPath);
		}
	}
		
	return NewPath;
	*/
}

void AFlockingGrid::OnStartPathfinding()
{
	bCanStartPathfinding = true;
}


// Called every frame
void AFlockingGrid::Tick(float DeltaTime)
{
	if (!bCanStartPathfinding) return;
	if (!bAllNodesAdded) return;
	
	if (bUseFlowFillAlgorithm && !bUseAStarAlgorithm)
	{
		FlockingPathfinder->UpdateNodesFlowField();
	}

	/*
	else if(!bUseFlowFillAlgorithm && bUseAStarAlgorithm)
	{
		FlockingPathfinder->FindPath();
	}
	
	
	if (bDebug)
	{
		if (PathWaypoints.Num() > 0)
		{
			OnDebugPathDraw();
		}
	}
	*/
}


void AFlockingGrid::OnUpdatedPathFound()
{
	if (FlockingPathfinder->bHasPath)
	{
		if (!FlockingPathfinder->bIsOldPathStillValid)
		{
			UE_LOG(LogTemp, Warning, TEXT("Updated path in grid"));
			//PathWaypoints = FlockingPathfinder->WayPoints;
			//PathWaypoints = ListOfPoints;
			//Algo::Reverse(PathWaypoints);
			bHasUpdatedPath = true;
		}
	}
}


void AFlockingGrid::OnNoNeedUpdate()
{
	bHasUpdatedPath = false;
}

float AFlockingGrid::GetNodeRadius() const 
{
	return NodeRadius;
}


bool AFlockingGrid::PathSuccessful(TArray<FVector> PathWaypoints) const
{
	return PathWaypoints.Num() > 0;
}


FVector AFlockingGrid::GetWaypointAtIndex(int Index, TArray<FVector> PathWaypoints) const
{
	if (Index >= 0 && Index < PathWaypoints.Num())
	{
		return PathWaypoints[Index];
	}
	// Return an invalid vector if the index is out of range.
	UE_LOG(LogTemp, Warning, TEXT("Invalid index"));
	return FVector::ZeroVector;
}

bool AFlockingGrid::IsWaypointAtIndexValid(int Index, TArray<FVector> PathWaypoints) const
{
	return Index >= 0 && Index < PathWaypoints.Num();
}


int AFlockingGrid::GetPathLength(TArray<FVector> PathWaypoints) const
{
	return PathWaypoints.Num();
}

void AFlockingGrid::OnDebugPathDraw(TArray<FVector> PathWaypoints)
{
	
	if (PathWaypoints.Num() != 0)
	{
		for(FVector Loc  : PathWaypoints)
		{
			auto Node = GetNodeFromWorldLocation(Loc); 
			ensure(Node != nullptr);
			DrawDebugSphere(GetWorld(), Node->GetWorldCoordinate(), NodeRadius, 10, FColor::Black, false, 0.1);
		}
	}

	auto Node = GetNodeFromWorldLocation(TargetLocation); 
	DrawDebugSphere(GetWorld(), Node->GetWorldCoordinate(), NodeDiameter, 10, FColor::Cyan, false, 0.1);

	auto StartNode = GetNodeFromWorldLocation(StartLocation); 
	DrawDebugSphere(GetWorld(), StartNode->GetWorldCoordinate(), NodeDiameter, 10, FColor::Purple, false, 0.1);

}



void AFlockingGrid::CreateGrid()
{
	GridLengthX = FMath::RoundToInt(GridSize.X / NodeDiameter);
	GridLengthY = FMath::RoundToInt(GridSize.Y / NodeDiameter);
	GridLengthZ = FMath::RoundToInt(GridSize.Z / NodeDiameter);


	Nodes = new FlockingNode[GridLengthX * GridLengthY * GridLengthZ]; 

	// The grid's pivot is in the center, need its position as if pivot was in the bottom left corner 
	FVector GridBottomLeft = GetActorLocation();
	GridBottomLeft.X -= GridSize.X / 2;
	GridBottomLeft.Y -= GridSize.Y / 2;
	GridBottomLeft.Z -= GridSize.Z / 2;

	GridBottomLeftLocation = GridBottomLeft; 

	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OverlappingActors;

	AActor* OverlapActor = GetWorld()->SpawnActor<AActor>(OverlapCheckActorClass, GetActorLocation(),FRotator::ZeroRotator); 
	
	for(int x = 0; x < GridLengthX; x++)
	{
		for(int y = 0; y < GridLengthY; y++)
		{
			for(int z = 0; z < GridLengthZ; z++)
			{
				FVector NodePos = GridBottomLeft;
				NodePos.X += x * NodeDiameter + NodeRadius; 
				NodePos.Y += y * NodeDiameter + NodeRadius;
				NodePos.Z += z * NodeDiameter + NodeRadius;

				OverlapActor->SetActorLocation(NodePos);
				OverlapActor->GetOverlappingActors(OverlappingActors);

				int NewMovementPenalty = OverlappingActors.IsEmpty() ? 0 : MovementPenalty;
				AddToArray(x, y, z, FlockingNode(OverlappingActors.IsEmpty(), NodePos,  x, y, z, NewMovementPenalty));
			}
		}
	}
	OverlapActor->Destroy();
	bAllNodesAdded = true;
}

void AFlockingGrid::AddToArray(const int IndexX, const int IndexY, const int IndexZ, const FlockingNode Node)
{
	Nodes[GetIndex(IndexX, IndexY, IndexZ)] = Node;
}

FlockingNode* AFlockingGrid::GetNodeFromGrid(const int IndexX, const int IndexY, const int IndexZ) const
{
	return &Nodes[GetIndex(IndexX, IndexY, IndexZ)];
}

TArray<FlockingNode*> AFlockingGrid::GetNeighbours(FlockingNode* Node) const
{
	TArray<FlockingNode*> Neighbours;

	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			for(int z = -1; z <= 1; z++)
			{
				if(x == 0 && y == 0 && z == 0) 
					continue;

				const int GridX = Node->GridX + x; 
				const int GridY = Node->GridY + y;
				const int GridZ = Node->GridZ + z;


				// if any index is out of bounds 
				if(GridX < 0 || GridX > GridLengthX - 1 || GridY < 0 || GridY > GridLengthY - 1 || GridZ < 0 || GridZ > GridLengthZ - 1) continue;

				Neighbours.Add(GetNodeFromGrid(GridX, GridY, GridZ)); 
			}
		}
	}

	return Neighbours; 
}

int AFlockingGrid::GetIndex(const int IndexX, const int IndexY, const int IndexZ) const
{
	return (IndexX * (GridLengthY * GridLengthZ)) + (IndexY * GridLengthZ) + IndexZ;
}


FlockingNode* AFlockingGrid::GetNodeFromWorldLocation(FVector NodeWorldLocation) 
{
	// position relative to grids bottom left corner 
	const float GridRelativeX = NodeWorldLocation.X  - GridBottomLeftLocation.X; 
	const float GridRelativeY = NodeWorldLocation.Y - GridBottomLeftLocation.Y;
	const float GridRelativeZ = NodeWorldLocation.Z - GridBottomLeftLocation.Z;


	// checks how many nodes fit in relative position for array indexes 
	const int x = FMath::Clamp(FMath::RoundToInt((GridRelativeX - NodeRadius) / NodeDiameter), 0, GridLengthX - 1);
	const int y = FMath::Clamp(FMath::RoundToInt((GridRelativeY - NodeRadius) / NodeDiameter), 0, GridLengthY - 1);
	const int z = FMath::Clamp(FMath::RoundToInt((GridRelativeZ - NodeRadius) / NodeDiameter), 0, GridLengthZ- 1);


	return GetNodeFromGrid(x, y, z);
}

void AFlockingGrid::OnDebugDraw()
{

	// Draw border of grid 
	DrawDebugBox(GetWorld(), GetActorLocation(), FVector(GridSize.X / 2, GridSize.Y / 2, GridSize.Z / 2), FColor::Red, true);

	// draw each node where unwalkable nodes are red and walkable green
	int ActualArrayCount = 0; 
	for(int x = 0; x < GridLengthX; x++)
	{
		for(int y = 0; y < GridLengthY; y++)
		{
			for(int z = 0; z < GridLengthZ; z++)
			{
				FlockingNode* Node = GetNodeFromGrid(x, y, z);
				FColor Color = Node->IsWalkable() ? FColor::Green : FColor::Red;
				if (!Node->IsWalkable())
				{
					//UE_LOG(LogTemp, Warning, TEXT("Costs %i"), Node->MovementPenalty)
					DrawDebugBox(GetWorld(), Node->GetWorldCoordinate(), FVector(NodeRadius, NodeRadius, NodeRadius), Color, true);
				}

				
			}
		}
	}
	

	

	/*

	if (bUseAStarAlgorithm)
	{
		UE_LOG(LogTemp, Warning, TEXT("true"));

		ensure(FlockingPathfinder != nullptr);
		//if (!FlockingPathfinder->bHasPath) return;
		
			
		DrawDebugSphere(GetWorld(), FlockingPathfinder->TargetLocation, NodeRadius, 100, FColor::Red, true);
		DrawDebugSphere(GetWorld(), StartLocation, NodeRadius, 100, FColor::Red, true);

		UE_LOG(LogTemp, Warning, TEXT("Loc %f, %f, %f"),FlockingPathfinder->TargetLocation.X, FlockingPathfinder->TargetLocation.Y, FlockingPathfinder->TargetLocation.Z);

		
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("false"));

	}
	*/
	
}
