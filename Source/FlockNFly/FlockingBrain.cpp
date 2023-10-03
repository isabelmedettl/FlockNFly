// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBrain.h"

#include "FlockingBaseActor.h"
#include "FlockingGrid.h"
#include "FlockNFlyCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFlockingBrain::AFlockingBrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Target1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Target1Box"));
	Target1Box->SetupAttachment(RootComponent);
	Target2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Target2Box"));
	Target2Box->SetupAttachment(RootComponent);
	Target3Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Target3Box"));
	Target3Box->SetupAttachment(RootComponent);
	Target4Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Target4Box"));
	Target4Box->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFlockingBrain::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AFlockNFlyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	// spawn boids
	if( PlayerCharacter != nullptr)
	{
		//EntityTargetLocation = PlayerCharacter->GetActorLocation();
		
		if (SpawnHeight < 0.f)
		{
			SpawnHeight = GetActorLocation().Y;
		}
		
		if (NumberOfEntities > 0 && IsValid(FlockingBaseActorClass))
		{
			SpawnBoids();
		}
		
	}
	DistanceBetweenEntities = DesiredVisionRadius - 50.f;

	GetWorldTimerManager().SetTimer(SetGridPointerHandle, this, &AFlockingBrain::SetGridPointer, 0.1, false);

	if (bTimerTarget)
	{
		Target1 = Target1Box->GetComponentLocation();
		Target2 = Target2Box->GetComponentLocation();
		Target3 = Target3Box->GetComponentLocation();
		Target4 = Target4Box->GetComponentLocation();
		
		GetWorldTimerManager().SetTimer(ChangeTargetTimerHandle, this, &AFlockingBrain::ChangeTarget, ChangeTargetDelay, true, 0.1 );
	}

}

// Called every frame
void AFlockingBrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bHasAssignedBoids)
	{
		for (int i = 0; i < EntitiesFlockingData.Num(); i++)
		{
			Entities[i]->SetFlockingDataPointer(EntitiesFlockingData[i]);
		}
		bHasAssignedBoids = true;
	}

	if (!bIsGridSet) return;
		
	UpdateTimerTarget();
	DrawDebugSphere(GetWorld(), EntityTargetLocation, 30.f, 40, FColor::Red, false, 0.1, 1, 0);
	FlockingGrid->TargetLocation = EntityTargetLocation;

	if (!bUseAStarPathfinding)
	{
		for (int i = 0 ; i <EntitiesFlockingData.Num(); i++)
		{
			EntitiesFlockingData[i].SteerForce = CalculateSteerForce(i);
			CalculateNewVelocity(i);
			Entities[i]->UpdateLocation(DeltaTime);
			//Here you could call for collision avoidance checks and methods (aka tracing collision checks and calculation, not pathfinding)
		}
	}
	else
	{
		for (int i = 0 ; i <EntitiesFlockingData.Num(); i++)
		{
			
			//EntitiesFlockingData[i].Location = FlockingGrid->StartLocation;
			DecideTargetFindingMethod(i);
		}
		
	}

	

	if (!bTimerTarget)
	{
		EntityTargetLocation = PlayerCharacter->CurrentTargetLocation;
	}
	else
	{
		PlayerCharacter->CurrentTargetLocation = EntityTargetLocation;
	}
}

void AFlockingBrain::ChangeTarget() 
{
	bIsTimeToSwitchTarget = !bIsTimeToSwitchTarget;
}

void AFlockingBrain::UpdateTimerTarget()
{
	// Check if it's time to switch to the next target
	if (bIsTimeToSwitchTarget)
	{
		CurrentTargetIndex = (CurrentTargetIndex + 1) % 4;
		switch (CurrentTargetIndex)
		{
		case 0:
			EntityTargetLocation = Target1;
			break;
		case 1:
			EntityTargetLocation = Target2;
			break;
		case 2:
			EntityTargetLocation = Target3;
			break;
		case 3:
			EntityTargetLocation = Target4;
			break;

		default:
			EntityTargetLocation = Target1;
		}
		bIsTimeToSwitchTarget = false;
	}
}

