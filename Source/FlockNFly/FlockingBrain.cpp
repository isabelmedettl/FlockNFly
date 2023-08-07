// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBrain.h"

#include "BoidCharacter.h"
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
		//FlockingBaseActorClass = AFlockingBaseActor::StaticClass();
		
		if (SpawnHeight < 0.f)
		{
			SpawnHeight = GetActorLocation().Y;
		}
		
		if (NumberOfEntities > 0 && IsValid(FlockingBaseActorClass))
		{
			SpawnBoids();
		}
	}
}

// Called every frame
void AFlockingBrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFlockingBrain::SpawnBoids()
{
	if (!GetWorld()) return;
	if (NumberOfEntities != 0)
	{
		CalculatePossibleSpawnFormation();
		
		for (FVector Loc : SpawnLocations)
		{
			SpawnEntity(Loc);
		}
		
	}	
}

void AFlockingBrain::CalculatePossibleSpawnFormation()
{
	if (NumberOfEntities <= 0)
	{
		EntityRows = EntityColumns = 0;
	}

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

bool AFlockingBrain::CheckCollisionAtSpawnLocation(const FVector NewLocation)
{
	return true;
}

void AFlockingBrain::SpawnEntity(const FVector SpawnLocation)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFlockingBaseActor* NewFlockingEntity = GetWorld()->SpawnActor<AFlockingBaseActor>(FlockingBaseActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	Entities.Add(NewFlockingEntity);
}

void AFlockingBrain::ApplyBehaviors()
{
}

void AFlockingBrain::AdjustForCohesion()
{
	for (int i = 0; i < Entities.Num() - 1; i++)
	{
		if (FVector::Dist(Entities[i]->GetActorLocation(), Entities[i + 1]->GetActorLocation()))
		{
			
		}
	}
}

FVector AFlockingBrain::CalculateComputationDirection()
{
	return FVector::ZeroVector;
}

// https://www.youtube.com/watch?v=IoKfQrlQ7rA
// https://github.com/nature-of-code/noc-examples-processing/blob/master/chp06_agents/NOC_6_08_SeparationAndSeek/Vehicle.pde
// ide - weight för hur viktigt det är att följa target/ följa flocken

