// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridGenerator.h"
#include <initializer_list>
#include <vector>

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


GridGenerator::GridGenerator(const float& _radius /*= 1.0*/, const uint8& _numSubdivisions /*= 0*/)
	: radius(_radius), numSubdivisions(_numSubdivisions)
{
	buildNewGrid();
}

GridGenerator::~GridGenerator()
{
	deleteOwnedItems(tiles);
	deleteOwnedItems(nodes);
	deleteOwnedItems(edges);
}

void GridGenerator::rebuildGrid(const float& newRadius /*= 1.0*/, const uint8& newNumSubdivisions /*= 0*/)
{
	radius = newRadius;
	numSubdivisions = newNumSubdivisions;
	buildNewGrid();
}

GridTilePtr GridGenerator::getTile(const FVector& vec) const
{
	if (tiles.Contains(createKeyForVector(vec)))
	{
		return tiles[createKeyForVector(vec)];
	}
	return nullptr;
}

GridEdgePtr GridGenerator::getEdge(const FVector& vec) const
{
	if (edges.Contains(createKeyForVector(vec)))
	{
		return edges[createKeyForVector(vec)];
	}
	return nullptr;
}

GridNodePtr GridGenerator::getNode(const FVector& vec) const
{
	if (nodes.Contains(createKeyForVector(vec)))
	{
		return nodes[createKeyForVector(vec)];
	}
	return nullptr;
}

GridTilePtrList GridGenerator::getTiles() const
{
	GridTilePtrList lockedTiles;
	tiles.GenerateValueArray(lockedTiles);
	return lockedTiles;
}

GridEdgePtrList GridGenerator::getEdges() const
{
	GridEdgePtrList lockedEdges;
	edges.GenerateValueArray(lockedEdges);
	return lockedEdges;
}

GridNodePtrList GridGenerator::getNodes() const
{
	GridNodePtrList lockedNodes;
	nodes.GenerateValueArray(lockedNodes);
	return lockedNodes;
}

float GridGenerator::getRadius() const
{
	return radius;
}

float GridGenerator::getVolume() const
{
	float totalVolume = 0.0;
	for (const GridTilePtr& face : getTiles())
	{
		totalVolume += face->getEnclosedVolume();
	}
	return totalVolume;
}

float GridGenerator::getSurfaceArea() const
{
	float totalSurfaceArea = 0.0;
	for (const GridTilePtr& face : getTiles())
	{
		totalSurfaceArea += face->getArea();
	}
	return totalSurfaceArea;
}

void GridGenerator::buildNewGrid()
{
	deleteOwnedItems(tiles);
	deleteOwnedItems(nodes);
	deleteOwnedItems(edges);
	createBaseGrid();
	for (size_t i = 0; i < numSubdivisions; i++)
	{
		subdivideGrid();
	}
}

