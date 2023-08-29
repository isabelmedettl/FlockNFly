// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryGenerator.generated.h"

UCLASS()
class FLOCKNFLY_API AGeometryGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGeometryGenerator();

	UPROPERTY(EditAnywhere)
	float GridX = 0.f;;

	UPROPERTY(EditAnywhere)
	float GridY = 0.f;

	UPROPERTY(EditAnywhere)
	float GridZ = 0.f;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:
	FString Seed = "u45on30b56qewiurq34qv934e";


	FString ConvertSeedToBit();

};
