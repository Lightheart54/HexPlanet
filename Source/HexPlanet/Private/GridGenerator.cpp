// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridGenerator.h"
#include <initializer_list>
#include <limits>
#include <vector>
#include <utility>
#include <cassert>

template<typename _ValueType, typename ...Args>
void packArray(TArray<_ValueType>& target, const Args& ...args)
{
	const int numElements = sizeof...(args);
	std::vector<_ValueType> tempArray = {args...};
	for (size_t i = 0; i < numElements; i++)
	{
		target.Add(tempArray[i]);
	}
}

template <typename _ValueType, typename ...Args>
TArray<_ValueType> initializeArray(const _ValueType& firstValue,const Args& ...args)
{
	TArray<_ValueType> newArray;
	packArray(newArray, firstValue, args...);
	return newArray;
}

template <typename _KeyType, typename _ValueType>
void addToMappedList(TMap < _KeyType, TArray<_ValueType>>& targetMap, const _KeyType& mapKey, const _ValueType& newValue)
{
	if (targetMap.Contains(mapKey))
	{
		if (!targetMap[mapKey].Contains(newValue))
		{
			targetMap[mapKey].Add(newValue);
		}
	}
	else
	{
		targetMap.Add(mapKey, initializeArray(newValue));
	}
}

GridGenerator::GridGenerator(const uint8& _numSubdivisions /*= 0*/) : numSubdivisions(_numSubdivisions),
nextEdgeNumber(0),nextTileNumber(0),nextNodeNumber(0)
{
	buildNewGrid();
}

GridGenerator::~GridGenerator()
{
	deleteOwnedItems(tiles);
	deleteOwnedItems(nodes);
	deleteOwnedItems(edges);
}

void GridGenerator::rebuildGrid(const uint8& newNumSubdivisions /*= 0*/)
{
	if (newNumSubdivisions < numSubdivisions)
	{
		numSubdivisions = newNumSubdivisions;
		buildNewGrid();
	}
	else
	{
		for (size_t i = numSubdivisions; i < newNumSubdivisions; i++)
		{
			subdivideGrid(false);
		}
		numSubdivisions = newNumSubdivisions;
	}
}

const uint8& GridGenerator::getNumSubdivisions() const
{
	return numSubdivisions;
}

FVector GridGenerator::getEdgeLocation(const int32& edgeIndex, const float& radius) const
{
	return edgeLocations[edgeIndex]*radius;
}

const FVector& GridGenerator::getEdgeLocation(const int32& edgeIndex) const
{
	return edgeLocations[edgeIndex];
}

FVector GridGenerator::getNodeLocation(const int32& nodeIndex, const float& radius) const
{
	return nodeLocations[nodeIndex]*radius;
}

const FVector& GridGenerator::getNodeLocation(const int32& nodeIndex) const
{
	return nodeLocations[nodeIndex];
}

FVector GridGenerator::getTileLocation(const int32& tileIndex, const float& radius) const
{
	return tiles[tileIndex]->getPosition(radius);
}

const FVector& GridGenerator::getTileLocation(const int32& tileIndex) const
{
	return nodeLocations[tileIndex];
}

GridTilePtr GridGenerator::getTile(const int32& tileIndex) const
{
	return tiles[tileIndex];
}

GridEdgePtr GridGenerator::getEdge(const int32& edgeIndex) const
{
	return edges[edgeIndex];
}

GridNodePtr GridGenerator::getNode(const int32& nodeIndex) const
{
	return nodes[nodeIndex];
}

GridTilePtrList GridGenerator::getTiles() const
{
	return tiles;
}

GridEdgePtrList GridGenerator::getEdges() const
{
	return edges;
}

GridNodePtrList GridGenerator::getNodes() const
{
	return nodes;
}

float GridGenerator::getVolume(const float& radius) const
{
	float totalVolume = 0.0;
	for (const GridTilePtr& face : tiles)
	{
		totalVolume += face->getEnclosedVolume(radius);
	}
	return totalVolume;
}

float GridGenerator::getSurfaceArea(const float& radius) const
{
	float totalSurfaceArea = 0.0;
	for (const GridTilePtr& face : tiles)
	{
		totalSurfaceArea += face->getArea(radius);
	}
	return totalSurfaceArea;
}

