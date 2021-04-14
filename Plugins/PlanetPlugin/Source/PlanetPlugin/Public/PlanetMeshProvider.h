// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshProvider.h"
#include "PlanetMeshProvider.generated.h"

DECLARE_DELEGATE_ThreeParams(FGetSectionMeshForLODDelegate, int32, int32, FRuntimeMeshRenderableMeshData&);
/**
 * 
 */
UCLASS()
class PLANETPLUGIN_API UPlanetMeshProvider : public URuntimeMeshProvider
{
	GENERATED_BODY()

private:
	mutable FCriticalSection PropertySyncRoot;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetDisplayMaterial, BlueprintSetter = SetDisplayMaterial)
	UMaterialInterface* DisplayMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetTime, BlueprintSetter = SetTime)
	float Time;

public:

	UPlanetMeshProvider();

	float SphereRadius = 600000.0;
	float GetSphereRadius() { return SphereRadius; };
	void SetSphereRadius(float V) { SphereRadius = V; }


	UFUNCTION(Category = "RuntimeMesh|Providers|Sphere", BlueprintCallable)
	UMaterialInterface* GetSphereMaterial() const;
	UFUNCTION(Category = "RuntimeMesh|Providers|Sphere", BlueprintCallable)
	void SetSphereMaterial(UMaterialInterface* InSphereMaterial);

	UFUNCTION(BlueprintCallable)
	UMaterialInterface* GetDisplayMaterial() const;

	UFUNCTION(BlueprintCallable)
	void SetDisplayMaterial(UMaterialInterface* InMaterial);

	UFUNCTION(BlueprintCallable)
	float GetTime() const;

	UFUNCTION(BlueprintCallable)
	void SetTime(float InTime);

	//UPROPERTY()
	//APlanetActor *Planet;

	FGetSectionMeshForLODDelegate SectionMeshForLODDelegate;

protected:

	virtual void Initialize() override;
	virtual bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData) override;
	virtual FRuntimeMeshCollisionSettings GetCollisionSettings() override;
	FBoxSphereBounds GetBounds() override;
	bool IsThreadSafe() override;
	
};
