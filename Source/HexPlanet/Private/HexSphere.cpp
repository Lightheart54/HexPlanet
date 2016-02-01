// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "HexSphere.h"
#include "GridGenerator.h"
#include "ContinentGenerator.h"
#include <limits>

// Sets default values
AHexSphere::AHexSphere() 
{
	USceneComponent* SphereComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	
	framesPerRotation = 2000;

	//Grid Settings	
	gridRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GridRoot"));
	gridRoot->AttachTo(RootComponent);
	radius = 50;
	numSubvisions = 0;
	gridGenerator = new GridGenerator(numSubvisions);
	surfaceArea = gridGenerator->getSurfaceArea(radius);
	volume = gridGenerator->getVolume(radius);
	GridTilePtrList tiles = gridGenerator->getTiles();
	numTiles = tiles.Num();
	inGame = false;

	//tectonic plate settings
	continentGen = CreateDefaultSubobject<UContinentGenerator>(TEXT("ContinentGenerator"));
	continentGen->installGrid(this, gridGenerator);
	renderPlates = false;
	platesRendered = false;
	buildPlates = true;
	tectonicPlateLineDrawer = CreateDefaultSubobject<ULineBatchComponent>(TEXT("TectonicPlateDrawer"));
	plateBoundaryLineDrawer = CreateDefaultSubobject<ULineBatchComponent>(TEXT("TectonicPlateBoundaryDrawer"));
	renderPlatesBoundaries = false;
	calcPlateBoundaries = true;
	plateBoundariesRendered = false;

	//land massing settings
	calcLandMasses = true;
	landmassesRendered = false;
	landMassLineDrawer = CreateDefaultSubobject<ULineBatchComponent>(TEXT("LandMassDrawer"));

	//rendered mesh settings
	pentagonMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PentagonStaticMesh"));
	pentagonMeshComponent->AttachTo(RootComponent);
	hexagonMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexagonStaticMesh"));
	hexagonMeshComponent->AttachTo(RootComponent);
	PentagonMeshInnerRadius = 20;
	HexagonMeshInnerRadius = 20;
	tileFillRatio = 0.95;
	GridTiles.Empty();
	instancesDirty = true;

#ifdef WITH_EDITOR
	debugMesh = CreateDefaultSubobject<ULineBatchComponent>(TEXT("DebugMeshRoot"));
	subdivisionPreviewMesh = CreateDefaultSubobject<ULineBatchComponent>(TEXT("SubdivisionPreviewGenerator"));
	debugMesh->AttachTo(RootComponent);
	subdivisionPreviewMesh->AttachTo(RootComponent);
	renderNodes = false;
	renderEdges = true;
	displayEdgeLengths = false;
	displayTileMeshes = true;
	previewNextSubdivision = false;
	inGameNumTiles = tiles.Num();
#endif // WITH_EDITOR

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AHexSphere::BeginPlay()
{
	Super::BeginPlay();
	instancesDirty = true;
	inGame = true;
}

// Called every frame
void AHexSphere::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	if (numSubvisions > gridGenerator->getNumSubdivisions())
	{
		calculateMesh(gridGenerator->getNumSubdivisions()+1);
	}
	else if (instancesDirty)
	{
		rebuildInstances();
	}
	else if (buildPlates)
	{
		buildPlates = false;
		continentGen->buildTectonicPlates();
		platesRendered = false;
	}
	else if (platesRendered != renderPlates)
	{
		displayTectonicPlates();
	}
	else if (calcPlateBoundaries)
	{
		calcPlateBoundaries = false;
		continentGen->setUpPlateBoundaries();
		plateBoundariesRendered = false;
	}
	else if (plateBoundariesRendered != renderPlatesBoundaries)
	{
		displayPlateBoundaries();
	}
	else if (calcLandMasses)
	{
		calcLandMasses = false;
		continentGen->calculateLandMasses();
		landmassesRendered = false;
	}
	else if (landmassesRendered != continentGen->overlayLandWaterMap)
	{
		displayLandMasses();
	}
	else
	{
		//rotate the sphere
		if (framesPerRotation > 0)
		{
			FRotator currentRotation = GetActorRotation();
			currentRotation.Yaw += 360.0 / framesPerRotation;
			SetActorRotation(currentRotation);
			tectonicPlateLineDrawer->SetRelativeRotation(currentRotation);
		}
	}
}

