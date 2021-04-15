// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriangleArea.h"
#include "FQuatDouble.h"
#include "PlanetMeshProvider.h"
#include "RuntimeMeshComponent.h"
#include "TerrainUpdateThread.h"
#include "Planet.generated.h"

UCLASS()
class PLANETPLUGIN_API APlanet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//更新地形
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void UpdateTerrain();

	//创建地形
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void CreateTerrain();

	//异步更新地形
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void AsyncUpdateTerrain();

	//异步加载地形
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void AsyncCreateTerrain();

	//检查并且异步更新地形
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void CheckUpdateTerrain();

	//获取点的6边形坐标
	UFUNCTION(BlueprintCallable, Category = "Planet")
	FIntVector GetHexXY(const FVector& Point, int Level);

	//设置到地面的中心点
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void SetActorGroundPointAndRotator(const AActor *Actor, const FVector &Point, bool FacePlayer);

	//计算射线点击的地面点
	UFUNCTION(BlueprintCallable, Category = "Planet")
	FVector GetHitPoint(const FVector &Src, const FVector &Dir);

	//计算射线点击的6边形中心点周边的6个6边形
	UFUNCTION(BlueprintCallable, Category = "Planet")
	TArray<FVector> GetHitAroundHex(const FVector &Src, const FVector &Dir);

	//计算射线点击的6边形6个顶点
	UFUNCTION(BlueprintCallable, Category = "Planet")
	TArray<FVector> GetHitHexPoint(const FVector &Src, const FVector &Dir);

	void GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData);

public:

	UPROPERTY(Category = "Planet", VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
	class URuntimeMeshComponent* RuntimeMeshComponent;

	UPROPERTY()
	UPlanetMeshProvider *PlanetProvider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	class UMaterialInterface* MeshMaterialLand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	class UMaterialInterface* MeshMaterialOcean;

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
	bool bDrawOcean = true;

	//最小边长是高度的比例
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float EdgeHeightRatio = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int DefaultMaxDepth = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float CullingAngleScale = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	bool bGenHeight = true;

private:

	//获取球半径
	float  GetPlanetRadius() const { return SphereRadius; };

	//获取点的地形高度
	float GetNoiseTerrainHeight(const FVector2D &Point) const;

	//获取点的地形高度
	float GetNoiseTerrainHeight(const FVector &Point);

	FVectorDouble3D Vector3dFromHexXY(const FIntVector &Point, int Level);

	//获取玩家中心点
	FVectorDouble3D GetPlayerHexCenterNormal();

	//获取创建地形的高度
	float GetCreateElevation() { return CreateElevation; }

	//检查地形是否需要更新
	bool TerrainUpdateCheck();

	//获取地形高度
	double GetTerrainHeight(const FVectorDouble3D &Point) const;

	void UpdateVerticesPoint(int Index, FQuatDouble &QuatD, FVectorDouble3D &CNormal);

	void UpdateVerticesTangent(int Index);

	int AddTerrainPoint(const FVectorDouble3D &Point, const FVector2D &UV);

	void CreateVerticesRecursive(const TArray<FVectorDouble3D> &Points,
		const TArray<FIntPoint> &HexPoints,	const FVectorDouble3D &Center,
		int AreaIndex,	double Size = 1.0, 	int Depth = 0);

private:

	//R表示6边形纹理坐标
	TArray<FColor> TerrainColors;

	//X:倾斜度
	//Y:高度
	TArray<FVector2D> TextureCoordinates;

	float UpdatePlanetRatio = 0.1;

	bool bUpdateTerrain = false;

	TArray<FVector> DrawTerrainVertices;

	TArray<FVectorDouble3D> TerrainVertices;

	TArray<int32> TerrainTriangles;

	TArray<FVector> TerrainNormals;

	TMap<uint64, uint32>  VertexHash;

	static TArray<FTriangleArea> RegularPentagonTriangleAreas;

	FTerrainUpdateThread *TerrainLoad;

	FVectorDouble3D CenterNormal; //中心点法向

	float CreateElevation = 0.0;

	bool bIsNeedTerrainUpdate = false;

	//创建地形实际最大深度
	int CurrentDepthMax = 0;

	//每层的最大角度
	TArray<double> DepthMaxAngle;

	double CullingAngle; //基础地形最大夹角
	double CullingLength;

	mutable FCriticalSection UpdateTerrainSync;
};
