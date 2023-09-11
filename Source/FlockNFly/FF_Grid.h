#pragma once
#include "Components/BoxComponent.h"


struct FFGrid //oduglig
{
	FFGrid()
	{
		SizeX = 0;
		SizeY = 0;
		SizeZ = 0;
		NodeSize = 5.f;
		TopLeft = FVector::ZeroVector;
		BottomRight = FVector::ZeroVector;
	}

	int SizeX = 3000;
	
	int SizeY= 3000;
	
	int SizeZ= 3000;

	FVector TopLeft = FVector::ZeroVector;
	FVector BottomRight = FVector::ZeroVector;

	int NodeSize = 5;
	
	
	void CalculateGrid(FVector Center, int X, int Y, int Z, int NSize)
	{
		// Calculate half extents in each dimension
		float HalfSizeX = SizeX * 0.5f * NodeSize;
		float HalfSizeY = SizeY * 0.5f * NodeSize;
		float HalfSizeZ = SizeZ * 0.5f * NodeSize;

		// Calculate the minimum and maximum bounds
		TopLeft = Center - FVector(HalfSizeX, HalfSizeY, HalfSizeZ);
		//BottomRight = Center + FVector(HalfSizeX, HalfSizeY, HalfSizeZ);

		// Update the UBoxComponent to represent the grid's bounds

		/*

		// Calculate and draw debug spheres for each grid node
		for (int32 X = 0; X < SizeX; X++)
		{
			for (int32 Y = 0; Y < SizeY; Y++)
			{
				for (int32 Z = 0; Z < SizeZ; Z++)
				{
					FVector NodeLocation = TopLeft + FVector(X * NodeSize, Y * NodeSize, Z * NodeSize);
					GridArray.Add(NodeLocation);
					}
			}
		}
		*/
	}

	// box comp, size x y z / vector, 
};

namespace GridFunctions
{
	inline FVector CalculateGrid(FVector Center, int SizeX, int SizeY, int SizeZ, int NodeSize)
	{
		// Calculate half extents in each dimension
		float HalfSizeX = SizeX * 0.5f * NodeSize;
		float HalfSizeY = SizeY * 0.5f * NodeSize;
		float HalfSizeZ = SizeZ * 0.5f * NodeSize;

		// Calculate the minimum and maximum bounds
		FVector TopLeft = Center - FVector(HalfSizeX, HalfSizeY, HalfSizeZ);
		return TopLeft;
		//BottomRight = Center + FVector(HalfSizeX, HalfSizeY, HalfSizeZ);

		// Update the UBoxComponent to represent the grid's bounds

		/*

		// Calculate and draw debug spheres for each grid node
		for (int32 X = 0; X < SizeX; X++)
		{
			for (int32 Y = 0; Y < SizeY; Y++)
			{
				for (int32 Z = 0; Z < SizeZ; Z++)
				{
					FVector NodeLocation = TopLeft + FVector(X * NodeSize, Y * NodeSize, Z * NodeSize);
					GridArray.Add(NodeLocation);
					}
			}
		}
		*/
	}

	
}


struct FFWayPoint //kanske separat om den är stor, men tror inte det
{
	// loc 
};
