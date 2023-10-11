#pragma once

namespace EntityFlockingFunctions
{
	/** Calculates speed depending on how close entity is to current target location*/
	float CalculateSpeed(const FVector &CurrentTargetLocation, const FVector &EntityLocation, const float EntityMaxSpeed, const float DesiredRadiusToTarget);

	/** Calculates seeking force to target by calculating vector resulting from subtracting the desired position from the current position. The result is the appropriate velocity
	 * Not used when using pathfinding*/
	FVector CalculateSeekForce (const FVector &CurrentTargetLocation, const FVector &EntityLocation, const FVector &EntityVelocity, const float EntityMaxSpeed, const float EntityMaxForce, const float DesiredRadiusToTarget);

	/** Calculates entities to steer away from any neighbor that is within view and within a prescribed minimum separation distance*/
	FVector CalculateSeparationForce(const int Counter, FVector &Separation); // pekare till funktioner??

	/** Calculates entities position to correspond with average alignment of nearby entities, taking the position of entities within certain radius and steers entity towards the average position of those entites*/
	FVector CalculateCohesionForce(const int Counter,  FVector &CenterOfMass, const FVector &Location); //totalcohesionforce kan vara const, gör kopia på den och skicka tillbaka den, bool/void istället

	/** Attempts to match the velocity of other entities inside this entity´s visible range by adding neighbours velocity to computation vector*/
	FVector CalculateAlignmentForce(const int Counter, FVector &TotalAlignmentForce);
};
