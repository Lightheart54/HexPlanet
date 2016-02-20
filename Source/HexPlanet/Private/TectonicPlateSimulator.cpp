// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "TectonicPlateSimulator.h"
#include "SimplexNoiseBPLibrary.h"
#include <limits>

static const float SEA_LEVEL = 1.0;

// Sets default values for this component's properties
UTectonicPlateSimulator::UTectonicPlateSimulator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	myMesher = nullptr; 
	myGrid = nullptr;
	plateSeed = FMath::Rand();
	numBasePlates = 12;
	numBaseSubplates = 4;
	addSubplatesAfterNSteps = 6;
	percentTilesForShapeReseed = 0.05;
	percentTilesForBorderReseed = 0.75;
	showPlateOverlay = false;
	stopAfterFirstPlate = false; 
	plateToShowCenterOfMassDebugPoints = -1;
	overlayMaterial = nullptr;
	heightMapSeed = FMath::Rand();
	numOctaves = 1; 
	percentOcean = 60;
	percentContinentalCrust = 65;
	baseContinentalHeight = 0.95;
	oceanicCrustRoughnessFactor = 0.4;
	continentalCrustFactorRoughness = 0.5;
	showInitialContinents = false;
	plateDirectionSeed = FMath::Rand();
	maxErrosionAmount = 0.1;
	errosionHeightCutoff = 95;
	radiusAboutCollisionCellToDistributeCrust = 5;
	heightMapMaterial = nullptr;
	overlayMeshIndex = -1;
	heightMapMeshIndex = -1;
	updateMesh = false;

	simulationTimeStep = 0;
	maxTimeSteps = -1;
	lithosphereDensity = 3.4;
	oceanicCrustDensity = 3.0;
	continentalCrustDensity = 2.6;
	oceanicWaterDensity = 1.025;
	// ...
}


// Called when the game starts
void UTectonicPlateSimulator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTectonicPlateSimulator::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	
	if (!updateMesh)
	{
		if (simulationTimeStep < maxTimeSteps || maxTimeSteps < 0)
		{
			executeTimeStep();
		}
		updateMesh = true;
	}
	else
	{
		if (showPlateOverlay)
		{
			meshTectonicPlateOverlay();
		}
		else if (showBaseHeightMap)
		{
			createHeightMapMesh();
		}
		updateMesh = false;
	}
	
	// ...
}

void UTectonicPlateSimulator::generateInitialHeightMap()
{
	//use 3d simplex noise to generate a continuous random starting height map
	//for our sphere
	TArray<float> initialHeightMap;
	initialHeightMap.SetNumZeroed(myGrid->numNodes);
	USimplexNoiseBPLibrary::setNoiseSeed(heightMapSeed);
	for (int32 nodeIndex = 0; nodeIndex < myGrid->numNodes;++nodeIndex)
	{
		FVector nodeLocation = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[nodeIndex]);
		float magReduction = 1;
		for (int32 octave = 0; octave < numOctaves;++octave)
		{
			initialHeightMap[nodeIndex] += USimplexNoiseBPLibrary::SimplexNoise3D(nodeLocation.X, nodeLocation.Y, nodeLocation.Z)/magReduction;
			nodeLocation *= 2;
			magReduction *= 2;
		}
	}
	
	//create a distribution of the height map in order to separate between oceanic crust and 
	//continental crust
	TArray<float> heightToSort(initialHeightMap);
	heightToSort.Sort();
	int32 oceanDepthIndex = FMath::FloorToInt(heightToSort.Num()*percentOcean / 100.0);
	int32 continentalCrustIndex = FMath::FloorToInt(heightToSort.Num()*(100.0-percentContinentalCrust) / 100.0);
	float minHeight = heightToSort[0];
	float maxHeight = heightToSort.Last();
	float baseOceanDepth = heightToSort[oceanDepthIndex] - minHeight; //use this as the normalization factor so that seaLevel == 1
	baseContinentalHeight = (heightToSort[continentalCrustIndex]-minHeight)/baseOceanDepth;
	TArray<FColor> continentKeyColor;
	continentKeyColor.SetNumZeroed(myGrid->numNodes);
	crustCells.SetNumZeroed(myGrid->numNodes);
	//normalize and separate into oceanic crust and continental crust
	for (int32 nodeIndex = 0; nodeIndex < myGrid->numNodes; ++nodeIndex)
	{
		initialHeightMap[nodeIndex] -= minHeight;
		initialHeightMap[nodeIndex] /= baseOceanDepth;
		if (initialHeightMap[nodeIndex] >= baseContinentalHeight)
		{
			initialHeightMap[nodeIndex] = SEA_LEVEL + (initialHeightMap[nodeIndex] - SEA_LEVEL)*continentalCrustFactorRoughness;
			if (initialHeightMap[nodeIndex] >= SEA_LEVEL)
			{
				continentKeyColor[nodeIndex] = FColor::Green;
			}
			else
			{
				continentKeyColor[nodeIndex] = FColor(0, 255, 255);
			}
		}
		else
		{
			initialHeightMap[nodeIndex] -= baseContinentalHeight / 2.0; //center the oceanic crust about a 0 height
			initialHeightMap[nodeIndex] *= oceanicCrustRoughnessFactor; //scale it by the roughness factor
			initialHeightMap[nodeIndex] += baseContinentalHeight * oceanicCrustRoughnessFactor / 2.0; //re-baseline it so that the minimum value is 0.0
			continentKeyColor[nodeIndex] = FColor::Blue;
		}
		crustCells[nodeIndex] = createBaseCrustCell(nodeIndex, initialHeightMap[nodeIndex]);
	}
	baseContinentalHeight *= continentalCrustFactorRoughness / baseOceanDepth;
	if (showBaseHeightMap)
	{
		createHeightMapMesh();
	}

	if (showInitialContinents)
	{
		TArray<float> overlayRadii;
		overlayRadii.Init(myMesher->baseMeshRadius, myGrid->numNodes);
		overlayMeshIndex = myMesher->buildNewMesh(overlayRadii, continentKeyColor, overlayMaterial, overlayMeshIndex);
	}
}

