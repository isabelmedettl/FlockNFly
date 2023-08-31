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

	BitArray = ConvertSeedToBit();
	GenerateGrid();

	
	
}

// Called every frame
void AGeometryGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	
	//UE_LOG(LogTemp, Warning, TEXT("bit ,%s"), *NewSeed);
}

TArray<uint8> AGeometryGenerator::ConvertSeedToBit()
{
	
	TArray<uint8> BinaryString;

	for (uint64 x = 0; x < Seed.Len(); ++x)
	{
		uint8 ASCIIvalue = static_cast<uint8>(Seed[x]);
		BinaryString.Add(ASCIIvalue);
	}
	
	return BinaryString;
	
}

namespace BitFuncs
{
	uint8 GetBitValueAtIndex(TArray<uint8> &IntArray, int Index)
	{
		int BitIndex = Index / 8;
		int Byte = IntArray[BitIndex];
		return Byte >> (Index % 8) & 1U; 
	}

	// bestämma thresholds för hur många bits value som ska räknas in i x, y och z
	//när bit 1, de närmast thresholds värde avgör x y z värde på meshar
	// sen threshold hur många som måste bli 0 efter
	
}

void AGeometryGenerator::GenerateGrid()
{

	if (Grid.BitGrid.Num() <=0)
	{
		Grid.BitGrid.SetNum(Grid.GridX);
		for (int x = 0; x < Grid.GridX; ++x)
		{
			Grid.BitGrid[x].SetNum(Grid.GridY);
			for (int y = 0; y <Grid.GridY; ++y)
			{
				Grid.BitGrid[x][y].SetNumUninitialized(Grid.GridZ);
			}
		}
	}
	
	
	for (int x = 0; x < Grid.GridX; x++)
	{
		for (int y = 0; y <Grid.GridY; y++)
		{
			for (int z = 0; z < Grid.GridZ; z++)
			{
				Grid.BitGrid[x][y][z] = BitFuncs::GetBitValueAtIndex(BitArray, x + y + z) == 1;
				UE_LOG(LogTemp, Warning, TEXT("bit ,%u"), (bool) Grid.BitGrid[x][y][z]);
			}
		}
	}
}

void AGeometryGenerator::SmoothMap()
{
	
}

int AGeometryGenerator::NeighbourhoodCount()
{
	return 0;
}