void AFlockingBrain::SetGridPointer()
{
	// Assign Grid if not already assigned 
	if(!FlockingGrid)
		FlockingGrid = Cast<AFlockingGrid>(UGameplayStatics::GetActorOfClass(this, AFlockingGrid::StaticClass()));

	//FlockingGrid = PlayerCharacter->FlockingGrid;
	ensure (FlockingGrid != nullptr);
	FlockingGrid->TargetLocation = EntityTargetLocation;
	bIsGridSet = true;
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
	
	/** Calculates seeking force to target by calculating vector resulting from subtracting the desired position from the current position. The result is the appropriate velocity
	 * Not used when using pathfinding*/
	FVector CalculateSeekForce (const FVector &CurrentTargetLocation, const FVector &EntityLocation,
		const FVector &EntityVelocity, const float EntityMaxSpeed, const float EntityMaxForce, const float DesiredRadiusToTarget)
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
		
		else // makes entities move slower and almost stopping when reaching their target
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
	// these can be used if you want a wobbly movement when boids come close to target, where the entities dont stop but paonce around the target. To use - remove first two lines below out-commented code.
	const float NewSpeed = EntityFlockingFunctions::CalculateSpeed(EntitiesFlockingData[IndexOfData].TargetLocation, EntitiesFlockingData[IndexOfData].Location, EntitiesFlockingData[IndexOfData].MaxSpeed, DesiredVisionRadius);
	const float OldSpeed = EntitiesFlockingData[IndexOfData].CurrentSpeed;
	EntitiesFlockingData[IndexOfData].CurrentSpeed = FMath::Lerp(OldSpeed, NewSpeed, 0.3);
	
	//const float OldSpeed = EntitiesFlockingData[IndexOfData].CurrentSpeed;
	//EntitiesFlockingData[IndexOfData].CurrentSpeed = FMath::Lerp(OldSpeed, MaxSpeed, 0.1);
	EntitiesFlockingData[IndexOfData].Acceleration = EntitiesFlockingData[IndexOfData].SteerForce / EntitiesFlockingData[IndexOfData].Mass;
	EntitiesFlockingData[IndexOfData].Velocity += EntitiesFlockingData[IndexOfData].Acceleration;
	EntitiesFlockingData[IndexOfData].Velocity = EntitiesFlockingData[IndexOfData].Velocity.GetClampedToMaxSize(EntitiesFlockingData[IndexOfData].CurrentSpeed);
	
}

bool AFlockingBrain::IsWithinFieldOfView(float AngleToView, const FVector &EntityLocation, const FVector &EntityVelocity, const FVector &Direction) const
{
	// Calculate the angle between the actor's forward vector and the direction vector to the target
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(EntityVelocity.GetSafeNormal(),Direction.GetSafeNormal())));

	/*
	if (bDebug)
	{
		FVector ConeOrigin = EntityLocation;
		FVector ConeDirection = EntityVelocity.GetSafeNormal();
		float ConeLength = DesiredVisionRadius * 2;
		float ConeAngle = FMath::DegreesToRadians(AngleToView);

		DrawDebugCone(GetWorld(), ConeOrigin, ConeDirection, ConeLength, ConeAngle, ConeAngle, 30, FColor::Red, false, 0.1, 0, 4);
		DrawDebugSphere(GetWorld(), EntityLocation, DesiredVisionRadius * 2, 15, FColor::Cyan, false, 0.1, 0, 1);
	}
	*/
	
	// Check if the target is within the specified angle and radius
	if (AngleDegrees <= AngleToView/ 2 && Direction.Size() <= DesiredVisionRadius * 2 )
	{
		return true;
	}
	return false;
}

bool AFlockingBrain::CollisionOnPathToTarget(int Index)
{
	FHitResult HitResult;
	FVector Origin, Extent;
	GetActorBounds(true, Origin, Extent);
	
	// Define the object types to trace against
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

	TArray<AActor*> IgnoreActors = TArray<AActor*>();
	IgnoreActors.Add(this);

	const FVector Direction = (EntityTargetLocation - EntitiesFlockingData[Index].Location).GetSafeNormal();
	const FVector TraceLocation = EntitiesFlockingData[Index].Location + Direction * DesiredVisionRadiusToTarget;
	
	bool bHit;
	if (bDebug)
	{
		bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), EntitiesFlockingData[Index].Location,
		TraceLocation, TraceRadius , ObjectTypes, true, IgnoreActors, EDrawDebugTrace::ForDuration,
		HitResult, true, FColor::Green, FLinearColor::Blue,  0.1f);
	}
	else
	{
		bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), EntitiesFlockingData[Index].Location,
		TraceLocation, TraceRadius , ObjectTypes, true, IgnoreActors, EDrawDebugTrace::None,
		HitResult, true, FColor::Transparent, FLinearColor::Transparent,  0.1f);
	}
	if (bHit)
	{
		FoundObstacle = HitResult;
		//HitResult.GetActor()->Destroy();
		//DrawDebugSphere(GetWorld(), TraceLocation, 50.f, 12, FColor::Blue, true, 0.1, 0, 1);

		return true;
	}
	return false;
	
}

