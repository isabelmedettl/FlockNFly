// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlockingBaseActor.h"
#include "GameFramework/Actor.h"
#include "FlockingBrain.generated.h"

class UBoxComponent;
class AFlockingGrid;
class AFlockingBaseActor;
class AFlockNFlyCharacter;
class USphereComponent;
class FlockingNode;

USTRUCT()
struct FFlockingActorData 
{
	GENERATED_BODY()
	
	FFlockingActorData()
	{
		Velocity = FVector::ZeroVector;
		DistanceToTarget = 0.f; //ersätt alla beräkningar på detta med variabeln, görs typ 20 ggr i onödan ALT Ändra till direction för det behövs typ mer??
		Acceleration = FVector::ZeroVector;
		SteerForce = FVector::ZeroVector;
		Location = FVector::ZeroVector;
		TargetLocation = FVector::ZeroVector;
		MaxSpeed = -1.0f;
		CurrentSpeed = -1.f;
		MaxForce = -1.0f;
		Mass = -1.0f;
		NumNeighbours = -1;
		NumNeighboursTowardTarget = -1;
		ID = -1;
		bIsLeader = false;
		
	}
	/** Current velocity of entity*/
	FVector Velocity;

	/** Current distance to target location*/
	float DistanceToTarget;

	/** Current acceleration of entity*/
	FVector Acceleration;

	/** Current steering force of entity*/
	FVector SteerForce;

	/** Current location of entity in world space */
	FVector Location;

	/** Location or moving object in world space to which entity is steering towards*/
	FVector TargetLocation = FVector::ZeroVector;

	/** Limits magnitude of velocity vector */
	float MaxSpeed = -1.0f;

	/** Current speed of entity */
	float CurrentSpeed = -1.0f;

	/** Limits magnitude of acceleration vector */
	float MaxForce = -1.0f;

	/** Mass of entity*/
	float Mass = -1.0f;
	
	/** Unique number for idintification*/
	int ID = -1;

	/** Number of neighbours, aka other entities withing field of view of entity*/
	int NumNeighbours = -1;

	/** Number of neighbours, aka other entities withing field of view of entity when looking at target*/
	int NumNeighboursTowardTarget =-1;

	/** Flag for leader entity, aka the entity that is closest to current target location*/
	bool bIsLeader;

};

UCLASS()
class FLOCKNFLY_API AFlockingBrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlockingBrain();
	
	/** Subclass of boid character to be spawned*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AFlockingBaseActor> FlockingBaseActorClass;

	/** Bool for debugging*/
	UPROPERTY(EditAnywhere, Category= "Debug")
	bool bDebug = false;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// =========== Debugging and target decisions for debugging =========== //
	void ChangeTarget();
	
	UPROPERTY(EditAnywhere)
	FVector Target1 = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector Target2 = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector Target3 = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector Target4 = FVector::ZeroVector;

	bool bIsTimeToSwitchTarget = false;

	int CurrentTargetIndex = 0;

	void UpdateTimerTarget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTimerTarget = false;

	FTimerHandle ChangeTargetTimerHandle;

	UPROPERTY(EditAnywhere)
	float ChangeTargetDelay = 5.f;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Target1Box;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Target2Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Target3Box;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Target4Box;
