// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HexSphere.generated.h"

class GridGenerator;
class UGridTileComponent;

UCLASS(ClassGroup = "HexGrid", meta = (BlueprintSpawnableComponent))
class HEXPLANET_API AHexSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHexSphere();

	UPROPERTY(VisibleAnywhere, Category = "Grid Properties")
	float surfaceArea;

	UPROPERTY(VisibleAnywhere, Category = "Grid Properties")
	float volume;

	UPROPERTY(VisibleAnywhere, Category = "Grid Properties")
	uint32 numTiles;

	UPROPERTY(EditAnywhere, Category = "Grid Properties",
		meta = (ClampMin = "0.1", UIMin = "0.1"))
	float radius;

	UPROPERTY(EditAnywhere, Category = "Grid Properties",
		meta = (ClampMin = "0.1", UIMin = "0.1", ClampMax = "1.1", UIMax = "1.1"))
	float tileFillRatio;

	UPROPERTY(EditAnywhere, Category = "Grid Properties",
		meta = (ClampMin = "0", UIMin = "0"))
	uint32 numSubvisions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	UStaticMesh* PentagonMesh; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	float PentagonMeshInnerRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	UStaticMesh* HexagonMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	float HexagonMeshInnerRadius;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	virtual void Destroyed() override;

	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) override;

	UFUNCTION(BlueprintPure, Category = "GridNavigation")
	TArray<UGridTileComponent*> GetGridTiles() const;

	UFUNCTION(BlueprintPure, Category = "GridNavigation")
	UGridTileComponent* GetGridTile(const FString& tileKey) const;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);


	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool renderEdges;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool renderNodes;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool displayEdgeLengths;
	UPROPERTY(EditAnywhere, Category = "Debug|MapDisplay")
		bool displayTileMeshes;
	UPROPERTY(EditAnywhere, Category = "Debug|MapDisplay", meta = (ToolTip = "Warning This Can Have Significant Performance Implications"))
		bool displayCollisionTileMeshes;
#endif


protected:
	void calculateMesh();
	void rebuildInstances();
	UInstancedStaticMeshComponent* hexagonMeshComponent;
	UInstancedStaticMeshComponent* pentagonMeshComponent;
	TMap<FString,UGridTileComponent*> GridTiles;
	USceneComponent* gridRoot;
	GridGenerator* gridGenerator;

#if WITH_EDITOR
	ULineBatchComponent* debugMesh;
	TArray<UTextRenderComponent*> debugTextArray;
	void buildDebugMesh();
	void rebuildDebugMesh();
	void updateDebugText();
#endif
};
