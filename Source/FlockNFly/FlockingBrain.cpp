// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBrain.h"

#include "BoidCharacter.h"
#include "FlockNFlyCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFlockingBrain::AFlockingBrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpawnArea = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
	
}

// Called when the game starts or when spawned
void AFlockingBrain::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AFlockNFlyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if( PlayerCharacter != nullptr)
	{
		BoidClass = PlayerCharacter->StaticClass();
		
		if (CalculatePossibleSpawnLocations())
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
	/*
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
	
}

bool AFlockingBrain::CalculatePossibleSpawnLocations()
{
	if (!GetWorld() || NumberOfBoids <= 0 || !BoidClass)
	{
		return false;
	}
	
	const double AngleIncrement = 360.0f / NumberOfBoids;	

	/* for (int32 i = 0; i < NumEntities; ++i)
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
	 *
	 * 
	 */
	return true;
}

