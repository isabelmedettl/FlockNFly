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

void AFlockingBrain::SpawnEntity(const FVector &SpawnLocation, int32 ID)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFlockingBaseActor* NewFlockingEntity = GetWorld()->SpawnActor<AFlockingBaseActor>(FlockingBaseActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	NewFlockingEntity->FlockingData.ID = ID;
	Entities.Add(NewFlockingEntity);
	// TODO: pointer to collection of entities ? Fråga Mikael pga känns onödigt och brain bör hantera allt själv men man måste räkna på medelvärdet?? Svårt
}



void AFlockingBrain::ApplyBehaviors()
{
	for (AFlockingBaseActor* Entity : Entities)
	{
		Entity->UpdateFlocking(Entities, CohesionWeight, AlignmentWeight, SeparationWeight);
		// TODO: Skicka ref till samlingen av pointers - najs eller bajs? Kanske iterera och samla sammanlagda vectorer och counts, skicka de till alla entiterer istället för att de ska göra jobbet?
		// TODO: Just nu är det dubbla loopar, borde gå att se till att alla 3 flocking-lagar inte behöver uppdateras hela tiden, redundant att göra alla checkar?
	}
}


