// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HexSphere.generated.h"

class GridGenerator;
class UGridTileComponent;
class UContinentGenerator;

USTRUCT()
struct FGridTileSet
{
	GENERATED_BODY()

	TArray<int32> containedTiles;
	TArray<int32> boarderEdges;
};

UCLASS(ClassGroup = "HexGrid", meta = (BlueprintSpawnableComponent))
class HEXPLANET_API AHexSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHexSphere();


	UPROPERTY(EditAnywhere, Category = "Movement")
	int32 framesPerRotation;

	UPROPERTY(VisibleAnywhere, Category = "Grid Properties")
	float surfaceArea;
	UPROPERTY(VisibleAnywhere, Category = "Grid Properties")
	float volume;
	UPROPERTY(VisibleAnywhere, Category = "Grid Properties")
	int32 numTiles;
	UPROPERTY(EditAnywhere, Category = "Grid Properties",
		meta = (ClampMin = "0.1", UIMin = "0.1"))
	float radius;
	UPROPERTY(EditAnywhere, Category = "Grid Properties",
		meta = (ClampMin = "0.1", UIMin = "0.1", ClampMax = "1.1", UIMax = "1.1"))
	float tileFillRatio;
	UPROPERTY(EditAnywhere, Category = "Grid Properties",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 numSubvisions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	UStaticMesh* PentagonMesh; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	float PentagonMeshInnerRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	UStaticMesh* HexagonMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = StaticMeshes)
	float HexagonMeshInnerRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	bool renderPlates;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGeneration")
	bool renderLandmasses;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WorldGeneration")
	UContinentGenerator* continentGen;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	virtual void Destroyed() override;

	virtual void PostLoad() override;

	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintPure, Category = "GridNavigation")
	TArray<UGridTileComponent*> GetGridTiles() const;

	UFUNCTION(BlueprintPure, Category = "GridNavigation")
	UGridTileComponent* GetGridTile(const int32& tileKey) const;
	
	UFUNCTION(BlueprintPure, Category = "TilePosition")
	FTransform getTileTransform(const int32& tileKey, const float& baseTileInnerRadius);

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool renderEdges;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool renderNodes;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool renderTileCenters;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool renderEdgeCenters;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool displayNodeNames;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool displayEdgeLengths;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool previewNextSubdivision;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		int8 numPreviewSubdivions;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool displayTileMeshes;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay", meta = (ToolTip = "Warning This Can Have Significant In Editor Performance Implications"))
		bool displayCollisionTileMeshes;
#endif


protected:
	void calculateMesh(const int8& localNumSubdivisions);
	void rebuildInstances(bool buildCollisionComponents);


	UInstancedStaticMeshComponent* hexagonMeshComponent;
	UInstancedStaticMeshComponent* pentagonMeshComponent;
	USceneComponent* gridRoot;
	GridGenerator* gridGenerator;
	TArray<UGridTileComponent*> GridTiles;

	ULineBatchComponent* tectonicPlateLineDrawer;
	bool inGame;
	bool buildPlates;
	bool platesRendered;
	void displayTectonicPlates();
	bool calcLandMasses;
	bool landmassesRendered;
	ULineBatchComponent* landMassLineDrawer;
	void displayLandMasses();

#if WITH_EDITOR
	TArray<USceneComponent*> GridTileBuckets;
	ULineBatchComponent* debugMesh;
	TArray<UTextRenderComponent*> debugTextArray;
	ULineBatchComponent* subdivisionPreviewMesh;
	void buildDebugMesh();
	void rebuildDebugMesh();
	void updateDebugText();
	void genSubdivisionPreview();

	void previewTileSubdivision(uint32 tileIndex, FVector centerPoint);
	bool instancesDirty;
#endif
};
