// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "GridGenerator.h"
#include "HexSphere.h"
#include "ContinentGenerator.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EPlateTypeEnum : uint8
{
	PT_Land 	UMETA(DisplayName = "Land"),
	PT_Ocean 	UMETA(DisplayName = "Ocean"),
	PT_Unassigned	UMETA(DisplayName = "Unassigned")
};
//Tectonic Plates set their type in tag[0]
//				  list their borders in tag[1...n]


UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EPlateBoarderType : uint8
{
	PB_DIVERGENT 	UMETA(DisplayName = "Divergent"),
	PB_CONVERGENT 	UMETA(DisplayName = "Convergent")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EPlateBoarderTagIndex : uint8
{
	PBT_PLATE_TYPE 	UMETA(DisplayName = "Plate Type"),
	PBT_PLATE_1 	UMETA(DisplayName = "Plate 1 Index"),
	PBT_PLATE_2 	UMETA(DisplayName = "Plate 2 Index")
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UContinentGenerator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UContinentGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 tectonicPlateSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 numberOfPlateSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 numberOfSubPlateSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 addSubPlatesAfterIteration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TectonicPlates")
		int32 tectonicPlateBoundarySeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing")
		int32 landMassingSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
		float percentOcean;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "10", UIMax = "10"))
		int32 minPrimaryPlateTypeSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "1", UIMin = "1", ClampMax = "10", UIMax = "10"))
		int32 primaryPlateTypeDensity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "10", UIMax = "10"))
		int32 minSecondaryPlateTypeSeeds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LandMassing",
		meta = (ClampMin = "0", UIMin = "0", ClampMax = "10", UIMax = "10"))
		int32 secondaryPlateTypeDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|MapDisplay")
		bool overlayLandWaterMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|MapDisplay")
		UInstancedStaticMeshComponent* hexagonLandInstanceMesher;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|MapDisplay")
		UInstancedStaticMeshComponent* pentagonLandInstanceMesher;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|MapDisplay")
		UInstancedStaticMeshComponent* hexagonOceanInstanceMesher;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|MapDisplay")
		UInstancedStaticMeshComponent* pentagonOceanInstanceMesher;

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void installGrid(AHexSphere* myOwner, GridGenerator* gridGenerator);
	
	void buildTectonicPlates();
	void setUpPlateBoundaries();
	void calculateLandMasses();

	void createVoronoiDiagramFromSeedSets(GridTilePtrList &gridTiles, TArray<FGridTileSet>& seedSets, const uint32& maxNumIterations = -1);

	TArray<FGridTileSet> subdivideSetIntoSubgroups(const FGridTileSet& tileSet, const TArray<int32> subgroupSeedCount,
		const int32& groupSeed);
	bool addTileToTileSet(FGridTileSet& tileSet, const uint32& seedTile, GridTilePtrList& availableTiles);

	TArray<int32> getSetBorderTiles(const FGridTileSet& tileSet) const;
	void addTileSetToTileSet(FGridTileSet& set1, const FGridTileSet& set2);

protected:
	AHexSphere* gridOwner;
	GridGenerator* gridGen;
	TArray<FGridTileSet> subdividePlate(const FGridTileSet &plateSet, const int32& plateSeed);
};