void AHexSphere::Destroyed()
{
	delete gridGenerator;
	gridGenerator = nullptr;
	Super::Destroyed();
}

void AHexSphere::PostLoad()
{
#ifdef WITH_EDITOR
	calculateMesh(numPreviewSubdivions);
	rebuildInstances();
	inGameNumTiles = 2 + (10 * FMath::Pow(3, numSubvisions));
#endif
	Super::PostLoad();
}

void AHexSphere::PostInitProperties()
{
#ifdef WITH_EDITOR
	calculateMesh(numPreviewSubdivions);
	rebuildInstances();
	inGameNumTiles = 2 + (10 * FMath::Pow(3, numSubvisions));
#endif
	Super::PostInitProperties();
}

TArray<FGridTile> AHexSphere::GetGridTiles() const
{
	TArray<FGridTile> valArray;
	GridTiles.GenerateValueArray(valArray);
	return valArray;
}

FGridTile AHexSphere::GetGridTile(const int32& tileKey) const
{
	if (GridTiles.Num() < tileKey)
	{
		return GridTiles[tileKey];
	}
	return FGridTile();
}

void AHexSphere::UpdateGridTile(const FGridTile& newTile)
{
	if (GridTiles.Num() < newTile.tileIndex)
	{
		GridTiles[newTile.tileIndex] = newTile;
	}
}

#if WITH_EDITOR  
void AHexSphere::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	//Get all of our components  
	TArray<UActorComponent*> OwnedComponents;
	GetComponents(OwnedComponents);

	//Get the name of the property that was changed  
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// We test using GET_MEMBER_NAME_CHECKED so that if someone changes the property name  
	// in the future this will fail to compile and we can update it.  
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, numSubvisions)))
	{
		inGameNumTiles = 2 + (10 * FMath::Pow(3, numSubvisions));
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, numPreviewSubdivions)))
	{
		calculateMesh(numPreviewSubdivions);
		rebuildInstances();
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, radius))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, HexagonMeshInnerRadius))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, PentagonMeshInnerRadius))
		||(PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, PentagonMesh))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, HexagonMesh))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, tileFillRatio))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, displayTileMeshes)))
	{
		hexagonMeshComponent->SetVisibility(displayTileMeshes);
		pentagonMeshComponent->SetVisibility(displayTileMeshes);
		rebuildInstances();
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, renderNodes))
		 ||(PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, renderEdges))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, renderTileCenters))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, renderEdgeCenters)))
	{
		rebuildDebugMesh();
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, displayEdgeLengths))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, displayNodeNames)))
	{
		updateDebugText();
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, previewNextSubdivision)))
	{
		genSubdivisionPreview();
	}
	
	

	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AHexSphere::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	rebuildDebugMesh();
	genSubdivisionPreview();
}

#endif  

void AHexSphere::calculateMesh(const int8& localNumSubdivisions)
{
	//clean up the old grid
	instancesDirty = true;
	pentagonMeshComponent->ClearInstances();
	hexagonMeshComponent->ClearInstances();
	gridGenerator->rebuildGrid(localNumSubdivisions);
	surfaceArea = gridGenerator->getSurfaceArea(radius);
	volume = gridGenerator->getVolume(radius);
	GridTilePtrList tiles = gridGenerator->getTiles();
	numTiles = tiles.Num();


#ifdef WITH_EDITOR
	rebuildDebugMesh();
	updateDebugText();
	genSubdivisionPreview();
#endif // WITH_EDITOR
}

