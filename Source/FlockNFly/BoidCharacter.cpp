// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidCharacter.h"

#include "FlockNFlyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABoidCharacter::ABoidCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

// Called when the game starts or when spawned
void ABoidCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(!GetController())
	{
		SpawnDefaultController();
	}
	
	PlayerCharacter = Cast<AFlockNFlyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if( PlayerCharacter != nullptr )
	{
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PlayerCharacter->GetActorLocation()));

		// Calculate the location of the AI character relative to the player, and rotation
		CurrentTargetLocation = PlayerCharacter->GetActorLocation();
		const FVector DirectionToTarget = CurrentTargetLocation - GetActorLocation();
		const FRotator InitialRotation = DirectionToTarget.ToOrientationRotator();
		SetActorRotation(InitialRotation);
	}
	// If set to debug, initiate looping timer to call on draw debug function
	if (bDebug)
	{
		GetWorldTimerManager().SetTimer(DebugTimerHandle, this, &ABoidCharacter::OnDebug, DrawDebugDelay, true, 0.1f); // Delay default = 2.f;
	}
}


// Called every frame
void ABoidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if( PlayerCharacter != nullptr )
	{
		CurrentTargetLocation = PlayerCharacter->GetActorLocation();
	}
	MoveTowardsLocation(DeltaTime);
	
}

void ABoidCharacter::MoveTowardsLocation(float DeltaTime)
{
	BoidData.CurrentSpeed = FMath::Lerp(BoidData.CurrentSpeed, BoidData.TargetSpeed, 0.1);

	// Rotation
	const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTargetLocation);
	SetActorRotation(FRotator(0, NewRotation.Yaw -90, 0));
	
	BoidData.Direction = CurrentTargetLocation - GetActorLocation();
	BoidData.Direction.Normalize();

	// Calculate the distance between the current location and the target location
	BoidData.DistanceToTarget = FVector::Distance(CurrentTargetLocation, GetActorLocation());

	// Calculate distance the actor can move in this step based on the speed variable
	float MoveDistance = BoidData.CurrentSpeed * DeltaTime;
	MoveDistance = FMath::Min(MoveDistance, BoidData.DistanceToTarget);
	const FVector NewLocation = GetActorLocation() + BoidData.Direction * MoveDistance;
	SetActorLocation(NewLocation);

	// Update data in struct
	BoidData.Velocity = GetCharacterMovement()->Velocity;
}

void ABoidCharacter::OnDebug() const
{
	DrawDebugSphere(GetWorld(), CurrentTargetLocation, 30.f, 30, FColor::Black, false,0.2f);
}


// Called to bind functionality to input
void ABoidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

