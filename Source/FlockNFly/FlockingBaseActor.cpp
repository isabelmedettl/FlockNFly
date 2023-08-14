// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBaseActor.h"
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
	FlockingData.Size = CollisionComponent->GetScaledSphereRadius();
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
		CurrentTargetLocation = PlayerCharacter->GetActorLocation() + PlayerCharacter->GetActorForwardVector() * FlockingData.PreferredDistanceToTarget;
	}

	// TODO: Make it not setting location but by applying force to velocity
	Seek(DeltaTime);
	SetActorLocation(GetActorLocation() + FlockingData.Velocity);
}

void AFlockingBaseActor::ApplyForce(FVector Force)
{
	FlockingData.Velocity += Force;
}


void AFlockingBaseActor::Seek(float DeltaTime)
{
	FlockingData.CurrentSpeed = FMath::Lerp(FlockingData.CurrentSpeed, FlockingData.TargetSpeed, 0.1);

	// Rotation
	const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTargetLocation);
	SetActorRotation(FRotator(0, NewRotation.Yaw -90, 0));
	
	FlockingData.Direction = CurrentTargetLocation - GetActorLocation();
	FlockingData.Direction.Normalize();
	// direction * vel / deltatime
	// addera det där till current position

	// Calculate the distance between the current location and the target location
	FlockingData.DistanceToTarget = FVector::Distance(CurrentTargetLocation, GetActorLocation());
	
	// Calculate distance the actor can move in this step based on the speed variable
	float MoveDistance = FlockingData.CurrentSpeed * DeltaTime;
	MoveDistance = FMath::Min(MoveDistance, FlockingData.DistanceToTarget);
	FlockingData.Velocity = FlockingData.Direction * MoveDistance;
	//const FVector NewDirection = GetActorLocation() + FlockingData.Velocity;
	//return NewDirection;
	
}

void AFlockingBaseActor::UpdateFlocking(TArray<AFlockingBaseActor*> &Entities, double SeekWeight, double CohesionWeight, double AlignmentWeight, double SeparationWeight)
{
	Cohesion = UpdateCohesion(Entities);

	// apply weights to forces
	Cohesion.X *= CohesionWeight;
	Cohesion.Y *= CohesionWeight;

	// Separation and alignment
	
	ApplyForce(Cohesion);
}

FVector AFlockingBaseActor::UpdateCohesion(TArray<AFlockingBaseActor*> &Entities) const
{
	FVector DesiredCohesion = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingData.ID != Other->FlockingData.ID)
		{
			if ((Other->GetActorLocation() - GetActorLocation()).Length() < FlockingData.MaxDesiredDistanceToNeighbours * 2)
			{
				DrawDebugLine(GetWorld(), GetActorLocation(), Other->GetActorLocation(), FColor::Cyan, false, 0.1f, 0, 10);
				DesiredCohesion.X += Other->GetActorLocation().X;
				DesiredCohesion.Y += Other->GetActorLocation().Y;
				Counter++;
			}
		}
	}
	
	if (Counter > 0)
	{
		const FVector NewDirection = FVector(DesiredCohesion.X / Counter, DesiredCohesion.Y / Counter, GetActorLocation().Z);
		return NewDirection;
	}
	return FVector::ZeroVector;
}

FVector AFlockingBaseActor::UpdateSeparation(TArray<AFlockingBaseActor*>& Entities) const
{
	FVector DesiredCohesion = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingData.ID != Other->FlockingData.ID)
		{
			if ((Other->GetActorLocation() - GetActorLocation()).Length() > FlockingData.MinDesiredDistanceToNeighbours * 2)
			{
				// https://github.com/jyanar/Boids/blob/master/src/Boid.cpp
				// https://github.com/nature-of-code/noc-examples-processing/blob/master/chp06_agents/NOC_6_08_SeparationAndSeek/Vehicle.pde
			}
		}
	}
}


void AFlockingBaseActor::OnDebug() const
{
	DrawDebugSphere(GetWorld(), CurrentTargetLocation, 30.f, 30, FColor::Black, false,0.2f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingData.MaxDesiredDistanceToNeighbours, 30, FColor::Red, false, 0.2f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingData.MinDesiredDistanceToNeighbours, 30, FColor::Green, false, 0.2f);

}