void GridGenerator::buildNewGrid()
{
	nextEdgeNumber = 0;
	nextNodeNumber = 0;
	nextTileNumber = 0;
	createBaseGrid();
	for (size_t i = 0; i < numSubdivisions; i++)
	{
		subdivideGrid(false);
	}
}

void GridGenerator::createBaseGrid()
{
	//creating base icosahedron
	float x = 1;
	float z = (1 + FMath::Sqrt(5)) / 2.0;
	float baseHedronRadius = FMath::Sqrt(x*x + z*z);
	float pointMagnitudeRatio = 1.0 / baseHedronRadius;
	x *= pointMagnitudeRatio;
	z *= pointMagnitudeRatio;

	tiles.SetNumZeroed(12);
	edges.SetNumZeroed(10);
	nodes.SetNumZeroed(12);
	nodeLocations.SetNumZeroed(12);
	edgeLocations.SetNumZeroed(10);

	nodeLocations[0][0] = 0;
	nodeLocations[0][1] = x;
	nodeLocations[0][2] = z;

	nodeLocations[1][0] = 0;
	nodeLocations[1][1] = x;
	nodeLocations[1][2] = -z;

	nodeLocations[2][0] = 0;
	nodeLocations[2][1] = -x;
	nodeLocations[2][2] = z;

	nodeLocations[3][0] = 0;
	nodeLocations[3][1] = -x;
	nodeLocations[3][2] = -z;

	nodeLocations[4][0] = z;
	nodeLocations[4][1] = 0;
	nodeLocations[4][2] = x;

	nodeLocations[5][0] = -z;
	nodeLocations[5][1] = 0;
	nodeLocations[5][2] = x;

	nodeLocations[6][0] = z;
	nodeLocations[6][1] = 0;
	nodeLocations[6][2] = -x;

	nodeLocations[7][0] = -z;
	nodeLocations[7][1] = 0;
	nodeLocations[7][2] = -x;

	nodeLocations[8][0] = x;
	nodeLocations[8][1] = z;
	nodeLocations[8][2] = 0;

	nodeLocations[9][0] = x;
	nodeLocations[9][1] = -z;
	nodeLocations[9][2] = 0;

	nodeLocations[10][0] = -x;
	nodeLocations[10][1] = z;
	nodeLocations[10][2] = 0;

	nodeLocations[11][0] = -x;
	nodeLocations[11][1] = -z;
	nodeLocations[11][2] = 0;

	while(nextNodeNumber < 12)
	{
		createNode(nodeLocations[nextNodeNumber]);
	}
	edges[createEdge(0, 4)]->tiles[0] = 8;
	edges[createEdge(4, 9)]->tiles[0] = 6;
	edges[createEdge(9, 11)]->tiles[0] = 3;
	edges[createEdge(11, 5)]->tiles[0] = 7;
	edges[createEdge(5, 0)]->tiles[0] = 10;
	edges[0]->tiles[1] = 2;
	edges[1]->tiles[1] = 2;
	edges[2]->tiles[1] = 2;
	edges[3]->tiles[1] = 2;
	edges[4]->tiles[1] = 2;
	createTile(2, initializeArray<int32>(0, 4, 9, 11, 5));


	edges[createEdge(8, 6)]->tiles[0] = 4;
	edges[createEdge(6, 3)]->tiles[0] = 9;
	edges[createEdge(3, 7)]->tiles[0] = 11;
	edges[createEdge(7, 10)]->tiles[0] = 5;
	edges[createEdge(10, 8)]->tiles[0] = 0;
	edges[5]->tiles[1] = 1;
	edges[6]->tiles[1] = 1;
	edges[7]->tiles[1] = 1;
	edges[8]->tiles[1] = 1;
	edges[9]->tiles[1] = 1;
	createTile(1, initializeArray<int32>(8, 6, 3, 7, 10));

	subdivideGrid();	
}