FVector AFlockingBrain::CalculateCollisionAvoidanceForce(int Index)
{
	// Vector A, Difference between entities position and  obstacle's position
	const FVector Difference = FoundObstacle.ImpactPoint - EntitiesFlockingData[Index].Location;
	// Vector p, Projection vector of difference and entities curr direction
	const FVector Projection  = FVector::DotProduct(Difference, EntitiesFlockingData[Index].Velocity) * EntitiesFlockingData[Index].Velocity.GetSafeNormal(); // direction

	// Vector B
	const FVector DifferenceFromProjection = Difference - Projection;

	// Test intersection, Check if the magnitude projection is less than the magnitude of vector v. If this condition is met, it indicates that the obstacle is in the direction of entities movement
	//	Check if the magnitude of vector b is less than the approximate radius of obstacle
	const float ProjectionMagnitude = Projection.Length();
	const float HeadingMagnitude = EntitiesFlockingData[Index].Velocity.Length();
	
	// If true = entity is within the proximity of the obstacle
	if (ProjectionMagnitude < HeadingMagnitude && DifferenceFromProjection.Length() < DesiredVisionRadiusToTarget)
	{
		// Collision is impending, calculate corrective steering
		const FVector CollisionAvoidanceForce = (HeadingMagnitude  / Difference.Size()) * EntitiesFlockingData[Index].MaxForce * EntitiesFlockingData[Index].Velocity;
		DrawDebugLine(GetWorld(), EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Location + CollisionAvoidanceForce, FColor::Blue, true, 0.1, 0, 1);

		return CollisionAvoidanceForce;
	}
	return FVector::ZeroVector;
}

/** Calculates path following force when using flow field pathfinding, by getting direction of current node and then calculating a force that steers the entity towards the flow field direction */
// Improvements: adjust the force magnitude based on the entity's proximity to the path. For example, if the entity is close to the path, reduce the magnitude of PathFollowingForce.
// Could be done by calculating the distance to the nearest point on the path, adjusting the scaling factor based on the distance to the path eg FMath::Lerp or any other curve to define how the scaling factor changes with distance
FVector AFlockingBrain::CalculatePathFollowingForce(int Index)
{
	ensure (FlockingGrid != nullptr);
	FVector PathFollowingForce;

	// Check if the entity is inside the flow field
	const FVector CurrentLocation = EntitiesFlockingData[Index].Location;
	// Get the direction from the flow field
	const FVector MoveDir = FlockingGrid->GetDirectionBasedOnWorldLocation(CurrentLocation).GetSafeNormal();
	const float DistanceToTarget = (EntityTargetLocation - CurrentLocation).Length(); 
	
	if (DistanceToTarget >= DesiredRadiusToTarget)
	{
		const FVector Desired = MoveDir * EntitiesFlockingData[Index].MaxSpeed; 
		PathFollowingForce = Desired - EntitiesFlockingData[Index].Velocity;
		PathFollowingForce /= EntitiesFlockingData[Index].MaxSpeed;
		PathFollowingForce *= EntitiesFlockingData[Index].MaxForce;
	}
		
	else // makes entities move slower and almost stopping when reaching their target
	{
		float RampedSpeed = EntitiesFlockingData[Index].MaxSpeed * (DistanceToTarget / DesiredRadiusToTarget);
		float ClippedSpeed = FMath::Min(RampedSpeed, EntitiesFlockingData[Index].MaxSpeed);
			
		const FVector Desired = MoveDir * ClippedSpeed; 
		PathFollowingForce = Desired - EntitiesFlockingData[Index].Velocity;
	}

	return PathFollowingForce;
}

