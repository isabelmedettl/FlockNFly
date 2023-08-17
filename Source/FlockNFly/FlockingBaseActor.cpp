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
	// nödlösning för att testa att köra flock-update innan seek
	FlockingData.Velocity = FVector::ForwardVector;
}

// Called every frame
void AFlockingBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if( PlayerCharacter != nullptr )
	{
		CurrentTargetLocation = PlayerCharacter->GetActorLocation() + PlayerCharacter->GetActorForwardVector() * FlockingData.PreferredDistanceToTarget;
	}

	FlockingData.Velocity += FlockingData.SteerForce;
	FlockingData.Velocity = FlockingData.Velocity.GetClampedToMaxSize(FlockingData.MaxSpeed);
	SetActorLocation(GetActorLocation() + FlockingData.Velocity * DeltaTime);
	
}

void AFlockingBaseActor::ApplyForce(FVector Force)
{
	FlockingData.Velocity += Force;
	//FlockingData.MaxAcceleration += Force;
}

FVector AFlockingBaseActor::LimitForce(FVector& CurrentForce)
{
	const double Size = CurrentForce.Length();
	if (Size > FlockingData.MaxForce)
	{
		return FVector(CurrentForce.X / Size, CurrentForce.Y / Size, CurrentForce.Z);
	}
	return FVector::ZeroVector;
}




void AFlockingBaseActor::CalculateSteeringForce()
{
	FlockingData.CurrentSpeed = FMath::Lerp(FlockingData.CurrentSpeed, FlockingData.TargetSpeed, 0.1);

	const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTargetLocation);
	SetActorRotation(FRotator(0, NewRotation.Yaw -90, 0));

	FlockingData.SteerForce = (CurrentTargetLocation - GetActorLocation()).GetSafeNormal() * FlockingData.MaxSpeed;
	const FVector Current = (FlockingData.SteerForce - FlockingData.Velocity).GetClampedToMaxSize(FlockingData.MaxForce);

	FlockingData.Acceleration = Current / FlockingData.Mass;

	
}


void AFlockingBaseActor::UpdateFlocking(TArray<AFlockingBaseActor*> &Entities, double CohesionWeight, double AlignmentWeight, double SeparationWeight)
{
	
	Separation = UpdateSeparation(Entities);
	//Cohesion = UpdateCohesion(Entities);
	//Alignment = UpdateAlignment(Entities);
	
	// apply weights to forces
	Separation *= SeparationWeight;
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Sep Force: %f, %f, %f"), Separation.X, Separation.Y, Separation.Z));
	FlockingData.Velocity += Separation;
	FlockingData.SteerForce += Separation;
	//Alignment *= AlignmentWeight;
	//Cohesion *= CohesionWeight;

	ApplyForce(Separation);
	//ApplyForce(Cohesion);
	//ApplyForce(Alignment);
	CalculateSteeringForce();
	Separation = FVector::ZeroVector;
}


FVector AFlockingBaseActor::UpdateCohesion(TArray<AFlockingBaseActor*> &Entities) 
{
	FVector DesiredCohesion = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingData.ID != Other->FlockingData.ID)
		{
			if ((Other->GetActorLocation() - GetActorLocation()).Length() < FlockingData.DesiredCohesionRadius * 2)
			{
				//if (DrawDebugDelay)
					//DrawDebugLine(GetWorld(), GetActorLocation(), Other->GetActorLocation(), FColor::Cyan, false, 0.1f, 0, 10);
				OtherNeighbour = Other;
				DesiredCohesion += Other->GetActorLocation();
				Counter++;
			}
		}
	}
	
	if (Counter > 0)
	{
		FVector NewDirection = FVector(DesiredCohesion.X / Counter, DesiredCohesion.Y / Counter, DesiredCohesion.Z / Counter);
		NewDirection = LimitForce(NewDirection);
		return NewDirection;
	}
	return FVector::ZeroVector;
}

FVector AFlockingBaseActor::UpdateSeparation(TArray<AFlockingBaseActor*>& Entities)
{
	FVector SeparationForce = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingData.ID != Other->FlockingData.ID)
		{
			double Distance = (GetActorLocation() - Other->GetActorLocation()).Length();
			// if other entity is too close, move away from it
			if (Distance < FlockingData.DesiredSeparationRadius * 2)
			{
				FVector Difference = GetActorLocation() - Other->GetActorLocation();
				SeparationForce += Difference;
				//DesiredSeparation = FlockingData.Velocity + DesiredSeparation;
				Counter++;
				//return DesiredSeparation;
			}
		}
	}
	
	// Adds average difference of locations to velocity
	if (Counter > 0)
	{
		SeparationForce /= Counter;
		SeparationForce *= -1;
		SeparationForce.Normalize();
		if (bDebug)
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + SeparationForce * 20, FColor::Purple, false, 0.1f, 0, 10);

		return SeparationForce;
		
	}
	return FVector::ZeroVector;
}

FVector AFlockingBaseActor::UpdateAlignment(TArray<AFlockingBaseActor*>& Entities)
{
	FVector TotalAlignmentVector = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingData.ID != Other->FlockingData.ID)
		{
			if ((Other->GetActorLocation() - GetActorLocation()).Length() < FlockingData.DesiredAlignmentRadius * 2 && (Other->GetActorLocation() - GetActorLocation()).Length() > 0 )
			{
				TotalAlignmentVector += Other->FlockingData.Velocity;
				Counter++;
			}
		}
	}

	// If other entities are close enough to aline with, divide total vel by number of entities = average of velocity
	if (Counter > 0)
	{
		TotalAlignmentVector = FVector(TotalAlignmentVector.X / Counter, TotalAlignmentVector.Y / Counter, TotalAlignmentVector.Z / Counter);
		TotalAlignmentVector.Normalize();
		TotalAlignmentVector *= FlockingData.CurrentSpeed;
		
		if (bDebug)
		{
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + TotalAlignmentVector * 10, FColor::Yellow, false, 0.1f, 0, 10);
		}
		FVector DesiredAlignment = TotalAlignmentVector - FlockingData.Velocity;
		DesiredAlignment = LimitForce(DesiredAlignment);
		return DesiredAlignment;
	}
	return FVector::ZeroVector;
}


void AFlockingBaseActor::OnDebug() const
{
	DrawDebugSphere(GetWorld(), CurrentTargetLocation, 30.f, 30, FColor::Black, false,0.2f);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingData.DesiredAlignmentRadius, 30, FColor::Red, false, 0.2f);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingData.DesiredCohesionRadius, 30, FColor::Green, false, 0.2f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingData.DesiredSeparationRadius, 30, FColor::Green, false, 0.2f);

}


