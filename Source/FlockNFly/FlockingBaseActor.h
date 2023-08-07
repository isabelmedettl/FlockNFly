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
		TargetSpeed = 100.f;
		CurrentSpeed = TargetSpeed;
		Direction = FVector::ZeroVector;
		PreferredDistanceToTarget = 200.f;
		DistanceToTarget = 0.f;
		bIsClosestToTarget = false;
		Size = 0.f;
	}

	/** Current velocity of entity*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	FVector Velocity;

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

	/** Set radius to other entites that are considered neighbours to entity*/
	UPROPERTY(VisibleAnywhere, Category="Flocking")
	double DesiredDistanceToNeighbours = 0.f;
	
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

	/** Moves character towards specified current target location*/
	FVector CalculateDirectionToTarget(float DeltaTime);

	/** Applies cohesion rules*/
	FVector Cohere(const TArray<AFlockingBaseActor*> *Entities);

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

	
};
