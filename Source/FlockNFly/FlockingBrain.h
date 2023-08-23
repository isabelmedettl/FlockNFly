// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlockingBaseActor.h"
#include "GameFramework/Actor.h"
#include "FlockingBrain.generated.h"

class AFlockingBaseActor;
class AFlockNFlyCharacter;
class USphereComponent;

USTRUCT()
struct FFlockingActorData // ta bort all skit, doubles är onödigt, använd floats pga kostsamhet och inte prec. Gör om så det inte är properties, utan setts från brain
	{
	GENERATED_BODY()

	
	FFlockingActorData()
	{
		Velocity = FVector::ZeroVector;
		Acceleration = FVector::ZeroVector;
		SteerForce = FVector::ZeroVector;
		Location = FVector::ZeroVector;
		DesiredSeparationRadius = 200.f;
		DesiredCohesionRadius = 300.f;
		DesiredAlignmentRadius = 400.f;
		ID = 0; // bort
	}
	

	/** Current velocity of entity*/
	FVector Velocity;

	/** Current acceleration of entity*/
	FVector Acceleration;

	/** Current steering force of entity*/
	FVector SteerForce;

	/** Current location of entity in world space */
	FVector Location;
	
	/** Distance of field of vision for separation between entities */
	float DesiredSeparationRadius = 200.f;

	/** Maximal distance of vision for calculating average position amongst neighbour entities and moving towards that point */
	float DesiredCohesionRadius = 300.f;

	/** Distance of field of vision for calculating average velocity of nearby entities*/
	float DesiredAlignmentRadius = 300.f;

	/** Unique number for idintification*/
	int32 ID; // TA bort

};

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

	/** Bool for debugging*/
	UPROPERTY(EditAnywhere, Category= "Debug")
	bool bDebug = false;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	

private:


	/** Array containing Flocking data structs to all active entities, mapped to Entities*/
	UPROPERTY(VisibleAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	TArray<FFlockingActorData> EntitiesFlockingData;
	
	/** Array containing pointers to all entities that the actor is managing*/
	UPROPERTY(VisibleAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	TArray<AFlockingBaseActor*> Entities;

	
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

	/** Pointer to player Character*/
	AFlockNFlyCharacter* PlayerCharacter;

	void SpawnEntity(const FVector &SpawnLocation, int32 ID);


	// ======= Weights and methods for flocking behaviors ========= //

	/** Looping timer handle for applying specified behaviors on entities*/
	FTimerHandle ApplyBehaviorTimerHandle;

	/** Delay for how often to apply behaviors on entities, deciding how often to apply vector forces*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	double ApplyBehaviorDelay = 0.1f;

	/** Multiplyer for applying cohesion force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 CohesionWeight = 1.2f;

	/** Multiplyer for applying cohesion force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 SeekWeight = 1.f;
	
	/** Multiplyer for applying separation force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 SeparationWeight = 3.f;

	/** Multiplyer for applying alignment force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	int32 AlignmentWeight = 1.2f;

	/** Loops through collection of entities and applies behaviors */
	void ApplyBehaviors();

	// ========= Debugging =============== //
	/** Tick timer for debugging */
	FTimerHandle DebugTimerHandle;

	/** Defines how often to draw debug shapes*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	float DrawDebugDelay = 2.f;

	/** Method for debugging, for drawing variables in world space*/
	void OnDebug(FVector &Location) const;

	/** Method for debugging, draws line between two points */
	void OnDebugLine(FVector &FromLocation, FVector &ToLocation) const;
	

	
	// =========== Flocking variables ============= //
	/** Distance of field of vision for separation between entities */
	UPROPERTY(EditAnywhere, Category="Flocking")
	float DesiredSeparationRadius = 200.f;

	/** Maximal distance of vision for calculating average position amongst neighbour entities and moving towards that point */
	UPROPERTY(EditAnywhere, Category="Flocking")
	float DesiredCohesionRadius = 300.f;

	/** Distance of field of vision for calculating average velocity of nearby entities*/
	UPROPERTY(EditAnywhere, Category="Flocking")
	float DesiredAlignmentRadius = 300.f;

	// =========== Flocking force vectors to apply to entities ============ //
	
	/** Separation vector*/ 
	FVector Separation = FVector::ZeroVector;
	
	/** Cohesion vector*/
	FVector Cohesion = FVector::ZeroVector;

	/** Alignment vector */
	FVector Alignment = FVector::ZeroVector;

	
	
};