FVector AFlockingBrain::DecideTargetFindingMethod(int Index)
{
	FVector TargetFindingForce;
	if (bUseFlowFieldPathfinding)
	{
		bUseAStarPathfinding = false;
		FlockingGrid->bUseFlowFillAlgorithm = true;
		FlockingGrid->bUseAStarAlgorithm = false;
		TargetFindingForce = CalculatePathFollowingForce(Index);
	}
	else if (bUseAStarPathfinding)
	{
		bUseFlowFieldPathfinding = false;
		FlockingGrid->bUseFlowFillAlgorithm = false;
		FlockingGrid->bUseAStarAlgorithm = true;
		FlockingGrid->StartLocation = EntitiesFlockingData[Index].Location;
		FlockingGrid->TargetLocation = EntityTargetLocation;


		//TargetFindingForce = EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[Index].TargetLocation, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, EntitiesFlockingData[Index].MaxSpeed, EntitiesFlockingData[Index].MaxForce, DesiredVisionRadius);

		// add a*pathfinding method;
		//TargetFindingForce = EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[Index].TargetLocation, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, EntitiesFlockingData[Index].MaxSpeed, EntitiesFlockingData[Index].MaxForce, DesiredVisionRadius);

		
	}
	else
	{
		FlockingGrid->bUseFlowFillAlgorithm = false;
		FlockingGrid->bUseAStarAlgorithm= false;
		TargetFindingForce = EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[Index].TargetLocation, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, EntitiesFlockingData[Index].MaxSpeed, EntitiesFlockingData[Index].MaxForce, DesiredVisionRadius);
	}
	return TargetFindingForce;
}



FVector AFlockingBrain::CalculateSteerForce(const int Index)
{
	int Counter = 0;
	FVector TotalSeparationForce = FVector::ZeroVector;
	FVector CenterOfMass = FVector::ZeroVector;
	FVector TotalVelocity = FVector::ZeroVector;

	ensure(Entities[Index] != nullptr);
		
	EntitiesFlockingData[Index].TargetLocation = EntityTargetLocation;
	EntitiesFlockingData[Index].bIsLeader = false;

	int LeaderIndex = Index;
	const FVector CurrentSeekForce = DecideTargetFindingMethod(Index);
	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		// here you can add and check if entity is leader, and then make path-following or seeking-target calculations only for that entity
		// would be more effective performance-wise, but entities will risk collision with objects
		if (EntitiesFlockingData[Index].DistanceToTarget > EntitiesFlockingData[i].DistanceToTarget) LeaderIndex = i;
		
		ensure(Entities[Index]->FlockingActorData != nullptr);
		if (EntitiesFlockingData[Index].ID == EntitiesFlockingData[i].ID) { continue; }
		const FVector DirectionToNeighbour = (EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location);

		if (IsWithinFieldOfView(NeighbourFieldOfViewAngle, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, DirectionToNeighbour) /* narrow view to see neighbours to current target location*/)
		{		 
			if (DirectionToNeighbour.Length() < DesiredVisionRadius * 2) 
			{
				TotalSeparationForce += EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location;
				CenterOfMass += EntitiesFlockingData[i].Location;
				TotalVelocity += EntitiesFlockingData[i].Velocity;
				Counter++;
			}
		}
		EntitiesFlockingData[i].bIsLeader = false;
	}
	EntitiesFlockingData[LeaderIndex].bIsLeader = true;
	FlockingGrid->StartLocation = EntitiesFlockingData[LeaderIndex].Location;
	EntitiesFlockingData[Index].NumNeighbours = Counter;

	Separation = EntityFlockingFunctions::CalculateSeparationForce(Counter, TotalSeparationForce);
	Separation *= SeparationWeight;

	Cohesion = EntityFlockingFunctions::CalculateCohesionForce(Counter, CenterOfMass, EntitiesFlockingData[Index].Location); 
	Cohesion *= CohesionWeight;
	
	Alignment = EntityFlockingFunctions::CalculateAlignmentForce(Counter,TotalVelocity);
	Alignment *= AlignmentWeight;
	
	FVector TotalForce = CurrentSeekForce + Cohesion + Alignment + Separation * SeparationMultiplyer;
	
	return TotalForce;
}

/* Ta reda på vad som ska beräknas, skicka en request, dela upp beräkningar över frames
 *
 * en är master, resten får en relativ position till mastern.
 * kommer alla requesta path hela tiden? Justera path så de inte förlorar flockingbeteende.
 *
 * (kanske den i mitten av flocken? Googla)
 */










		