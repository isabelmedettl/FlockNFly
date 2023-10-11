// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingBrain.h"

#include "FlockingBaseActor.h"
#include "FlockingGrid.h"
#include "FlockNFlyCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EntityFlockingFunctions.h"

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

	if (!bIsGridSet)
	{
		return;
	}
	
	if (!FlockingGrid->bAllNodesAdded)
	{
		return;
	}
		
	UpdateTimerTarget();
	DrawDebugSphere(GetWorld(), EntityTargetLocation, 30.f, 40, FColor::Red, false, 0.1, 1, 0);
	FlockingGrid->TargetLocation = EntityTargetLocation;

	for (int i = 0 ; i <EntitiesFlockingData.Num(); i++)
	{
		EntitiesFlockingData[i].SteerForce = CalculateSteerForce(i);
		CalculateNewVelocity(i);
		Entities[i]->UpdateLocation(DeltaTime);
		//Here you could call for collision avoidance checks and methods (aka tracing collision checks and calculation, not pathfinding)
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
	{
		FlockingGrid = Cast<AFlockingGrid>(UGameplayStatics::GetActorOfClass(this, AFlockingGrid::StaticClass()));
	}

	//FlockingGrid = PlayerCharacter->FlockingGrid;
	ensure (FlockingGrid != nullptr);
	FlockingGrid->TargetLocation = EntityTargetLocation;
	bIsGridSet = true;
	// if no waypoint radius is set - set to half node radius
	if (WaypointReachedRadius <= 0)
	{
		WaypointReachedRadius = FlockingGrid->GetNodeRadius() / 2;
	}
}

void AFlockingBrain::SpawnBoids()
{
	if (GetWorld() == nullptr)
	{
		return;
	}
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
	
	if (bDebug && bDebugVision)
	{
		FVector ConeOrigin = EntityLocation;
		FVector ConeDirection = EntityVelocity.GetSafeNormal();
		float ConeLength = DesiredVisionRadius * 2;
		float ConeAngle = FMath::DegreesToRadians(AngleToView);

		DrawDebugCone(GetWorld(), ConeOrigin, ConeDirection, ConeLength, ConeAngle, ConeAngle, 30, FColor::Red, false, 0.1, 0, 4);
		DrawDebugSphere(GetWorld(), EntityLocation, DesiredVisionRadius * 2, 15, FColor::Cyan, false, 0.1, 0, 1);
	}
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

FVector AFlockingBrain::CalculateForceBasedOnDistance(const int Index, const float Radius, const FVector& CurrentPoint, const FVector& DirectionToPoint)
{
	const float DistanceToTarget = (CurrentPoint - EntitiesFlockingData[Index].Location).Length();

	if (DistanceToTarget > Radius)
	{
		const FVector Desired = DirectionToPoint * EntitiesFlockingData[Index].MaxSpeed;
		FVector NewForce = Desired - EntitiesFlockingData[Index].Velocity;
		NewForce /= EntitiesFlockingData[Index].MaxSpeed;
		NewForce *= EntitiesFlockingData[Index].MaxForce;
		return NewForce;
	}
	const float RampedSpeed = EntitiesFlockingData[Index].MaxSpeed * (DistanceToTarget / DesiredRadiusToTarget);
	const float ClippedSpeed = FMath::Min(RampedSpeed, EntitiesFlockingData[Index].MaxSpeed);

	const FVector Desired = DirectionToPoint * ClippedSpeed;
	FVector NewForce = Desired - EntitiesFlockingData[Index].Velocity;
	NewForce /= EntitiesFlockingData[Index].MaxSpeed;
	NewForce *= EntitiesFlockingData[Index].MaxForce;
	return NewForce;
}

/** Calculates path following force when using flow field pathfinding, by getting direction of current node and then calculating a force that steers the entity towards the flow field direction */
// Improvements: adjust the force magnitude based on the entity's proximity to the path. For example, if the entity is close to the path, reduce the magnitude of PathFollowingForce.
// Could be done by calculating the distance to the nearest point on the path, adjusting the scaling factor based on the distance to the path eg FMath::Lerp or any other curve to define how the scaling factor changes with distance
FVector AFlockingBrain::CalculateFlowFieldPathfindingForce(int Index)
{
	ensure (FlockingGrid != nullptr);

	FVector PathFollowingForce;
	// Check if the entity is inside the flow field
	const FVector CurrentLocation = EntitiesFlockingData[Index].Location;
	// Get the direction from the flow field
	const FVector MoveDirection = FlockingGrid->GetDirectionBasedOnWorldLocation(CurrentLocation).GetSafeNormal();
	const float DistanceToTarget = (EntityTargetLocation - CurrentLocation).Length(); 

	//return CalculateForceBasedOnDistance(Index, DesiredRadiusToTarget, CurrentLocation, MoveDirection);
	if (DistanceToTarget >= DesiredRadiusToTarget)
	{
		const FVector Desired = MoveDirection * EntitiesFlockingData[Index].MaxSpeed; 
		PathFollowingForce = Desired - EntitiesFlockingData[Index].Velocity;
		PathFollowingForce /= EntitiesFlockingData[Index].MaxSpeed;
		
		return PathFollowingForce *= EntitiesFlockingData[Index].MaxForce;
	}
	const float RampedSpeed = EntitiesFlockingData[Index].MaxSpeed * (DistanceToTarget / DesiredRadiusToTarget);
	const float ClippedSpeed = FMath::Min(RampedSpeed, EntitiesFlockingData[Index].MaxSpeed);
	const FVector Desired = MoveDirection * ClippedSpeed; 
	PathFollowingForce = Desired - EntitiesFlockingData[Index].Velocity;
	PathFollowingForce /= EntitiesFlockingData[Index].MaxSpeed;

	return PathFollowingForce *= EntitiesFlockingData[Index].MaxForce;
}

FVector AFlockingBrain::CalculateAStarPathfindingForce(int Index)
{
	ensure (FlockingGrid != nullptr);
	
	EntitiesFlockingData[Index].PathWaypointIndex = 0; //redundant??
	
	TArray<FVector> Waypoints = FlockingGrid->RequestPath(EntitiesFlockingData[Index].Location, FlockingGrid->StartLocation);

	if (EntitiesFlockingData[Index].bIsLeader)
	{
		Waypoints = FlockingGrid->RequestPath(EntitiesFlockingData[Index].Location, EntityTargetLocation);
	}
	
	// Check if the entity has reached the current waypoint.
	if (Waypoints.Num() <= 0) // kolla grid, kanske early return innan
	{
		return FVector::One(); // returnera nåt annat än one, returnera en variabel som sträng eller annat som visar att det gick åt helvete
	}
	FVector CurrentWaypoint = Waypoints[EntitiesFlockingData[Index].PathWaypointIndex];

	if (bDebug)
	{
		DrawDebugSphere(GetWorld(), CurrentWaypoint, 30.f, 40, FColor::Green, false, 3, 1, 0);
	}
	FVector DirectionToWaypoint = (CurrentWaypoint - EntitiesFlockingData[Index].Location).GetSafeNormal();
	const float DistanceToWaypoint = DirectionToWaypoint.Length();
	
	//CurrentLocation is close enough to NextWaypoint
	if (DistanceToWaypoint > 0.1f)
	{
		return FVector::One();
	}
	// Entity has reached the current waypoint
	EntitiesFlockingData[Index].PathWaypointIndex++;
		
	// Check if there are more waypoints left
	if (EntitiesFlockingData[Index].PathWaypointIndex <= Waypoints.Num())
	{
		CurrentWaypoint = Waypoints[EntitiesFlockingData[Index].PathWaypointIndex];
	}
	DirectionToWaypoint = (CurrentWaypoint - EntitiesFlockingData[Index].Location).GetSafeNormal();
	return CalculateForceBasedOnDistance(Index, 0.1f, CurrentWaypoint, DirectionToWaypoint);
}

FVector AFlockingBrain::CalculateTargetFollowingForce(const int Index) 
{
	if (bUseFlowFieldPathfinding)
	{
		bUseAStarPathfinding = false;
		FlockingGrid->bUseFlowFillAlgorithm = true;
		FlockingGrid->bUseAStarAlgorithm = false;
		return CalculateFlowFieldPathfindingForce(Index);
	}
	FlockingGrid->bUseFlowFillAlgorithm = false;
	if(bUseAStarPathfinding)
	{
		bUseFlowFieldPathfinding = false;
		FlockingGrid->bUseAStarAlgorithm = true;
		FlockingGrid->TargetLocation = EntityTargetLocation;
		EntitiesFlockingData[Index].bIsFollowingPath = true;
		return  CalculateAStarPathfindingForce(Index);
	}
	FlockingGrid->bUseAStarAlgorithm= false;
	return EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[Index].TargetLocation, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, EntitiesFlockingData[Index].MaxSpeed, EntitiesFlockingData[Index].MaxForce, DesiredVisionRadius);
}