FCrustCellData UTectonicPlateSimulator::createBaseCrustCell(const int32& cellIndex, const float& cellHeight) const
{
	//water height is 1.0;
	FCrustCellData newCellData;
	newCellData.gridLoc = myGrid->gridLocationsM[cellIndex];
	newCellData.cellHeight = cellHeight;
	newCellData.owningPlate = -1;
	newCellData.cellTimeStamp = simulationTimeStep;
	//get the cell area
	//approximately 4*pi*radius^2/numNodes
	newCellData.crustArea = 4 * PI*FMath::Pow(myMesher->baseMeshRadius, 2) / myGrid->numNodes;
	float crustMass = continentalCrustDensity*newCellData.crustArea;

	if (cellHeight < SEA_LEVEL)
	{
		float lowerThickness = ((SEA_LEVEL - cellHeight)*oceanicWaterDensity + cellHeight*oceanicCrustDensity) / (lithosphereDensity - oceanicCrustDensity);
		newCellData.crustThickness = cellHeight + lowerThickness;
		newCellData.crustDensity = oceanicCrustDensity;
	}
	else
	{
		float lowerThickness = cellHeight*continentalCrustDensity / (lithosphereDensity - continentalCrustDensity);
		newCellData.crustThickness = cellHeight + lowerThickness;
		newCellData.crustDensity = continentalCrustDensity;
	}
	return newCellData;
}

void UTectonicPlateSimulator::buildTectonicPlates()
{
	TArray<TArray<int32>> currentPlateSets;
	currentPlateSets.Empty();
	TArray<bool> usedTiles;
	FMath::RandInit(plateSeed);
	usedTiles.Init(true, myGrid->numNodes);
	addNewSeedSetsToSetArray(usedTiles, currentPlateSets,numBasePlates);
	createVoronoiDiagramFromSeedSets(currentPlateSets,usedTiles, addSubplatesAfterNSteps);
	addNewSeedSetsToSetArray(usedTiles, currentPlateSets, numBaseSubplates);
	createVoronoiDiagramFromSeedSets(currentPlateSets, usedTiles);

	// rebuild the plates from a random set of seed tiles inside of the plate
	// to adjust the overall shape of the plate
	rebuildTectonicPlates(currentPlateSets, percentTilesForShapeReseed);
	// rebuild the plates from a random set of seed tiles inside of the plate
	// to adjust the shape of plate borders
	rebuildTectonicPlates(currentPlateSets, percentTilesForBorderReseed);

	currentPlates.Empty();
	currentPlates.SetNumZeroed(currentPlateSets.Num());
	for (int32 plateIndex = 0; plateIndex < currentPlateSets.Num();++plateIndex)
	{
		currentPlates[plateIndex] = createTectonicPlate(plateIndex, currentPlateSets[plateIndex]);
		if (stopAfterFirstPlate)
		{
			break;
		}
	}

	if (showPlateOverlay)
	{
		meshTectonicPlateOverlay();
	}
}