void AHexSphere::rebuildInstances()
{
	instancesDirty = false;
	pentagonMeshComponent->ClearInstances();
	hexagonMeshComponent->ClearInstances();
	pentagonMeshComponent->SetStaticMesh(PentagonMesh);
	hexagonMeshComponent->SetStaticMesh(HexagonMesh);
	GridTilePtrList tiles = gridGenerator->getTiles();
	if (GridTiles.Num() < tiles.Num())
	{
		int32 tileKey = GridTiles.Num();
		while (GridTiles.Num() < tiles.Num())
		{
			FGridTile newTile;
			newTile.tileIndex = tileKey;
			GridTiles.Add(tileKey, newTile);
			++tileKey;
		}
	}

	for (const GridTilePtr& tile : tiles)
	{
		int32 tileKey = tile->getIndex();
		GridEdgePtrList tileEdges = tile->getEdges();
		if (tileEdges.Num() == 5 && PentagonMesh == nullptr)
		{
			continue;
		}
		else if (HexagonMesh == nullptr && tileEdges.Num() == 6)
		{
			continue;
		}

		float baseInnerRadius;
		UStaticMesh* myMesh = nullptr;
		int32 instanceMeshNum = 0;
		UInstancedStaticMeshComponent* tileMapMesher;
		if (tileEdges.Num() == 5)
		{
			baseInnerRadius = PentagonMeshInnerRadius;
			tileMapMesher = pentagonMeshComponent;
			myMesh = PentagonMesh;
		}
		else
		{
			baseInnerRadius = HexagonMeshInnerRadius;
			tileMapMesher = hexagonMeshComponent;
			myMesh = HexagonMesh;
		}
		FTransform instanceTransform = calcTileTransform(tileKey, baseInnerRadius);
		if (displayTileMeshes && !inGame)
		{
			tileMapMesher->AddInstance(instanceTransform);
		}

		FGridTile& thisTile = GridTiles[tileKey];
		thisTile.neighbors = tile->getNeighborIndexes();
		thisTile.tileTransform = instanceTransform;
	}
}



FTransform AHexSphere::getTileTransform(const int32& tileKey) const
{
	return GridTiles[tileKey].tileTransform;
}

TArray<FGridTileSet> AHexSphere::GetTileSets(const ETileSetTypeEnum& tileKey) const
{
	if (GridTileSets.Contains(tileKey))
	{
		return GridTileSets[tileKey];
	}
	return TArray<FGridTileSet>();
}

FGridTileSet AHexSphere::GetTileSet(const ETileSetTypeEnum& tileKey, const int32& setNum) const
{
	if (GridTileSets.Contains(tileKey))
	{
		if (GridTileSets[tileKey].Num() > setNum)
		{
			return GridTileSets[tileKey][setNum];
		}
	}
	return FGridTileSet();
}

void AHexSphere::AddOrUpdateTileSet(FGridTileSet& tileSet)
{
	if (!GridTileSets.Contains(tileSet.setType))
	{
		GridTileSets.Add(tileSet.setType, TArray<FGridTileSet>());
		tileSet.setIndex = std::numeric_limits<int32>::max();
	}

	if (GridTileSets[tileSet.setType].Num() > tileSet.setIndex)
	{
		GridTileSets[tileSet.setType][tileSet.setIndex] = tileSet;
	}
	else
	{
		int32 newIndex = GridTileSets[tileSet.setType].Add(tileSet);
		GridTileSets[tileSet.setType][tileSet.setIndex].setIndex = newIndex;
		tileSet.setIndex = newIndex;
	}
}

