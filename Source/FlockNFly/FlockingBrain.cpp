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

	// Start looping timer to update flocking entities behavior
	//GetWorldTimerManager().SetTimer(ApplyBehaviorTimerHandle, this, &AFlockingBrain::CalculateSteerForce, ApplyBehaviorDelay, true,0.1f);
	
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
	
	for (int i = 0 ; i <EntitiesFlockingData.Num(); i++)
	{
		EntitiesFlockingData[i].SteerForce = CalculateSteerForce(i);
		CalculateNewVelocity(i);
		Entities[i]->UpdateLocation(DeltaTime);
	}

	// kör entities update
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
		else
		{
			float RampedSpeed = EntityMaxSpeed * (Distance.Length() / DesiredRadiusToTarget);
			float ClippedSpeed = FMath::Min(RampedSpeed, EntityMaxSpeed);
			
			const FVector Desired = (Distance).GetSafeNormal() * ClippedSpeed ; 
			NewSteeringForce = Desired - EntityVelocity;
			
		
			
			//const FVector Desired = (Distance).GetSafeNormal() * (Distance.Length() / DesiredRadiusToTarget); 
			//NewSteeringForce = Desired - EntityVelocity;
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
	EntitiesFlockingData[IndexOfData].MaxSpeed = EntityFlockingFunctions::CalculateSpeed(EntitiesFlockingData[IndexOfData].TargetLocation, EntitiesFlockingData[IndexOfData].Location, EntitiesFlockingData[IndexOfData].MaxSpeed, DesiredRadiusToTarget);
	EntitiesFlockingData[IndexOfData].Acceleration = EntitiesFlockingData[IndexOfData].SteerForce / EntitiesFlockingData[IndexOfData].Mass;
	EntitiesFlockingData[IndexOfData].Velocity += EntitiesFlockingData[IndexOfData].Acceleration;
	EntitiesFlockingData[IndexOfData].Velocity = EntitiesFlockingData[IndexOfData].Velocity.GetClampedToMaxSize(EntitiesFlockingData[IndexOfData].MaxSpeed);
}


FVector AFlockingBrain::CalculateSteerForce(const int Index)
{
	int SeparationCounter = 0;
	int CohesionCounter = 0;
	int AlignmentCounter = 0;
	FVector TotalSeparationForce = FVector::ZeroVector;
	FVector CenterOfMass = FVector::ZeroVector;
	FVector CurrentSeekForce = FVector::ZeroVector;
	FVector TotalForce = FVector::ZeroVector;;

	ensure(Entities[Index] != nullptr);
		
	EntitiesFlockingData[Index].TargetLocation = EntityTargetLocation;
	CurrentSeekForce = EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[Index].TargetLocation, EntitiesFlockingData[Index].Location, EntitiesFlockingData[Index].Velocity, EntitiesFlockingData[Index].MaxSpeed, EntitiesFlockingData[Index].MaxForce, DesiredRadiusToTarget);
	Entities[Index]->UpdateSteerForce(CurrentSeekForce);
	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		ensure(Entities[Index]->FlockingActorData != nullptr);
		if (EntitiesFlockingData[Index].ID == EntitiesFlockingData[i].ID) { continue; }
		float Distance = (EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location).Length();
		if (Distance < DesiredSeparationRadius * 2) 
		{
			TotalSeparationForce += EntitiesFlockingData[i].Location - EntitiesFlockingData[Index].Location;
			SeparationCounter++;
		}
		if (Distance < DesiredCohesionRadius * 2 && Distance > DesiredSeparationRadius * 2)
		{
			Cohesion += EntitiesFlockingData[Index].Location;
			CohesionCounter++;
		}
		if (Distance < DesiredAlignmentRadius * 2)
		{
			Alignment += EntitiesFlockingData[i].Velocity;
			AlignmentCounter++;
		}
	}
	
	Separation = EntityFlockingFunctions::CalculateSeparationForce(SeparationCounter, TotalSeparationForce); // spara ner i strukten
	Separation *= SeparationWeight;

	Cohesion = EntityFlockingFunctions::CalculateCohesionForce(CohesionCounter, CenterOfMass, EntitiesFlockingData[Index].Location); // skicka ref till total cohesion force
	Cohesion *= CohesionWeight;

	Alignment = EntityFlockingFunctions::CalculateAlignmentForce(AlignmentCounter,Alignment);
	Alignment *= AlignmentWeight;
	
	TotalForce += CurrentSeekForce + Separation * 20 + Cohesion + Alignment * 10;
	Separation = FVector::ZeroVector;
	
	return TotalForce;
}

/* Ta reda på vad som ska beräknas, skicka en request, dela upp beräkningar över frames
 *
 * en är master, resten får en relativ position till mastern.
 * kommer alla requesta path hela tiden? Justera path så de inte förlorar flockingbeteende.
 *
 * (kanske den i mitten av flocken? Googla)
 */