void UTectonicPlateSimulator::addNewSeedSetsToSetArray(TArray<bool> &usedTiles, TArray<TArray<int32>> &plateSets, const int32& numNewSets)
{
	for (int it = 0; it < numNewSets; ++it)
	{
		TArray<int32> newSet;
		int32 seedTile;
		seedTile = getNextAvailableSeedTile(usedTiles, plateSets);
		if (seedTile != -1)
		{
			newSet.Add(seedTile);
			usedTiles[seedTile] = false;
			plateSets.Add(newSet);
		}
		else
		{
			break;
		}
	}
}

int32 UTectonicPlateSimulator::getNextAvailableSeedTile(TArray<bool> &usedTiles, TArray<TArray<int32>> & plateSets)
{
	bool success = false;
	int32 seedTile;
	while (!success)
	{
		seedTile = FMath::RandRange(0, myGrid->numNodes - 1);
		if (usedTiles[seedTile])
		{
			return seedTile;
		}
		else
		{
			int32 numUsedTiles = 0;
			for (const TArray<int32>& existingSet : plateSets)
			{
				numUsedTiles += existingSet.Num();
			}
			if (numUsedTiles >= myGrid->numNodes)
			{
				break;
			}
		}
	}
	return -1;
}

void UTectonicPlateSimulator::createVoronoiDiagramFromSeedSets(TArray<TArray<int32>>& seedSets,
	TArray<bool>& tileAvailability, const int32& maxNumIterations /*= -1*/)
{
	//produce our voronoi diagram using Manhattan distances
	int32 currentIt = 0;
	bool tryAgain = true;
	while (tryAgain)
	{
		tryAgain = false;
		for (TArray<int32>& tileSet : seedSets)
		{
			int32 startNumTiles = tileSet.Num();
			myGrid->expandTileSet(tileSet, tileAvailability);
			tryAgain |= startNumTiles < tileSet.Num();
		}
		if (maxNumIterations == currentIt)
		{
			break;
		}
		currentIt++;
	}
}

void UTectonicPlateSimulator::rebuildTectonicPlates(TArray<TArray<int32>>& plateSets, const float& percentTilesForReseed)
{
	TArray<bool> usedTiles;
	usedTiles.Init(true, myGrid->numNodes);
	for (TArray<int32>& tileSet : plateSets)
	{
		int32 numSubSeeds = FMath::RoundToInt(tileSet.Num() * percentTilesForReseed) + 1;
		TArray<int32> subSeedNums;
		for (int32 subseedNum = 0; subseedNum < numSubSeeds; ++subseedNum)
		{
			int32 newSubSeed = FMath::RandRange(0, tileSet.Num() - 1);
			subSeedNums.Add(tileSet[newSubSeed]);
		}
		tileSet.Empty();
		for (const int32& subSeed : subSeedNums)
		{
			tileSet.Add(subSeed);
			usedTiles[subSeed] = false;
		}
	}
	createVoronoiDiagramFromSeedSets(plateSets,usedTiles);
}

void UTectonicPlateSimulator::meshTectonicPlateOverlay()
{
	TArray<FColor> vertexColors;
	TArray<float> vertexRadii;
	vertexColors.SetNumZeroed(myGrid->numNodes);
	vertexRadii.SetNumZeroed(myGrid->numNodes);
	for (const FTectonicPlate& tectonicPlate : currentPlates)
	{
		//make new color
		uint8 rValue = FMath::RandRange(0.0, 255);
		uint8 gValue = FMath::FRandRange(0.0, 255);
		uint8 bValue = FMath::FRandRange(0.0, 255);
		FColor plateColor(rValue, gValue, bValue);
		for (const int32& nodeIndex : tectonicPlate.ownedCrustCells)
		{
			vertexColors[nodeIndex] = plateColor;
			vertexRadii[nodeIndex] = myMesher->baseMeshRadius;
		}
		myMesher->debugLineOut->DrawPoint(myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[tectonicPlate.centerOfMassIndex])
			* myMesher->baseMeshRadius*1.1, plateColor, 10, 2);
		if (stopAfterFirstPlate)
		{
			break;
		}
	}
	overlayMeshIndex = myMesher->buildNewMesh(vertexRadii, vertexColors, overlayMaterial, overlayMeshIndex);
}

