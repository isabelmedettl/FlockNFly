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
		FlockingBaseActorClass = AFlockingBaseActor::StaticClass();
		
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
		
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Prepare to spawn ")));

		for (FVector Location : SpawnLocations)
		{
			SpawnEntity(Location);
		}
	}	
}

void AFlockingBrain::CalculatePossibleSpawnFormation()
{
	if (NumberOfEntities <= 0)
	{
		EntityRows = EntityColumns = 0;
	}

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Calculating spawn formation ")));
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
	for (int X = StartX; X < EndX; X++)
	{
		for (int Y = StartY; Y < EndY; Y++)
		{
			FVector SpawnLocation = FVector::ZeroVector;
			if (Counter > 0)
			{
				SpawnLocation = GetActorLocation() + FVector(X * DistanceBetweenEntities, Y * DistanceBetweenEntities, SpawnHeight);
				DrawDebugSphere(GetWorld(), SpawnLocation, 30.f, 30, FColor::Black, false,6.f);
				Counter--;
			}
			if (SpawnLocation != FVector::ZeroVector)
			{
				SpawnLocations.Add(SpawnLocation);
			}
		}
	}
	
}

bool AFlockingBrain::CheckCollisionAtSpawnLocation(const FVector NewLocation)
{
	return true;
}

void AFlockingBrain::SpawnEntity(FVector SpawnLocation)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFlockingBaseActor* NewFlockingEntity = GetWorld()->SpawnActor<AFlockingBaseActor>(FlockingBaseActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	Entities.Add(NewFlockingEntity);
}

/*
	for (int32 i = 0; i < NumberOfBoids; ++i)
	{
		float AngleInDegrees = i * AngleIncrement;
		float AngleInRadians = FMath::DegreesToRadians(AngleInDegrees);
		FVector SpawnLocation = Center + FVector(FMath::Cos(AngleInRadians), FMath::Sin(AngleInRadians), 0) * Radius;

		// Check for overlap at the potential spawn location
		FCollisionShape CollisionShape;
		CollisionShape.SetSphere(EntitySize);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(nullptr); // Ignore any specific actors if needed

		bool bCanSpawn = !World->OverlapAnyTestByChannel(SpawnLocation, FQuat::Identity, ECC_Pawn, CollisionShape, QueryParams);

		if (bCanSpawn)
		{
			AYourBoidActor* NewBoid = World->SpawnActor<YourBoidActor>(BoidClass, SpawnLocation, FRotator::ZeroRotator);

			// Customize any boid-specific properties if needed
			// e.g., NewBoid->SetMaxSpeed(100.0f);
		}
		else
		{
			// Handle overlapping spawn location, e.g., adjust the position or skip spawning this boid.
			// You could try offsetting the SpawnLocation, decreasing NumEntities, or using a different formation.
			Return false
		}
	}


	MITT

	
bool AFlockingBrain::CalculatePossibleSpawnFormation()
{
	if (!GetWorld() || NumberOfBoids <= 0 || !BoidClass)
	{
		return false;
	}
	
	const double AngleIncrement = 360.0f / NumberOfBoids;

	// calculating positions in world space by evenly dividing specified circle based on number of boids to be spawned
	FVector SpawnLocation = FVector::ZeroVector;
	for (int32 i = 0; i < NumberOfBoids; i++)
	{
		double AngleDegrees = i * AngleIncrement;
		double AngleRadians = FMath::DegreesToRadians(AngleDegrees);
		SpawnLocation = SpawnArea->GetComponentLocation() + FVector(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0) * SpawnArea->GetScaledSphereRadius();
		SpawnLocations.Add(SpawnLocation);
	}
	
	return true;
}
	 *
	 *
	 *
	 *
* /*
	* UWorld* World = GetWorld(); // Get a reference to the current world

	if (!World)
	{
		return;
	}

	// Assuming you have a reference to your boid actor class (YourBoidActor)
	TSubclassOf<YourBoidActor> BoidClass; 

	TArray<FVector> FormationPositions = CalculateCircularFormationPositions(Center, Radius, NumEntities);

	for (const FVector& SpawnPosition : FormationPositions)
	{
		AYourBoidActor* NewBoid = World->SpawnActor<YourBoidActor>(BoidClass, SpawnPosition, FRotator::ZeroRotator);
		
		// Customize any boid-specific properties if needed
		// e.g., NewBoid->SetMaxSpeed(100.0f);
	}
	 */
	 
