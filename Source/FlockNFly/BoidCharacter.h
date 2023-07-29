// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BoidCharacter.generated.h"

class AFlockNFlyCharacter;

/**  Struct containing boid data*/
USTRUCT()
struct FBoidData
{
	GENERATED_BODY()
	
	FBoidData()
	{
		Velocity = FVector::ZeroVector;
		CurrentSpeed = 0.f;
		TargetSpeed = 0.f;
		Direction = FVector::ZeroVector;
		PreferredDistance = 0.f;
		DistanceToTarget = 0.f;
		bIsClosestToTarget = false;
	}

	/** Current velocity of entity*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	FVector Velocity;

	/** Current speed of boid*/
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	double CurrentSpeed = 550.f;

	/** Target speed of boid to lerp toward*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	double TargetSpeed = 550.f;

	/** Current direction in which entity is moving in i world space*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	FVector Direction;

	/** Distance to keep from other entities*/
	UPROPERTY(EditAnywhere, Category = "Flocking")
	double PreferredDistance = 0.f;

	/** Distance to current target*/
	UPROPERTY(VisibleAnywhere, Category = "Flocking")
	double DistanceToTarget = 0.f;

	/** True if is closest to target*/
	UPROPERTY(EditAnywhere, Category = "Flocking")
	bool bIsClosestToTarget;
	
};


UCLASS()
class FLOCKNFLY_API ABoidCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABoidCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Bool for debugging*/
	UPROPERTY(EditAnywhere, Category= "Debug")
	bool bDebug = false;

	/** Struct containing data for flocking behavior*/
	UPROPERTY(EditAnywhere, Category= "Flocking")
	FBoidData BoidData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Moves character towards specified current target location*/
	void MoveTowardsLocation(float DeltaTime);

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