FTectonicPlate UTectonicPlateSimulator::createTectonicPlate(const int32& plateIndex, const TArray<int32>& plateCellIndexes)
{
	FTectonicPlate newPlate;
	newPlate.plateIndex = plateIndex;
	newPlate.currentVelocity = FVector(0, 0, 0);
	newPlate.ownedCrustCells.Empty(plateCellIndexes.Num());
	for (const int32& ownedCell : plateCellIndexes)
	{
		crustCells[ownedCell].owningPlate = newPlate.plateIndex;
		newPlate.ownedCrustCells.Add(ownedCell);
	}
	updatePlateCenterOfMass(newPlate);
	updatePlateBoundingRadius(newPlate);	
	return newPlate;
}

void UTectonicPlateSimulator::updatePlateCenterOfMass(FTectonicPlate &newPlate) const
{
	if (newPlate.ownedCrustCells.Num()==0)
	{
		newPlate.centerOfMassIndex = -1;
	}
	else
	{
		FVector massMomentArm(0, 0, 0);
		float totalMass = 0.0;
		for (const int32& plateCellIndex : newPlate.ownedCrustCells)
		{
			const FCrustCellData& plateCell = crustCells[plateCellIndex];
			float cellMass = plateCell.crustThickness*plateCell.crustArea*plateCell.crustDensity;
			totalMass += cellMass;
			massMomentArm += cellMass * myGrid->getNodeLocationOnSphere(plateCell.gridLoc)
				*(myMesher->baseMeshRadius + plateCell.cellHeight - plateCell.crustThickness / 2);
		}
		FVector centerOfMass = massMomentArm / totalMass;
		newPlate.plateTotalMass = totalMass;
		if (showPlateOverlay && plateToShowCenterOfMassDebugPoints == newPlate.plateIndex)
		{
			myMesher->debugLineOut->DrawPoint(centerOfMass * 1.05*myMesher->baseMeshRadius / FMath::Sqrt(FVector::DotProduct(centerOfMass, centerOfMass)),
				FLinearColor::Blue, 10, 2);
			newPlate.centerOfMassIndex = myGrid->mapPosToTileIndex(centerOfMass, myMesher->debugLineOut, myMesher->baseMeshRadius);
		}
		else
		{
			newPlate.centerOfMassIndex = myGrid->mapPosToTileIndex(centerOfMass);
		}
	}
}

void UTectonicPlateSimulator::updatePlateBoundingRadius(FTectonicPlate& newPlate) const
{
	float boundingRadius = 0.0;
	FVector plateCenterDir = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[newPlate.centerOfMassIndex]);
	for (const int32& plateCellIndex : newPlate.ownedCrustCells)
	{
		const FCrustCellData& plateCell = crustCells[plateCellIndex];
		FVector cellCenter = myGrid->getNodeLocationOnSphere(plateCell.gridLoc);
		float cellArcDistance = FMath::Acos(FVector::DotProduct(plateCenterDir, cellCenter));
		newPlate.plateBoundingRadius = FMath::Max(cellArcDistance, newPlate.plateBoundingRadius);
	}
}

void UTectonicPlateSimulator::initializePlateDirections()
{
	FMath::RandInit(plateDirectionSeed);
	for (FTectonicPlate& tecPlate : currentPlates)
	{
		FVector dirVector(FMath::FRandRange(-PI, PI), FMath::FRandRange(-PI, PI), 0);
		dirVector /= FMath::Sqrt(FVector::DotProduct(dirVector, dirVector));
		// set the initial velocity to be roughly one tile per step
		dirVector *= FMath::FRandRange(0, (PI - FMath::Acos(FMath::Sqrt(5)/3.0))/myGrid->gridFrequency);
		tecPlate.currentVelocity = dirVector;
	}
}

