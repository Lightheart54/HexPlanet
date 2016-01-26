#pragma once

template<typename _ManagedType>
TArray<TSharedPtr<_ManagedType>> lockList(const TArray<TWeakPtr<_ManagedType>>& wPtrList)
{
	TArray<TSharedPtr<_ManagedType>> lockedPointers;
	for (const TWeakPtr<_ManagedType>& wPtr: wPtrList)
	{
		lockedPointers.Add(wPtr.Pin());
	}
	return lockedPointers;
}

FString createKeyForVector(const FVector& vectorKey);

class GridNode;
typedef TWeakPtr<GridNode> GridNodeWPtr;
typedef TArray<GridNodeWPtr> GridNodeWPtrList;
typedef TSharedPtr<GridNode> GridNodePtr;
typedef TArray<GridNodePtr> GridNodePtrList;
typedef TMap<FString, GridNodePtr> GridNodeMap;
class GridEdge;
typedef TWeakPtr<GridEdge> GridEdgeWPtr;
typedef TArray<GridEdgeWPtr> GridEdgeWPtrList;
typedef TSharedPtr<GridEdge> GridEdgePtr;
typedef TArray<GridEdgePtr> GridEdgePtrList;
typedef TMap<FString, GridEdgePtr> GridEdgeMap;
class GridTile;
typedef TWeakPtr<GridTile> GridTileWPtr;
typedef TArray<GridTileWPtr> GridTileWPtrList;
typedef TSharedPtr<GridTile> GridTilePtr;
typedef TArray<GridTilePtr> GridTilePtrList;
typedef TMap<FString, GridTilePtr> GridTileMap;