FTransform AHexSphere::calcTileTransform(const int32& tileKey, const float& baseTileInnerRadius)
{
	GridTilePtr tile = gridGenerator->getTile(tileKey);
	GridEdgePtrList tileEdges = tile->getEdges();
	FVector tileCenter = tile->getPosition(radius);
	FVector zDir;
	float localSphereRadius;
	tileCenter.ToDirectionAndLength(zDir, localSphereRadius);
	FVector tileInnerRadiusVec = tileEdges[0]->getPosition(radius);
	tileInnerRadiusVec -= tileInnerRadiusVec.ProjectOnTo(zDir);
	float tileInnerRadius = FMath::Sqrt(FVector::DotProduct(tileInnerRadiusVec, tileInnerRadiusVec));
	FVector yVec;
	GridTilePtrList gridNeighbors = tile->getNeighbors();
	if (tileEdges.Num() == 5)
	{
		yVec = tileInnerRadiusVec;
	}
	else
	{
		GridTilePtr refNeighbor = gridNeighbors[0];
		FVector refenceVec = refNeighbor->getPosition(radius);
		gridNeighbors.Sort([&](const GridTile& neighbor1, const GridTile& neighbor2)->bool
		{
			return FVector::DistSquared(refenceVec, neighbor1.getPosition(radius)) > FVector::DistSquared(refenceVec, neighbor2.getPosition(radius));
		});
		GridTilePtr oppositeNeighbor = gridNeighbors[0];
		yVec = refenceVec - oppositeNeighbor->getPosition(radius);
	}
	yVec -= yVec.ProjectOnTo(zDir);
	float yVecMag;
	FVector yDir;
	yVec.ToDirectionAndLength(yDir, yVecMag);
	FVector xVec = FVector::CrossProduct(yDir, zDir);
	FVector xDir;
	float tangetRad;
	xVec.ToDirectionAndLength(xDir, tangetRad);
	FTransform instanceTransform(xDir, yDir, zDir, tileCenter);

	FVector scaleVector = FVector(tileInnerRadius* tileFillRatio / baseTileInnerRadius,
		tileInnerRadius* tileFillRatio / baseTileInnerRadius,
		tileInnerRadius / baseTileInnerRadius);

	instanceTransform.SetScale3D(scaleVector);
	return instanceTransform;
}

void AHexSphere::displayTectonicPlates()
{
	platesRendered = renderPlates;
	tectonicPlateLineDrawer->Flush();
	if (renderPlates)
	{
		FVector centerPoint = GetActorLocation();
		for (const FGridTileSet& plateSet:GridTileSets[ETileSetTypeEnum::ST_PLATE])
		{
			for (const uint32& edgeIndex : plateSet.boarderEdges)
			{
				tectonicPlateLineDrawer->DrawLine(centerPoint + gridGenerator->getEdge(edgeIndex)->getStartPoint()->getPosition(radius*1.1),
					centerPoint + gridGenerator->getEdge(edgeIndex)->getEndPoint()->getPosition(radius*1.1), FLinearColor::Red, 2, 0.5);
			}
		}
	}
}

void AHexSphere::displayLandMasses()
{
	landmassesRendered = renderLandmasses;
	continentGen->hexagonLandInstanceMesher->ClearInstances();
	continentGen->pentagonLandInstanceMesher->ClearInstances();
	continentGen->hexagonOceanInstanceMesher->ClearInstances();
	continentGen->pentagonOceanInstanceMesher->ClearInstances();
	if (renderLandmasses)
	{
		FGridTileSet landTileSet = GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP][uint8(EPlateTypeEnum::PT_Land)];
		for (const int32& landTile : landTileSet.containedTiles)
		{
			GridTilePtr tilePtr = gridGenerator->getTile(landTile);
			GridEdgePtrList tileEdges = tilePtr->getEdges();
			UInstancedStaticMeshComponent* tileMesher;
			float baseInnerRadius;
			if (tileEdges.Num() == 5)
			{
				baseInnerRadius = PentagonMeshInnerRadius;
				tileMesher = continentGen->pentagonLandInstanceMesher;
			}
			else
			{
				baseInnerRadius = HexagonMeshInnerRadius;
				tileMesher = continentGen->hexagonLandInstanceMesher;
			}
			FTransform instanceTransform = getTileTransform(landTile);
			tileMesher->AddInstance(instanceTransform);
		}
		FGridTileSet oceanTileSet = GridTileSets[ETileSetTypeEnum::ST_TERRAIN_GROUP][uint8(EPlateTypeEnum::PT_Ocean)];
		for (const int32& oceanTile : oceanTileSet.containedTiles)
		{
			GridTilePtr tilePtr = gridGenerator->getTile(oceanTile);
			GridEdgePtrList tileEdges = tilePtr->getEdges();
			UInstancedStaticMeshComponent* tileMesher;
			float baseInnerRadius;
			if (tileEdges.Num() == 5)
			{
				baseInnerRadius = PentagonMeshInnerRadius;
				tileMesher = continentGen->pentagonOceanInstanceMesher;
			}
			else
			{
				baseInnerRadius = HexagonMeshInnerRadius;
				tileMesher = continentGen->hexagonOceanInstanceMesher;
			}
			FTransform instanceTransform = getTileTransform(oceanTile);
			tileMesher->AddInstance(instanceTransform);
		}
	}
}