void UTectonicPlateSimulator::erodeCell(FCrustCellData& targetCell)
{
	//for the moment we're going to simply apply a smoothing algorithm to mimic
	//the effects of erosion, once we have a system for weather calculation
	//we can use that to drive the erosion process
	if (targetCell.cellHeight >= errosionHeightCutoff * SEA_LEVEL / 100.0)//the base continental crust height level value
	{
		//smooth the cell with it's neighbors
		TArray<int32> crustNeighbors = myGrid->getTileNeighborIndexes(targetCell.gridLoc);

		//find the lower neighbors
		crustNeighbors.RemoveAll([&](const int32& neighborIndex)->bool
		{
			return crustCells[neighborIndex].cellHeight >= targetCell.cellHeight;
		});
		if (crustNeighbors.Num() == 0)
		{
			//we're the lowest point around, therefore we're done here
			return;
		}
		//sort in descending order
		crustNeighbors.Sort([&](const int32& neighbor1, const int32& neighbor2)->bool
		{
			return crustCells[neighbor1].cellHeight >= crustCells[neighbor2].cellHeight;
		});

		//find note the minimum height difference between this cell and its lower neighbors
		//this represents the volume of material that would need to be removed to make the cell
		//the same height as its next tallest neighbor
		float minHeightDifference = targetCell.cellHeight - crustCells[crustNeighbors[0]].cellHeight;
		//setting a maxErrosion amount allows for us to keep jagged cliffs for a time
		if (minHeightDifference > SEA_LEVEL*maxErrosionAmount / 100.0)
		{
			minHeightDifference = SEA_LEVEL*maxErrosionAmount / 100.0;
		}

		//find the sum of the height differences between the cell's next tallest neighbor and the rest
		//of its shorter neighbors. This represents the capacity of its neighbors to receive material
		//without growing taller than the next tallest neighbor
		float baseNeighborCapacity = 0.0;
		for (const int32& neighborIndex : crustNeighbors)
		{
			baseNeighborCapacity += targetCell.cellHeight - minHeightDifference - crustCells[neighborIndex].cellHeight;
		}

		//two options, first the neighboring cells have enough capacity to receive all of the material to be removed
		// or they don't. If they don't reduce the minHeightDifference to their total capacity
		if (baseNeighborCapacity < minHeightDifference)
		{
			minHeightDifference = baseNeighborCapacity;
		}

		//now spread that total material about the lower neighbors
		targetCell.cellHeight -= minHeightDifference;
		TArray<int32> cellsToRelevel = crustNeighbors;
		cellsToRelevel.Add(targetCell.gridLoc.tileIndex);
		while (minHeightDifference > 0 && crustNeighbors.Num() > 0)
		{
			TArray<int32> indexesToRemove;
			for (int32 neighborIndex = 0; neighborIndex < crustNeighbors.Num(); ++neighborIndex)
			{
				float amountToAdd = minHeightDifference / (crustNeighbors.Num() - neighborIndex);
				// increase the neighbor to a maximum of the cell's new height
				if (targetCell.cellHeight <= crustCells[crustNeighbors[neighborIndex]].cellHeight)
				{
					//this index is done
					indexesToRemove.Add(crustNeighbors[neighborIndex]);
				}
				else if (targetCell.cellHeight >= crustCells[crustNeighbors[neighborIndex]].cellHeight + amountToAdd)
				{
					crustCells[crustNeighbors[neighborIndex]].cellHeight += amountToAdd;
					crustCells[crustNeighbors[neighborIndex]].crustThickness += amountToAdd;
					minHeightDifference -= amountToAdd;
				}
				else
				{
					minHeightDifference -= targetCell.cellHeight - crustCells[crustNeighbors[neighborIndex]].cellHeight;
					crustCells[crustNeighbors[neighborIndex]].crustThickness += targetCell.cellHeight - crustCells[crustNeighbors[neighborIndex]].cellHeight;
					crustCells[crustNeighbors[neighborIndex]].cellHeight = targetCell.cellHeight;
					//this index is done
					indexesToRemove.Add(crustNeighbors[neighborIndex]);
				}
			}
			for (const int32& cellToRemove : indexesToRemove)
			{
				crustNeighbors.Remove(cellToRemove);
			}
		}
		//anything we couldn't remove add it back to the cell;
		targetCell.cellHeight += minHeightDifference;
		for (const int32& cellIndexToUpdate : cellsToRelevel)
		{
			updateCrustCellHeight(crustCells[cellIndexToUpdate]);
		}
	}
}

void UTectonicPlateSimulator::updateCrustCellHeight(FCrustCellData& crustCell)
{
	//approx mass
	float startingHeight = crustCell.cellHeight;
	float startingDraft = crustCell.crustThickness - startingHeight;
	float startingWaterDepth = SEA_LEVEL - startingHeight;
	//need to address water here
	if (startingWaterDepth <= 0)
	{
		float cellMass = crustCell.crustThickness*crustCell.crustDensity;
		float cellDraft = cellMass / lithosphereDensity;
		crustCell.cellHeight = crustCell.crustThickness - cellDraft;
	}
	else
	{
		crustCell.cellHeight = (crustCell.crustThickness*(lithosphereDensity - crustCell.crustDensity) 
								- SEA_LEVEL*oceanicWaterDensity)
								/ (lithosphereDensity - oceanicWaterDensity);
	}
}

