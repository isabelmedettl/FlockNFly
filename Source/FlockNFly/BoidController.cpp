// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidController.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void ABoidController::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	SetAim();
}

void ABoidController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ABoidController::SetAim()
{
	FRotator AimRotation;
	
	if (PlayerCharacter)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Has player")));
		AimRotation = UKismetMathLibrary::FindLookAtRotation(GetCharacter()->GetActorLocation(), PlayerCharacter->GetActorLocation());
	}
	GetCharacter()->SetActorRotation(FRotator(AimRotation.Pitch, AimRotation.Yaw, 0.f));
	MoveToActor(PlayerCharacter, 0.1, true);
}
