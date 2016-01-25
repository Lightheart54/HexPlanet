// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridGenerator.h"
#include <initializer_list>
#include <vector>

template <typename _ArrayType>
void cleanWPtrs(_ArrayType& wPtrList)
{
	auto revIt = wPtrList.RemoveAll([](const typename _ArrayType::ElementType& wptr)->bool
	{
		return !wptr.IsValid();
	});
}

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


GridGenerator::GridGenerator(const float& _radius /*= 1.0*/, const uint8& _numSubdivisions /*= 0*/)
	: radius(_radius), numSubdivisions(_numSubdivisions)
{
	buildNewGrid();
}

GridGenerator::~GridGenerator()
{

}

void GridGenerator::rebuildGrid(const float& newRadius /*= 1.0*/, const uint8& newNumSubdivisions /*= 0*/)
{
	radius = newRadius;
	numSubdivisions = newNumSubdivisions;
	buildNewGrid();
}

void GridGenerator::rebuildGridForAGivenTileArea(const float& newRadius, const float& tileArea)
{
	radius = newRadius;
	float sphereSurfaceArea = 4 * PI * radius*radius;
	float numberOfTiles = sphereSurfaceArea / tileArea;
	numSubdivisions = 0;
	if (numberOfTiles > 12)
	{
		unsigned int numVert = 20;
		numberOfTiles -= 2;
		numberOfTiles /= 10;
		numSubdivisions = unsigned int(FMath::RoundToInt(FMath::Loge(numberOfTiles) / FMath::Loge(3)));
	}
	buildNewGrid();
}

GridTilePtr GridGenerator::getTile(const FVector& vec) const
{
	if (tiles.Contains(vec))
	{
		return tiles[vec];
	}
	return nullptr;
}

GridEdgePtr GridGenerator::getEdge(const FVector& vec) const
{
	if (edges.Contains(vec))
	{
		return edges[vec];
	}
	return nullptr;
}