void UTectonicPlateSimulator::updateCellLocation(FCrustCellData& cellToUpdate)
{
	FTectonicPlate owningPlate = currentPlates[cellToUpdate.owningPlate];
	FRectGridLocation centerOfRotation = myGrid->gridLocationsM[owningPlate.centerOfMassIndex];
	FVector plateLocationOnSphere = myGrid->getNodeLocationOnSphere(centerOfRotation);
	FVector plateVelocity = owningPlate.currentVelocity;
	//first rotate the cell about the center of rotation first
	FVector cellLocationOnSphere = myGrid->getNodeLocationOnSphere(cellToUpdate.gridLoc);
	FVector2D oldCellSphericalLocation = cellLocationOnSphere.UnitCartesianToSpherical();
	//TODO add shear to this to model the tearing that would occur far from the plate center of rotation
	FVector rotatedCellLocationOnSphere = cellLocationOnSphere.RotateAngleAxis(plateVelocity.Z * 180.0 / PI, plateLocationOnSphere);
	//now find the new location on the sphere from the angular rotation about the sphere center
	rotatedCellLocationOnSphere /= FMath::Sqrt(FVector::DotProduct(rotatedCellLocationOnSphere, rotatedCellLocationOnSphere));
	FVector2D cellSphericalLocation = rotatedCellLocationOnSphere.UnitCartesianToSpherical();
	cellSphericalLocation.X += plateVelocity.X;
	cellSphericalLocation.Y += plateVelocity.Y;
	FVector newLocationOnSphere = cellSphericalLocation.SphericalToUnitCartesian();
	cellToUpdate.cellVelocity = cellSphericalLocation - oldCellSphericalLocation;

	int32 newIndex = myGrid->mapPosToTileIndex(newLocationOnSphere);
	cellToUpdate.gridLoc = myGrid->gridLocationsM[newIndex];
}

