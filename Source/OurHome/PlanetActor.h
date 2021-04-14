// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriangleArea.h"
#include "TerrainLoadThread.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "FQuatDouble.h"
#include "Components/RuntimeMeshComponentStatic.h"
#include "PlanetMeshProvider.h"
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

	void OnConstruction(const FTransform& Transform) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	double GetTerrainHeight(const FVectorDouble3D &Point) const;

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
	void CheckUpdateTerrain();

	UFUNCTION(BlueprintCallable)
	FIntVector GetHexXY(const FVector& Point, int Level);


	UFUNCTION(BlueprintCallable)
	bool CmpPointInTriangle(const FVector& A, const FVector& B, const FVector& C, const FVector& P)
	{
		return UPlanetFunctionLib::PointInTriangle(A, B, C, P);
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

	UFUNCTION(BlueprintCallable, Category = "Planet")
	FVector GetHitPoint(const FVector &Src, const FVector &Dir);

	bool TerrainUpdateCheck();

public:

	UPROPERTY(Category = "Planet", VisibleAnywhere, BlueprintReadOnly, Meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|RuntimeMesh", AllowPrivateAccess = "true"))
	class URuntimeMeshComponent* RuntimeMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	UPlanetMeshProvider *PlanetProvider;

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
	int HexDepth = 12; 

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
	bool bTerrainCollisionEnable = false;

	//最小边长是高度的比例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float EdgeHeightRatio = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int DefaultMaxDepth = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float CullingAngleScale = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	bool bGenHeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	TArray<int32> TerrainTriangles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	TArray<FVector> TerrainNormals;

	double CullingAngle; //基础地形最大夹角
	double CullingLength;

	void GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData);
	
	mutable FCriticalSection UpdateTerrainSync;

public:

	float  GetPlanetRadius() const { return SphereRadius; };

	float GetNoiseTerrainHeight(const FVector2D &Point) const;

	float GetNoiseTerrainHeight(const FVector &Point);

	FVectorDouble3D Vector3dFromHexXY(const FIntVector &Point, int Level);

	FVectorDouble3D GetPlayerHexCenterNormal();

	float GetCreateElevation() { return CreateElevation; }

	void UpdateFoliage();

	TArray<FColor> TerrainColors;

	//X:倾斜度
	//Y:高度
	TArray<FVector2D> TextureCoordinates;


	float UpdatePlanetRatio = 0.1;

	bool bUpdateTerrain = false;
private:

	void UpdateVerticesPoint(int Index, FQuatDouble &QuatD, FVectorDouble3D &CNormal);

	void UpdateVerticesTangent(int Index);

	int AddTerrainPoint(const FVectorDouble3D &Point, const FVector2D &UV);

	void CreateVerticesRecursive(
		const TArray<FVectorDouble3D> &Points,
		const TArray<FIntPoint> &HexPoints,
		const FVectorDouble3D &Center,
		int AreaIndex,
		double Size = 1.0, 
		int Depth = 0);

private:

	TMap<int, float> LevelAngle;

	TArray<FVectorDouble3D> TerrainVertices;

	TArray<FVectorDouble2D> SphericalCoordinateVertices;


	TMap<uint64, uint32>  VertexHash;

	TArray<float>  VerticesHeight;

	//X:单个6边形坐标
	//Y:到创建中心点夹角
	TArray<FVector2D> HexTextureCoordinates;


	TArray<FVector> DrawWaterVertices;

	static TArray<FTriangleArea> RegularPentagonTriangleAreas;

	TMap<uint64, FHexGridInfo> HexGrids;

	TerrainLoadThread *TerrainLoad;

	FVectorDouble3D CenterNormal; //中心点法向

	float CreateElevation = 0.0;

	bool bIsNeedTerrainUpdate = false;

	//创建地形实际最大深度
	int CurrentDepthMax = 0;

	//每层的最大角度
	TArray<double> DepthMaxAngle;
};