GridNodePtr GridGenerator::getNode(const FVector& vec) const
{
	if (nodes.Contains(vec))
	{
		return nodes[vec];
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
	tiles.Empty(12);
	nodes.Empty(20);
	edges.Empty(30);
	createBaseGrid();
	cleanUpMemberWPtrLists();
	for (size_t i = 0; i < numSubdivisions; i++)
	{
		subdivideGrid();
		cleanUpMemberWPtrLists();
	}
}

void GridGenerator::createBaseGrid()
{
	//creating base dodecahedron
	float h = (FMath::Sqrt(5) - 1) / 2.0;
	float x = 1 + h;
	float z = 1 - FMath::Pow(h, 2);
	float crossEdgeVertexLength = 2 * z;
	float circumscribedRadius = crossEdgeVertexLength / 20 * FMath::Sqrt(250 + 110 * FMath::Sqrt(5));
	float circumscribingRadius = FMath::Sqrt(3.0);
	float pointMagnitudeRatio = radius / circumscribedRadius;
	x *= pointMagnitudeRatio;
	z *= pointMagnitudeRatio;

	TArray<FVector> vertexPos;
	vertexPos.AddZeroed(20);
	vertexPos[0][0] = 1 * pointMagnitudeRatio;
	vertexPos[0][1] = 1 * pointMagnitudeRatio;
	vertexPos[0][2] = 1 * pointMagnitudeRatio;
	vertexPos[1][0] = 1 * pointMagnitudeRatio;
	vertexPos[1][1] = 1 * pointMagnitudeRatio;
	vertexPos[1][2] = -1 * pointMagnitudeRatio;
	vertexPos[2][0] = 1 * pointMagnitudeRatio;
	vertexPos[2][1] = -1 * pointMagnitudeRatio;
	vertexPos[2][2] = 1 * pointMagnitudeRatio;
	vertexPos[3][0] = 1 * pointMagnitudeRatio;
	vertexPos[3][1] = -1 * pointMagnitudeRatio;
	vertexPos[3][2] = -1 * pointMagnitudeRatio;
	vertexPos[4] = -vertexPos[3];
	vertexPos[5] = -vertexPos[2];
	vertexPos[6] = -vertexPos[1];
	vertexPos[7] = -vertexPos[0];
	vertexPos[8][0] = 0.0;
	vertexPos[8][1] = x;
	vertexPos[8][2] = z;
	vertexPos[9][0] = 0.0;
	vertexPos[9][1] = x;
	vertexPos[9][2] = -z;
	vertexPos[10] = -vertexPos[9];
	vertexPos[11] = -vertexPos[8];
	vertexPos[12][0] = x;
	vertexPos[12][1] = z;
	vertexPos[12][2] = 0.0;
	vertexPos[13][0] = x;
	vertexPos[13][1] = -z;
	vertexPos[13][2] = 0.0;
	vertexPos[14] = -vertexPos[13];
	vertexPos[15] = -vertexPos[12];
	vertexPos[16][0] = z;
	vertexPos[16][1] = 0.0;
	vertexPos[16][2] = x;
	vertexPos[17][0] = -z;
	vertexPos[17][1] = 0.0;
	vertexPos[17][2] = x;
	vertexPos[18] = -vertexPos[17];
	vertexPos[19] = -vertexPos[16];

	for (const FVector& pVec : vertexPos)
	{
		createNode(pVec);
	}
	createTile(initializeArray(vertexPos[0],	vertexPos[16],	vertexPos[2],	vertexPos[13],	vertexPos[12] ));
	createTile(initializeArray(vertexPos[1],	vertexPos[12],	vertexPos[13],	vertexPos[3],	vertexPos[18] ));
	createTile(initializeArray(vertexPos[0],	vertexPos[8],	vertexPos[4],	vertexPos[17],	vertexPos[16] ));
	createTile(initializeArray(vertexPos[6],	vertexPos[10],	vertexPos[2],	vertexPos[16],	vertexPos[17] ));
	createTile(initializeArray(vertexPos[5],	vertexPos[14],	vertexPos[4],	vertexPos[8],	vertexPos[9]  ));
	createTile(initializeArray(vertexPos[1],	vertexPos[9],	vertexPos[8],	vertexPos[0],	vertexPos[12] ));
	createTile(initializeArray(vertexPos[3],	vertexPos[13],	vertexPos[2],	vertexPos[10],	vertexPos[11] ));
	createTile(initializeArray(vertexPos[7],	vertexPos[11],	vertexPos[10],	vertexPos[6],	vertexPos[15] ));
	createTile(initializeArray(vertexPos[5],	vertexPos[9],	vertexPos[1],	vertexPos[18],	vertexPos[19] ));
	createTile(initializeArray(vertexPos[7],	vertexPos[19],	vertexPos[18],	vertexPos[3],	vertexPos[11] ));
	createTile(initializeArray(vertexPos[5],	vertexPos[19],	vertexPos[7],	vertexPos[15],	vertexPos[14] ));
	createTile(initializeArray(vertexPos[4],	vertexPos[14],	vertexPos[15],	vertexPos[6],	vertexPos[17] ));
}

void GridGenerator::subdivideGrid()
{
	GridTileMap oldTiles = tiles;
	GridEdgeMap oldEdges = edges;
	GridNodeMap oldNodes = nodes;
	tiles.Empty(tiles.Num()+nodes.Num());
	nodes.Empty(2*edges.Num());
	edges.Empty(3*edges.Num());

	//first subdivide the old tiles, while we're at it keep track of the new corners that
	//will go with each of the old corners to make our new tiles
	TMap<GridNodePtr, GridNodePtrList> newTileCornerMap;
	for (auto tilePair : oldTiles)
	{
		FVector tileCenter = tilePair.Value->getPosition();
		float tileRad;
		FVector tileCenterUV;
		tileCenter.ToDirectionAndLength(tileCenterUV, tileRad);
		GridEdgePtrList tileEdges = tilePair.Value->getEdges();
		GridNodePtrList newCorners;
		for (const GridEdgePtr& tileEdge : tileEdges)
		{
			FVector edgeCenter = tileEdge->getPosition();
			FVector edgeCenterUV = edgeCenter/FMath::Sqrt(FVector::DotProduct(edgeCenter,edgeCenter));
			FVector edgestart = tileEdge->getEndPoints()[0]->getPosition();
			FVector edgeEnd = tileEdge->getEndPoints()[1]->getPosition();
			FVector cenMid = edgeCenter - tileCenter;
			float magCenMid = FMath::Sqrt(FVector::DotProduct(cenMid,cenMid));
			FVector cenMidUV = cenMid / magCenMid;
			FVector cenEnd = edgeEnd - tileCenter;
			FVector cenEndUV = cenEnd / FMath::Sqrt(FVector::DotProduct(cenEnd, cenEnd));
			//the new half edge length must preserve the interior angle of tile for the edge
			//and also result in a corner that is the half length is the distance between the
			//splitting plane along the miter resulting between the tiles that are going to be
			//created for either corner
			float sineHalfTileInteriorAngle = FMath::Sqrt(1 - FMath::Pow(FVector::DotProduct(cenMidUV, cenEndUV), 2));
			float halfEdgeAngleCos = FMath::Abs(FVector::DotProduct(tileCenterUV, edgeCenterUV));
			float newHalfEdgeLength = (magCenMid* sineHalfTileInteriorAngle) / (1 + sineHalfTileInteriorAngle / halfEdgeAngleCos);
			float projectedLength = newHalfEdgeLength / halfEdgeAngleCos;
			FVector newCornerLoc = edgeCenter + -1 * cenMidUV*projectedLength;//cenMidUV points from the tile center to the edgeCenter
			GridNodePtr newCorner = createNode(newCornerLoc);
			newCorners.Add(newCorner);
			for (const GridNodePtr& oldCorner : tileEdge->getEndPoints())
			{
				if (newTileCornerMap.Contains(oldCorner))
				{
					newTileCornerMap[oldCorner].Add(newCorner);
				}
				else
				{
					newTileCornerMap.Add(oldCorner, initializeArray( newCorner ));
				}
			}
		}
		//remake our old tile with its new corners
		createTile(newCorners);
	}

	//now make our new tiles
	for (auto& newTileCornerPair : newTileCornerMap)
	{
		createTile(newTileCornerPair.Value);
	}
}

void GridGenerator::cleanUpMemberWPtrLists() const
{
	for (auto tilePair : tiles)
	{
		cleanWPtrs(tilePair.Value->edges);
	}
	for (auto edgePair : edges)
	{
		cleanWPtrs(edgePair.Value->tiles);
		cleanWPtrs(edgePair.Value->nodes);
	}
	for (auto nodePair : nodes)
	{
		cleanWPtrs(nodePair.Value->edges);
	}
}

GridNodePtr GridGenerator::createNode(const FVector& pos)
{
	GridNodePtr newNode = MakeShareable(new GridNode(pos));
	nodes.Add(pos, newNode);
	return newNode;
}

GridEdgePtr GridGenerator::createEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint)
{
	GridEdgePtr newEdge = MakeShareable(new GridEdge(startPoint, endPoint));
	startPoint->edges.Add(newEdge);
	endPoint->edges.Add(newEdge);
	return newEdge;
}