bool UTectonicPlateSimulator::executeTimeStep()
{
	for (FCrustCellData& crustData : crustCells)
	{
		//first erode the cells
		erodeCell(crustData);
	}


	//next move them
	//set up a array to indicate which cells have been claimed post move
	TArray<bool> claimedLocations;
	claimedLocations.Init(false, crustCells.Num());
	TArray<FCrustCellData> newCrustCells;
	newCrustCells.SetNumZeroed(crustCells.Num());


	TArray<FCrustCellData> subductions;
	TArray<FCrustCellData> collisions;
	for (FCrustCellData& crustData : crustCells)
	{
		//move the cell
		updateCellLocation(crustData);
		int32 crustDataIndex = crustData.gridLoc.tileIndex;

		//now check for collisions
		if (!claimedLocations[crustDataIndex])
		{
			//this location hasn't been claimed yet no collision here
			claimedLocations[crustDataIndex] = true;
			newCrustCells[crustDataIndex] = crustData;
			continue;
			//we're done here
		}

		if (newCrustCells[crustDataIndex].owningPlate == crustData.owningPlate)
		{
			//we're piling up on ourselves
			//just add our mass to the existing tile mass
			transferCrustFromTargetCellToExistingCell(newCrustCells[crustDataIndex], crustData, 1.0);

			//we're done here
			continue;
		}

		//we have a collision
		const bool prev_is_oceanic = crustCells[crustDataIndex].cellHeight < baseContinentalHeight;
		const bool this_is_oceanic = crustData.cellHeight < baseContinentalHeight;

		int32 prev_TimeStamp = newCrustCells[crustDataIndex].cellTimeStamp;
		int32 my_TimeStamp = crustData.cellTimeStamp;
		//find out which cell gets subducted (i.e. is less buoyant), 
		//its either the lower one or the younger one
		const bool prev_is_buoyant = (newCrustCells[crustDataIndex].cellHeight > crustData.cellHeight)
			|| ((newCrustCells[crustDataIndex].cellHeight + 2 * std::numeric_limits<float>::epsilon() > crustData.cellHeight)
				&& (newCrustCells[crustDataIndex].cellHeight < crustData.cellHeight + 2 * std::numeric_limits<float>::epsilon())
				&& (prev_TimeStamp >= my_TimeStamp)); //if they're effectively the same height take the younger one

		if (this_is_oceanic && prev_is_buoyant)
		{
			//this plate is being subducted
			//we're going underneath the other plate
			subductions.Add(crustData);
		}
		else if (prev_is_oceanic)
		{
			//this other plate is being subducted
			//add a reference to the previous ownerPlate's local crustcell
			subductions.Add(newCrustCells[crustDataIndex]);
			newCrustCells[crustDataIndex] = crustData;
		}
		else
		{
			//we're just straight colliding
			//the bigger plate gets ownership
			if (currentPlates[crustData.owningPlate].plateTotalMass > currentPlates[newCrustCells[crustDataIndex].owningPlate].plateTotalMass)
			{
				//we're the bigger plate, take ownership
				collisions.Add(newCrustCells[crustDataIndex]);
				newCrustCells[crustDataIndex] = crustData;
			}
			else
			{
				collisions.Add(crustData);
			}
		}

	}

	//create new crust where we don't have a plate owning the area
	for (int32 locationIndex = 0; locationIndex < claimedLocations.Num(); ++locationIndex)
	{
		if (!claimedLocations[locationIndex])
		{
			buildNewCrustFromPlateDivergence(locationIndex, newCrustCells);
		}
	}

	//transfer the data
	crustCells = newCrustCells;

	for (FTectonicPlate& tecPlate : currentPlates)
	{
		tecPlate.ownedCrustCells.SetNum(0);
	}

	//now we can update the plates as to who the own now
	for (FCrustCellData& crustData : crustCells)
	{
		currentPlates[crustData.owningPlate].ownedCrustCells.Add(crustData.gridLoc.tileIndex);
	}

	//alright, now we can handle each collision
	for (FCrustCellData& collisionLocation : subductions)
	{
		//the amount of crust that gets scrapped off is based upon how hard the
		//plate will be pushed against the overlapping plate once it is no longer held down
		//by water
		//right now we're going to say that its everything about the isostatic zero line
		float percentCrustToTransfer = collisionLocation.crustDensity / lithosphereDensity;
		FCrustCellData& targetCell = newCrustCells[collisionLocation.gridLoc.tileIndex];
		//get the cells surrounding the targetCell
		TArray<int32> potentialLocations = myGrid->getTileIndexesNStepsAway(collisionLocation.gridLoc, radiusAboutCollisionCellToDistributeCrust);
		FTectonicPlate& targetPlate = currentPlates[targetCell.owningPlate];
		scatterMassOverArea(targetPlate, potentialLocations, collisionLocation, percentCrustToTransfer);
		float massTransfered = percentCrustToTransfer*collisionLocation.crustThickness*collisionLocation.crustDensity;
		FVector2D velocityChange = targetCell.cellVelocity - collisionLocation.cellVelocity;
		velocityChange *= massTransfered;
		applyForceToPlate(targetPlate,targetCell.gridLoc, velocityChange);
	}
	for (FCrustCellData& collisionLocation : collisions)
	{
		//we're going to scatter the crust from the collision around the area,
		//with the folding ratio being transfered to the new plate and the rest staying on this plate
		FCrustCellData& targetCell = crustCells[collisionLocation.gridLoc.tileIndex];
		FTectonicPlate& targetPlate = currentPlates[targetCell.owningPlate];
		FTectonicPlate& smallerPlate = currentPlates[collisionLocation.owningPlate];
		float dyingCellMass = collisionLocation.crustDensity*collisionLocation.crustThickness;
		float massToTransfer = dyingCellMass*foldingRatio;
		float massToKeep = dyingCellMass - massToTransfer;
		applyForceToPlate(targetPlate, targetCell.gridLoc, (targetCell.cellVelocity-collisionLocation.cellVelocity)*massToTransfer);
		applyForceToPlate(smallerPlate, targetCell.gridLoc, (collisionLocation.cellVelocity-targetCell.cellVelocity)*massToTransfer);
		//get the cells surrounding the targetCell
		TArray<int32> potentialLocations = myGrid->getTileIndexesNStepsAway(collisionLocation.gridLoc, radiusAboutCollisionCellToDistributeCrust);
		scatterMassOverArea(targetPlate, potentialLocations, collisionLocation, foldingRatio);
		if (!scatterMassOverArea(smallerPlate, potentialLocations, collisionLocation, 1-foldingRatio))
		{
			//if there isn't anywhere left on the smaller plate to recieve it, put it all on the bigger plate
			scatterMassOverArea(targetPlate, potentialLocations, collisionLocation, 1 - foldingRatio);
			applyForceToPlate(targetPlate, targetCell.gridLoc, (targetCell.cellVelocity - collisionLocation.cellVelocity)*massToKeep);
		}
	}

	for (FTectonicPlate& tecPlate : currentPlates)
	{
		updatePlateCenterOfMass(tecPlate);
	}

	++simulationTimeStep;
	//return whether or not we had any continental collisions
	return collisions.Num() > 0;
}

