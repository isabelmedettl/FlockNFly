#include "Pathfinder.h"

#include "FlockingGrid.h"
#include "FlockingHeap.h"
#include "FlockingNode.h"
#include "FlockNFlyCharacter.h"

Pathfinder::Pathfinder(APawn* PlayerActor, AFlockingGrid* MapGrid) : PlayerPawn(PlayerActor), Grid(MapGrid)
{
	PlayerCharacter = Cast<AFlockNFlyCharacter>(PlayerPawn);
	ensure(PlayerCharacter != nullptr);
	
}

Pathfinder::~Pathfinder()
{
	//delete OpenSet;
}

namespace NodeFunctions
{
	int GetDistanceBetweenNodes(const FlockingNode* NodeA, const FlockingNode* NodeB)
	{
		const int DistanceX = FMath::Abs(NodeA->GridX - NodeB->GridX);
		const int DistanceY = FMath::Abs(NodeA->GridY - NodeB->GridY);
		const int DistanceZ = FMath::Abs(NodeA->GridZ - NodeB->GridZ);
		
		if (DistanceX >= DistanceY && DistanceX >= DistanceZ)
		{
			// X is the longest distance
			return 14 * DistanceY + 10 * (DistanceX - DistanceY) + 14 * DistanceZ;
		}
		if (DistanceY >= DistanceX && DistanceY >= DistanceZ)
		{
			// Y is the longest distance
			return 14 * DistanceX + 10 * (DistanceY - DistanceX) + 14 * DistanceZ;
		}
		// Z is the longest distance
		return 14 * DistanceX + 14 * DistanceY + 10 * (DistanceZ - DistanceX);
	}

	
	TArray<FVector> SimplifyPath(const TArray<FlockingNode*>& Path)
	{
		TArray<FVector> Waypoints;
		FVector DirectionOld = FVector::ZeroVector;

		for (int32 i = 1; i < Path.Num(); i++)
		{
			FVector DirectionNew = Path[i - 1]->GetWorldCoordinate() - Path[i]->GetWorldCoordinate();
			DirectionNew.Normalize();

			if (!DirectionNew.Equals(DirectionOld, KINDA_SMALL_NUMBER))
			{
				Waypoints.Add(Path[i]->GetWorldCoordinate());
			}

			DirectionOld = DirectionNew;
		}

		return Waypoints;
	}


	//TArray<FlockingNode*> RetracePath(const FlockingNode* NodeA,  FlockingNode* NodeB)
	TArray<FVector> RetracePath(const FlockingNode* NodeA,  FlockingNode* NodeB)
	{
		TArray<FlockingNode*> Path;
		FlockingNode* Current = NodeB;

		while(Current != NodeA)
		{
			Path.Add(Current);
			Current = Current->ParentNode;
		}
		TArray<FVector> Waypoints = SimplifyPath(Path);
		return Waypoints;
	}

	/**Checks if moving from Current to NextNode would be a diagonal move, if any coordinate is same in Next och Current there is no diagonal move */
	bool IsMovingDiagonally(const FlockingNode* Current, const FlockingNode* Next)
	{
		return !(Current->GridX == Next->GridX || Current->GridY == Next->GridY || Current->GridZ == Next->GridZ); 
	}

	/** Gets cost from current to next*/
	int GetCostToNode(const FlockingNode* Current, const FlockingNode* Next)
	{
		return IsMovingDiagonally(Current, Next) ? 14 : 10; 
	}
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

bool Pathfinder::FindPath(FVector &Start, FVector &End)
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

	// here you could check that startnode is walkable as well, but because the flocking entities have a float-like behaviour sometimes
	// they wouldnt be able to get out of collision if they were in one. 
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
	            int NewMovementCostToNeighbour = CurrentNode->GetGCost() + NodeFunctions::GetDistanceBetweenNodes(CurrentNode, Neighbour) + CurrentNode->MovementPenalty;
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
void Pathfinder::ResetFlowFieldNodeCosts()
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

void Pathfinder::SetDirectionInUnWalkableNode(FlockingNode* NeighbourNode)
{
	// to find lowest cost, set lowest cost to max value
	int LowestCostAwayFromNode = INT_MAX;
	// check all neighbours. If naiogbour has a lower cost than lowest and is walkable, set direction to it
	for (FlockingNode* NeighboursNeighbour : Grid->GetNeighbours(NeighbourNode))
	{
		const int CostToNeighbour = NodeFunctions::GetCostToNode(NeighbourNode, NeighboursNeighbour);
		if (NeighboursNeighbour->IsWalkable() && CostToNeighbour < LowestCostAwayFromNode)
		{
			NeighbourNode->SetDirection(NeighboursNeighbour);
			LowestCostAwayFromNode = CostToNeighbour;
		}
	}
}
