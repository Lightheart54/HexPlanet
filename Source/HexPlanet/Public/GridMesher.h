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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshProperties",
		meta = (ClampMin = "0.1", UIMin = "0.1"))
	float baseMeshRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshProperties")
	UMaterialInterface* baseMeshMaterial;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MeshProperties")
	int32 numMeshes;

	UPROPERTY(EditAnywhere, Category = "DebugGridProperties")
	bool renderNodes;
	UPROPERTY(EditAnywhere, Category = "DebugGridProperties")
	ULineBatchComponent* debugLineOut;


	UFUNCTION(BlueprintCallable, Category = "MeshProduction")
	int32 buildNewMesh(const TArray<float>& vertexRadii, const TArray<FColor>& vertexColors, UMaterialInterface* newMeshMaterial);

	USphereGrid* myGrid;
	void rebuildBaseMeshFromGrid();
};
