// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridMesher.h"




UGridMesher::UGridMesher()
{
	bWantsBeginPlay = false;
	PrimaryComponentTick.bCanEverTick = false;
	myGrid = nullptr;
	radius = 200;

	renderNodes = false;
	renderCorners = false;
	debugLineOut = CreateDefaultSubobject<ULineBatchComponent>(TEXT("DebugLineDrawer"));
	numberOfNodesToMesh = -1;
}


void UGridMesher::rebuildBaseMeshFromGrid()
{
	ClearAllMeshSections();
	
#ifdef WITH_EDITOR
	debugLineOut->Flush();
#endif

	if (myGrid != nullptr)
	{

#ifdef WITH_EDITOR
		int32 numCorners = 0;
#endif

		for (const FRectGridLocation& gridLoc : myGrid->gridLocationsM)
		{
			/*void CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices,
				const TArray<int32>& Triangles, const TArray<FVector>& Normals,
				const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColors,
				const TArray<FProcMeshTangent>& Tangents, bool bCreateCollision);*/
			TArray<FVector> Vertices;
			TArray<int32> Triangles;
			TArray<FVector> Normals;
			TArray<FVector2D> UV0;
			TArray<FColor> VertexColors;
			TArray<FProcMeshTangent> Tangents;

			Normals.Add(myGrid->getNodeLocationOnSphere(gridLoc.gridPositions[0].uPos, gridLoc.gridPositions[0].vPos));
			Vertices.Add(Normals[0]*radius);

#ifdef WITH_EDITOR
			if (renderNodes)
			{
				debugLineOut->DrawPoint(Vertices[0], FLinearColor::Blue, 8, 2);
			}
#endif

			TArray<int32> locNeighbors = myGrid->getTileNeighborIndexes(gridLoc);
			Vertices.AddZeroed(locNeighbors.Num());
			Triangles.AddZeroed(locNeighbors.Num() * 3);
			Normals.AddZeroed(locNeighbors.Num());
			VertexColors.Init( FColor::Blue, locNeighbors.Num() + 1);

			for (int32 cornerIndex = 0; cornerIndex < locNeighbors.Num(); ++cornerIndex)
			{
				FVector currentPos = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[locNeighbors[cornerIndex]].gridPositions[0].uPos,
																	myGrid->gridLocationsM[locNeighbors[cornerIndex]].gridPositions[0].vPos);

				int32 nextIndex = cornerIndex < locNeighbors.Num() - 1 ? cornerIndex + 1 : 0;
				FVector nextPos = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[locNeighbors[nextIndex]].gridPositions[0].uPos,
					myGrid->gridLocationsM[locNeighbors[nextIndex]].gridPositions[0].vPos);
				FVector newCorner = (Normals[0] + currentPos + nextPos)/3;
				newCorner /= FMath::Sqrt(FVector::DotProduct(newCorner, newCorner));
				Normals[cornerIndex + 1] = newCorner;
				Vertices[cornerIndex + 1] = Normals[cornerIndex + 1] * radius;
#ifdef WITH_EDITOR
				if (renderCorners)
				{
					debugLineOut->DrawPoint(Vertices[cornerIndex + 1], FLinearColor::Red, 8, 2);
				}
#endif

				Triangles[cornerIndex * 3 + 1] = cornerIndex + 1;
				Triangles[cornerIndex * 3 + 2] = nextIndex + 1;
			}

			CreateMeshSection(gridLoc.tileIndex, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, false);

#ifdef WITH_EDITOR
			if (numberOfNodesToMesh > 0 && numCorners == numberOfNodesToMesh)
			{
				break;
			}
			else
			{
				numCorners++;
			}
#endif
		}
	}
}
