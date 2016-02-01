// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HexSphere.generated.h"

class GridGenerator;
class UContinentGenerator;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ETileSetTypeEnum : uint8
{
	ST_PLATE 			UMETA(DisplayName = "Plate"),
	ST_PLATE_BOUNDARY 	UMETA(DisplayName = "Plate Boundary"),
	ST_TERRAIN_GROUP 	UMETA(DisplayName = "Terrain Group")
};

USTRUCT(BlueprintType)
struct FGridTileSet
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSetStruct")
	FString setName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSetStruct")
	int32 setIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSetStruct")
	ETileSetTypeEnum setType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSetStruct")
	TArray<int32> containedTiles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSetStruct")
	TArray<int32> boarderEdges;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileSetStruct")
	TArray<int32> setTags;
};

USTRUCT(BlueprintType)
struct FGridTile
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileStruct")
	int32 tileIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileStruct")
	TArray<int32> neighbors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileStruct")
	TArray<int32> owningTileSets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TileStruct")
	FTransform tileTransform;

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

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Properties",
		meta = (ClampMin = "0.1", UIMin = "0.1"))
	float radius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Properties",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "17", UIMax = "17"))
	int32 numSubvisions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Properties",
		meta = (ClampMin = "0.1", UIMin = "0.1", ClampMax = "1.1", UIMax = "1.1"))
	float tileFillRatio; 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Properties")
	float surfaceArea;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Properties")
	float volume;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Properties")
	int32 numTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grid Properties")
	TArray<FGridTile> GridTiles;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<FGridTile> GetGridTiles() const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	FGridTile GetGridTile(const int32& tileKey) const;
	UFUNCTION(BlueprintCallable, Category = "Grid Properties")
	void UpdateGridTile(const FGridTile& newTile);
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	FTransform getTileTransform(const int32& tileKey) const;


	TMap < ETileSetTypeEnum, TArray<FGridTileSet>> GridTileSets;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	TArray<FGridTileSet> GetTileSets(const ETileSetTypeEnum& tileKey) const;
	UFUNCTION(BlueprintPure, Category = "Grid Properties")
	FGridTileSet GetTileSet(const ETileSetTypeEnum& tileKey, const int32& setNum) const;
	UFUNCTION(BlueprintCallable, Category = "Grid Properties")
	void AddOrUpdateTileSet(FGridTileSet& tileSet);


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
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "17", UIMax = "17"))
		int8 numPreviewSubdivions;
	UPROPERTY(EditAnywhere, Category = "Debug|GridDisplay")
		bool displayTileMeshes;
	UPROPERTY(VisibleAnywhere, Category = "Grid Properties")
		int32 inGameNumTiles;
#endif


protected:
	void calculateMesh(const int8& localNumSubdivisions);
	void rebuildInstances();
	FTransform calcTileTransform(const int32& tileKey, const float& baseTileInnerRadius);
	
	UInstancedStaticMeshComponent* hexagonMeshComponent;
	UInstancedStaticMeshComponent* pentagonMeshComponent;
	USceneComponent* gridRoot;
	GridGenerator* gridGenerator;

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
