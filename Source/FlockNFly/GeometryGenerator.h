// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryGenerator.generated.h"


class UBoxComponent;

USTRUCT()
struct FGrid
{
	GENERATED_BODY()

	FGrid()
	{
		GridX = 0.f;
		GridY = 0.f;
		GridZ = 0.f;

		
	}

	UPROPERTY(EditAnywhere, Category="Grid")
	int GridX = 0.f;

	UPROPERTY(EditAnywhere, Category="Grid")
	int GridY = 0.f;

	UPROPERTY(EditAnywhere, Category="Grid")
	int GridZ = 0.f;
	
	TArray<TArray<TBitArray<>>> BitGrid = TArray<TArray<TBitArray<>>>();

};


USTRUCT()
struct FRoom
{
	GENERATED_BODY()

	FRoom()
	{
		Location = FVector::ZeroVector;
		Size = FVector::ZeroVector;
	}

	FVector Location;

	FVector Size;
};


UCLASS()
class FLOCKNFLY_API AGeometryGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGeometryGenerator();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	

	UPROPERTY(EditAnywhere)
	FGrid Grid;

	UPROPERTY(EditAnywhere)
	int NumberOfRooms;

	UPROPERTY(EditAnywhere)
	UBoxComponent* WorldBounds;

	UPROPERTY(EditAnywhere)
	int32 IntSeed;
	
private:

	FVector Size;

	FRandomStream RandomStream;

	/** Collection of room structs*/
	TArray<FRoom> Rooms = TArray<FRoom>();

	/** Seed to procedural generation, deciding how noise is generated*/
	FString Seed = "u45on30b56qewiurq34qv934e";

	/** Function to check if a location is within the bounds and get the bounds*/
	bool IsWithinWorldBounds(FVector &Location);

	/** Array to store seed translation into bits*/
	TArray<uint8> BitArray = TArray<uint8>();

	/** Converts Fstring seed to and array of unsigned ints, aka a bit array*/
	TArray<uint8> ConvertSeedToBit();

	/** Converts seed to bit stream, and creates rooms*/
	void GenerateRooms();

	/** Calculates and generates a 3d grid of bit values*/
	void GenerateGrid();

	void SmoothMap();

	int NeighbourhoodCount();

};
