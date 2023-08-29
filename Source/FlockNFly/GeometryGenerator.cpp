// Fill out your copyright notice in the Description page of Project Settings.


#include "GeometryGenerator.h"

// Sets default values
AGeometryGenerator::AGeometryGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AGeometryGenerator::BeginPlay()
{
	Super::BeginPlay();

	
	
}

// Called every frame
void AGeometryGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FString NewSeed = ConvertSeedToBit();
	UE_LOG(LogTemp, Warning, TEXT("bit ,%s"), *NewSeed);
}

FString AGeometryGenerator::ConvertSeedToBit()
{
	FString BinaryString;

	for (uint64 i = 0; i < Seed.Len(); ++i)
	{
		uint8 ASCIIvalue = static_cast<uint8>(Seed[i]);
		FString CharBinary;
		for (int32 BitIndex = 7; BitIndex >=0; --BitIndex)
		{
			bool Bit = (ASCIIvalue >> BitIndex) & 1;
			CharBinary += Bit ? TEXT("1") : TEXT("0");
		}
		BinaryString += CharBinary;
	}
	return BinaryString;
}

