// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlockingBaseActor.generated.h"

class USphereComponent;
class AFlockNFlyCharacter;
struct FFlockingActorData;
class AFlockingBrain;

/**
 *
 * Ju närmare obsticle desto starkare färg
 * Leader markeras med färg
 * 
 * 
 */

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

	UPROPERTY(EditAnywhere, Category="Mesh")
	UMaterial* BasicMaterial;

	UPROPERTY(EditAnywhere, Category="Mesh")
	UMaterial* LeaderMaterial;
	
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
	
	
};

