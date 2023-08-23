// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBrain.h"

#include "FlockingBaseActor.h"
#include "FlockNFlyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFlockingBrain::AFlockingBrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AFlockingBrain::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AFlockNFlyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if( PlayerCharacter != nullptr)
	{
		if (SpawnHeight < 0.f)
		{
			SpawnHeight = GetActorLocation().Y;
		}
		
		if (NumberOfEntities > 0 && IsValid(FlockingBaseActorClass))
		{
			SpawnBoids();
		}
	}

	// Start looping timer to update flocking entities behavior
	//GetWorldTimerManager().SetTimer(ApplyBehaviorTimerHandle, this, &AFlockingBrain::ApplyBehaviors, ApplyBehaviorDelay, true,0.1f);
}

// Called every frame
void AFlockingBrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ApplyBehaviors();
}

void AFlockingBrain::SpawnBoids()
{
	if (!GetWorld()) return;
	if (NumberOfEntities != 0)
	{
		CalculatePossibleSpawnFormation();
		
		int32 Counter = 0;
		for (FVector Loc : SpawnLocations)
		{
			SpawnEntity(Loc, Counter);
			Counter++;
		}
	}	
}

void AFlockingBrain::CalculatePossibleSpawnFormation()
{
	if (NumberOfEntities <= 0)
	{
		EntityRows = EntityColumns = 0;
	}
	else if(NumberOfEntities == 1)
	{
		SpawnLocations.Add(GetActorLocation());
	}
	else if(NumberOfEntities == 2)
	{
		SpawnLocations.Add(FVector(GetActorLocation().X, GetActorLocation().Y  + DistanceBetweenEntities, GetActorLocation().Z));
		SpawnLocations.Add(GetActorLocation());
	}
	else if(NumberOfEntities == 3)
	{
		SpawnLocations.Add(GetActorLocation());
		SpawnLocations.Add(FVector (GetActorLocation().X + DistanceBetweenEntities * FMath::Cos(60.f), GetActorLocation().Y + DistanceBetweenEntities * FMath::Sin(60.f), GetActorLocation().Z ));
		SpawnLocations.Add(FVector (GetActorLocation().X + DistanceBetweenEntities * FMath::Cos(-60.f), GetActorLocation().Y + DistanceBetweenEntities * FMath::Sin(-60.f), GetActorLocation().Z ));
	}
	else if(NumberOfEntities == 4)
	{
		SpawnLocations.Add(FVector(GetActorLocation().X - DistanceBetweenEntities, GetActorLocation().Y - DistanceBetweenEntities,  GetActorLocation().Z ));
		SpawnLocations.Add(FVector(GetActorLocation().X - DistanceBetweenEntities, GetActorLocation().Y + DistanceBetweenEntities,  GetActorLocation().Z ));
		SpawnLocations.Add(FVector(GetActorLocation().X + DistanceBetweenEntities, GetActorLocation().Y + DistanceBetweenEntities,  GetActorLocation().Z ));
		SpawnLocations.Add(FVector(GetActorLocation().X + DistanceBetweenEntities, GetActorLocation().Y - DistanceBetweenEntities,  GetActorLocation().Z ));
	}
	else
	{
		
		// Calculate the ideal square shape based on number of boids to spawn
		const double IdealWidth = UKismetMathLibrary::Sqrt(NumberOfEntities);
		const double IdealHeight = UKismetMathLibrary::Sqrt(NumberOfEntities);

		// Calculate number of cols and rows based on ideal shape
		EntityColumns = FMath::CeilToInt32(IdealHeight);
		EntityRows = FMath::CeilToInt32(IdealWidth);
		
		// Calculate locations for boids to spawn at in world space, depending on actors placement
		int32 StartX = -(EntityColumns - 1) / 2;
		int32 EndX = (EntityColumns + 1) / 2;
		int32 StartY = -(EntityRows - 1) / 2;
		int32 EndY = (EntityRows + 1) / 2;
	
		// Spawn boids in calculated locations
		int32 Counter = NumberOfEntities;
		for (int X = StartX; X < EndX; X++)
		{
			for (int Y = StartY; Y < EndY; Y++)
			{
				if (Counter > 0)
				{
					SpawnLocations.Add(FVector( GetActorLocation() + FVector(X * DistanceBetweenEntities, Y * DistanceBetweenEntities, SpawnHeight)));
					Counter--;
				}
			}
		}
	}
}

void AFlockingBrain::SpawnEntity(const FVector &SpawnLocation, int32 ID)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AFlockingBaseActor* NewFlockingEntity = GetWorld()->SpawnActor<AFlockingBaseActor>(FlockingBaseActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	ensure (NewFlockingEntity != nullptr);
	//DrawDebugSphere(GetWorld(), NewFlockingEntity->GetActorLocation(), 80.f, 30, FColor::Blue, true,10.f);
	
	
	FFlockingActorData NewEntityData;

	// redundant
	NewEntityData.DesiredSeparationRadius = DesiredSeparationRadius;
	NewEntityData.DesiredCohesionRadius = DesiredCohesionRadius;
	NewEntityData.DesiredAlignmentRadius = DesiredAlignmentRadius;
	//NewEntityData.Location = SpawnLocation;

	
	const int32 EntityDataIndex = EntitiesFlockingData.Add(NewEntityData);
	NewEntityData.ID = EntityDataIndex;
	NewFlockingEntity->SetFlockingDataPointer(&EntitiesFlockingData[EntityDataIndex], EntityDataIndex);
	ensure (&EntitiesFlockingData[EntityDataIndex] != nullptr);

	//NewFlockingEntity->SetFlockingDataProperties( SpawnLocation, DesiredSeparationRadius, DesiredCohesionRadius, DesiredAlignmentRadius, EntityDataIndex);
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FString::Printf(TEXT("Vel loc set in brain: %f, %f, %f, ID: %i"), NewEntityData.Location.X, NewEntityData.Location.Y, NewEntityData.Location.Z, NewEntityData.ID));

	
	//DrawDebugSphere(GetWorld(), NewFlockingEntity->FlockingActorData->Location, 80.f, 30, FColor::Black, true,10.f);
	//DrawDebugSphere(GetWorld(), NewEntityData.Location, 80.f, 30, FColor::Red, true,10.f);
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FString::Printf(TEXT("Loc Data in brain %f, %f, %f, ID = %i "), NewEntityData.Location.X, NewEntityData.Location.Y, NewEntityData.Location.Z, NewEntityData.ID));
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, FString::Printf(TEXT("Desired Sep Data set in brain: %f ID: %i"), NewEntityData.DesiredSeparationRadius, NewEntityData.ID));


	Entities.Add(NewFlockingEntity);


	/*

	
	

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Entity index %i "), index));
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Entity data index %i "), EntityDataIndex));
	*/

}