void GridGenerator::subdivideEdge(const int32& baseEdge, TMap<int32, TArray<int32>>& nodeToNodesMapOut)
{
	int32 startPointIndex = edges[baseEdge]->startPoint;
	int32 endPointIndex = edges[baseEdge]->endPoint;

	FVector startPoint = nodeLocations[startPointIndex];
	FVector endPoint = nodeLocations[endPointIndex];

	int32 tile1Index = edges[baseEdge]->tiles[0];
	int32 tile2Index = edges[baseEdge]->tiles[1];

	FVector cap1 = nodeLocations[tile1Index];
	FVector cap2 = nodeLocations[tile2Index];

	int32 newStartPoint = createNode(findAveragePoint(startPoint, endPoint, cap1));
	int32 newEndPoint = createNode(findAveragePoint(startPoint, endPoint, cap2));

	addToMappedList(nodeToNodesMapOut, edges[baseEdge]->startPoint, newStartPoint);
	addToMappedList(nodeToNodesMapOut, edges[baseEdge]->endPoint, newStartPoint);
	addToMappedList(nodeToNodesMapOut, edges[baseEdge]->tiles[0], newStartPoint);
	addToMappedList(nodeToNodesMapOut, edges[baseEdge]->startPoint, newEndPoint);
	addToMappedList(nodeToNodesMapOut, edges[baseEdge]->endPoint, newEndPoint);
	addToMappedList(nodeToNodesMapOut, edges[baseEdge]->tiles[1], newEndPoint);
	edges[baseEdge]->setStartAndEndPoints(newStartPoint, newEndPoint);
}

void GridGenerator::subdivideGrid(bool skipTiles)
{
	TMap<int32, TArray<int32>> newTileCornerMap;

	int32 baseEdgeNum = edges.Num();
	tiles.SetNumZeroed(nodes.Num());
	nodes.SetNumZeroed(nodes.Num() + 2 * edges.Num());
	edges.SetNumZeroed(3 * edges.Num());
	nodeLocations.SetNumZeroed(nodes.Num() + 2 * edges.Num());
	edgeLocations.SetNumZeroed(3 * edges.Num());

	for (size_t edgeNum = 0; edgeNum < baseEdgeNum; ++edgeNum)
	{
		subdivideEdge(edgeNum, newTileCornerMap);
	}
	if (!skipTiles)
	{
		//now make our new tiles
		for (auto& newTileCornerPair : newTileCornerMap)
		{
			createTile(newTileCornerPair.Key, newTileCornerPair.Value);
		}
	}
	else
	{
		tiles.Empty();
	}
}

int32 GridGenerator::createNode(const FVector& pos)
{
	if (nodes[nextNodeNumber] == nullptr)
	{
		int32 newIndex = nextNodeNumber;
		nodeLocations[nextNodeNumber] = pos;
		GridNodePtr newNode = new GridNode(newIndex, this);
		nodes[nextNodeNumber] = newNode;
	}
	return nextNodeNumber++;
}

int32 GridGenerator::createEdge(const int32& startPoint, const int32& endPoint)
{
	if (edges[nextEdgeNumber] == nullptr)
	{
		edgeLocations[nextEdgeNumber] = findAveragePoint(nodeLocations[endPoint], nodeLocations[startPoint]);
		edges[nextEdgeNumber] = new GridEdge(nextEdgeNumber, this);
	}
	edges[nextEdgeNumber]->setStartAndEndPoints(startPoint, endPoint);
	return nextEdgeNumber++;
}

FVector GridGenerator::findAveragePoint(const FVector& vec1, const FVector& vec2) const
{
	FVector average3d = vec1 + vec2;
	float averageMag = FMath::Sqrt(FVector::DotProduct(average3d,average3d));
	average3d /= averageMag;
	return average3d;
}

FVector GridGenerator::findAveragePoint(const FVector& vec1, const FVector& vec2, const FVector& vec3) const
{
	FVector average3d = vec1 + vec2 + vec3;
	float averageMag = FMath::Sqrt(FVector::DotProduct(average3d, average3d));
	average3d /= averageMag;
	return average3d;
}

