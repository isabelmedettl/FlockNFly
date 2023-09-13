// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBrain.h"

#include "FlockingBaseActor.h"
#include "FlockNFlyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "FF_Grid.h"

// Sets default values
AFlockingBrain::AFlockingBrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//FFGrid Grid = FFGrid();
	//GridInfo = Grid;
}

// Called when the game starts or when spawned
void AFlockingBrain::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AFlockNFlyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if( PlayerCharacter != nullptr)
	{
		EntityTargetLocation = PlayerCharacter->GetActorLocation();
		
		if (SpawnHeight < 0.f)
		{
			SpawnHeight = GetActorLocation().Y;
		}
		
		if (NumberOfEntities > 0 && IsValid(FlockingBaseActorClass))
		{
			SpawnBoids();
		}
	}

	//GridInfo.SizeX = 3000;
	//GridInfo.SizeY = 3000;
	//GridInfo.SizeZ = 3000;

	//GridInfo.CalculateGrid(GetActorLocation(), 3000, 3000, 3000, 5);
	//UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f"), GridInfo.TopLeft.X, GridInfo.TopLeft.Y, GridInfo.TopLeft.Z);

	DistanceBetweenEntities = DesiredVisionRadius - 50.f;
}

// Called every frame
void AFlockingBrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	EntityTargetLocation = PlayerCharacter->CurrentTargetLocation;
	if (!bHasAssignedBoids)
	{
		for (int i = 0; i < EntitiesFlockingData.Num(); i++)
		{
			Entities[i]->SetFlockingDataPointer(EntitiesFlockingData[i]);
		}
		bHasAssignedBoids = true;
	}

	//float ClosestDistanceToPlayer = 10000000.f;
	//int IndexOfLeader = 1;
	for (int i = 0 ; i <EntitiesFlockingData.Num(); i++)
	{
		//EntitiesFlockingData[i].bIsLeader = false;
		//const FVector Distance = EntityTargetLocation - EntitiesFlockingData[i].Location;
		//if (Distance.Length() < ClosestDistanceToPlayer /*&& HasClearViewOfTarget(EntitiesFlockingData[i].Location, i, Distance)*/)
		//{
			//ClosestDistanceToPlayer = Distance.Length();
			//EntitiesFlockingData[i].bIsLeader = true;
		//}
		
		EntitiesFlockingData[i].SteerForce = CalculateSteerForce(i);
		CalculateNewVelocity(i);
		Entities[i]->UpdateLocation(DeltaTime);
		if (EntitiesFlockingData[i].bIsLeader)
		{
			// Collision checks
		}
	}
}

