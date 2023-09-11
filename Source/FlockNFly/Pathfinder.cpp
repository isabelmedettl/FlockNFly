// Fill out your copyright notice in the Description page of Project Settings.


#include "Pathfinder.h"

#include "Components/BoxComponent.h"

// Sets default values
APathfinder::APathfinder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridBox = CreateDefaultSubobject<UBoxComponent>("GridBox");
	GridBox->SetupAttachment(RootComponent);

	GridBox->SetBoxExtent(FVector(GridWorldSizeX,GridWorldSizeY,GridWorldSizeZ));

}

// Called when the game starts or when spawned
void APathfinder::BeginPlay()
{
	Super::BeginPlay();

	CreateGrid();
}



// Called every frame
void APathfinder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APathfinder::CreateGrid()
{
	// Calculate the starting position based on the grid size and cell spacing
	//FVector StartPosition = FVector(GetActorLocation().X - GridSizeX * FlockingGrid.CellSize / 2, GetActorLocation().Y - GridSizeY * FlockingGrid.CellSize  / 2, GetActorLocation().Z -  GridSizeZ * FlockingGrid.CellSize  / 2);

	
	int GridSizeX = GridWorldSizeX / FlockingGrid.CellSize;
	int GridSizeY = GridWorldSizeY/ FlockingGrid.CellSize;
	int GridSizeZ = GridWorldSizeZ/ FlockingGrid.CellSize;

	FVector StartPosition = GetActorLocation() - GetActorLocation().ForwardVector * GridSizeX / 2 - GetActorLocation().RightVector * GridSizeY / 2 - GetActorLocation().UpVector * GridSizeZ / 2;
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("SIZE: %i"), FlockingGrid.CellSize));

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("XYZ: %i, %i, %i"), GridSizeX, GridSizeY, GridSizeZ));

	DrawDebugSphere(GetWorld(), StartPosition, 20.f, 12, FColor::Green, true, 3, 0, 1);

	
	// Create the grid of cell structs
	FVector CellPosition = FVector::ZeroVector;
	for (int X = 0; X < GridSizeX; X++)
	{
		for (int Y = 0; Y < GridSizeY; Y++)
		{
			for (int Z = 0; Z < GridSizeZ; Z++)
			{
				FFlockingGridCell Cell;
				CellPosition = StartPosition + FVector((X * FlockingGrid.CellSize ) + (Y *  FlockingGrid.CellSize ) +  (Z *  FlockingGrid.CellSize ));
				Cell.Position = CellPosition;

				
				UE_LOG(LogTemp, Warning, TEXT("Cell pos %f, %f, %f"), CellPosition.X, CellPosition.Y, CellPosition.Z);
				FlockingGrid.FlockingGrid.Add(Cell);
				

				
				//DrawDebugSphere(GetWorld(), CellPosition , 20.f, 12, FColor::Green, true, 3, 0, 1);

				// Create a cell struct and store its information
				//FFlockingGridCell Cell;



				
			}
		}
	}
	
}






