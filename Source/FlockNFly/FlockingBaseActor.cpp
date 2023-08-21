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
	//FlockingActorData = FFlockingActorData();
	//FlockingActorData.Size = CollisionComponent->GetScaledSphereRadius(); 	
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
		CurrentTargetLocation = PlayerCharacter->GetActorLocation() + PlayerCharacter->GetActorForwardVector() + PreferredDistanceToTarget;
	}

	FlockingActorData->Location = GetActorLocation();
	
	const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(FlockingActorData->Location, CurrentTargetLocation);
	SetActorRotation(FRotator(0, NewRotation.Yaw -90, 0));

	FlockingActorData->Acceleration = FlockingActorData->SteerForce / Mass;
	//FlockingActorData.Velocity += FlockingActorData.SteerForce * DeltaTime;
	FlockingActorData->Velocity += FlockingActorData->Acceleration;
	
	SetActorLocation(FlockingActorData->Location + FlockingActorData->Velocity * DeltaTime);
	FlockingActorData->Velocity = FlockingActorData->Velocity.GetClampedToMaxSize(MaxSpeed);

	
}

FVector AFlockingBaseActor::CalculateSeekForce()
{
	const FVector Desired = (CurrentTargetLocation - FlockingActorData->Location).GetSafeNormal() * MaxSpeed;
	FVector NewSteeringForce = Desired - FlockingActorData->Velocity;
	NewSteeringForce /= MaxSpeed;
	NewSteeringForce *= MaxForce;
	
	return NewSteeringForce;
}


