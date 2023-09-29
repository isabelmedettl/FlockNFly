#include "Pathfinder.h"

#include "FlockingGrid.h"
#include "FlockingNode.h"
#include "FlockNFlyCharacter.h"

Pathfinder::Pathfinder(APawn* PlayerActor, AFlockingGrid* MapGrid) : PlayerPawn(PlayerActor), Grid(MapGrid)
{
	PlayerCharacter = Cast<AFlockNFlyCharacter>(PlayerPawn);
	ensure(PlayerCharacter != nullptr);
}

namespace FlowFieldFunctions 
{
	/**Checks if moving from Current to NextNode would be a diagonal move, if any coordinate is same in Next och Current there is no diagonal move */
	bool IsMovingDiagonally(const FlockingNode* Current, const FlockingNode* Next)
	{
		return !(Current->GridX == Next->GridX || Current->GridY == Next->GridY || Current->GridZ == Next->GridZ); 
	}

	/** Gets cost from current to next, returning set values that may be changed?*/
	int GetCostToNode(const FlockingNode* Current, const FlockingNode* Next)
	{
		return IsMovingDiagonally(Current, Next) ? 14 : 10; 
	}
}

// Inspiration from https://leifnode.com/2013/12/flow-field-pathfinding/ and wordpress.com/2015/02/cs380_researchpaper_flockingflowfieldpathfining1.pdf
void Pathfinder::UpdateNodesFlowField()
{
	TargetLocation = PlayerCharacter->CurrentTargetLocation;
	FlockingNode* TargetNode = Grid->GetNodeFromWorldLocation(TargetLocation);

	// If target has not moved, updating flow field is redundant
	if(TargetNode == OldTargetNode) return;

	OldTargetNode = TargetNode;

	ResetNodeCosts();

	TargetNode->SetCost(0);

	TArray<FlockingNode*> NotVisited;
	TSet<FlockingNode*> Visited;

	NotVisited.Add(TargetNode);

	while(!NotVisited.IsEmpty())
	{
		// Set the current node to first item in list, then remove it from not visited and add to visited
		FlockingNode* Current = NotVisited[0];
		NotVisited.RemoveAt(0);
		Visited.Add(Current);

		for (FlockingNode* Neighbour : Grid->GetNeighbours(Current))
		{
			// if walkable node, set its direction to nearest walkable so entity doesnt proceed walking into unwalkable
			if (!Neighbour->IsWalkable())
			{
				// only set if it hasnt been set before, this is a static map
				if (bIsDirectionInUnWalkableNodesSet)
				{
					continue;
				}
				SetDirectionInUnWalkableNode(Neighbour);
			}
			// Calculate cost to travel to neighbor from current node
			const int NewNeighbourCost = FlowFieldFunctions::GetCostToNode(Current, Neighbour) + Current->GetCost();
			// if path is cheaper than neighbours current cost, add that node to Notvisited if its not been visited before,update direction and cost
			if (NewNeighbourCost <Neighbour->GetCost())
			{
				if (!Visited.Contains(Neighbour))
				{
					NotVisited.Add(Neighbour);
				}

				Neighbour->SetCost(NewNeighbourCost);
				Neighbour->SetDirection(Current);

				
				if (Grid->bDebug)
				{
					DrawDebugLine(Grid->GetWorld(), Current->GetWorldCoordinate() + FVector::UpVector * 10,
					Current->GetWorldCoordinate() + Current->GetDirection() * 20 + FVector::UpVector * 10,
					FColor::Cyan, false, 0.2, 0, 5); 
				}
			}
		}
		
	}
	bIsDirectionInUnWalkableNodesSet = true; // set to true so it is only checked/set once 

}

void Pathfinder::ResetNodeCosts()
{
	for(int x = 0; x < Grid->GridLengthX; x++)
		for(int y = 0; y < Grid->GridLengthY; y++)
			for(int z = 0; z < Grid->GridLengthZ; z++)
			Grid->GetNodeFromGrid(x, y, z)->SetCost(INT_MAX); 
}

void Pathfinder::SetDirectionInUnWalkableNode(FlockingNode* NeighbourNode)
{
	// to find lowest cost, set lowest cost to max value
	int LowestCostAwayFromNode = INT_MAX;
	// check all neighbours. If naiogbour has a lower cost than lowest and is walkable, set direction to it
	for (FlockingNode* NeighboursNeighbour : Grid->GetNeighbours(NeighbourNode))
	{
		const int CostToNeighbour = FlowFieldFunctions::GetCostToNode(NeighbourNode, NeighboursNeighbour);
		if (NeighboursNeighbour->IsWalkable() && CostToNeighbour < LowestCostAwayFromNode)
		{
			NeighbourNode->SetDirection(NeighboursNeighbour);
			LowestCostAwayFromNode = CostToNeighbour;
		}
	}
	// DEBUGGING DRAWING DIRECTION IN UNWALKABLE NODES 
	if(Grid->bDebug)
		DrawDebugLine(Grid->GetWorld(), NeighbourNode->GetWorldCoordinate() + FVector::UpVector * 10,
		NeighbourNode->GetWorldCoordinate() + NeighbourNode->GetDirection() * 20 + FVector::UpVector * 10,
		FColor::Cyan, true, -1, 0, 5);
}
