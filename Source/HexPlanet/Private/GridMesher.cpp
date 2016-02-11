// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "GridMesher.h"




UGridMesher::UGridMesher()
{
	bWantsBeginPlay = false;
	PrimaryComponentTick.bCanEverTick = false;
	myGrid = nullptr;
	radius = 200;
	meshMaterial = nullptr;

	renderNodes = false;
	//renderCorners = false;
	debugLineOut = CreateDefaultSubobject<ULineBatchComponent>(TEXT("DebugLineDrawer"));
	//numberOfNodesToMesh = -1;
}


void UGridMesher::rebuildBaseMeshFromGrid()
{
	ClearAllMeshSections();
	
	debugLineOut->Flush();

	if (myGrid != nullptr)
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

		for (const FRectGridLocation& gridLoc : myGrid->gridLocationsM)
		{
			FVector tilePos = myGrid->getNodeLocationOnSphere(gridLoc.gridPositions[0].uPos, gridLoc.gridPositions[0].vPos);
			Normals.Add(tilePos);
			Vertices.Add(tilePos * radius);
			if (renderNodes)
			{
				debugLineOut->DrawPoint(tilePos * radius, FLinearColor::Blue, 8, 2);
			}
		}

		for (int32 uLoc = 0; uLoc < myGrid->rectilinearGridM.Num(); ++ uLoc)
		{
			for (int32 vLoc = 0; vLoc <= myGrid->gridFrequency * 2;++vLoc)
			{
				if (vLoc != myGrid->gridFrequency * 2)
				{
					//upperTriangle
					int32 vertU = uLoc;
					int32 vertV = vLoc;
					Triangles.Add(myGrid->rectilinearGridM[vertU][vertV]);
					++vertV;
					Triangles.Add(myGrid->rectilinearGridM[vertU][vertV]);
					--vertV;
					myGrid->decrementU(vertU,vertV);
					Triangles.Add(myGrid->rectilinearGridM[vertU][vertV]);
				}
				if (vLoc != 0)
				{
					//lowerTriangle
					int32 vertU = uLoc;
					int32 vertV = vLoc;
					Triangles.Add(myGrid->rectilinearGridM[vertU][vertV]);
					myGrid->decrementU(vertU, vertV);
					Triangles.Add(myGrid->rectilinearGridM[vertU][vertV]);
					--vertV;
					Triangles.Add(myGrid->rectilinearGridM[vertU][vertV]);
				}
			}
		}
		
		CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, false);
		SetMaterial(0, meshMaterial);
	}
}

