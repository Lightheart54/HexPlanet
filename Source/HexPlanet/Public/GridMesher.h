// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProceduralMeshComponent.h"
#include "SphereGrid.h"
#include "GridMesher.generated.h"

/**
 * Generates a mesh from a grid
 */
UCLASS()
class HEXPLANET_API UGridMesher : public UProceduralMeshComponent
{
	GENERATED_BODY()
	
public:
	UGridMesher();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridProperties",
		meta = (ClampMin = "0.1", UIMin = "0.1"))
	float radius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridProperties")
	UMaterialInterface* meshMaterial;

	UPROPERTY(EditAnywhere, Category = "DebugGridProperties")
	bool renderNodes;
	UPROPERTY(EditAnywhere, Category = "DebugGridProperties")
	ULineBatchComponent* debugLineOut;

	USphereGrid* myGrid;
	void rebuildBaseMeshFromGrid();
};
