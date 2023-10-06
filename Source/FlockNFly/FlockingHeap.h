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
	
	//FlockingNode* Items;
	TArray<FlockingNode*> Items;
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
					if (FlockingNodeComparison::IsGreaterThan(*Items[ChildIndexLeft],*Items[ChildIndexRight])) SwapIndex = ChildIndexRight;
				}

                if (FlockingNodeComparison::IsGreaterThan(*Item,*Items[SwapIndex]))
				{
					Swap(Item, Items[SwapIndex]);
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

		ensure(Items.IsValidIndex(ParentIndex));
		while (true)
		{
			FlockingNode* ParentItem = Items[ParentIndex];
			ensure(ParentItem != nullptr);
            if (FlockingNodeComparison::IsLessThan(*Item, *ParentItem))
			{
				Swap(Item, ParentItem);
			}
			else
			{
				break;
			}
			ParentIndex = (Item->HeapIndex - 1) / 2;
			ensure(Items.IsValidIndex(ParentIndex));
		}
	}

	void Swap(FlockingNode* ItemA, FlockingNode* ItemB)
	{
		ensure(ItemA != nullptr);
		ensure(ItemB != nullptr);
		std::swap(Items[ItemA->HeapIndex], Items[ItemB->HeapIndex]);
		const int ItemAIndex = ItemA->HeapIndex;
		ItemA->HeapIndex = ItemB->HeapIndex;
		ItemB->HeapIndex = ItemAIndex;
	}

public:

	int MaxHeapSize = 0;

	/** Empty constructor*/
	FlockingHeap(){}

	/*
	virtual ~FlockingHeap()
	{
		delete [] Items;
	}
	*/
	
	FlockingHeap(int MHeapSize)
	{
		MaxHeapSize = MHeapSize;
		Items.SetNum(MaxHeapSize);
		ensure(Items.Num() == MaxHeapSize);
		CurrentItemCount = 0;
	}

	void Add(FlockingNode* Item)
	{
		Item->HeapIndex = CurrentItemCount;
		ensure(Items.IsValidIndex(Item->HeapIndex));
		Items[CurrentItemCount] = Item;
		SortUp(Item);
		CurrentItemCount++;
	}
	

	FlockingNode* RemoveFirst()
	{
		FlockingNode* FirstItem = Items[0];
		CurrentItemCount--;
		Items[0] = Items[CurrentItemCount];
		Items[0]->HeapIndex = 0;
		SortDown(Items[0]);
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
		ensure(Item != nullptr);
		ensure(Items.IsValidIndex(Item->HeapIndex));
		//ensure(Items[Item->HeapIndex] != nullptr);
		return Items[Item->HeapIndex] != nullptr && FlockingNodeComparison::IsEqualTo(*Items[Item->HeapIndex], *Item);
	}

	/*
	// Overload the [] operator for indexing
	FlockingNode* operator[](int Index)
	{
		return &Items[Index];
	}

	const FlockingNode* operator[](int Index) const
	{
		return &Items[Index];
	}
	*/
	
};
