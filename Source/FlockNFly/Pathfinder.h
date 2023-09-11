// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pathfinder.generated.h"

USTRUCT()
struct FFlockingGridCell
{
	GENERATED_BODY()

	FFlockingGridCell()
	{
		Position = FVector::ZeroVector;
		bWalkable = false;
	}

	FVector Position;

	bool bWalkable;
	
};

USTRUCT()
struct FFlockingGrid
{
	GENERATED_BODY()

	FFlockingGrid()
	{
		Center = FVector::ZeroVector;
		TopLeft = FVector::ZeroVector;
		BottomRight = FVector::ZeroVector;
		CellSize = -1;
	}

	FVector Center;

	FVector TopLeft;

	FVector BottomRight;

	/** Array of cell structs, representing a grid*/
	TArray<FFlockingGridCell> FlockingGrid = TArray<FFlockingGridCell>();


	UPROPERTY(EditAnywhere)
	int CellSize;
	
};



class UBoxComponent;
UCLASS()
class FLOCKNFLY_API APathfinder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APathfinder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="Grid")
	UBoxComponent* GridBox;


private:

	UPROPERTY(EditAnywhere, Category="Grid", meta=(AllowPrivateAccess = true))
	FFlockingGrid FlockingGrid;

	/** X size of navigation area*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	int GridWorldSizeX = 3000;

	/** Y size of navigation area*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	int GridWorldSizeY= 3000;

	/** Z size of navigation area*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	int GridWorldSizeZ= 3000;

	/** Calculates grid depending on extents and node size*/
	void CreateGrid();
	 
};
