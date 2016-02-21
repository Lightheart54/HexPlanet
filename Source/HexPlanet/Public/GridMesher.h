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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MeshProperties")
	int32 numMeshes;

	UPROPERTY(EditAnywhere, Category = "DebugGridProperties")
	bool renderNodes; 
	UPROPERTY(EditAnywhere, Category = "DebugGridProperties")
	bool renderBaseMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshProperties")
	UMaterialInterface* baseMeshMaterial;
	UPROPERTY(EditAnywhere, Category = "DebugGridProperties")
	ULineBatchComponent* debugLineOut;


	UFUNCTION(BlueprintCallable, Category = "MeshProduction")
	int32 buildNewMesh(const TArray<float>& vertexRadii, const TArray<FColor>& vertexColors, const TArray<FVector>& vertexNormals, UMaterialInterface* newMeshMaterial, int32 meshToRebuild = -1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseMesh")
	USphereGrid* myGrid;

	void rebuildBaseMeshFromGrid();
	FVector calculateVertexNormal(const FRectGridLocation& gridLoc, const TArray<float>& vertexRadii) const;
};
