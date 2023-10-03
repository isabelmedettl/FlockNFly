#pragma once


#include "CoreMinimal.h"
#include "FlockingNode.h"

class AFlockNFlyCharacter;
class AFlockingGrid;

namespace FlockingNodeComparison
{
	// Comparison functions for FlockingNode class

	inline bool IsGreaterThan(const FlockingNode& NodeA, const FlockingNode& NodeB)
	{
		if (NodeA.FCost == NodeB.FCost)
		{
			return NodeA.HCost > NodeB.HCost;
		}
		return NodeA.FCost > NodeB.FCost;
	}

	inline bool IsLessThan(const FlockingNode& NodeA, const FlockingNode& NodeB)
	{
		if (NodeA.FCost == NodeB.FCost)
		{
			return NodeA.HCost < NodeB.HCost;
		}
		return NodeA.FCost < NodeB.FCost;
	}

	inline bool IsEqualTo(const FlockingNode& NodeA, const FlockingNode& NodeB)
	{
		return NodeA.FCost == NodeB.FCost && NodeA.HCost == NodeB.HCost;
	}
}

class FLOCKNFLY_API FlockingHeap
{
	
	FlockingNode* Items; 
	int CurrentItemCount = 0;
	

	void SortDown(FlockingNode* Item)
	{
		while (true)
		{
			int ChildIndexLeft = Item->HeapIndex * 2 + 1;
			int ChildIndexRight = Item->HeapIndex * 2 + 2;
			int SwapIndex = 0;

			if (ChildIndexLeft < CurrentItemCount)
			{
				SwapIndex = ChildIndexLeft;

				if (ChildIndexRight < CurrentItemCount)
				{
					if (FlockingNodeComparison::IsLessThan(Items[ChildIndexLeft],Items[ChildIndexRight])) SwapIndex = ChildIndexRight;
				}

                if (FlockingNodeComparison::IsLessThan(*Item,Items[SwapIndex]))
				{
					Swap(Item, &Items[SwapIndex]);
				}
				else {
					return;
				}
			}
			else
			{
				return;
			}
		}
}

	void SortUp(FlockingNode* Item)
	{
		int ParentIndex = (Item->HeapIndex - 1) / 2;

		while (true)
		{
			FlockingNode* ParentItem = &Items[ParentIndex];
            if (FlockingNodeComparison::IsGreaterThan(*Item, *ParentItem))
			{
				Swap(Item, ParentItem);
			}
			else
			{
				break;
			}
			ParentIndex = (Item->HeapIndex - 1) / 2;
		}
	}

	void Swap(FlockingNode* ItemA, FlockingNode* ItemB)
	{
		std::swap(Items[ItemA->HeapIndex], Items[ItemB->HeapIndex]);
		const int ItemAIndex = ItemA->HeapIndex;
		ItemA->HeapIndex = ItemB->HeapIndex;
		ItemB->HeapIndex = ItemAIndex;
	}

public:

	/** Empty constructor*/
	FlockingHeap(){}

	virtual ~FlockingHeap()
	{
		delete [] Items;
	}
	
	FlockingHeap(int MaxHeapSize)
	{
		Items = new FlockingNode[MaxHeapSize]; 
		//Items.Reserve(MaxHeapSize);
		CurrentItemCount = 0;
	}

	void Add(FlockingNode* Item)
	{
		Item->HeapIndex = CurrentItemCount;
		Items[CurrentItemCount] = *Item;
		SortUp(Item);
		CurrentItemCount++;

	}

	FlockingNode* RemoveFirst()
	{
		FlockingNode* FirstItem = &Items[0];
		CurrentItemCount--;
		Items[0] = Items[CurrentItemCount];
		Items[0].HeapIndex = 0;
		SortDown(&Items[0]);
		return FirstItem;
	}

	void UpdateItem(FlockingNode* Item)
	{
		SortUp(Item);
	}

	int Count() const
	{
		return CurrentItemCount;
	}

	bool Contains(FlockingNode* Item) 
	{
		return FlockingNodeComparison::IsEqualTo(Items[Item->HeapIndex], *Item);
	}

	// Overload the [] operator for indexing
	FlockingNode* operator[](int Index)
	{
		return &Items[Index];
	}

	const FlockingNode* operator[](int Index) const
	{
		return &Items[Index];
	}

	
};
