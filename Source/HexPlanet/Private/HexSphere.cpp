// Fill out your copyright notice in the Description page of Project Settings.

#include "HexPlanet.h"
#include "HexSphere.h"
#include "ProceduralMeshComponent.h"
#include "GridGenerator.h"

// Sets default values
AHexSphere::AHexSphere() 
{
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;	
	gridRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GridRoot"));
	gridRoot->AttachTo(RootComponent);
	pentagonMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PentagonStaticMesh"));
	pentagonMeshComponent->AttachTo(gridRoot);
	hexagonMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexagonStaticMesh"));
	hexagonMeshComponent->AttachTo(gridRoot);
	radius = 50;
	numSubvisions = 0;
	gridGenerator = new GridGenerator(radius, numSubvisions);
	surfaceArea = gridGenerator->getSurfaceArea();
	volume = gridGenerator->getVolume();
	GridTilePtrList tiles = gridGenerator->getTiles();
	numTiles = tiles.Num();
	PentagonMeshInnerRadius = 20;
	HexagonMeshInnerRadius = 20;
	tileFillRatio = 0.95;
#ifdef WITH_EDITOR
	debugMesh = CreateDefaultSubobject<ULineBatchComponent>(TEXT("DebugMeshRoot"));
	debugMesh->AttachTo(RootComponent);
	renderNodesAndEdges = false;
	displayEdgeLengths = false;
#endif // WITH_EDITOR

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AHexSphere::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHexSphere::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AHexSphere::Destroyed()
{
	delete gridGenerator;
	gridGenerator = nullptr;
	AActor::Destroyed();
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
		calculateMesh();
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, radius))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, HexagonMeshInnerRadius))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, PentagonMeshInnerRadius))
		||(PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, PentagonMesh))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, HexagonMesh))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, tileFillRatio)))
	{
		rebuildInstances();
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, renderNodesAndEdges)))
	{
		rebuildDebugMesh();
	}
	else if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, displayEdgeLengths)))
	{
		updateDebugText();
	}
	

	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif  

void AHexSphere::calculateMesh()
{
	//clean up the old grid
	pentagonMeshComponent->ClearInstances();
	hexagonMeshComponent->ClearInstances();
	gridGenerator->rebuildGrid(radius, numSubvisions);
	surfaceArea = gridGenerator->getSurfaceArea();
	volume = gridGenerator->getVolume();
	GridTilePtrList tiles = gridGenerator->getTiles();
	numTiles = tiles.Num();
	rebuildInstances();
}

void AHexSphere::rebuildInstances()
{
	pentagonMeshComponent->ClearInstances();
	hexagonMeshComponent->ClearInstances();
	pentagonMeshComponent->SetStaticMesh(PentagonMesh);
	hexagonMeshComponent->SetStaticMesh(HexagonMesh);
	GridTilePtrList tiles = gridGenerator->getTiles();
	for (const GridTilePtr& tile : tiles)
	{		
		GridEdgePtrList tileEdges = tile->getEdges();
		if (tileEdges.Num() == 5 && PentagonMesh == nullptr)
		{
			continue;
		}
		else if (HexagonMesh == nullptr && tileEdges.Num()==6)
		{
			continue;
		}
		FVector tileCenter = tile->getPosition();
		FVector zDir;
		float localSphereRadius;
		tileCenter.ToDirectionAndLength(zDir, localSphereRadius);
		float vectorRatio = radius / localSphereRadius;
		tileCenter *= vectorRatio;
		FVector tileInnerRadiusVec = tileEdges[0]->getPosition() * vectorRatio;
		tileInnerRadiusVec -= tileInnerRadiusVec.ProjectOnTo(zDir);
		float tileInnerRadius = FMath::Sqrt(FVector::DotProduct(tileInnerRadiusVec, tileInnerRadiusVec));
		FVector yVec;
		if (tileEdges.Num() == 5)
		{
			yVec = tileInnerRadiusVec;
		}
		else
		{
			GridTilePtrList gridNeighbors = tile->getNeighbors();
			GridTilePtr refNeighbor = gridNeighbors[0];
			FVector refenceVec = refNeighbor->getPosition();
			gridNeighbors.Sort([&refenceVec](const GridTile& neighbor1, const GridTile& neighbor2)->bool
			{
				return FVector::DistSquared(refenceVec, neighbor1.getPosition()) > FVector::DistSquared(refenceVec, neighbor2.getPosition());
			});
			GridTilePtr oppositeNeighbor = gridNeighbors[0];
			yVec = refenceVec - oppositeNeighbor->getPosition();
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

		if (tileEdges.Num() == 5)
		{
			FVector scaleVector(tileInnerRadius* tileFillRatio / PentagonMeshInnerRadius, tileInnerRadius* tileFillRatio / PentagonMeshInnerRadius, tileInnerRadius / PentagonMeshInnerRadius);
			instanceTransform.SetScale3D(scaleVector);
			pentagonMeshComponent->AddInstance(instanceTransform);
		}
		else
		{
			FVector scaleVector(tileInnerRadius* tileFillRatio / HexagonMeshInnerRadius, tileInnerRadius* tileFillRatio / HexagonMeshInnerRadius, tileInnerRadius / HexagonMeshInnerRadius);
			instanceTransform.SetScale3D(scaleVector);
			hexagonMeshComponent->AddInstance(instanceTransform);
		}
	}

#ifdef WITH_EDITOR
	rebuildDebugMesh();
	updateDebugText();
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void AHexSphere::buildDebugMesh()
{
	GridNodePtrList gridNodes = gridGenerator->getNodes();
	FVector centerPoint = GetActorLocation();
	for (const GridNodePtr& gridNode : gridNodes)
	{
		debugMesh->DrawPoint(centerPoint+(gridNode->getPosition())*radius/gridGenerator->getRadius(), FLinearColor::Blue, 8, 2);
	}

	GridEdgePtrList gridEdges = gridGenerator->getEdges();
	for (const GridEdgePtr& gridEdge : gridEdges)
	{
		debugMesh->DrawLine(centerPoint+(gridEdge->getStartPoint()->getPosition())*radius / gridGenerator->getRadius(),
			centerPoint + (gridEdge->getEndPoint()->getPosition())*radius / gridGenerator->getRadius(), FLinearColor::Green, 2, 0.5);
		debugMesh->DrawPoint(centerPoint + (gridEdge->getPosition())*radius / gridGenerator->getRadius(), FLinearColor::Red, 8, 2);
	}

	GridTilePtrList gridTiles = gridGenerator->getTiles();
	for (const GridTilePtr& gridTile : gridTiles)
	{
		debugMesh->DrawPoint(centerPoint + (gridTile->getPosition())*radius / gridGenerator->getRadius(), FLinearColor::Yellow, 8, 2);
	}
}

void AHexSphere::rebuildDebugMesh()
{
	debugMesh->Flush();

	if (renderNodesAndEdges)
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
			FText nodeText = FText::FromString(FString::SanitizeFloat((gridEdge->getLength())*radius / gridGenerator->getRadius()));
			UTextRenderComponent* nodePos = NewObject<UTextRenderComponent>(this);
			debugTextArray.Add(nodePos);
			nodePos->SetText(nodeText);
			nodePos->SetRelativeLocation((gridEdge->getPosition())*radius / gridGenerator->getRadius());
			nodePos->SetWorldSize(1);
			nodePos->SetTextRenderColor(FColor::Black);
			nodePos->AttachTo(RootComponent);
		}
	}
	
}

#endif



