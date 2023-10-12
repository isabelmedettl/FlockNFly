 // Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBaseActor.h"

#include "FlockingBrain.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFlockingBaseActor::AFlockingBaseActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FlockingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoidMesh"));
	FlockingMeshComponent->SetupAttachment(GetRootComponent());
	FlockingMeshComponent->SetVisibility(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionComponent->SetupAttachment(FlockingMeshComponent);
}

// Called when the game starts or when spawned
void AFlockingBaseActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFlockingBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ensure( FlockingActorData != nullptr);

	FlockingActorData->DistanceToTarget = (FlockingActorData->TargetLocation - FlockingActorData->Location).Length();
	if (FlockingActorData->bIsLeader)
	{
		FlockingMeshComponent->SetMaterial(0, LeaderMaterial);
	}
	else
	{
		FlockingMeshComponent->SetMaterial(0, BasicMaterial);
	}	
}

void AFlockingBaseActor::UpdateLocation(float DeltaTime)
{
	if (FlockingActorData)
	{
		FlockingActorData->Location = GetActorLocation(); 
		SetActorLocation(FlockingActorData->Location + FlockingActorData->Velocity * DeltaTime);		
	}
}

void AFlockingBaseActor::SetFlockingDataPointer(FFlockingActorData& Pointer)
{
	FlockingActorData = &Pointer;
	FlockingActorData->Location = GetActorLocation();
	bHasEntityDataBeenSet = true;
}

