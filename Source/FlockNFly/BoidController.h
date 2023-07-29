// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BoidController.generated.h"

/**
 * 
 */
UCLASS()
class FLOCKNFLY_API ABoidController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	ACharacter* PlayerCharacter;

	/** Sets rotation to face player pawn*/
	UFUNCTION()
	void SetAim();
	
};
