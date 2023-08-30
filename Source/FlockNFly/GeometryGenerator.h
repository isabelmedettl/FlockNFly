// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryGenerator.generated.h"


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
	
	TArray<TArray<TArray<int>>> IntGrid = TArray<TArray<TArray<int>>>();

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
	
private:
	FString Seed = "u45on30b56qewiurq34qv934e";

	

	TArray<uint8> BitArray = TArray<uint8>();

	TArray<uint8> ConvertSeedToBit();

	void GenerateGrid();

};
