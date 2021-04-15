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

	//���µ���
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void UpdateTerrain();

	//��������
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void CreateTerrain();

	//�첽���µ���
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void AsyncUpdateTerrain();

	//�첽���ص���
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void AsyncCreateTerrain();

	//��鲢���첽���µ���
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void CheckUpdateTerrain();

	//��ȡ���6��������
	UFUNCTION(BlueprintCallable, Category = "Planet")
	FIntVector GetHexXY(const FVector& Point, int Level);

	//���õ���������ĵ�
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void SetActorGroundPointAndRotator(const AActor *Actor, const FVector &Point, bool FacePlayer);

	//�������ߵ���ĵ����
	UFUNCTION(BlueprintCallable, Category = "Planet")
	FVector GetHitPoint(const FVector &Src, const FVector &Dir);

	//�������ߵ����6�������ĵ��ܱߵ�6��6����
	UFUNCTION(BlueprintCallable, Category = "Planet")
	TArray<FVector> GetHitAroundHex(const FVector &Src, const FVector &Dir);

	//�������ߵ����6����6������
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

	//��С�߳��Ǹ߶ȵı���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float EdgeHeightRatio = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	int DefaultMaxDepth = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	float CullingAngleScale = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet")
	bool bGenHeight = true;

private:

	//��ȡ��뾶
	float  GetPlanetRadius() const { return SphereRadius; };

	//��ȡ��ĵ��θ߶�
	float GetNoiseTerrainHeight(const FVector2D &Point) const;

	//��ȡ��ĵ��θ߶�
	float GetNoiseTerrainHeight(const FVector &Point);

	FVectorDouble3D Vector3dFromHexXY(const FIntVector &Point, int Level);

	//��ȡ������ĵ�
	FVectorDouble3D GetPlayerHexCenterNormal();

	//��ȡ�������εĸ߶�
	float GetCreateElevation() { return CreateElevation; }

	//�������Ƿ���Ҫ����
	bool TerrainUpdateCheck();

	//��ȡ���θ߶�
	double GetTerrainHeight(const FVectorDouble3D &Point) const;

	void UpdateVerticesPoint(int Index, FQuatDouble &QuatD, FVectorDouble3D &CNormal);

	void UpdateVerticesTangent(int Index);

	int AddTerrainPoint(const FVectorDouble3D &Point, const FVector2D &UV);

	void CreateVerticesRecursive(const TArray<FVectorDouble3D> &Points,
		const TArray<FIntPoint> &HexPoints,	const FVectorDouble3D &Center,
		int AreaIndex,	double Size = 1.0, 	int Depth = 0);

private:

	//R��ʾ6������������
	TArray<FColor> TerrainColors;

	//X:��б��
	//Y:�߶�
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

	FVectorDouble3D CenterNormal; //���ĵ㷨��

	float CreateElevation = 0.0;

	bool bIsNeedTerrainUpdate = false;

	//��������ʵ��������
	int CurrentDepthMax = 0;

	//ÿ������Ƕ�
	TArray<double> DepthMaxAngle;

	double CullingAngle; //�����������н�
	double CullingLength;

	mutable FCriticalSection UpdateTerrainSync;
};
