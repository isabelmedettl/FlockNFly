/// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingGrid.h"

#include "Pathfinder.h"
#include "Kismet/GameplayStatics.h"

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

	NodeDiameter = NodeRadius * 2;
	
	CreateGrid();
	
	if(bDebug) OnDebugDraw();

	FlockingPathfinder = new Pathfinder(UGameplayStatics::GetPlayerPawn(this, 0), this);
}


// Called every frame
void AFlockingGrid::Tick(float DeltaTime)
{
	FDateTime StartTime = FDateTime::UtcNow();
	if (bUseFlowFillAlgorithm )
	{
		FlockingPathfinder->UpdateNodesFlowField();
	}
	
	const float TimeElapsedInMs = (FDateTime::UtcNow() - StartTime).GetTotalMilliseconds();
	UE_LOG(LogTemp, Display, TEXT("%f ms each tick to update map"), TimeElapsedInMs)
	
	// Debugging node where target location is
	if(bDebug)
	{
		auto PlayerNode = GetNodeFromWorldLocation(UGameplayStatics::GetPlayerPawn(this, 0)->GetActorLocation()); 
		DrawDebugSphere(GetWorld(), PlayerNode->GetWorldCoordinate(), NodeRadius, 10, FColor::Cyan);

		// and neighbours
		for(auto Node : GetNeighbours(PlayerNode))
		{
			if(Node->IsWalkable())// if it's walkable
			{
				DrawDebugSphere(GetWorld(), Node->GetWorldCoordinate(), NodeRadius, 10, FColor::Purple);
			} 
		}
	}
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

	AActor* OverlapActor = GetWorld()->SpawnActor<AActor>(OverlapCheckActorClass, GetActorLocation(),
														  FRotator::ZeroRotator); 
	
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

				AddToArray(x, y, z, FlockingNode(OverlappingActors.IsEmpty(), NodePos,  x, y, z));
			}
		}
	}
	OverlapActor->Destroy(); 
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

	// -1 to plus 1 in each direction to get every neighbour node 
	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			for(int z = -1; z <= 1; z++)
			{
				if(x == 0 && y == 0) // itself 
					continue;

				const int GridX = Node->GridX + x; 
				const int GridY = Node->GridY + y;
				const int GridZ = Node->GridZ + z;


				// if any index is out of bounds 
				if(GridX < 0 || GridX > GridLengthX - 1 || GridY < 0 || GridY > GridLengthY - 1 || GridZ < 0 || GridZ > GridLengthZ - 1)
					continue;

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


FlockingNode* AFlockingGrid::GetNodeFromWorldLocation(const FVector NodeWorldLocation) 
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
	DrawDebugBox(GetWorld(), GetActorLocation(), FVector(GridSize.X / 2, GridSize.Y / 2, 3), FColor::Red, true);

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
				DrawDebugBox(GetWorld(), Node->GetWorldCoordinate(), FVector(NodeRadius, NodeRadius, NodeRadius), Color, true);
			}
		}
	}

	// prints some stuff 
	UE_LOG(LogTemp, Warning, TEXT("diameter: %f"), NodeDiameter)
	UE_LOG(LogTemp, Warning, TEXT("Grid Length: (X: %i, Y: %i, Z: %i)"), GridLengthX, GridLengthY, GridLengthZ)
	UE_LOG(LogTemp, Warning, TEXT("GridSize: %s"), *GridSize.ToString())

	UE_LOG(LogTemp, Warning, TEXT("Number of nodes: %i"), GridLengthX * GridLengthY * GridLengthZ)

}
