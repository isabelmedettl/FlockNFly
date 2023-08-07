// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBaseActor.h"

#include "BoidCharacter.h"
#include "FlockNFlyCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFlockingBaseActor::AFlockingBaseActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoidMesh"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetVisibility(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionComponent->SetupAttachment(GetRootComponent());
	BoidData.Size = CollisionComponent->GetScaledSphereRadius();
}

// Called when the game starts or when spawned
void AFlockingBaseActor::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AFlockNFlyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if( PlayerCharacter != nullptr )
	{
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerCharacter->GetActorLocation()));

		// Calculate the location of the AI character relative to the player, and rotation
		CurrentTargetLocation = PlayerCharacter->GetActorLocation();
		const FVector DirectionToTarget = CurrentTargetLocation - GetActorLocation();
		const FRotator InitialRotation = DirectionToTarget.ToOrientationRotator();
		SetActorRotation(InitialRotation);
	}
	// If set to debug, initiate looping timer to call on draw debug function
	if (bDebug)
	{
		GetWorldTimerManager().SetTimer(DebugTimerHandle, this, &AFlockingBaseActor::OnDebug, DrawDebugDelay, true, 0.1f); // Delay default = 2.f;
	}
	
}

// Called every frame
void AFlockingBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if( PlayerCharacter != nullptr )
	{
		CurrentTargetLocation = PlayerCharacter->GetActorLocation() + PlayerCharacter->GetActorForwardVector() * 30.f;
	}
	MoveTowardsLocation(DeltaTime);
}


void AFlockingBaseActor::MoveTowardsLocation(float DeltaTime)
{
	BoidData.CurrentSpeed = FMath::Lerp(BoidData.CurrentSpeed, BoidData.TargetSpeed, 0.1);

	// Rotation
	const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTargetLocation);
	SetActorRotation(FRotator(0, NewRotation.Yaw -90, 0));
	
	BoidData.Direction = CurrentTargetLocation - GetActorLocation();
	BoidData.Direction.Normalize();

	// Calculate the distance between the current location and the target location
	BoidData.DistanceToTarget = FVector::Distance(CurrentTargetLocation, GetActorLocation());

	// Calculate distance the actor can move in this step based on the speed variable
	float MoveDistance = BoidData.CurrentSpeed * DeltaTime;
	MoveDistance = FMath::Min(MoveDistance, BoidData.DistanceToTarget);
	const FVector NewLocation = GetActorLocation() + BoidData.Direction * MoveDistance;
	SetActorLocation(NewLocation);

	// Update data in struct
	//BoidData.Velocity = GetCharacterMovement()->Velocity;
}

void AFlockingBaseActor::OnDebug() const
{
	DrawDebugSphere(GetWorld(), CurrentTargetLocation, 30.f, 30, FColor::Black, false,0.2f);
}