void AFlockingBrain::SpawnBoids()
{
	if (!GetWorld()) return;
	if (NumberOfEntities != 0)
	{		
		CalculatePossibleSpawnFormation();
		
		int Counter = 0;
		for (FVector Loc : SpawnLocations)
		{
			SpawnEntity(Loc, Counter);
			Counter++;
		}
	}
	
	//DrawDebugSphere(GetWorld(), GridInfo.TopLeft, 50.f, 12, FColor::Green, true, 3, 0, 1);
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
		EntityColumns = FMath::CeilToInt(IdealHeight);
		EntityRows = FMath::CeilToInt(IdealWidth);
		
		// Calculate locations for boids to spawn at in world space, depending on actors placement
		int StartX = -(EntityColumns - 1) / 2;
		int EndX = (EntityColumns + 1) / 2;
		int StartY = -(EntityRows - 1) / 2;
		int EndY = (EntityRows + 1) / 2;
		
		// Spawn boids in calculated locations
		int Counter = NumberOfEntities;
		
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

void AFlockingBrain::SpawnEntity(const FVector &SpawnLocation, int ID)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AFlockingBaseActor* NewFlockingEntity = GetWorld()->SpawnActor<AFlockingBaseActor>(FlockingBaseActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	ensure (NewFlockingEntity != nullptr);
	
	// create struct with flocking data, set variables in struct, assign pointer in spawned entity to that struct in array and add both entity and struct in collections
	FFlockingActorData NewEntityData;
		
	const int EntityDataIndex = EntitiesFlockingData.Add(NewEntityData); // arrayen får en kopia, här har jag 2 kopior - en i metoden, en i arrayen
	FFlockingActorData &MyFD = EntitiesFlockingData[EntityDataIndex]; 
	
	MyFD.ID = EntityDataIndex;
	MyFD.TargetLocation = EntityTargetLocation;
	MyFD.Mass = Mass;
	MyFD.MaxForce =MaxForce;
	MyFD.MaxSpeed = MaxSpeed;
	
	NewFlockingEntity->SetFlockingDataPointer(EntitiesFlockingData[EntityDataIndex]); 
	ensure (&EntitiesFlockingData[EntityDataIndex] != nullptr);
	
	Entities.Add(NewFlockingEntity);
}


namespace EntityFlockingFunctions
{
	/** Calculates speed depending on how close entity is to current target location*/
	float CalculateSpeed(const FVector &CurrentTargetLocation, const FVector &EntityLocation, const float EntityMaxSpeed, const float DesiredRadiusToTarget)
	{
		const FVector Distance = CurrentTargetLocation - EntityLocation;
		if (Distance.Length() <= DesiredRadiusToTarget)
		{
			const float RampedSpeed = EntityMaxSpeed * (Distance.Length() / DesiredRadiusToTarget);
			const float ClippedSpeed = FMath::Min(RampedSpeed, EntityMaxSpeed);
			return ClippedSpeed;
		}
		return EntityMaxSpeed;
	}
	
	/** Calculates vector resulting from subtracting the desired position from the current position. The result is the appropriate velocity */
	FVector CalculateSeekForce (const FVector &CurrentTargetLocation, const FVector &EntityLocation, const FVector &EntityVelocity, const float EntityMaxSpeed, const float EntityMaxForce, const float DesiredRadiusToTarget)
	{
		const FVector Distance = CurrentTargetLocation - EntityLocation;
		FVector NewSteeringForce;

		if (Distance.Length() >= DesiredRadiusToTarget)
		{
			const FVector Desired = (Distance).GetSafeNormal() * EntityMaxSpeed; 
			NewSteeringForce = Desired - EntityVelocity;
			NewSteeringForce /= EntityMaxSpeed;
			NewSteeringForce *= EntityMaxForce;
		}
		
		else // redundant kankse?? Testa
		{
			float RampedSpeed = EntityMaxSpeed * (Distance.Length() / DesiredRadiusToTarget);
			float ClippedSpeed = FMath::Min(RampedSpeed, EntityMaxSpeed);
			
			const FVector Desired = (Distance).GetSafeNormal() * ClippedSpeed; 
			NewSteeringForce = Desired - EntityVelocity;
		}
		return NewSteeringForce;
	}

	/** Calculates entities to steer away from any neighbor that is within view and within a prescribed minimum separation distance*/
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

	/** Calculates entities position to correspond with average alignment of nearby entities, taking the position of entities within certain radius and steers entity towards the average position of those entites*/
	FVector CalculateCohesionForce(int Counter,  FVector &CenterOfMass, const FVector &Location) //totalcohesionforce kan vara const, gör kopia på den och skicka tillbaka den, bool/void istället
	{
		if (Counter > 0)
		{
			CenterOfMass /= Counter;
			FVector TotalCohesionForce = FVector(CenterOfMass.X - Location.X, CenterOfMass.Y - Location.Y, CenterOfMass.Z - Location.Z);
			TotalCohesionForce.Normalize();
			return TotalCohesionForce; //returnerar kopia
		}
		return FVector::ZeroVector;
	}

	/** Attempts to match the velocity of other entities inside this entity´s visible range by adding neighbours velocity to computation vector*/
	FVector CalculateAlignmentForce(int Counter, FVector &TotalAlignmentForce)
	{
		if (Counter > 0)
		{
			TotalAlignmentForce /= Counter;
			TotalAlignmentForce.Normalize();
			return TotalAlignmentForce;
		}
		return FVector::ZeroVector;
	}
}


void AFlockingBrain::CalculateNewVelocity(const int IndexOfData)
{
	EntitiesFlockingData[IndexOfData].CurrentSpeed = EntityFlockingFunctions::CalculateSpeed(EntitiesFlockingData[IndexOfData].TargetLocation, EntitiesFlockingData[IndexOfData].Location, EntitiesFlockingData[IndexOfData].MaxSpeed, DesiredVisionRadius);
	EntitiesFlockingData[IndexOfData].Acceleration = EntitiesFlockingData[IndexOfData].SteerForce / EntitiesFlockingData[IndexOfData].Mass;
	EntitiesFlockingData[IndexOfData].Velocity += EntitiesFlockingData[IndexOfData].Acceleration;
	EntitiesFlockingData[IndexOfData].Velocity = EntitiesFlockingData[IndexOfData].Velocity.GetClampedToMaxSize(EntitiesFlockingData[IndexOfData].MaxSpeed);
}

bool AFlockingBrain::IsWithinFieldOfView(float AngleToView, const FVector &EntityLocation, const int EntityIndex, const FVector &Direction)
{
	// Calculate the angle between the actor's forward vector and the direction vector to the target
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Direction.GetSafeNormal(), Entities[EntityIndex]->GetActorForwardVector())));

	/*
	if (EntitiesFlockingData[EntityIndex].bIsLeader)
	{
		DrawDebugCone(GetWorld(), EntityLocation, Direction, Direction.Size(),
		FMath::Acos(FVector::DotProduct(Direction.GetSafeNormal(), Entities[EntityIndex]->GetActorForwardVector())),
		FMath::Acos(FVector::DotProduct(Direction.GetSafeNormal(), Entities[EntityIndex]->GetActorForwardVector())),
		30, FColor::Red, false, 0.4, 0, 1 );
	}
	*/
	

	// Check if the target is within the specified angle and radius
	if (AngleDegrees <= AngleToView/ 2 && Direction.Size() <= DesiredRadiusToTarget )
	{
		return true;
	}
	return false;
}

