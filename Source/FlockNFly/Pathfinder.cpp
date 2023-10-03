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
		int DistanceX = FMath::Abs(NodeA->GridX - NodeB->GridX);
		int DistanceY = FMath::Abs(NodeA->GridY - NodeB->GridY);
		int DistanceZ = FMath::Abs(NodeA->GridZ - NodeB->GridZ);;


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
		else
		{
			// Z is the longest distance
			return 14 * DistanceX + 14 * DistanceY + 10 * (DistanceZ - DistanceX);
		}
	}

	/*
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
	*/

	TArray<FlockingNode*> RetracePath(const FlockingNode* NodeA,  FlockingNode* NodeB)
	{
		TArray<FlockingNode*> Path;
		FlockingNode* Current = NodeB;

		while(Current != NodeA)
		{
			Path.Add(Current);
			Current = Current->ParentNode;
		}
		//TArray<FVector> Waypoints = SimplifyPath(Path);
		return Path;
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

	while(!NotVisited.IsEmpty())
	{
		// Set the current node to first item in list, then remove it from not visited and add to visited
		FlockingNode* Current = NotVisited[0];
		NotVisited.RemoveAt(0);
		Visited.Add(Current);
		//Current->SetIsInVisited(true);

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
					//Neighbour->SetIsInVisited(false);
				}

				Neighbour->SetFlowFieldCost(NewNeighbourCost);
				Neighbour->SetDirection(Current);
			}
		}
	}
	bIsDirectionInUnWalkableNodesSet = true; // set to true so it is only checked/set once 
}


void Pathfinder::FindPath()
{
	if (Grid->TargetLocation == FVector::ZeroVector || Grid->StartLocation == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("locations are 0"));
		return;
	}

	TargetLocation = PlayerCharacter->CurrentTargetLocation;
	FlockingNode* StartNode = Grid->GetNodeFromWorldLocation(Grid->StartLocation);
	FlockingNode* EndNode = Grid->GetNodeFromWorldLocation(TargetLocation);
	//If target has not moved, updating path is redundant
	//if(EndNode == OldEndNode) return;
	//if (StartNode == OldStartNode) return;

	//if(!bHasResetAStarNodeCosts) ResetAStarNodeCosts();

	OldEndNode = EndNode;
	OldStartNode = StartNode;

	StartNode->ParentNode = StartNode;
	StartNode->SetGCost(0.f);
	StartNode->SetFCost(0.f);
	StartNode->SetHCost(0.f);

	bool bPathSuccess = false;

	if (StartNode->bWalkable && EndNode->bWalkable)
	{
		// Create open and closed sets, should probably be something else or not created every time
		//OpenSet = new FlockingHeap(Grid->GetGridMaxSize());
		TArray<FlockingNode*> OpenSet;
		TSet<FlockingNode*> ClosedSet;

		OpenSet.Add(StartNode);

		ensure(StartNode != nullptr);
		StartNode->SetIsInOpenSet(true);
		
		
		while (OpenSet.Num() > 0)
		{
	        // Find the node in the open set with the lowest FCost

			//FlockingNode* CurrentNode = OpenSet->RemoveFirst();
			FlockingNode* CurrentNode = OpenSet[0];

			// dumma lösningen
			for (int i = 1; i < OpenSet.Num(); i++)
			{
				if (OpenSet[i]->FCost < CurrentNode->FCost ||OpenSet[i]->FCost == CurrentNode->FCost && OpenSet[i]->HCost < CurrentNode->HCost)
				{
					CurrentNode = OpenSet[i];
				}
			}

			OpenSet.Remove(CurrentNode); // dumma lösningnen
			ClosedSet.Add(CurrentNode);

	        // If the current node is the end node, path found
	        if (CurrentNode == EndNode)
	        {
	        	bPathSuccess = true;
		        UE_LOG(LogTemp, Warning, TEXT("PathFound"));
				break;

	        }

	        // Iterate through the neighbors of the current node
	        for (FlockingNode* Neighbour : Grid->GetNeighbours(CurrentNode))
	        {
	            // Skip unwalkable or nodes in the closed set
	            if (!Neighbour->IsWalkable() || ClosedSet.Contains(Neighbour))
	            {
	                continue;
	            }
	        	
	            // Calculate tentative GCost from the start node to this neighbor
		        //CurrentNode->SetGCost(NodeFunctions::GetCostToNode(CurrentNode, Neighbour));
	            int NewMovementCostToNeighbour = CurrentNode->GetGCost() + NodeFunctions::GetDistanceBetweenNodes(CurrentNode, Neighbour);
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
                		//Neighbour->SetIsInOpenSet(true);
	                }
	            	/*
            		else
            		{
            			OpenSet.UpdateItem(Neighbour);
            		}
	            	*/
	            }
	        }
		}
	}
	if (bPathSuccess)
	{
		Path = NodeFunctions::RetracePath(StartNode, EndNode);
		bHasPath = true;
		//Grid->OnDebugPathDraw();
		Grid->OnPathFound();
	}
	else
	{
		// OnGridpaht not found? Känns konstigt
	}
}
	
// Helper function to find the node with the lowest FCost in the open set
FlockingNode* Pathfinder::FindLowestFCostNode(const TArray<FlockingNode*>& NewSet)
{
	FlockingNode* LowestFCostNode = nullptr;
	float LowestFCost = MAX_FLT;

	for (FlockingNode* Node : NewSet)
	{
		ensure(Node != nullptr);
		if (Node->GetFCost() < LowestFCost)
		{
			LowestFCost = Node->GetFCost();
			LowestFCostNode = Node;
		}
	}

	ensure(LowestFCostNode != nullptr);
	return LowestFCostNode;

	/*
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
	*/
}

float Pathfinder::DistanceBetweenNodes(const FlockingNode* NodeA, const FlockingNode* NodeB)
{
	return FVector::Distance(NodeA->GetWorldCoordinate(), NodeB->GetWorldCoordinate());
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

void Pathfinder::ResetAStarNodeCosts()
{
	for(int x = 0; x < Grid->GridLengthX; x++)
	{
		for(int y = 0; y < Grid->GridLengthY; y++)
		{
			for(int z = 0; z < Grid->GridLengthZ; z++)
			{
				Grid->GetNodeFromGrid(x, y, z)->SetGCost(INT_MAX);
				Grid->GetNodeFromGrid(x, y, z)->SetHCost(INT_MAX);
				Grid->GetNodeFromGrid(x, y, z)->SetFCost(INT_MAX);
			}
		}
	}
	bHasResetAStarNodeCosts = true;
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