int32 GridGenerator::createTile(const int32& baseNodeNum, TArray<int32> tileNodes)
{
	GridTilePtr newTile = tiles[baseNodeNum];
	if (newTile == nullptr)
	{
		tiles[baseNodeNum] = new GridTile(baseNodeNum, tileNodes.Num(), this);
		newTile = tiles[baseNodeNum];
	}
	
	int32 startNode = tileNodes[0];
	FVector startPos = nodeLocations[startNode];
	tileNodes.Sort([&](const int32& node1, const int32& node2)->bool
	{
		return FVector::Dist(startPos, nodeLocations[node1]) < FVector::Dist(startPos, nodeLocations[node2]);
	});
	TArray<std::pair<int32,int32>> edgePairs;
	edgePairs.SetNum(tileNodes.Num());
	edgePairs[0] = { tileNodes[0],tileNodes[1] };
	edgePairs[1] = { tileNodes[2],tileNodes[0] };
	if (tileNodes.Num() == 5)
	{
		edgePairs[4] = { tileNodes[4],tileNodes[3] };
	}
	else
	{
		edgePairs[4] = { tileNodes[5],tileNodes[4] };
		edgePairs[5] = { tileNodes[5],tileNodes[3] };
	}
	if (FVector::DistSquared(nodeLocations[tileNodes[1]], nodeLocations[tileNodes[3]])
		< FVector::DistSquared(nodeLocations[tileNodes[1]], nodeLocations[tileNodes[4]]))
	{
		edgePairs[2] = { tileNodes[1],tileNodes[3] };
		edgePairs[3] = { tileNodes[2],tileNodes[4] };
	}
	else
	{
		edgePairs[2] = { tileNodes[1],tileNodes[4] };
		edgePairs[3] = { tileNodes[2],tileNodes[3] };
	}

	uint8 edgeIt = 0;
	for (const std::pair<int32,int32>& edgePair : edgePairs)
	{
		GridNodePtr node1 = nodes[edgePair.first];
		GridNodePtr node2 = nodes[edgePair.second];
		if (node1->myEdges[0] == node2->myEdges[0])
		{
			newTile->myEdges[edgeIt] = node1->myEdges[0];
		}
		else if (node1->myEdges[0] == node2->myEdges[1])
		{
			newTile->myEdges[edgeIt] = node1->myEdges[0];
		}
		else if (node1->myEdges[0] == node2->myEdges[2])
		{
			newTile->myEdges[edgeIt] = node1->myEdges[0];
		}
		else if (node1->myEdges[1] != std::numeric_limits<int32>::max())
		{
			if (node1->myEdges[1] == node2->myEdges[0])
			{
				newTile->myEdges[edgeIt] = node1->myEdges[1];
			}
			else if (node1->myEdges[1] == node2->myEdges[1])
			{
				newTile->myEdges[edgeIt] = node1->myEdges[1];
			}
			else if (node1->myEdges[1] == node2->myEdges[2])
			{
				newTile->myEdges[edgeIt] = node1->myEdges[1];
			}
			else if (node1->myEdges[2] != std::numeric_limits<int32>::max())
			{
				if (node1->myEdges[2] == node2->myEdges[0])
				{
					newTile->myEdges[edgeIt] = node1->myEdges[2];
				}
				else if (node1->myEdges[2] == node2->myEdges[1])
				{
					newTile->myEdges[edgeIt] = node1->myEdges[2];
				}
				else if (node1->myEdges[2] == node2->myEdges[2])
				{
					newTile->myEdges[edgeIt] = node1->myEdges[2];
				}
				else
				{
					newTile->myEdges[edgeIt] = createEdge(node1->myIndex, node2->myIndex);
				}
			}
			else
			{
				newTile->myEdges[edgeIt] = createEdge(node1->myIndex, node2->myIndex);
			}
		}
		else
		{
			newTile->myEdges[edgeIt] = createEdge(node1->myIndex, node2->myIndex);
		}
		GridEdgePtr myEdge = edges[newTile->myEdges[edgeIt]];
		if (myEdge->tiles[0] != baseNodeNum && myEdge->tiles[1] != baseNodeNum)
		{
			if (myEdge->tiles[0] == std::numeric_limits<int32>::max())
			{
				myEdge->tiles[0] = baseNodeNum;
			}
			else
			{
				myEdge->tiles[1] = baseNodeNum;
			}
		}
		++edgeIt;
	}
	return baseNodeNum;
}