void AFlockingBrain::CalculateLeader()
{
	int Index = 0;
	for (int i = 0; i < EntitiesFlockingData.Num() - 1; i++)
	{
		if (EntitiesFlockingData[i].DistanceToTarget < EntitiesFlockingData[i+1].DistanceToTarget)
		{
			Index = i;
		}else
		{
			Index = i+1;
		}
	}

	EntitiesFlockingData[Index].bIsLeader = true;
}


FVector AFlockingBrain::CalculateSteerForce(const int Index)
{
	int Counter = 0;
	FVector TotalSeparationForce = FVector::ZeroVector;
	FVector CenterOfMass = FVector::ZeroVector;
	FVector CurrentSeekForce = FVector::ZeroVector;
	FVector TotalForce = FVector::ZeroVector;

	ensure(Entities[Index] != nullptr);
		
	EntitiesFlockingData[Index].TargetLocation = EntityTargetLocation;
	
	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		if (EntitiesFlockingData[Index].ID == EntitiesFlockingData[i].ID) { continue; }
		const FVector DirectionToNeighbour = (EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location);

		//EntitiesFlockingData[Index].bIsLeader = (EntitiesFlockingData[i].DistanceToTarget >= EntitiesFlockingData[Index].DistanceToTarget) ? true : false;
		//EntitiesFlockingData[i].bIsLeader = (EntitiesFlockingData[i].DistanceToTarget >= EntitiesFlockingData[Index].DistanceToTarget) ? false : true;
		if (EntitiesFlockingData[i].bIsLeader)
		{
			CurrentSeekForce = EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[Index].TargetLocation, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, EntitiesFlockingData[Index].MaxSpeed, EntitiesFlockingData[Index].MaxForce, DesiredVisionRadius);
		}
		
		Entities[i]->UpdateSteerForce(CurrentSeekForce);
		ensure(Entities[Index]->FlockingActorData != nullptr);

		if (IsWithinFieldOfView(NeighbourFieldOfViewAngle, EntitiesFlockingData[Index].Location, Index, DirectionToNeighbour) /* narrow view to see neighbours to current target location*/)
		{
			if (DirectionToNeighbour.Length() < DesiredVisionRadius * 2) 
			{
				TotalSeparationForce += EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location;
				Cohesion += EntitiesFlockingData[Index].Location;
				Alignment += EntitiesFlockingData[i].Velocity;
				Counter++;
			}
		}
		
			/* SENAST SOM FUNKADE
		if (EntitiesFlockingData[i].bIsLeader)
		{
			CurrentSeekForce = EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[Index].TargetLocation, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, EntitiesFlockingData[Index].MaxSpeed, EntitiesFlockingData[Index].MaxForce, DesiredVisionRadius);
		}
		Entities[i]->UpdateSteerForce(CurrentSeekForce);
		ensure(Entities[Index]->FlockingActorData != nullptr);
		if (EntitiesFlockingData[Index].ID == EntitiesFlockingData[i].ID) { continue; }
		float Distance = (EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location).Length();
		//DrawDebugSphere(GetWorld(), EntitiesFlockingData[i].Location , DesiredVisionRadius, 40, FColor::Green, false, 0.5, 0, 1);

		if (Distance < DesiredVisionRadius * 2) 
		{
			TotalSeparationForce += EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location;
			Cohesion += EntitiesFlockingData[Index].Location;
			Alignment += EntitiesFlockingData[i].Velocity;
			Counter++;
		}
			*/
	}
	EntitiesFlockingData[Index].NumNeighbours = Counter;
	Separation = EntityFlockingFunctions::CalculateSeparationForce(Counter, TotalSeparationForce); // spara ner i strukten
	Separation *= SeparationWeight;

	Cohesion = EntityFlockingFunctions::CalculateCohesionForce(Counter, CenterOfMass, EntitiesFlockingData[Index].Location); // skicka ref till total cohesion force
	Cohesion *= CohesionWeight;
	
	Alignment = EntityFlockingFunctions::CalculateAlignmentForce(Counter,Alignment);
	Alignment *= AlignmentWeight;
	
	
	//TotalForce += CurrentSeekForce + Separation * 20 + Cohesion + Alignment;
	TotalForce = CurrentSeekForce + Cohesion + Alignment + Separation * SeparationMultiplyer;
	
	return TotalForce;
}

/* Ta reda på vad som ska beräknas, skicka en request, dela upp beräkningar över frames
 *
 * en är master, resten får en relativ position till mastern.
 * kommer alla requesta path hela tiden? Justera path så de inte förlorar flockingbeteende.
 *
 * (kanske den i mitten av flocken? Googla)
 */










		