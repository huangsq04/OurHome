// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "RegularPentagonTriangleArea.h"
#include "TerrainLoadThread.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "FQuatDouble.h"
#include "PlanetActor.generated.h"

class APlanetActor;

USTRUCT(BlueprintType)
struct OURHOME_API FHexCellInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiffAngleCenter;

};
USTRUCT(BlueprintType)
struct OURHOME_API FHexGridInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int VerticeIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntVector> HexLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> SidesVerticeIndex;

	TMap<uint32, int32> Sides;

	FVectorDouble2D SphericalCoordinate;

	FVectorDouble3D Point;

	uint32 CenterHash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TerrainHeight;

	void AddSide(uint32 K, int32 V)
	{
		if (!Sides.Contains(K))
		{
			Sides.Add(K, V);
			SidesVerticeIndex.Add(V);
		}
	}

	TArray<TArray<FTransform>> FoliageDatas;
	TArray<TArray<int>> FoliageInstanceIndex;
};

USTRUCT(BlueprintType)
struct OURHOME_API FPlanetFoliage
{
	GENERATED_BODY()

	UHierarchicalInstancedStaticMeshComponent *Foliage() { return FoliageComponent; };

	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent *FoliageComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaceMinHeight = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaceMaxHeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = { 1,1,1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	bool bReceivesDecals = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	bool bCastDynamicShadow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float Density = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	uint8 DensityNoiseIndex = 6;
};


UCLASS()
class OURHOME_API APlanetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	float GetTerrainHeight(const FVector &Point) const;

	UFUNCTION(BlueprintCallable)
	FHexGridInfo GetTerrainPosition(const FVector &Point);

	UFUNCTION(BlueprintCallable)
	void UpdateTerrain();

	UFUNCTION(BlueprintCallable)
	void CreateTerrain();

	UFUNCTION(BlueprintCallable)
	void AsyncUpdateTerrain();

	UFUNCTION(BlueprintCallable)
	void AsyncCreateTerrain();

	UFUNCTION(BlueprintCallable)
	FIntVector GetHexXY(const FVector& Point, int Level);


	UFUNCTION(BlueprintCallable)
	bool CmpPointInTriangle(const FVector& A, const FVector& B, const FVector& C, const FVector& P)
	{
		return PointInTriangle(A, B, C, P);
	}

	UFUNCTION(BlueprintCallable)
	void SetActorGroundPointAndRotator(const AActor *Actor, const FVector &Point, bool FacePlayer);

	UFUNCTION(BlueprintCallable)
	TArray<FHexGridInfo> TempShowRangePos(const FVector &Pos, float V); //Temp
	UFUNCTION(BlueprintCallable)
	TArray<FHexGridInfo> TempShowRangePosA(const FIntVector &Pos, float V); //Temp
	UFUNCTION(BlueprintCallable)
	TArray<FHexGridInfo> TempGetHexGridInfo(const int Begin, const int End); //Temp

	UFUNCTION(BlueprintCallable)
	TArray<FVector>  GetHexRandomPoint(const FHexGridInfo& Hex, int Density, int DensityIndex = 3);

	UFUNCTION(BlueprintImplementableEvent, Category = "Planet")
	void OnCreateHexObject(const FVector &Location, const FRotator &Rotator);

	UFUNCTION(BlueprintCallable)
	void CreateProceduralMesh();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	class UProceduralMeshComponent* PlanetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	class UMaterialInterface* MeshMaterialLand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	class UMaterialInterface* MeshMaterialOcean;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	TArray<FPlanetFoliage>  PlanetFoliages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float SphereRadius = 600000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int NoiseForNum = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	FVector NoiseScale = {10.0, 0.5, 500.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	TArray<FVector> NoiseArgs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float RatioSize = 16.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int PatchResolution = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int MaxDepth = 12; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	bool DrawOcean = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	bool DrawRivers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float ScaleRivers = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	TArray<FVector2D> Rivers;

	UPROPERTY(BlueprintReadOnly, Category = "Planet")
	TArray<FVector> DrawTerrainVertices;

	UPROPERTY(transient)
	TArray<UHierarchicalInstancedStaticMeshComponent*> FoliageComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float TrianglesScaleBase = 1000.0;

public:

	float  GetPlanetRadius() const { return SphereRadius; };

	float GetNoiseTerrainHeight(const FVector2D &Point) const;

	float GetNoiseTerrainHeight(const FVector &Point);

	FVectorDouble3D Vector3dFromHexXY(const FIntVector &Point, int Level);

	FVectorDouble3D GetPlayerHexCenterNormal();

	float GetCreateElevation() { return CreateElevation; }

	void UpdateFoliage();

private:

	void UpdateVerticesPoint(int Index, float PointScale,
		FQuatDouble &QuatD, FVectorDouble3D &CNormal);

	void UpdateVerticesTangent(int Index);

	int AddTerrainPoint(const FVectorDouble3D &Point, const FVector2D &UV);

	void CreateVerticesRecursive(
		const TArray<FVectorDouble3D> &Points,
		const TArray<FIntPoint> &HexPoints,
		const FVectorDouble3D &Center,
		int AreaIndex,
		double Size = 1.0, 
		int Depth = 0);

	void UpdateProceduralMesh();

private:

	TMap<int, float> LevelAngle;

	TArray<FVectorDouble3D> TerrainVertices;

	TArray<FVectorDouble2D> SphericalCoordinateVertices;

	TArray<int32> TerrainTriangles;

	TMap<uint32, uint32>  VertexHash;

	TArray<float>  VerticesHeight;

	//X:单个6边形坐标
	//Y:到创建中心点夹角
	TArray<FVector2D> HexTextureCoordinates;

	//X:倾斜度
	//Y:高度
	TArray<FVector2D> TextureCoordinates;

	TArray<FVector> DrawWaterVertices;

	double MaxAngle;

	double MaxDist;

	static TArray<FRegularPentagonTriangleArea> RegularPentagonTriangleAreas;

	TMap<uint32, FHexGridInfo> HexGrids;

	float RatioSizeScale = 1.0;

	TerrainLoadThread *TerrainLoad;

	bool ReqUpdateProceduralMesh = false;

	bool ReqCreateProceduralMesh = false;

	FVectorDouble3D CenterNormal; //中心点法向

	float CreateElevation = 0.0;
};