void AFlockingBrain::OnDebug(FVector &Location) const
{
	//DrawDebugSphere(GetWorld(), CurrentTargetLocation, 30.f, 30, FColor::Black, false,0.2f);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingActorData.DesiredAlignmentRadius, 30, FColor::Red, false, 0.2f);
	//DrawDebugSphere(GetWorld(), Location, 30.f, 30, FColor::Green, false, 0.01f);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), FlockingActorData->DesiredSeparationRadius, 30, FColor::Green, false, 0.2f);
	

}

void AFlockingBrain::OnDebugLine(FVector &FromLocation, FVector &ToLocation) const
{
	//	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FlockingActorData.Velocity, FColor::Purple, false, 0.1f, 0, 10);
}


/*
		// ReSharper disable once CppTooWideScopeInitStatement
		AActor* Hej; // pointern ser ut att vara valid men är inne på iokänt minne, den är nullptr men är inte det pga har något random värde som finns i minnet
		Hej2(Hej);
		if (Hej != nullptr)
		{
			ensure(false);
		}
		*/



/*int SeparationCounter = 0;
	int CohesionCounter = 0;
	int AlignmentCounter = 0;
	FVector TotalSeparationForce = FVector::ZeroVector;
	//FVector TotalCohesionForce = FVector::ZeroVector;
	FVector CenterOfMass = FVector::ZeroVector;
	FVector CurrentSeekForce = FVector::ZeroVector;
	FVector TotalForce = FVector::ZeroVector;;

	for (int i = 0; i < EntitiesFlockingData.Num(); i++)
	{
		ensure(Entities[i] != nullptr);
		
		EntitiesFlockingData[i].TargetLocation = EntityTargetLocation;
		CurrentSeekForce = EntityFlockingFunctions::CalculateSeekForce(EntitiesFlockingData[i].TargetLocation, EntitiesFlockingData[i].Location, EntitiesFlockingData[i].Velocity, EntitiesFlockingData[i].MaxSpeed, EntitiesFlockingData[i].MaxForce);
		Entities[i]->UpdateSteerForce(CurrentSeekForce);
		
		for (int j = 0; j < EntitiesFlockingData.Num(); j++)
		{
			ensure(Entities[i]->FlockingActorData != nullptr);
			if (EntitiesFlockingData[i].ID != EntitiesFlockingData[j].ID)
			{
				float Distance = (EntitiesFlockingData[j].Location - EntitiesFlockingData[i].Location).Length();
				if (Distance < DesiredSeparationRadius * 2) 
				{
					TotalSeparationForce += EntitiesFlockingData[j].Location - EntitiesFlockingData[i].Location;
					SeparationCounter++;
				}
				if (Distance < DesiredCohesionRadius * 2 && Distance > DesiredSeparationRadius * 2 )
				{
					Cohesion += EntitiesFlockingData[j].Location;
					CohesionCounter++;
				}
				if (Distance < DesiredAlignmentRadius * 2 )
				{
					Alignment += EntitiesFlockingData[i].Velocity;
					AlignmentCounter++;
				}
			}
		}

		
		Separation = EntityFlockingFunctions::CalculateSeparationForce(SeparationCounter, TotalSeparationForce); // spara ner i strukten
		Separation *= SeparationWeight;

		Cohesion = EntityFlockingFunctions::CalculateCohesionForce(CohesionCounter, CenterOfMass, EntitiesFlockingData[i].Location); // skicka ref till total cohesion force
		Cohesion *= CohesionWeight;

		Alignment = EntityFlockingFunctions::CalculateAlignmentForce(AlignmentCounter,Alignment);
		Alignment *= AlignmentWeight;

		
		TotalForce += CurrentSeekForce + Separation * 10 + Cohesion + Alignment * 10;

		// 
		Entities[i]->UpdateSteerForce(TotalForce); // uppdateringen kan göras hos actorn, inte
		



		
		
		Separation = FVector::ZeroVector;
		//Cohesion = FVector::ZeroVector;
*/

/*
	if (AlignmentCounter > 0)
	{
		//EntitiesFlockingData[i].SteerForce += Separation * 300.f;
		DrawDebugLine(GetWorld(), EntitiesFlockingData[i].Location, EntitiesFlockingData[i].Location + EntitiesFlockingData[i].Velocity, FColor::Purple, false, 0.1f, 0, 10);
		//DrawDebugSphere(GetWorld(), CenterOfMass /= CohesionCounter, 30.f, 30, FColor::Green, false, 0.5f);
	}
	else
	{
		DrawDebugLine(GetWorld(), EntitiesFlockingData[i].Location, EntitiesFlockingData[i].Location + EntitiesFlockingData[i].Velocity, FColor::Red, false, 0.1f, 0, 10);

	}
	
	*/
		