protected:

	// ======= Bools and methods to adjust the way entities are searching for target location ==== //
	UPROPERTY(EditAnywhere, Category="Pathfinding", meta=(AllowPrivateAccess = true))
	bool bUseFlowFieldPathfinding = false;

	UPROPERTY(EditAnywhere, Category="Pathfinding", meta=(AllowPrivateAccess = true))
	bool bUseAStarPathfinding = false;

	/** Performs different target-seeking calculation depending on which pathfinding is used. If none are used, entites seek target without checking for collision*/
	FVector DecideTargetFindingMethod(int Index);
	
	/** Array containing Flocking data structs to all active entities, mapped to Entities*/
	UPROPERTY(VisibleAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	TArray<FFlockingActorData> EntitiesFlockingData;
	
	/** Array containing pointers to all entities that the actor is managing*/
	UPROPERTY(VisibleAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	TArray<AFlockingBaseActor*> Entities;

	/** Distance to keep from target*/
	UPROPERTY(VisibleAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	float PreferredDistanceToTarget = 50.f;
	
	/** Number of boids to spawn*/
	UPROPERTY(EditAnywhere, Category= "Spawning", meta =(AllowPrivateAccess = true))
	int NumberOfEntities = 0;

	/** Defines how many rows of boids to spawn, deciding placement of the boids in worlds space */
	int EntityRows = 0;
	
	/** Defines how many columns of boids to spawn, deciding placement of the boids in worlds space */
	int EntityColumns = 0;

	/** Defines distance in between boids, both at spawn and in formation of flock*/
	UPROPERTY(EditAnywhere, Category= "Spawning", meta = (AllowPrivateAccess = true))
	int DistanceBetweenEntities = 0;

	/** Defines height in world space at which boids are going to spawn*/
	UPROPERTY(EditAnywhere, Category="Spawning", meta = (AllowPrivateAccess = true))
	double SpawnHeight = 0.f;

	/** Array containing positions in world space to spawn boid to*/
	TArray<FVector> SpawnLocations = TArray<FVector>();

	/** Spawns specified number of boids in designated spawn area*/
	void SpawnBoids();

	/** Calculates spawn positions for boids given number of boids and distance between them
	 */
	void CalculatePossibleSpawnFormation();


	/** Pointer to player character*/
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess = true))
	AFlockNFlyCharacter* PlayerCharacter;
	

	void SpawnEntity(const FVector &SpawnLocation, int ID);
	
	// ======= Weights and methods for flocking behaviors ========= //

		/** Multiplyer for applying cohesion force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flocking", meta=(AllowPrivateAccess = true))
	int CohesionWeight = 4;

	/** Multiplyer for applying cohesion force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flocking",meta=(AllowPrivateAccess = true))
	int SeekWeight = 1;
	
	/** Multiplyer for applying separation force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flocking",meta=(AllowPrivateAccess = true))
	int SeparationWeight = 10;

	/** Multiplyer for applying alignment force*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flocking", meta=(AllowPrivateAccess = true))
	int AlignmentWeight = 2;

	/** Additional multiplyer for separation force when adding separation vector to steer force */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Flocking",meta=(AllowPrivateAccess = true))
	int SeparationMultiplyer = 5;

	/** Limits magnitude of velocity vector */
	UPROPERTY(EditAnywhere, Category="Flocking, Entities", meta=(AllowPrivateAccess = true))
	float MaxSpeed = 400.f;

	/** Limits magnitude of acceleration vector */
	UPROPERTY(EditAnywhere, Category="Flocking, Entities",meta=(AllowPrivateAccess = true))
	float MaxForce = 50.f;

	/** Mass of entity*/
	UPROPERTY(EditAnywhere, Category="Flocking, Entities",meta=(AllowPrivateAccess = true))
	float Mass = 0.7f;

	/** Loops through collection of entities and applies behaviors */
	FVector CalculateSteerForce(const int Index);

	/** Calculates new location to set to entities, based on entities current velocity, mass and acceleration*/
	void CalculateNewVelocity(const int IndexOfData);

	/** Checks if entity is in field of view of another entity*/
	bool IsWithinFieldOfView(float AngleToView, const FVector &EntityLocation, const FVector &EntityVelocity, const FVector &Direction) const;
	
	
	/** Target location toward which entities should steer to*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flocking",meta=(AllowPrivateAccess = true))
	FVector EntityTargetLocation;

	
	// =========== Flocking variables ============= //
	
	/** Maximal distance of vision for entity */
	UPROPERTY(EditAnywhere, Category="Flocking")
	float DesiredVisionRadius = 300.f;

	/** Distance to target when entities should slow down*/
	UPROPERTY(EditAnywhere, Category="Flocking")
	float DesiredRadiusToTarget = 200.f;

	/** Distance to target defining if entity has vision of target*/
	UPROPERTY(EditAnywhere, Category="Flocking")
	float DesiredVisionRadiusToTarget = 500.f;

	/** Angle deciding the narrowness of entities field of vision*/
	UPROPERTY(EditAnywhere, Category="Flocking")
	float NeighbourFieldOfViewAngle = 270.f;

	/** Angle deciding the narrowness of entities field of vision when looking at target*/
	UPROPERTY(EditAnywhere, Category="Flocking")
	float TargetFieldOfViewAngle = 45.f;
	
	// =========== Flocking force vectors to apply to entities ============ //
	
	/** Separation vector*/ 
	FVector Separation = FVector::ZeroVector;
	
	/** Cohesion vector*/
	FVector Cohesion = FVector::ZeroVector;

	/** Alignment vector */
	FVector Alignment = FVector::ZeroVector;

	/** Bool for checking if all entity actors has had their pointer to data struct set*/
	bool bHasAssignedBoids = false;


	// ========== Collision - Currently not in use ==============//

	/** Checks for collision on leader entities path toward target location*/
	bool CollisionOnPathToTarget(int Index);

	/** Radius defining trace size*/
	UPROPERTY(EditAnywhere, Category="Collision", meta=(AllowPrivateAccess = true))
	float TraceRadius = 70.f;

	/** Calculates force to be added to avoid collision in path toward target location*/
	FVector CalculateCollisionAvoidanceForce(int Index);

	/** Ref to colliding obstacle*/
	FHitResult FoundObstacle;


	// ========== Pathfinding ========= //
	//FlockingNode* CurrentEntityNode = nullptr;

	AFlockingGrid* FlockingGrid = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AFlockingGrid> GridClass;

	/** Maximum force magnitude that can be applied to steer the entity along the path */
	UPROPERTY(EditAnywhere, Category="Pathfinding")
	float MaxForceForPathFollowing = 3.f;

	FVector CalculatePathFollowingForce(int Index);

	FTimerHandle SetGridPointerHandle;

	void SetGridPointer();

	/** Bool to check if grid is set, otherwise we wait until until it is*/
	bool bIsGridSet = false;
	
};