void AFlockingBaseActor::UpdateSteerForce(TArray<FFlockingActorData> EntitiesFlockingData) //TArray<FFlockingActorData*> FlockingActorDatas
{
	
	FlockingActorData->SteerForce = CalculateSeekForce();

	
	FVector SeparationForce = FVector::ZeroVector;
	int32 Counter = 0;

	for (FFlockingActorData Other : EntitiesFlockingData)
	{
		if (FlockingActorData->ID != Other.ID)
		{
			// göras i namespace?
			double Distance = (FlockingActorData->Location - Other.Location).Length();
			// if other entity is too close, move away from it
			if (Distance < FlockingActorData->DesiredSeparationRadius * 2)
			{
				FVector Difference = Other.Location -FlockingActorData->Location;
				SeparationForce += Difference;
				Counter++;
			}
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Tot Separation Force: %f, %f, %f, "), SeparationForce.X, SeparationForce.Y, SeparationForce.Z));

	// Adds average difference of locations to velocity
	if (Counter > 0)
	{
		SeparationForce /= Counter;
		SeparationForce *= -1;
		SeparationForce.Normalize();
	}

	SeparationForce *= 3;
	FlockingActorData->SteerForce += SeparationForce * 60;

	DrawDebugLine(GetWorld(), FlockingActorData->Location, FlockingActorData->Location + FlockingActorData->SteerForce, FColor::Red, false, 0.1f, 0, 10);


	
	
	/*

	Separation = CalculateSeparationForce(Entities);
	Cohesion = CalculateCohesionForce(Entities);
	Alignment = CalculateAlignmentForce(Entities);
	
	// apply weights to forces
	Separation *= SeparationWeight;
	Cohesion *= CohesionWeight;
	Alignment *= AlignmentWeight;

	
	FlockingActorData.SteerForce += Cohesion * MaxSpeed;
	FlockingActorData.SteerForce += Separation * MaxSpeed;
	FlockingActorData.SteerForce += Alignment * MaxSpeed;
	
	
	if (Cohesion != FVector::ZeroVector)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FlockingActorData.Velocity, FColor::Purple, false, 0.1f, 0, 10);
	}

	if (Separation != FVector::ZeroVector)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FlockingActorData.Velocity, FColor::Red, false, 0.1f, 0, 10);
	}
	
	
	if (Alignment != FVector::ZeroVector)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FlockingActorData.Velocity, FColor::Cyan, false, 0.1f, 0, 10);
	}
	
	//Cohesion = FVector::ZeroVector;
	//Separation = FVector::ZeroVector;
	*/
}

void AFlockingBaseActor::SetFlockingDataPointer(FFlockingActorData* Pointer)
{
	FlockingActorData = Pointer;
}

/*

FVector AFlockingBaseActor::CalculateCohesionForce(TArray<AFlockingBaseActor*> &Entities) 
{
	FVector CenterOfMass = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingActorData->ID != Other->FlockingActorData->ID)
		{
			if ((Other->FlockingActorData->Location - FlockingActorData->Location).Length() < FlockingActorData.DesiredCohesionRadius * 2 && (Other->FlockingActorData.Location - FlockingActorData.Location).Length() > FlockingActorData.DesiredSeparationRadius * 2 )
			{
				CenterOfMass += Other->FlockingActorData.Location;
				Counter++;
			}
		}
	}
	
	if (Counter > 0)
	{
		CenterOfMass /= Counter;
		FVector DesiredCohesion = FVector(CenterOfMass.X - FlockingActorData.Location.X, CenterOfMass.Y - FlockingActorData.Location.Y, CenterOfMass.Z - FlockingActorData.Location.Z);
		DesiredCohesion.Normalize();
		return DesiredCohesion;
	}
	return FVector::ZeroVector;
}

FVector AFlockingBaseActor::CalculateSeparationForce(TArray<AFlockingBaseActor*>& Entities)
{
	FVector SeparationForce = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingActorData.ID != Other->FlockingActorData.ID)
		{
			// göras i namespace?
			double Distance = (FlockingActorData.Location - Other->FlockingActorData.Location).Length();
			// if other entity is too close, move away from it
			if (Distance < FlockingActorData.DesiredSeparationRadius * 2)
			{
				FVector Difference = Other->FlockingActorData.Location -FlockingActorData.Location;
				SeparationForce += Difference;
				Counter++;
			}
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Tot Separation Force: %f, %f, %f, "), SeparationForce.X, SeparationForce.Y, SeparationForce.Z));

	// Adds average difference of locations to velocity
	if (Counter > 0)
	{
		SeparationForce /= Counter;
		SeparationForce *= -1;
		SeparationForce.Normalize();
		

		return SeparationForce;
		
	}
	return FVector::ZeroVector;
}

FVector AFlockingBaseActor::CalculateAlignmentForce(TArray<AFlockingBaseActor*>& Entities)
{
	FVector TotalVelocityVector = FVector::ZeroVector;
	int32 Counter = 0;

	for (AFlockingBaseActor* Other : Entities)
	{
		if (FlockingActorData.ID != Other->FlockingActorData.ID)
		{
			if ((Other->FlockingActorData.Location - FlockingActorData.Location).Length() < FlockingActorData.DesiredAlignmentRadius * 2 && (Other->FlockingActorData.Location - FlockingActorData.Location).Length() > 0 )
			{
				TotalVelocityVector += Other->FlockingActorData.Velocity;
				Counter++;
			}
		}
	}

	// If other entities are close enough to aline with, divide total vel by number of entities = average of velocity
	if (Counter > 0)
	{
		TotalVelocityVector /= Counter;
		TotalVelocityVector.Normalize();
		return TotalVelocityVector;
	}
	return FVector::ZeroVector;
}

*/
void AFlockingBaseActor::OnDebug() const
{
	DrawDebugSphere(GetWorld(), CurrentTargetLocation, 30.f, 30, FColor::Black, false,0.2f);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingActorData.DesiredAlignmentRadius, 30, FColor::Red, false, 0.2f);
	//DrawDebugSphere(GetWorld(), FlockingActorData.Location, FlockingActorData.DesiredCohesionRadius, 30, FColor::Green, false, 0.2f);
	DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingActorData->DesiredSeparationRadius, 30, FColor::Green, false, 0.2f);
//	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FlockingActorData.Velocity, FColor::Purple, false, 0.1f, 0, 10);


}