GridEdgePtr GridGenerator::createAndRegisterEdge(const GridNodePtr& startPoint, const GridNodePtr& endPoint)
{
	GridEdgePtr newEdge = createEdge(startPoint, endPoint);
	if (!edges.Contains(newEdge->getPosition()))
	{
		edges.Add( newEdge->getPosition(),newEdge );
	}
	else
	{
		newEdge = edges[newEdge->getPosition()];
	}
	return newEdge;
}

GridTilePtr GridGenerator::createTile(const GridEdgePtrList& edgeLoop)
{
	GridTilePtr newTile = MakeShareable(new GridTile(edgeLoop));
	tiles.Add(newTile->getPosition(),newTile);
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
		FVector currentPos = nextNode->getPosition();
		loopNodes.Sort(
			[&currentPos](const GridNodePtr& c1, const GridNodePtr& c2)->bool
		{
			FVector c1Pos = c1->getPosition() - currentPos;
			FVector c2Pos = c2->getPosition() - currentPos;
			return FVector::DotProduct(c1Pos, c1Pos) < FVector::DotProduct(c2Pos, c2Pos);
		});
		GridNodePtr option1 = loopNodes[1]; // cornerPoint[0] should now be the current point
		GridNodePtr option2 = loopNodes[2]; // cornerPoint[0] should now be the current point
		FVector c1Pos = option1->getPosition() - currentPos;
		FVector c2Pos = option2->getPosition() - currentPos;
		FVector c1Xc2 = FVector::CrossProduct(c1Pos, c2Pos);
		float orderIndicator = FVector::DotProduct(c1Xc2, currentPos);
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
