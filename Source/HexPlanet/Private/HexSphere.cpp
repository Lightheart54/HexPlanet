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
	radius = 1;
	numSubvisions = 0;
	gridGenerator = nullptr;

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

void AHexSphere::OnConstruction(const FTransform& Transform)
{
	gridGenerator = new GridGenerator(1.0, 0);
	RootComponent->SetWorldTransform(Transform);
	gridRoot = NewObject<USceneComponent>(this);
	gridRoot->AttachTo(RootComponent);
	rebuildMesh();
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
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, radius))
		|| (PropertyName == GET_MEMBER_NAME_CHECKED(AHexSphere, numSubvisions)))
	{
		rebuildMesh();
	}

	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif  

void AHexSphere::rebuildMesh()
{
	gridRoot->DestroyComponent();

	//clean up the old grid
	gridGenerator->rebuildGrid(radius, numSubvisions);
	surfaceArea = gridGenerator->getSurfaceArea();
	volume = gridGenerator->getVolume();
	GridTilePtrList tiles = gridGenerator->getTiles();
	numTiles = tiles.Num();
	gridRoot = NewObject<USceneComponent>(this);
	gridRoot->AttachTo(RootComponent);

	//for (const GridTilePtr& tile : tiles)
	//{
	//	FVector cen = tile->getPosition();
	//	GridNodePtrList tileNodes = tile->getNodes();
	//	FVector node1Pos = tileNodes[0]->getPosition();
	//	UProceduralMeshComponent* newMesh = createMeshForTile(cen, node1Pos, tileNodes.Num());
	//	newMesh->AttachTo(gridRoot);
	//}
}

UProceduralMeshComponent* AHexSphere::createMeshForTile(const FVector& tileCenter, const FVector& tileCorner1, const uint8& numCorners)
{
	FVector localOrigin = RootComponent->GetComponentLocation();
	FVector zVec = tileCenter;
	FVector yVec = tileCorner1 - tileCenter;
	FVector outDir;
	float radius;
	yVec.ToDirectionAndLength(outDir, radius);
	FVector xVec = FVector::CrossProduct(yVec, zVec);
	FVector::CreateOrthonormalBasis(xVec, yVec, zVec);
	FTransform instanceTransform(xVec, yVec, zVec, localOrigin);
	instanceTransform.SetScale3D(FVector(radius, radius, 0.0));

	numCorners;
	UProceduralMeshComponent* mesh = NewObject<UProceduralMeshComponent>(this);
	FVector cornerLoc(0.0, 1, 0.0);
	FVector xTangent(1, 0, 0);
	FVector normal(0.0, 0.0, -1);
	FColor color = FColor::Blue;
	float angle = 360.0 / numCorners;
	TArray<FVector> vertices;
	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FColor> vertexColors;
	TArray<FProcMeshTangent> tangents;
	vertices.Add(FVector(0.0, 0.0, 0.0));
	normals.Add(normal);
	vertexColors.Add(color);
	UV0.Add(FVector2D(0.0, 0.0));
	tangents.Add(FProcMeshTangent(xTangent,false));
	TArray<int32> Triangles;
	for (size_t it = 0; it < numCorners; ++it)
	{
		FVector loc = FRotator(0.0, 0.0, it*angle).RotateVector(cornerLoc);
		vertices.Add(loc);
		normals.Add(normal);
		vertexColors.Add(color);
		UV0.Add(FVector2D(loc.X, loc.Y));
		tangents.Add(FProcMeshTangent(FRotator(0.0, 0.0, it*angle).RotateVector(xTangent),false));
		Triangles.Add(0);
		Triangles.Add(it + 1);
		Triangles.Add(it == numCorners - 1 ? 1 : it + 2);
	}
	mesh->CreateMeshSection(1, vertices, Triangles, normals, UV0, vertexColors, tangents, false);
	mesh->SetWorldTransform(instanceTransform);
	return mesh;
}

