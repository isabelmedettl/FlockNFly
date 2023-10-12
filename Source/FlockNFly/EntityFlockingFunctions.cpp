#include "EntityFlockingFunctions.h"


namespace EntityFlockingFunctions
{
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
			const float RampedSpeed = EntityMaxSpeed * (Distance.Length() / DesiredRadiusToTarget);
			const float ClippedSpeed = FMath::Min(RampedSpeed, EntityMaxSpeed);
			const FVector Desired = (Distance).GetSafeNormal() * ClippedSpeed; 
			NewSteeringForce = Desired - EntityVelocity;
		}
		
		return NewSteeringForce;
	}

	FVector CalculateSeparationForce(const int Counter, FVector &Separation) 
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

	FVector CalculateCohesionForce(const int Counter,  FVector &CenterOfMass, const FVector &Location)
	{
		if (Counter > 0)
		{
			CenterOfMass /= Counter;
			FVector TotalCohesionForce = FVector(CenterOfMass.X - Location.X, CenterOfMass.Y - Location.Y, CenterOfMass.Z - Location.Z);
			TotalCohesionForce.Normalize();
			return TotalCohesionForce; 
		}
		return FVector::ZeroVector;
	}

	FVector CalculateAlignmentForce(const int Counter, FVector &TotalAlignmentForce)
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
