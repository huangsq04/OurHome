// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetMeshProvider.h"

UPlanetMeshProvider::UPlanetMeshProvider()
	:DisplayMaterial(nullptr)
{
}


UMaterialInterface* UPlanetMeshProvider::GetSphereMaterial() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return DisplayMaterial;
}

void UPlanetMeshProvider::SetSphereMaterial(UMaterialInterface* InSphereMaterial)
{
	FScopeLock Lock(&PropertySyncRoot);
	DisplayMaterial = InSphereMaterial;
	this->SetupMaterialSlot(0, FName("Sphere Base"), DisplayMaterial);
}

void UPlanetMeshProvider::Initialize()
{
	SetupMaterialSlot(0, FName("Sphere Base"), DisplayMaterial);

	TArray<FRuntimeMeshLODProperties> LODs;
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;
	LODs.Add(LODProperties);
	ConfigureLODs(LODs);

	FRuntimeMeshSectionProperties Properties;
	Properties.bCastsShadow = true;
	Properties.bIsVisible = true;
	Properties.MaterialSlot = 0;
	Properties.bWants32BitIndices = true;
	Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Frequent;

	CreateSection(0, 0, Properties);

	Properties.MaterialSlot = 1;
	CreateSection(0, 1, Properties);
}

bool UPlanetMeshProvider::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{
	check(LODIndex == 0 );
	if (SectionMeshForLODDelegate.IsBound())
	{
		SectionMeshForLODDelegate.Execute(LODIndex, SectionId, MeshData);
	}

	return true;
}

FRuntimeMeshCollisionSettings UPlanetMeshProvider::GetCollisionSettings()
{
	FRuntimeMeshCollisionSettings Settings;
	Settings.bUseAsyncCooking = false;
	Settings.bUseComplexAsSimple = false;

	Settings.Spheres.Emplace(GetSphereRadius());

	return Settings;
}

FBoxSphereBounds UPlanetMeshProvider::GetBounds()
{
	return FBoxSphereBounds(FSphere(FVector::ZeroVector, SphereRadius));
}

bool UPlanetMeshProvider::IsThreadSafe()
{
	return true;
}

UMaterialInterface* UPlanetMeshProvider::GetDisplayMaterial() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return DisplayMaterial;
}

void UPlanetMeshProvider::SetDisplayMaterial(UMaterialInterface* InMaterial)
{
	FScopeLock Lock(&PropertySyncRoot);
	DisplayMaterial = InMaterial;
}

float UPlanetMeshProvider::GetTime() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return Time;
}

void UPlanetMeshProvider::SetTime(float InTime)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		Time = InTime;
	}
	MarkLODDirty(0);
}