namespace EntityFuncs 
{
	FVector CalculateSeparationForce(int Counter, FVector &Separation) // pekare till funktioner?? 
	{
		if (Counter > 0)
		{
			Separation /= Counter;
			Separation *= -1;
			Separation.Normalize();
			return Separation;
		}
		return FVector::ZeroVector;
	}

	FVector CalculateCohesionForce(int Counter, FVector &TotalCohesionForce, FVector &CenterOfMass, const FVector &Location)
	{
		if (Counter > 0)
		{
			CenterOfMass /= Counter;
			FVector MyTotalCohesionForce = FVector(CenterOfMass.X - Location.X, CenterOfMass.Y - Location.Y, CenterOfMass.Z - Location.Z);
			MyTotalCohesionForce.Normalize();
			return MyTotalCohesionForce;
			
		}
		return FVector::ZeroVector;
	}
	
}


void AFlockingBrain::ApplyBehaviors()
{
	int SeparationCounter = 0;
	int CohesionCounter = 0;
	FVector TotalSeparationForce = FVector::ZeroVector;
	FVector TotalCohesionForce = FVector::ZeroVector;
	FVector CenterOfMass = FVector::ZeroVector;

	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		ensure(Entities[i] != nullptr);
		Entities[i]->UpdateSteerForce(EntitiesFlockingData, Separation);
		
		for (int j = 0; j < EntitiesFlockingData.Num(); j++)
		{
			ensure(Entities[i]->FlockingActorData != nullptr);
			if (EntitiesFlockingData[i].ID != EntitiesFlockingData[j].ID)
			{
				float Distance = (EntitiesFlockingData[j].Location - EntitiesFlockingData[i].Location).Length();
				if (Distance < DesiredSeparationRadius * 2) 
				{
					TotalSeparationForce += (EntitiesFlockingData[j].Location - EntitiesFlockingData[i].Location);
					SeparationCounter++;
				}
				if (Distance < DesiredCohesionRadius * 2 && Distance > DesiredSeparationRadius * 2)
				{
					CenterOfMass += EntitiesFlockingData[j].Location;
					CohesionCounter++;
				}
			}
		}
		Separation = EntityFuncs::CalculateSeparationForce(SeparationCounter, TotalSeparationForce);
		Separation *= SeparationWeight;
		//Entities[i]->UpdateSteerForce(EntitiesFlockingData, Separation);

		if (CohesionCounter > 0)
		{
			DrawDebugSphere(GetWorld(), CenterOfMass /= CohesionCounter, 30.f, 30, FColor::Green, false, 0.2f);
		}
		
		Cohesion = EntityFuncs::CalculateCohesionForce(CohesionCounter, TotalCohesionForce, CenterOfMass, EntitiesFlockingData[i].Location);
		Cohesion *= CohesionWeight;
		
		Entities[i]->UpdateSteerForce(EntitiesFlockingData, Cohesion);
		
		//EntitiesFlockingData[i].SteerForce += Separation * 300.f;
		Separation = FVector::ZeroVector;
		Cohesion = FVector::ZeroVector;

		
	}
}

void AFlockingBrain::OnDebug(FVector &Location) const
{
	//DrawDebugSphere(GetWorld(), CurrentTargetLocation, 30.f, 30, FColor::Black, false,0.2f);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingActorData.DesiredAlignmentRadius, 30, FColor::Red, false, 0.2f);
	DrawDebugSphere(GetWorld(), Location, 30.f, 30, FColor::Green, false, 0.01f);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingActorData->DesiredSeparationRadius, 30, FColor::Green, false, 0.2f);
	

}

void AFlockingBrain::OnDebugLine(FVector &FromLocation, FVector &ToLocation) const
{
	//	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FlockingActorData.Velocity, FColor::Purple, false, 0.1f, 0, 10);
}




/*
	for (AFlockingBaseActor* Entity : Entities)
	{
		ensure(Entity != nullptr);

		if (Entity->FlockingActorData != nullptr)
		{
			Entity->UpdateSteerForce(EntitiesFlockingData, TotalSeparationForce);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("all is shit ")));
		}
		
	}

	*/
	/*

	
		
	}
	
	
	
	//EntitiesFlockingData[i].SteerForce += TotalSeparationForce;
	
	*/


	
	/*
	// funkar
	
	*/
	


	
	
	
	
	/*
	for (AFlockingBaseActor* Entity : Entities) // iterera alla structar
	{
		
		Entity->UpdateSteerForce(Entities, SeekWeight, CohesionWeight, AlignmentWeight, SeparationWeight);

		// beräkna steerforce för varje entitet
		EntityFuncs::CalculateVelocity();
	}
	*/




