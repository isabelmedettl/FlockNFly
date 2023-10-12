#include "Pathfinder.h"

#include "FlockingGrid.h"
#include "FlockingHeap.h"
#include "FlockingNode.h"
#include "FlockNFlyCharacter.h"
#include "NodeFunctions.h"

Pathfinder::Pathfinder(APawn* PlayerActor, AFlockingGrid* MapGrid) : PlayerPawn(PlayerActor), Grid(MapGrid)
{
	PlayerCharacter = Cast<AFlockNFlyCharacter>(PlayerPawn);
	ensure(PlayerCharacter != nullptr);
	
}

void Pathfinder::UpdateNodesFlowField()
{
	TargetLocation = PlayerCharacter->CurrentTargetLocation;
	FlockingNode* TargetNode = Grid->GetNodeFromWorldLocation(TargetLocation);

	// If target has not moved, updating flow field is redundant
	if(TargetNode == OldTargetNode) return;

	OldTargetNode = TargetNode;

	ResetFlowFieldNodeCosts();

	TargetNode->SetFlowFieldCost(0);

	TArray<FlockingNode*> NotVisited;
	TSet<FlockingNode*> Visited;

	NotVisited.Add(TargetNode);
	TargetNode->SetIsInVisited(false);

	while(!NotVisited.IsEmpty())
	{
		// Set the current node to first item in list, then remove it from not visited and add to visited
		FlockingNode* Current = NotVisited[0];
		NotVisited.RemoveAt(0);
		Visited.Add(Current);
		Current->SetIsInVisited(true);

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
			const int NewNeighbourCost = NodeFunctions::GetCostToNode(Current, Neighbour) + Current->GetFlowFieldCost();
			// if path is cheaper than neighbours current cost, add that node to Notvisited if its not been visited before,update direction and cost
			if (NewNeighbourCost <Neighbour->GetFlowFieldCost())
			{
				if (!Visited.Contains(Neighbour))
				//if (!Neighbour->IsInVisited())
				{
					NotVisited.Add(Neighbour);
					Neighbour->SetIsInVisited(false);
				}

				Neighbour->SetFlowFieldCost(NewNeighbourCost);
				Neighbour->SetDirection(Current);
			}
		}
	}
	bIsDirectionInUnWalkableNodesSet = true; // set to true so it is only checked/set once 
}

bool Pathfinder::FindPath(const FVector &Start, const FVector &End)
{
	TArray<FVector> NewPath;
	//if (Grid->TargetLocation == FVector::ZeroVector || Grid->StartLocation == FVector::ZeroVector)
	if (Start == FVector::ZeroVector || End == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("locations are 0"));
		return false;
	}

	FlockingNode* StartNode = Grid->GetNodeFromWorldLocation(Start);
	FlockingNode* EndNode = Grid->GetNodeFromWorldLocation(End);
	
	ensure(StartNode != nullptr);
	ensure(EndNode != nullptr);
	
	//If target and entity has not moved, updating path is redundant
	if(EndNode == OldEndNode && StartNode == OldStartNode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Didnt need updating path"));
		bIsOldPathStillValid = true;
		Grid->OnNoNeedUpdate();
		return false;
	}
	
	OldEndNode = EndNode;
	OldStartNode = StartNode;

	StartNode->ParentNode = StartNode;
	StartNode->SetGCost(0.f);
	StartNode->SetFCost(0.f);
	StartNode->SetHCost(0.f);

	bool bPathSuccess = false;

	// here you could check that start node is walkable as well, but because the flocking entities have a float-like behaviour sometimes
	// they wouldn't be able to get out of collision if they were in one. 
	if (EndNode->bWalkable)
	{
		// Create open and closed sets, should probably be something else or not created every time
		FlockingHeap OpenSet = FlockingHeap(Grid->GetGridMaxSize());
		TSet<FlockingNode*> ClosedSet;

		OpenSet.Add(StartNode);
		StartNode->SetIsInOpenSet(true);

		while (OpenSet.Count() > 0)
		{
	        // Find the node in the open set with the lowest FCost
			FlockingNode* CurrentNode = OpenSet.RemoveFirst();
			
			ClosedSet.Add(CurrentNode);

	        // If the current node is the end node, path found
	        if (CurrentNode == EndNode)
	        {
	        	bPathSuccess = true;
	        	bIsOldPathStillValid = false;
	        	UE_LOG(LogTemp, Warning, TEXT("New PathFound"));
				break;
	        }

	        // Iterate through the neighbors of the current node
	        for (FlockingNode* Neighbour : Grid->GetNeighbours(CurrentNode))
	        {
	            // Skip unwalkable or nodes in the closed set
	        	ensure (Neighbour != nullptr);
	            if (!Neighbour->IsWalkable() || ClosedSet.Contains(Neighbour))
	            {
	                continue;
	            }
	        	
	            // Calculate tentative GCost from the start node to this neighbor
	            const int NewMovementCostToNeighbour = CurrentNode->GetGCost() + NodeFunctions::GetDistanceBetweenNodes(CurrentNode, Neighbour) + CurrentNode->MovementPenalty;
	            // If this path is better than the previous one, update the neighbor
	            if (NewMovementCostToNeighbour < Neighbour->GetGCost() || !OpenSet.Contains(Neighbour))
	            {
	                Neighbour->SetGCost(NewMovementCostToNeighbour);
	                Neighbour->SetHCost(NodeFunctions::GetDistanceBetweenNodes(Neighbour, EndNode));
	                Neighbour->SetFCost(Neighbour->GetGCost() + Neighbour->GetHCost());
	                Neighbour->ParentNode = CurrentNode;

	                // Add the neighbor to the open set if not already there
	                if (!OpenSet.Contains(Neighbour))
	                {
	                	OpenSet.Add(Neighbour);
	                }
            		else
            		{
            			OpenSet.UpdateItem(Neighbour);
            		}
	            }
	        }
		}
	}
	if (bPathSuccess)
	{
		bHasPath = true;
		OldWayPoints = WayPoints;
		WayPoints = NodeFunctions::RetracePath(StartNode, EndNode);
		return true;
	}
	bHasPath = false;
	return false;
}
void Pathfinder::ResetFlowFieldNodeCosts() const
{
	for(int x = 0; x < Grid->GridLengthX; x++)
	{
		for(int y = 0; y < Grid->GridLengthY; y++)
		{
			for(int z = 0; z < Grid->GridLengthZ; z++)
			{
				Grid->GetNodeFromGrid(x, y, z)->SetFlowFieldCost(INT_MAX);
			}
		}
	}
}

void Pathfinder::SetDirectionInUnWalkableNode(FlockingNode* NeighbourNode) const 
{
	// to find lowest cost, set lowest cost to max value
	int LowestCostAwayFromNode = INT_MAX;
	// check all neighbours. If neighbour has a lower cost than lowest and is walkable, set direction to it
	for (const FlockingNode* NeighboursNeighbour : Grid->GetNeighbours(NeighbourNode))
	{
		const int CostToNeighbour = NodeFunctions::GetCostToNode(NeighbourNode, NeighboursNeighbour);
		if (NeighboursNeighbour->IsWalkable() && CostToNeighbour < LowestCostAwayFromNode)
		{
			NeighbourNode->SetDirection(NeighboursNeighbour);
			LowestCostAwayFromNode = CostToNeighbour;
		}
	}
}