void UTectonicPlateSimulator::transferCrustFromTargetCellToExistingCell(FCrustCellData &existingCrust,const FCrustCellData &targetCell, float percentCrustTransfer)
{
	float totalThickness = existingCrust.crustThickness + targetCell.crustThickness * percentCrustTransfer;
	float weightedDensity = existingCrust.crustThickness * existingCrust.crustDensity
		+ targetCell.crustDensity*targetCell.crustThickness * percentCrustTransfer;
	existingCrust.crustThickness = totalThickness;
	existingCrust.crustDensity = weightedDensity / totalThickness;
	updateCrustCellHeight(existingCrust);
}

void UTectonicPlateSimulator::applyForceToPlate(FTectonicPlate& targetPlate, const FRectGridLocation& forceLocation, const FVector2D& sphericalForce)
{
	//for simplicities sake, we're just going to treat spherical coordinates like we're in 2d
	FVector2D plateCenter = myGrid->getNodeLocationOnSphere(myGrid->gridLocationsM[targetPlate.centerOfMassIndex]).UnitCartesianToSpherical();
	FVector2D forceLoc = myGrid->getNodeLocationOnSphere(forceLocation).UnitCartesianToSpherical();
	FVector2D forceMomentArm = forceLoc - plateCenter;
	//break the force down into it's components;
	FVector2D momentArmDir = forceMomentArm / FMath::Sqrt(FVector2D::DotProduct(forceMomentArm, forceMomentArm));
	FVector2D alignedForce = FVector2D::DotProduct(sphericalForce, momentArmDir)*momentArmDir;
	float forceTorque = FVector2D::CrossProduct(sphericalForce, forceMomentArm);
	FVector plateAcceleration;
	plateAcceleration.X = alignedForce.X / targetPlate.plateTotalMass;
	plateAcceleration.Y = alignedForce.Y / targetPlate.plateTotalMass;
	plateAcceleration.Z = forceTorque / targetPlate.plateTotalMass;
	targetPlate.currentVelocity += plateAcceleration;
}


bool UTectonicPlateSimulator::scatterMassOverArea(FTectonicPlate& targetPlate, TArray<int32> potentialLocations,const FCrustCellData& collisionLocation, float transferRatio)
{
	potentialLocations.RemoveAll([&](const int32& cellIndex)->bool
	{
		return crustCells[cellIndex].owningPlate != targetPlate.plateIndex;
	});
	//get noise for each location
	TArray<float> locationArray;
	float totalNoise = 0.0;
	for (const int32& targetLoc : potentialLocations)
	{
		FVector2D targetSphericalLoc = myGrid->getNodeLocationOnSphere(crustCells[targetLoc].gridLoc).UnitCartesianToSpherical();
		float locationNoise = USimplexNoiseBPLibrary::SimplexNoiseInRange2D(targetSphericalLoc.X, targetSphericalLoc.Y, 0.0, 1.0);
		totalNoise += locationNoise;
		locationArray.Add(locationNoise);
	}
	//normalize the noise so that it adds up to the folding ratio
	//and add that much mass to that cell
	for (int32 targetIndex = 0; targetIndex < potentialLocations.Num(); ++targetIndex)
	{
		FCrustCellData& targetCell = crustCells[potentialLocations[targetIndex]];
		transferCrustFromTargetCellToExistingCell(targetCell, collisionLocation, foldingRatio*locationArray[targetIndex] / totalNoise);
	}
	return potentialLocations.Num() != 0;
}

void UTectonicPlateSimulator::buildNewCrustFromPlateDivergence(const int32& locationIndex, TArray<FCrustCellData>& newCrustDataArray)
{
	FCrustCellData newCrust = createBaseCrustCell(locationIndex, baseContinentalHeight * 0.1);
	newCrust.owningPlate = crustCells[locationIndex].owningPlate;
	newCrustDataArray[locationIndex] = newCrust;
}

void UTectonicPlateSimulator::createHeightMapMesh()
{
	TArray<float> heightMapRadii;
	heightMapRadii.Init(myMesher->baseMeshRadius, myGrid->numNodes);
	for (const FCrustCellData& cellData : crustCells)
	{
		heightMapRadii[cellData.gridLoc.tileIndex] += cellData.cellHeight;
	}
	TArray<FColor> indexColors;
	heightMapMeshIndex = myMesher->buildNewMesh(heightMapRadii, indexColors, heightMapMaterial, heightMapMeshIndex);
}
