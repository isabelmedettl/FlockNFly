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
			DrawDebugSphere(GetWorld(), Loc , 30.f, 30, FColor::Black, true,0.2f);

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
		EntityRows = FMath::CeilToInt32(IdealHeight);
		
		// Calculate locations for boids to spawn at in world space, depending on actors placement
		int32 StartX = -(EntityColumns - 1) / 2;
		int32 EndX = (EntityColumns + 1) / 2;
		int32 StartY = -(EntityRows - 1) / 2;
		int32 EndY = (EntityRows + 1) / 2;
	
		// Spawn boids in calculated locations
		int32 Counter = NumberOfEntities;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		for (int X = StartX; X < EndX; X++)
		{
			for (int Y = StartY; Y < EndY; Y++)
			{
				if (Counter > 0)
				{
					FVector SpawnLocation = GetActorLocation() + FVector(X * DistanceBetweenEntities, Y * DistanceBetweenEntities, SpawnHeight);
					SpawnLocations.Add(SpawnLocation);
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
	FFlockingActorData NewEntityDataRef;
	const int32 EntityDataIndex = EntitiesFlockingData.Emplace(NewEntityDataRef);
	Entities.Add(NewFlockingEntity);
	
	NewFlockingEntity->SetFlockingDataPointer(&NewEntityDataRef);
	
	
	NewEntityDataRef.DesiredSeparationRadius = DesiredSeparationRadius;
	NewEntityDataRef.DesiredCohesionRadius = DesiredCohesionRadius;
	NewEntityDataRef.DesiredAlignmentRadius = DesiredAlignmentRadius;
	NewEntityDataRef.ID = EntityDataIndex;
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
}


void AFlockingBrain::ApplyBehaviors()
{
	int SepCounter = 0;
	int CohCounter = 0;
	int AlignCounter = 0;

	FVector TotalSeparationForce = FVector::ZeroVector;

	
	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		Entities[i]->UpdateSteerForce(EntitiesFlockingData);
	}

	
	/*
	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		Entities[i]->UpdateSteerForce(EntitiesFlockingData, Separation);
		for (int j = 0; j < EntitiesFlockingData.Num(); j++)
		{
			if (EntitiesFlockingData[i]->ID != EntitiesFlockingData[j]->ID)
			{
				float Distance = (EntitiesFlockingData[i]->Location - EntitiesFlockingData[j]->Location).Length();
				if (Distance < DesiredSeparationRadius * 2) 
				{
					TotalSeparationForce += (EntitiesFlockingData[i]->Location - EntitiesFlockingData[j]->Location );
					SepCounter++;
				}
			}
		}
		Separation = EntityFuncs::CalculateSeparationForce(SepCounter, TotalSeparationForce);
		Separation *= SeparationWeight;
		Entities[i]->UpdateSteerForce(EntitiesFlockingData, Separation);
		//EntitiesFlockingData[i].SteerForce += Separation * 300.f;
		

	}

	
	*/
	
	
	//EntitiesFlockingData[i].SteerForce += TotalSeparationForce;
	
	

	
	
	/*
	for (AFlockingBaseActor* Entity : Entities) // iterera alla structar
	{
		
		Entity->UpdateSteerForce(Entities, SeekWeight, CohesionWeight, AlignmentWeight, SeparationWeight);

		// beräkna steerforce för varje entitet
		EntityFuncs::CalculateVelocity();
	}
	*/
}



