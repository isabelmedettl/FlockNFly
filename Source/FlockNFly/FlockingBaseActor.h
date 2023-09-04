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
	

	/** Static mesh comp*/
	UPROPERTY(EditAnywhere, Category="Mesh")
	UStaticMeshComponent* FlockingMeshComponent;

	/** Static mesh comp*/
	UPROPERTY(EditAnywhere, Category="Collision")
	USphereComponent* CollisionComponent;
			
	/** Sets steering force to specified force
	 * @param Force vector to set steer force to
	 */
	void UpdateSteerForce(const FVector &Force) const;

	/** Sets new location of actor*/
	void UpdateLocation(float DeltaTime);

	/** Sets FlockingActorData pointer to received ref
	 * @param Pointer ref to set
	 */
	void SetFlockingDataPointer(FFlockingActorData &Pointer);
	
	/** Struct containing data for flocking behavior*/
	FFlockingActorData* FlockingActorData = nullptr;
	
private:

	/** Bool for checking if flocking data struct has been set*/
	bool bHasEntityDataBeenSet = false;
		
	// =========== Flocking rules vectors ============ //
	/** Separation vector*/ 
	//FVector Separation = FVector::ZeroVector;
	
	/** Cohesion vector*/
	//FVector Cohesion = FVector::ZeroVector;

	/** Alignment vector */
	//FVector Alignment = FVector::ZeroVector;
	

	/** Attempts to match the velocity of other entities inside this entity´s visible range by adding neighbours velocity to computation vector*/
	//FVector CalculateAlignmentForce(TArray<AFlockingBaseActor*> &Entities);

	

	// ============= Collision and 3D navigation =========== //
	// http://www.red3d.com/cwr/steer/gdc99/
	// https://github.com/darbycostello/Nav3D


	// https://github.com/FAUSheppy/DonAINavigation/blob/master/Source/DonAINavigation/Private/BehaviorTree/BTTask_FlyTo.cpp

	// https://www.youtube.com/watch?v=-oG_8z7b6eM

	// pathfinding
	// https://www.youtube.com/watch?v=GdhnLIvDQj4
	//https://www.youtube.com/watch?v=p3WcsO6pAmU
	
};