void GridGenerator::createBaseGrid()
{
	//creating base icosahedron
	float x = 1;
	float z = (1 + FMath::Sqrt(5)) / 2.0;
	float baseHedronRadius = FMath::Sqrt(x*x + z*z);
	float pointMagnitudeRatio = radius / baseHedronRadius;
	x *= pointMagnitudeRatio;
	z *= pointMagnitudeRatio;

	TArray<FVector> vertexPos;
	vertexPos.AddZeroed(12);
	vertexPos[0][0] = 0;
	vertexPos[0][1] = x;
	vertexPos[0][2] = z;

	vertexPos[1][0] = 0;
	vertexPos[1][1] = x;
	vertexPos[1][2] = -z;

	vertexPos[2][0] = 0;
	vertexPos[2][1] = -x;
	vertexPos[2][2] = z;

	vertexPos[3][0] = 0;
	vertexPos[3][1] = -x;
	vertexPos[3][2] = -z;

	vertexPos[4][0] = z;
	vertexPos[4][1] = 0;
	vertexPos[4][2] = x;

	vertexPos[5][0] = -z;
	vertexPos[5][1] = 0;
	vertexPos[5][2] = x;

	vertexPos[6][0] = z;
	vertexPos[6][1] = 0;
	vertexPos[6][2] = -x;

	vertexPos[7][0] = -z;
	vertexPos[7][1] = 0;
	vertexPos[7][2] = -x;

	vertexPos[8][0] = x;
	vertexPos[8][1] = z;
	vertexPos[8][2] = 0;

	vertexPos[9][0] = x;
	vertexPos[9][1] = -z;
	vertexPos[9][2] = 0;

	vertexPos[10][0] = -x;
	vertexPos[10][1] = z;
	vertexPos[10][2] = 0;

	vertexPos[11][0] = -x;
	vertexPos[11][1] = -z;
	vertexPos[11][2] = 0;

	for (const FVector& pVec : vertexPos)
	{
		createNode(pVec);
	}
	TMap<GridEdgePtr, TArray<FVector>> baseTriangleSets;
	baseTriangleSets.Add(createEdge(getNode(vertexPos[0]), getNode(vertexPos[4])),
						initializeArray(vertexPos[2], vertexPos[8]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[4]), getNode(vertexPos[9])),
		initializeArray(vertexPos[2], vertexPos[6]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[9]), getNode(vertexPos[11])),
		initializeArray(vertexPos[2], vertexPos[3]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[11]), getNode(vertexPos[5])),
		initializeArray(vertexPos[2], vertexPos[7]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[5]), getNode(vertexPos[0])),
		initializeArray(vertexPos[2], vertexPos[10]));

	baseTriangleSets.Add(createEdge(getNode(vertexPos[8]), getNode(vertexPos[6])),
		initializeArray(vertexPos[1], vertexPos[4]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[6]), getNode(vertexPos[3])),
		initializeArray(vertexPos[1], vertexPos[9]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[3]), getNode(vertexPos[7])),
		initializeArray(vertexPos[1], vertexPos[11]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[7]), getNode(vertexPos[10])),
		initializeArray(vertexPos[1], vertexPos[5]));
	baseTriangleSets.Add(createEdge(getNode(vertexPos[10]), getNode(vertexPos[8])),
		initializeArray(vertexPos[1], vertexPos[0]));

	TMap<GridNodePtr, GridNodePtrList> nodeToNodesMap;
	for (const auto& triangleSet : baseTriangleSets)
	{
		subdivideTriangle(triangleSet.Key, triangleSet.Value[0], nodeToNodesMap);
		subdivideTriangle(triangleSet.Key, triangleSet.Value[1], nodeToNodesMap);
	}
	for (auto setPair : baseTriangleSets)
	{
		delete setPair.Key;
	}
	baseTriangleSets.Empty();

	//now make our new tiles
	uint32 debugOut = nodeToNodesMap.Num();
	for (auto& nodeToTileNodesPair : nodeToNodesMap)
	{
		createTile(nodeToTileNodesPair.Value);
	}
	
}

void GridGenerator::subdivideTriangle(const GridEdgePtr& baseEdge, const FVector& TriangleCap, TMap<GridNodePtr, GridNodePtrList>& nodeToNodesMapOut)
{
	GridNodePtr node1 = baseEdge->getStartPoint();
	node1->edges.Empty();
	GridNodePtr node2 = baseEdge->getEndPoint();
	node2->edges.Empty();
	GridNodePtr node3 = createNode(TriangleCap);
	//First we need our new tile center
	GridNodePtr node123 = createNode(node1->getPosition() +node2->getPosition() +node3->getPosition());
	addToMappedList(nodeToNodesMapOut, node1, node123);
	addToMappedList(nodeToNodesMapOut, node2, node123);
	addToMappedList(nodeToNodesMapOut, node3, node123);
}

void GridGenerator::subdivideGrid()
{
	GridTileMap oldTiles = tiles;
	GridEdgeMap oldEdges = edges;
	tiles.Empty(nodes.Num());
	edges.Empty(3*edges.Num());

	TMap<GridNodePtr, GridNodePtrList> newTileCornerMap;
	for (const auto& oldEdge : oldEdges)
	{
		GridTilePtrList edgeTiles = oldEdge.Value->getTiles();
		for (const GridTilePtr& edgeTile : edgeTiles)
		{
			subdivideTriangle(oldEdge.Value, edgeTile->getPosition(), newTileCornerMap);
		}
	}
	deleteOwnedItems(oldEdges);
	deleteOwnedItems(oldTiles);

	//now make our new tiles
	for (auto& newTileCornerPair : newTileCornerMap)
	{
		createTile(newTileCornerPair.Value);
	}
}

