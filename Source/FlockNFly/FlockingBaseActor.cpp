 // Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBaseActor.h"

#include "FlockingBrain.h"
#include "FlockNFlyCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

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

	if (FlockingActorData->bIsLeader)
	{
		//FlockingMeshComponent->SetMaterial(LeaderMaterial->GetLinkerIndex(), LeaderMaterial);
	}
	
}

void AFlockingBaseActor::UpdateSteerForce(const FVector &Force) const // skicka ref = bra, gör ingen kopia.
{
	ensure(FlockingActorData != nullptr);
	FlockingActorData->SteerForce = Force;
	
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