FVector AFlockingBrain::CalculateSteerForce(const int Index) // bryta ut forloopen, kanske separat funktion??
{
	int Counter = 0;
	FVector TotalSeparationForce = FVector::ZeroVector;
	FVector CenterOfMass = FVector::ZeroVector;
	FVector TotalVelocity = FVector::ZeroVector;

	ensure(Entities[Index] != nullptr);
	EntitiesFlockingData[Index].TargetLocation = EntityTargetLocation;
	
	int LeaderIndex = Index;

	// here you can add and check if entity is leader, and then make A* path-following calculations only for that entity and the rest would get assigned waypoints
	// would be more effective performance-wise, but entities would risk colliding with other objects.
	EntitiesFlockingData[Index].bIsLeader = false;

	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		if (EntitiesFlockingData[Index].DistanceToTarget > EntitiesFlockingData[i].DistanceToTarget)
		{
			LeaderIndex = i;
		}
		
		ensure(Entities[Index]->FlockingActorData != nullptr);
		if (EntitiesFlockingData[Index].ID == EntitiesFlockingData[i].ID)
		{
			continue;
		}
		
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
	const FVector CurrentSeekForce = CalculateTargetFollowingForce(Index);
	EntitiesFlockingData[Index].NumNeighbours = Counter;

	Separation = EntityFlockingFunctions::CalculateSeparationForce(Counter, TotalSeparationForce);
	Separation *= SeparationWeight;

	Cohesion = EntityFlockingFunctions::CalculateCohesionForce(Counter, CenterOfMass, EntitiesFlockingData[Index].Location); 
	Cohesion *= CohesionWeight;
	
	Alignment = EntityFlockingFunctions::CalculateAlignmentForce(Counter,TotalVelocity);
	Alignment *= AlignmentWeight;
	
	const FVector TotalForce = CurrentSeekForce + Cohesion + Alignment + Separation * SeparationMultiplyer;
	
	return TotalForce;
}




		