GridNodePtr GridGenerator::createNode(FVector pos)
{
	//first normalize to the radius
	pos *= radius / FMath::Sqrt(FVector::DotProduct(pos, pos));
	if (nodes.Contains(createKeyForVector(pos)))
	{
		return nodes[createKeyForVector(pos)];
	}
	GridNodePtr newNode = new GridNode(pos);
	nodes.Add(newNode->mapKey(), newNode);
	return newNode;
}

GridEdgePtr GridGenerator::createEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint)
{
	GridEdgePtr newEdge = new GridEdge(startPoint, endPoint);
	startPoint->edges.Add(newEdge);
	endPoint->edges.Add(newEdge);
	return newEdge;
}

GridEdgePtr GridGenerator::createAndRegisterEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint)
{
	GridEdgePtr newEdge = createEdge(startPoint, endPoint);
	if (!edges.Contains(newEdge->mapKey()))
	{
		edges.Add( newEdge->mapKey(),newEdge );
	}
	else
	{
		newEdge = edges[newEdge->mapKey()];
	}
	return newEdge;
}

GridTilePtr GridGenerator::createTile(const GridEdgePtrList& edgeLoop)
{
	GridTilePtr newTile = new GridTile(edgeLoop);
	tiles.Add(newTile->mapKey(),newTile);
	registerTileWithEdges(newTile);
	return newTile;
}

GridTilePtr GridGenerator::createTile(const TArray<FVector>& nodeLocs)
{
	GridEdgePtrList edgeLoop = createEdgeLoop(nodeLocs);
	return createTile(edgeLoop);
}

GridTilePtr GridGenerator::createTile(const GridNodePtrList& loopNodes)
{
	GridEdgePtrList edgeLoop = createEdgeLoop(loopNodes);
	return createTile(edgeLoop);
}

GridEdgePtrList GridGenerator::createEdgeLoop(const TArray<FVector>& nodeLocs)
{
	GridNodePtrList loopNodes;
	for (size_t i = 0; i < nodeLocs.Num(); i++)
	{
		loopNodes.Add(getNode(nodeLocs[i]));
	}
	return createEdgeLoop(loopNodes);
}

GridEdgePtrList GridGenerator::createEdgeLoop(GridNodePtrList loopNodes)
{
	GridEdgePtrList edgeloop;

	GridNodePtr nextNode = loopNodes[0];
	GridNodePtr startNode = loopNodes[0];
	do
	{
		//find the two other closest corners
		loopNodes.Sort(
			[&nextNode](const GridNode& c1, const GridNode& c2)->bool
		{
			FVector c1Pos = c1.getPosition() - nextNode->getPosition();
			FVector c2Pos = c2.getPosition() - nextNode->getPosition();
			return FVector::DotProduct(c1Pos, c1Pos) < FVector::DotProduct(c2Pos, c2Pos);
		});
		GridNodePtr option1 = loopNodes[1]; // cornerPoint[0] should now be the current point
		GridNodePtr option2 = loopNodes[2]; // cornerPoint[0] should now be the current point
		FVector c1Pos = option1->getPosition() - nextNode->getPosition();
		FVector c2Pos = option2->getPosition() - nextNode->getPosition();
		FVector c1Xc2 = FVector::CrossProduct(c1Pos, c2Pos);
		float orderIndicator = FVector::DotProduct(c1Xc2, nextNode->getPosition());
		//if the orderIndicator is negative, it indicates that c1Xc2 is in the opposite
		//direction of the position vector for the current point, which we want
		//therefore option2 is the next point, otherwise option1 is the correct choice
		if (orderIndicator < 0)
		{
			edgeloop.Add(createAndRegisterEdge(nextNode, option1));
			nextNode = option1;
		}
		else
		{
			edgeloop.Add(createAndRegisterEdge(nextNode, option2));
			nextNode = option2;
		}

	} while (nextNode != startNode);

	return edgeloop;
}



void GridGenerator::registerTileWithEdges(const GridTilePtr& tileptr)
{
	GridEdgePtrList tileEdges = tileptr->getEdges();
	for (const GridEdgePtr& edge : tileEdges)
	{
		edge->tiles.Add(tileptr);
	}
}