void AHexSphere::displayPlateBoundaries()
{
	plateBoundariesRendered = renderPlatesBoundaries;
	plateBoundaryLineDrawer->Flush();
	if (renderPlatesBoundaries)
	{
		FVector centerPoint = GetActorLocation();
		for (const FGridTileSet& plateSet : GridTileSets[ETileSetTypeEnum::ST_PLATE_BOUNDARY])
		{
			for (const uint32& edgeIndex : plateSet.boarderEdges)
			{
				if (plateSet.setTags[0] == int8(EPlateBoarderType::PB_CONVERGENT))
				{
					plateBoundaryLineDrawer->DrawLine(centerPoint + gridGenerator->getEdge(edgeIndex)->getStartPoint()->getPosition(radius*1.1),
						centerPoint + gridGenerator->getEdge(edgeIndex)->getEndPoint()->getPosition(radius*1.1), FLinearColor::Blue, 2, 0.5);
				}
				else
				{
					plateBoundaryLineDrawer->DrawLine(centerPoint + gridGenerator->getEdge(edgeIndex)->getStartPoint()->getPosition(radius*1.1),
						centerPoint + gridGenerator->getEdge(edgeIndex)->getEndPoint()->getPosition(radius*1.1), FLinearColor::Yellow, 2, 0.5);
				}				
			}
		}
	}
}

#if WITH_EDITOR
void AHexSphere::buildDebugMesh()
{
	GridNodePtrList gridNodes = gridGenerator->getNodes();
	FVector centerPoint = GetActorLocation();
	if (renderNodes)
	{
		for (const GridNodePtr& gridNode : gridNodes)
		{
			debugMesh->DrawPoint(centerPoint + gridNode->getPosition(radius), FLinearColor::Blue, 8, 2);
		}
	}

	if (renderEdges|| renderEdgeCenters)
	{
		GridEdgePtrList gridEdges = gridGenerator->getEdges();
		for (const GridEdgePtr& gridEdge : gridEdges)
		{
			if (renderEdges)
			{
				debugMesh->DrawLine(centerPoint+gridEdge->getStartPoint()->getPosition(radius),
							centerPoint + gridEdge->getEndPoint()->getPosition(radius), FLinearColor::Green, 2, 0.5);
			}
			if (renderEdgeCenters)
			{
				debugMesh->DrawPoint(centerPoint + gridEdge->getPosition(radius), FLinearColor::Red, 8, 2);
			}
		}
	}

	if (renderTileCenters)
	{
		GridTilePtrList gridTiles = gridGenerator->getTiles();
		for (const GridTilePtr& gridTile : gridTiles)
		{
			debugMesh->DrawPoint(centerPoint + gridTile->getPosition(radius), FLinearColor::Yellow, 8, 2);
		}
	}
}

void AHexSphere::rebuildDebugMesh()
{
	debugMesh->Flush();

	if (renderNodes || renderEdges || renderTileCenters || renderEdgeCenters)
	{
		buildDebugMesh();
	}
}

void AHexSphere::updateDebugText()
{
	for (USceneComponent* debugText : debugTextArray)
	{
		debugText->DetachFromParent();
		debugText->DestroyComponent();
	}
	if (displayEdgeLengths)
	{
		GridEdgePtrList gridEdges = gridGenerator->getEdges();
		for (const GridEdgePtr& gridEdge : gridEdges)
		{
			FString stringForText = FString::FromInt(gridEdge->getIndex()).Append(FString(": ")).Append(FString::SanitizeFloat(gridEdge->getLength(radius)));
			FText nodeText = FText::FromString(stringForText);
			UTextRenderComponent* nodePos = NewObject<UTextRenderComponent>(this);
			nodePos->RegisterComponent();
			debugTextArray.Add(nodePos);
			nodePos->SetText(nodeText);
			nodePos->SetRelativeLocation(gridEdge->getPosition(radius));
			nodePos->SetWorldSize(1);
			nodePos->SetTextRenderColor(FColor::Black);
			nodePos->AttachTo(RootComponent);
		}
	}
	if (displayNodeNames)
	{
		GridNodePtrList gridNodes = gridGenerator->getNodes();
		for (GridNodePtr gridNode : gridNodes)
		{
			FString stringForText = FString::FromInt(gridNode->getIndex()).Append(FString(": ")).Append(gridNode->getPosition().ToString());
			FText nodeText = FText::FromString(stringForText);
			UTextRenderComponent* nodePos = NewObject<UTextRenderComponent>(this);
			nodePos->RegisterComponent();
			debugTextArray.Add(nodePos);
			nodePos->SetText(nodeText);
			nodePos->SetRelativeLocation(gridNode->getPosition(radius));
			nodePos->SetWorldSize(1);
			nodePos->SetTextRenderColor(FColor::Black);
			nodePos->AttachTo(RootComponent);
		}

	}
	
}

