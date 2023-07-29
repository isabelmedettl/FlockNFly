// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlockingBrain.generated.h"

class AFlockNFlyCharacter;
class USphereComponent;
class ABoidCharacter;
UCLASS()
class FLOCKNFLY_API AFlockingBrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlockingBrain();

	/** Area to spawn boids in */
	UPROPERTY(EditAnywhere)
	USphereComponent* SpawnArea;

	/** Subclass of boid character to be spawned*/
	TSubclassOf<ABoidCharacter> BoidClass;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	/** Number of boids to spawn*/
	UPROPERTY(EditAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	int32 NumberOfBoids = 0;

	/** Defines how many rows of boids to spawn, deciding placement of the boids in worlds space */
	UPROPERTY(EditAnywhere, Category= "Spawning", meta = (AllowPrivateAccess = true))
	int32 BoidRows = 0;
	
	/** Defines how many columns of boids to spawn, deciding placement of the boids in worlds space */
	UPROPERTY(EditAnywhere, Category= "Spawning", meta = (AllowPrivateAccess = true))
	int32 BoidColumns = 0;

	/** Defines distance in between boids, both at spawn and in formation of flock*/
	UPROPERTY(EditAnywhere, Category= "Spawning", meta = (AllowPrivateAccess = true))
	int32 DistanceBetweenBoids = 0;

	/** Array containing pointers to all boids that the actor is managing*/
	TArray<ABoidCharacter*> Boids = TArray<ABoidCharacter*>();

	/** Array containing positions in world space to spawn boid to*/
	TArray<FVector> SpawnPositions = TArray<FVector>();

	/** Spawns specified number of boids in designated spawn area*/
	void SpawnBoids();

	/** Calculates spawn positions for boids given number of boids and distance between them
	 * @return true if all locations can be calculated without collisions
	 */
	bool CalculatePossibleSpawnLocations();

	/** Pointer to player Character*/
	AFlockNFlyCharacter* PlayerCharacter;

	
};
