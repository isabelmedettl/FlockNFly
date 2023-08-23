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
	if (FlockingActorData)
	{
		/*
		if (bDebug)
		{
			GetWorldTimerManager().SetTimer(DebugTimerHandle, this, &AFlockingBaseActor::OnDebug, DrawDebugDelay, true, 0.1f); // Delay default = 2.f;
		}
		*/
		
		FlockingActorData->Location = GetActorLocation();
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

	// sätta location 
	
	ensure( FlockingActorData != nullptr);
	if (FlockingActorData)
	{
		FlockingActorData->Location = GetActorLocation(); // actorn ska bara sätta sin position beroende på strukten. 
		//DrawDebugSphere(GetWorld(), FlockingActorData->Location, 30.f, 30, FColor::Black, false,20.f);
		//DrawDebugSphere(GetWorld(), GetActorLocation(), 30.f, 30, FColor::Red, false,20.f);

	
		const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(FlockingActorData->Location, CurrentTargetLocation);
		SetActorRotation(FRotator(0, NewRotation.Yaw -90, 0));

		FlockingActorData->Acceleration = FlockingActorData->SteerForce / Mass; //Mass i strukten
		//FlockingActorData.Velocity += FlockingActorData.SteerForce * DeltaTime;
		FlockingActorData->Velocity += FlockingActorData->Acceleration;
	
		SetActorLocation(FlockingActorData->Location + FlockingActorData->Velocity * DeltaTime);
		//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FlockingActorData->Velocity , FColor::Purple, false, 0.1f, 0, 10);
		FlockingActorData->Velocity = FlockingActorData->Velocity.GetClampedToMaxSize(MaxSpeed);
	}
}

FVector AFlockingBaseActor::CalculateSeekForce()
{
	const FVector Desired = (CurrentTargetLocation - FlockingActorData->Location).GetSafeNormal() * MaxSpeed; // CurrentTargetLocation finnas i strukten
	FVector NewSteeringForce = Desired - FlockingActorData->Velocity;
	NewSteeringForce /= MaxSpeed;
	NewSteeringForce *= MaxForce;
	
	return NewSteeringForce;
}


void AFlockingBaseActor::UpdateSteerForce(const TArray<FFlockingActorData> &EntitiesData, FVector Force) // skicka ref = bra, gör ingen kopia.
{
	ensure(FlockingActorData != nullptr);
	FlockingActorData->SteerForce = CalculateSeekForce() + Force * 10;

	/*
	if (Force != FVector::ZeroVector)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + Force * 10, FColor::Red, false, 0.1f, 0, 10);
	}
	*/
}

void AFlockingBaseActor::SetFlockingDataPointer(FFlockingActorData* Pointer, int ID)
{
	ensure (Pointer != nullptr);
	FlockingActorData = Pointer;
	// set location till actors location
	FlockingActorData->ID = ID;
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FString::Printf(TEXT("Vel loc set in actor: %f, %f, %f, ID: %i"), FlockingActorData->Location.X, FlockingActorData->Location.Y, FlockingActorData->Location.Z, FlockingActorData->ID));

	//DrawDebugSphere(GetWorld(), FlockingActorData->Location, 80.f, 30, FColor::Black, true,10.f);
}

/*
void AFlockingBaseActor::SetFlockingDataProperties(const FVector &Location,
	float DesiredSeparationRadius, float DesiredCohesionRadius, float DesiredAlignmentRadius, int32 ID)
{
	
	FlockingActorData->Location = Location;
	FlockingActorData->DesiredSeparationRadius = DesiredSeparationRadius;
	FlockingActorData->DesiredCohesionRadius = DesiredCohesionRadius;
	FlockingActorData->DesiredAlignmentRadius = DesiredAlignmentRadius;
	FlockingActorData->ID = ID;

	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FString::Printf(TEXT("Vel loc set in actor: %f, %f, %f, ID: %i"), FlockingActorData->Location.X, FlockingActorData->Location.Y, FlockingActorData->Location.Z, FlockingActorData->ID));
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FString::Printf(TEXT("Raiuses set in actor: %f, %f, %f, ID: %i"), FlockingActorData->DesiredSeparationRadius, FlockingActorData->DesiredCohesionRadius,FlockingActorData->DesiredAlignmentRadius, FlockingActorData->ID));

}



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

/*
	FVector SeparationForce = FVector::ZeroVector;
	int32 Counter = 0;

	for (int i = 0; i < EntitiesData.Num(); i++)
	{
		if (FlockingActorData->ID != EntitiesData[i].ID)
		{
			// göras i namespace?
			double Distance = (FlockingActorData->Location - EntitiesData[i].Location).Length();
			// if other entity is too close, move away from it
			if (Distance < FlockingActorData->DesiredSeparationRadius * 2)
			{
				FVector Difference = EntitiesData[i].Location -FlockingActorData->Location;
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
	
	
	*/
	
	
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