void AHexSphere::genSubdivisionPreview()
{
	subdivisionPreviewMesh->Flush();
	if (previewNextSubdivision)
	{
		FVector centerPoint = GetActorLocation();
		GridTilePtr tile0 = gridGenerator->getTile(0);
		previewTileSubdivision(tile0->getIndex(), centerPoint);
		GridTilePtrList tile0Neighbors = tile0->getNeighbors();
		for (GridTilePtr neighbor : tile0Neighbors)
		{
			previewTileSubdivision(neighbor->getIndex(), centerPoint);
		}

	}
}

void AHexSphere::previewTileSubdivision(uint32 tileIndex, FVector centerPoint)
{
	GridTilePtr tile0 = gridGenerator->getTile(tileIndex);
	GridEdgePtrList tile0Edges = tile0->getEdges();
	for (const GridEdgePtr& tile0Edge : tile0Edges)
	{
		FVector spPos = tile0Edge->getStartPoint()->getPosition();
		FVector spPos3D = tile0Edge->getStartPoint()->getPosition(radius);
		FVector epPos = tile0Edge->getEndPoint()->getPosition();
		FVector epPos3D = tile0Edge->getEndPoint()->getPosition(radius);
		FVector t1Pos = tile0Edge->getTiles()[0]->getPosition();
		FVector t1Pos3D = tile0Edge->getTiles()[0]->getPosition(radius);
		FVector t2Pos = tile0Edge->getTiles()[1]->getPosition();
		FVector t2Pos3D = tile0Edge->getTiles()[1]->getPosition(radius);

		subdivisionPreviewMesh->DrawLine(centerPoint + spPos3D, centerPoint + epPos3D, FLinearColor::Green, 2, 1);
		FVector newSp = gridGenerator->findAveragePoint(spPos, epPos, t1Pos);
		FVector newSp3D = newSp*radius;
		FVector newEp = gridGenerator->findAveragePoint(spPos, epPos, t2Pos);
		FVector newEp3D = newEp*radius;
		subdivisionPreviewMesh->DrawLine(centerPoint + newSp3D, centerPoint + newEp3D, FLinearColor::Red, 2, 1);

		FVector spT1 = gridGenerator->findAveragePoint(spPos, t1Pos);
		FVector spT13D = spT1*radius;
		subdivisionPreviewMesh->DrawLine(centerPoint + newSp3D, centerPoint + spT13D, FLinearColor::Yellow, 2, 1);
		FVector epT1 = gridGenerator->findAveragePoint(epPos, t1Pos);
		FVector epT13D = epT1*radius;
		subdivisionPreviewMesh->DrawLine(centerPoint + newSp3D, centerPoint + epT13D, FLinearColor::Yellow, 2, 1);
		FVector spT2 = gridGenerator->findAveragePoint(spPos, t2Pos);
		FVector spT23D = spT2*radius;
		subdivisionPreviewMesh->DrawLine(centerPoint + newEp3D, centerPoint + spT23D, FLinearColor::Yellow, 2, 1);
		FVector epT2 = gridGenerator->findAveragePoint(epPos, t2Pos);
		FVector epT23D = epT2*radius;
		subdivisionPreviewMesh->DrawLine(centerPoint + newEp3D, centerPoint + epT23D, FLinearColor::Yellow, 2, 1);

	}
}


#endif



