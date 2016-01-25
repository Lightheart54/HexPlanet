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

template <typename ValueType>
struct TFVectorCantonMapKeyFuncs :
	BaseKeyFuncs<
	TPair<FVector, ValueType>,
	FVector
	>
{
private:
	typedef BaseKeyFuncs<
		TPair<FVector, ValueType>,
		FVector
	> Super;

public:
	typedef typename Super::ElementInitType ElementInitType;
	typedef typename Super::KeyInitType     KeyInitType;

	static KeyInitType GetSetKey(ElementInitType Element)
	{
		return Element.Key;
	}

	static bool Matches(KeyInitType A, KeyInitType B)
	{
		//round to the nearest thousandth
		float x1 = FMath::RoundToInt(A.X*1000);
		float y1 = FMath::RoundToInt(A.Y*1000);
		float z1 = FMath::RoundToInt(A.Z*1000);
		float x2 = FMath::RoundToInt(B.X * 1000);
		float y2 = FMath::RoundToInt(B.Y * 1000);
		float z2 = FMath::RoundToInt(B.Z * 1000);
		return x1==x2&&y1==y2&&z1==z2;
	}

	static uint32 GetKeyHash(KeyInitType Key)
	{
		float x = Key[0];
		float y = Key[1];
		float z = Key[2];
		float xy = std::sqrt(x*x + y*y);
		float theta = std::acos(x / xy);
		if (y<0)
		{
			theta += 180.0;
		}
		float phi = std::atan(z / xy) + 90.0;
		float cantonPair = 0.5*(theta + phi)*(theta + phi + 1) + phi;
		return uint32( FMath::RoundToInt(cantonPair*1000));
	}
};


class GridNode;
typedef TWeakPtr<GridNode> GridNodeWPtr;
typedef TArray<GridNodeWPtr> GridNodeWPtrList;
typedef TSharedPtr<GridNode> GridNodePtr;
typedef TArray<GridNodePtr> GridNodePtrList;
typedef TMap<FVector, GridNodePtr, FDefaultSetAllocator, TFVectorCantonMapKeyFuncs<GridNodePtr>> GridNodeMap;
class GridEdge;
typedef TWeakPtr<GridEdge> GridEdgeWPtr;
typedef TArray<GridEdgeWPtr> GridEdgeWPtrList;
typedef TSharedPtr<GridEdge> GridEdgePtr;
typedef TArray<GridEdgePtr> GridEdgePtrList;
typedef TMap<FVector, GridEdgePtr, FDefaultSetAllocator, TFVectorCantonMapKeyFuncs<GridEdgePtr>> GridEdgeMap;
class GridTile;
typedef TWeakPtr<GridTile> GridTileWPtr;
typedef TArray<GridTileWPtr> GridTileWPtrList;
typedef TSharedPtr<GridTile> GridTilePtr;
typedef TArray<GridTilePtr> GridTilePtrList;
typedef TMap<FVector, GridTilePtr, FDefaultSetAllocator, TFVectorCantonMapKeyFuncs<GridTilePtr>> GridTileMap;