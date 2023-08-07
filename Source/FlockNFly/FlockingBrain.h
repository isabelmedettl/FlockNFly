// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlockingBrain.generated.h"

class AFlockingBaseActor;
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

	/** Area to spawn boids in 
	UPROPERTY(EditAnywhere)
	USphereComponent* SpawnArea;
	*/

	/** Subclass of boid character to be spawned*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AFlockingBaseActor> FlockingBaseActorClass;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	/** Number of boids to spawn*/
	UPROPERTY(EditAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	int32 NumberOfEntities = 0;

	/** Defines how many rows of boids to spawn, deciding placement of the boids in worlds space */
	UPROPERTY(VisibleAnywhere, Category= "Spawning", meta = (AllowPrivateAccess = true))
	int32 EntityRows = 0;
	
	/** Defines how many columns of boids to spawn, deciding placement of the boids in worlds space */
	UPROPERTY(VisibleAnywhere, Category= "Spawning", meta = (AllowPrivateAccess = true))
	int32 EntityColumns = 0;

	/** Defines distance in between boids, both at spawn and in formation of flock*/
	UPROPERTY(EditAnywhere, Category= "Spawning", meta = (AllowPrivateAccess = true))
	int32 DistanceBetweenEntities = 0;

	/** Defines height in world space at which boids are going to spawn*/
	UPROPERTY(EditAnywhere, Category="Spawning", meta = (AllowPrivateAccess = true))
	double SpawnHeight = 0.f;

	/** Array containing pointers to all boids that the actor is managing*/
	TArray<AFlockingBaseActor*> Entities = TArray<AFlockingBaseActor*>();

	/** Array containing positions in world space to spawn boid to*/
	TArray<FVector> SpawnLocations = TArray<FVector>();

	/** Spawns specified number of boids in designated spawn area*/
	void SpawnBoids();

	/** Calculates spawn positions for boids given number of boids and distance between them
	 */
	void CalculatePossibleSpawnFormation();

	/** Check for overlap at a potential spawn location
	 * @param NewLocation location to check for collision
	 * @return true if collision is found
	 */
	bool CheckCollisionAtSpawnLocation(const FVector NewLocation);

	/** Pointer to player Character*/
	AFlockNFlyCharacter* PlayerCharacter;

	void SpawnEntity(const FVector SpawnLocation);


	// ======= Weights and methods for flocking behaviors ========= //

	/** Looping timer handle for applying specified behaviors on entities*/
	FTimerHandle ApplyBehaviorTimerHandle;

	/** Multiplyer for applying cohesion force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 CohesionWeight = 0.f;
	
	/** Multiplyer for applying separation force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 SeparationWeight = 0.f;

	/** Multiplyer for applying alignment force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 AlignmentWeight = 0.f;

	/** Loops through collection of entities and applies behaviors */
	void ApplyBehaviors();

	/** Adjust all flocking entities to uphold cohesion rules*/
	void AdjustForCohesion();

	/** Alters entities position to correspond with average alignment of nearby entities, taking the position of entities within certain radius and steers entity towards the average position of those entites*/
	FVector CalculateComputationDirection();
	
	
	// =========== Flocking variables ============= //
	/** Counter for how many neighbours an entity has that are within set radius of entity*/
	int32 CloseEntityCount = 0;

	
	
};
