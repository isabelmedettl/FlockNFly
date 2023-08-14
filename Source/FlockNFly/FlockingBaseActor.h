// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlockingBaseActor.generated.h"

class USphereComponent;
class AFlockNFlyCharacter;

/**  Struct containing boid data*/
USTRUCT()
struct FFlockingActorData
{
	GENERATED_BODY()
	
	FFlockingActorData()
	{
		Velocity = FVector::ZeroVector;
		Acceleration = FVector::ZeroVector;
		TargetSpeed = 100.f;
		CurrentSpeed = TargetSpeed;
		Direction = FVector::ZeroVector;
		PreferredDistanceToTarget = 200.f;
		DistanceToTarget = 0.f;
		bIsClosestToTarget = false;
		Size = 0.f;
		MaxDesiredDistanceToNeighbours = 300.f;
		MinDesiredDistanceToNeighbours = 150.f;
		ID = 0;
	}

	/** Current velocity of entity*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	FVector Velocity;

	/** Current acceleration of entity*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	FVector Acceleration;

	/** Current speed of boid*/
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	double CurrentSpeed = 100.f;

	/** Target speed of boid to lerp toward*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	double TargetSpeed = 100.f;

	/** Current direction in which entity is moving in i world space*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	FVector Direction;

	/** Distance to keep from other entities*/
	UPROPERTY(EditAnywhere, Category = "Flocking")
	double PreferredDistanceToTarget = 50.f;

	/** Distance to current target*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	double DistanceToTarget = 0.f;

	/** True if is closest to target*/
	UPROPERTY(EditAnywhere, Category = "Flocking")
	bool bIsClosestToTarget;

	/** Size of boid*/
	UPROPERTY(VisibleAnywhere, Category="Boid")
	double Size;

	/** Set max radius to other entites that are considered neighbours to entity*/
	UPROPERTY(EditAnywhere, Category="Flocking")
	float MaxDesiredDistanceToNeighbours = 0.f;

	/** Set min radius to other entites that are considered neighbours to entity*/
	UPROPERTY(EditAnywhere, Category="Flocking")
	float MinDesiredDistanceToNeighbours = 0.f;

	/** Unique number for idintification*/
	UPROPERTY(VisibleAnywhere, Category="Flocking")
	int32 ID;
	
};


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

	/** Struct containing data for flocking behavior*/
	UPROPERTY(EditAnywhere, Category= "Flocking")
	FFlockingActorData FlockingData;

	/** Static mesh comp*/
	UPROPERTY(EditAnywhere, Category="Mesh")
	UStaticMeshComponent* FlockingMeshComponent;

	/** Static mesh comp*/
	UPROPERTY(EditAnywhere, Category="Collision")
	USphereComponent* CollisionComponent;
			
	/** Applies three rules of flocking, modifying volocity accordingly and updates data*/
	void UpdateFlocking(TArray<AFlockingBaseActor*> &Entities, double SeekWeight, double CohesionWeight, double AlignmentWeight, double SeparationWeight);

protected:

	/** Moves character towards specified current target location*/
	void Seek(float DeltaTime);


private:

	/** Pointer toward player character*/
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess = true))
	AFlockNFlyCharacter* PlayerCharacter;
	
	/** Location in world space that boid moves towards currently*/
	FVector CurrentTargetLocation = FVector::ZeroVector;

	// ============= Debug ============= // 
	/** Method for debugging, for drawing variables in world space*/
	void OnDebug() const;

	/** Tick timer for debugging */
	FTimerHandle DebugTimerHandle;

	/** Defines how often to draw debug shapes*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	double DrawDebugDelay = 2.f;

	/** Alters entities position to correspond with average alignment of nearby entities, taking the position of entities within certain radius and steers entity towards the average position of those entites*/
	FVector UpdateCohesion(TArray<AFlockingBaseActor*> &Entities) const;

	/** Alters entities to steer away from any neighbor that is within view and within a prescribed minimum separation distance*/
	FVector UpdateSeparation(TArray<AFlockingBaseActor*> &Entities) const;

	/** Applies said force to movement vector*/
	void ApplyForce(FVector Force);

	/** Defines how much to slow down to make less abrupt stop*/
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = true))
	double SmoothSpeed = 0.5f;


	// =========== Flocking rules vectors ============ //
	/** Separation vector*/ 
	FVector Separation = FVector::ZeroVector;
	
	/** Cohesion vector*/
	FVector Cohesion = FVector::ZeroVector;

	/** Alignment vector */
	FVector Alignment = FVector::ZeroVector;


	// ============= Collision and 3D navigation =========== //
	// http://www.red3d.com/cwr/steer/gdc99/
	// https://github.com/darbycostello/Nav3D


	// https://github.com/FAUSheppy/DonAINavigation/blob/master/Source/DonAINavigation/Private/BehaviorTree/BTTask_FlyTo.cpp

	// https://www.youtube.com/watch?v=-oG_8z7b6eM

	// pathfinding
	// https://www.youtube.com/watch?v=GdhnLIvDQj4
	//https://www.youtube.com/watch?v=p3WcsO6pAmU
	
};

