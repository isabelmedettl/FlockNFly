// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlockingBaseActor.generated.h"

class USphereComponent;
class AFlockNFlyCharacter;
struct FFlockingActorData;
class AFlockingBrain;

UCLASS()
class FLOCKNFLY_API AFlockingBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlockingBaseActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Bool for debugging*/
	UPROPERTY(EditAnywhere, Category= "Debug")
	bool bDebug = false;

	/** Static mesh comp*/
	UPROPERTY(EditAnywhere, Category="Mesh")
	UStaticMeshComponent* FlockingMeshComponent;

	/** Static mesh comp*/
	UPROPERTY(EditAnywhere, Category="Collision")
	USphereComponent* CollisionComponent;
			
	/** Applies three rules of flocking, modifying volocity accordingly and updates data*/
	void UpdateSteerForce(const TArray<FFlockingActorData> &EntitiesFlockingData);

	void SetFlockingDataPointer(FFlockingActorData* Pointer);

	void SetFlockingDataProperties(FVector Velocity, FVector Acceleration, FVector, FVector Location, float DesiredSeparationRadius, float DesiredCohesionRadius, float DesiredAlignmentRadius, int32 ID);
	
	/** Struct containing data for flocking behavior*/
	FFlockingActorData* FlockingActorData;
	
private:

 
	
	/** Limits magnitude of velocity vector */
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	float MaxSpeed = 400.f;

	/** Limits magnitude of acceleration vector */
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	float MaxForce = 50.f;

	/** Mass of entity*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	float Mass = 0.7f;

	/** Distance to keep from other entities*/
	float PreferredDistanceToTarget = 50.f;
	
	/** Pointer toward player character*/
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess = true))
	AFlockNFlyCharacter* PlayerCharacter;
	
	/** Location in world space that boid moves towards currently*/
	FVector CurrentTargetLocation = FVector::ZeroVector;

	// ============= Debug ============= // 
	/** Method for debugging, for drawing variables in world space*/
	void OnDebug() const;

	/** Method for debugging, draws line between two points */
	//void OnDebugLine() const;

	/** Tick timer for debugging */
	FTimerHandle DebugTimerHandle;

	/** Defines how often to draw debug shapes*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	float DrawDebugDelay = 2.f;	

	// =========== Flocking rules vectors ============ //
	/** Separation vector*/ 
	FVector Separation = FVector::ZeroVector;
	
	/** Cohesion vector*/
	FVector Cohesion = FVector::ZeroVector;

	/** Alignment vector */
	FVector Alignment = FVector::ZeroVector;

	/** Alters entities position to correspond with average alignment of nearby entities, taking the position of entities within certain radius and steers entity towards the average position of those entites*/
	FVector CalculateCohesionForce(TArray<AFlockingBaseActor*> &Entities);

	/** Alters entities to steer away from any neighbor that is within view and within a prescribed minimum separation distance*/
	FVector CalculateSeparationForce(TArray<AFlockingBaseActor*> &Entities);

	/** Attempts to match the velocity of other entities inside this entity´s visible range by adding neighbours velocity to computation vector*/
	FVector CalculateAlignmentForce(TArray<AFlockingBaseActor*> &Entities);

	/** Take the vector resulting from subtracting the desired position from the current position. The result is the appropriate velocity */
	FVector CalculateSeekForce();

	/** */


	// ============= Collision and 3D navigation =========== //
	// http://www.red3d.com/cwr/steer/gdc99/
	// https://github.com/darbycostello/Nav3D


	// https://github.com/FAUSheppy/DonAINavigation/blob/master/Source/DonAINavigation/Private/BehaviorTree/BTTask_FlyTo.cpp

	// https://www.youtube.com/watch?v=-oG_8z7b6eM

	// pathfinding
	// https://www.youtube.com/watch?v=GdhnLIvDQj4
	//https://www.youtube.com/watch?v=p3WcsO6pAmU
	
};

