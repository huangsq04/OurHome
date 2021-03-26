// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetActor.h"
#include "Engine/Public/GeomTools.h"
#include "Math/UnrealMathUtility.h"
#include "SimplexNoiseBPLibrary.h"
#include "PlanetDatasLibrary.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Animation/SkeletalMeshActor.h"
#pragma optimize("", off)


TArray<FRegularPentagonTriangleArea> APlanetActor::RegularPentagonTriangleAreas;
// Sets default values
APlanetActor::APlanetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlanetMesh = CreateDefaultSubobject<UProceduralMeshComponent>("PlanetMesh");
	PlanetMesh->Mobility = EComponentMobility::Static;
	FRegularPentagonTriangleArea::CreateBaseDodecahedron(RegularPentagonTriangleAreas);
	TerrainLoad = nullptr;
	LevelAngle = { {9, 0.00148417137}, {10, 0.000742556120}, {11, 0.000371027971}, {12, 0.000185511512}, {13, 9.27560541e-05} };
}

// Called when the game starts or when spawned
void APlanetActor::BeginPlay()
{
	Super::BeginPlay();

	for (FPlanetFoliage &Item : PlanetFoliages)
	{
		UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent;
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_GrassCreateComp);
			HierarchicalInstancedStaticMeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, NAME_None, RF_Transient);
		}
		Item.FoliageComponent = HierarchicalInstancedStaticMeshComponent;
		HierarchicalInstancedStaticMeshComponent->SetStaticMesh(Item.StaticMesh);
		HierarchicalInstancedStaticMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		FoliageComponents.Add(HierarchicalInstancedStaticMeshComponent);
		HierarchicalInstancedStaticMeshComponent->RegisterComponent();
		HierarchicalInstancedStaticMeshComponent->Mobility = EComponentMobility::Static;
		HierarchicalInstancedStaticMeshComponent->bSelectable = false;
		HierarchicalInstancedStaticMeshComponent->bHasPerInstanceHitProxies = false;
		HierarchicalInstancedStaticMeshComponent->bReceivesDecals = Item.bReceivesDecals;
		static FName NoCollision(TEXT("NoCollision"));
		HierarchicalInstancedStaticMeshComponent->SetCollisionProfileName(NoCollision);
		HierarchicalInstancedStaticMeshComponent->bCastStaticShadow = false;
		HierarchicalInstancedStaticMeshComponent->CastShadow = Item.bCastDynamicShadow;
		HierarchicalInstancedStaticMeshComponent->bCastDynamicShadow = Item.bCastDynamicShadow;
	}

	if (TerrainLoad == nullptr)
	{
		TerrainLoad = new TerrainLoadThread();
	}
}
void APlanetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	TerrainLoad->Exit();
}
void APlanetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ReqCreateProceduralMesh)
	{
		CreateProceduralMesh();
		ReqCreateProceduralMesh = false;
	}

	if (ReqUpdateProceduralMesh)
	{
		UpdateProceduralMesh();

		ReqUpdateProceduralMesh = false;
	}
}

float APlanetActor::GetNoiseTerrainHeight(const FVector2D &Point) const
{
	float Height = 0;

	if (NoiseArgs.Num() > 0)
	{
		TArray<float> HS;
		float N = pow(2, NoiseArgs.Num()) - 1;
		for (int i = 0; i < NoiseArgs.Num(); i++)
		{
			FVector2D  Noise2D = Point * NoiseArgs[i].X;
			float H = USimplexNoiseBPLibrary::SimplexNoise2D(Noise2D.X, Noise2D.Y);
			HS.Add(H);
			H = (H / N ) * pow(2, NoiseArgs.Num() - (i + 1));
			Height = Height + H;
		}
		float SigmoidHeight = (Sigmoid(Height * NoiseScale.X) - 0.5) * 2.0;
		Height = SigmoidHeight;
		if (Height > 0) Height = Height * NoiseScale.Y;

		if (DrawRivers)
		{

		}

		float P = 0;
		for (int i = 0; i < NoiseArgs.Num(); i++)
		{
			P = P + (1 + HS[i]) * NoiseArgs[i].Z * 0.5;
		}
		float S = pow(P, NoiseForNum);

		float H = 0;
		for (int i = 0; i < NoiseArgs.Num(); i++)
		{
			H = H + (1 + HS[i]) * NoiseArgs[i].Y * 0.5;
		}
		Height = Height + S * H;

		Height = (Sigmoid(Height) - 0.5) * 2.0;

	}

	if (Point.X < 0.5)
	{
		Height = Height + Point.X * 2.0 - 1.0;
	}

	if (Point.X > PI - 0.5)
	{
		Height = Height + (PI - Point.X) * 2.0 - 1.0;
	}
	return Height;

}
float APlanetActor::GetTerrainHeight(const FVector &Point) const
{
	FVector2D XY = SphericalFromCartesian(Point).ToVector2D();
	return SphereRadius + GetNoiseTerrainHeight(XY) * NoiseScale.Z;
}
float APlanetActor::GetNoiseTerrainHeight(const FVector &Point)
{
	FVector2D XY = SphericalFromCartesian(Point).ToVector2D();
	return GetNoiseTerrainHeight(XY);
}
int APlanetActor::AddTerrainPoint(const FVectorDouble3D &Point, const FVector2D &UV)
{
	uint32 *Index;
	FVectorDouble2D XY = SphericalFromCartesian(Point);
	uint32 Hash = GetTypeHash(XY);
	Index = VertexHash.Find(Hash);
	if (Index != nullptr)
	{
		return *Index;
	}
	else
	{
		int VerticeIndex = SphericalCoordinateVertices.Num();
		VertexHash.Add(Hash, VerticeIndex);

		SphericalCoordinateVertices.Add(XY);
		HexTextureCoordinates.Add(UV);

		TerrainVertices.Add(Point);
		return  VerticeIndex;
	}

}

FQuatDouble GetQuatDouble(const FVectorDouble3D &A, const FVectorDouble3D &B, double AngleRad)
{
	FVectorDouble3D Axis = A ^B;
	Axis.Normalize();
	double Ag = A | B;
	double Angle = acos(Ag);
	FQuatDouble QuatA(Axis.ToVector(), 0);
	FQuatDouble QuatB(Axis.ToVector(), Angle);
	return FQuatDouble::Slerp(QuatA, QuatB, AngleRad);
}
FVectorDouble3D RotatorVector(const FVectorDouble3D &V, const FQuatDouble &Q)
{
	FVectorDouble3D R = Q.RotateVector(V);
	return R;
}

FVectorDouble3D ScaleVector(const FVectorDouble3D &V, const FVectorDouble3D &A, const FVectorDouble3D &B, double AngleRad)
{
	FVectorDouble3D D = B - A;
	FVectorDouble3D R = V + D * AngleRad;
	R.Normalize();
	return R;
}

void APlanetActor::UpdateVerticesPoint(int Index, float PointScale, 
	FQuatDouble &QuatD, FVectorDouble3D &CNormal)
{
	if (PointScale > 1.0)
	{
		float S = 1.0f - HexTextureCoordinates[Index].Y / (HexTextureCoordinates[0].Y * 1.02f);
		S = FMath::Clamp(S, 0.0f, 1.0f);
		PointScale = (1.0f + S * PointScale) * PointScale;
	}
	
	FVectorDouble3D NDP = QuatD.RotateVector(TerrainVertices[Index]);
	if (FMath::IsNaN(QuatD.X))
	{
		NDP = TerrainVertices[Index];
	}
	FVectorDouble3D DP = ScaleVector(CNormal, CNormal, NDP, PointScale);

	FVectorDouble2D XY = SphericalFromCartesian(DP);
	SphericalCoordinateVertices[Index] = XY;

	float Height = GetNoiseTerrainHeight(XY.ToVector2D());
	VerticesHeight[Index] = Height;

	DrawTerrainVertices[Index] = (DP * (SphereRadius + Height * NoiseScale.Z)).ToVector();

	if (DrawOcean)
	{
		DrawWaterVertices[Index] = (DP * SphereRadius).ToVector();
	}
	TextureCoordinates[Index] = { 0, Height };
}

void APlanetActor::UpdateVerticesTangent(int Index)
{
	FVector V1 = DrawTerrainVertices[TerrainTriangles[Index * 3]];
	FVector V2 = DrawTerrainVertices[TerrainTriangles[Index * 3 + 1]];
	FVector V3 = DrawTerrainVertices[TerrainTriangles[Index * 3 + 2]];
	FVector C = (V1 + V2 + V3) / 3.0;

	C.Normalize();
	FVector B = FVector::CrossProduct(V2 - V1, V2 - V3);
	B.Normalize();

	float Angle = acos(FVector::DotProduct(B, C));
	TextureCoordinates[TerrainTriangles[Index * 3]].X = (TextureCoordinates[TerrainTriangles[Index * 3]].X + Angle) / 2.0;
	TextureCoordinates[TerrainTriangles[Index * 3 + 1]].X = (TextureCoordinates[TerrainTriangles[Index * 3 + 1]].X + Angle) / 2.0;
	TextureCoordinates[TerrainTriangles[Index * 3 + 2]].X = (TextureCoordinates[TerrainTriangles[Index * 3 + 2]].X + Angle) / 2.0;
}

void APlanetActor::UpdateTerrain()
{
	if (SphericalCoordinateVertices.Num() == 0)
	{
		return;
	}

	FVectorDouble3D CNormal = GetPlayerHexCenterNormal();
	if (CNormal.X == 0 && CNormal.Y == 0 && CNormal.Z == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	APawn *Pawn = GEngine->GetFirstLocalPlayerController(World)->GetPawn();
	float Elevation = Pawn->GetActorLocation().Size();

	float PointScale = Elevation - (SphereRadius + TrianglesScaleBase);
	if (PointScale < 0)
	{
		PointScale = 1;
	}
	else if (PointScale > (SphereRadius + TrianglesScaleBase))
	{
		PointScale = MaxDepth;
	}
	else
	{
		PointScale = 1 + (MaxDepth / (SphereRadius + TrianglesScaleBase)) * PointScale;
	}

	FQuatDouble QuatD = GetQuatDouble(CenterNormal, CNormal, 1.0);

	ParallelFor(TerrainVertices.Num(),
		[&](int32 Index)
	{
		UpdateVerticesPoint(Index, PointScale, QuatD, CNormal);
	});

	//计算斜率
	ParallelFor(TerrainTriangles.Num() / 3,
		[&](int32 Index)
	{
		UpdateVerticesTangent(Index);
	});

	//更新地面植被
	UpdateFoliage();

	ReqUpdateProceduralMesh = true;
}

void APlanetActor::AsyncUpdateTerrain()
{
	if (!TerrainLoad->TerrainLoadFun.IsBound())
	{
		TerrainLoad->TerrainLoadFun.BindUObject(this, &APlanetActor::UpdateTerrain);
	}
}

void APlanetActor::AsyncCreateTerrain()
{
	if (TerrainLoad == nullptr)
	{
		TerrainLoad = new TerrainLoadThread();
	}

	if (!TerrainLoad->TerrainLoadFun.IsBound())
	{
		//TerrainLoad->TerrainLoadFun.BindUObject(this, &APlanetActor::UpdateTerrain);
		TerrainLoad->TerrainLoadFun.BindUObject(this, &APlanetActor::CreateTerrain);
	}
}
void APlanetActor::UpdateProceduralMesh()
{
	TArray<FVector> Normals;
	TArray<FVector2D> UV2;
	TArray<FVector2D> UV3;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	PlanetMesh->UpdateMeshSection(0, DrawTerrainVertices, Normals, TextureCoordinates,
		HexTextureCoordinates, UV2, UV3, VertexColors, Tangents);

	if (DrawOcean)
	{
		PlanetMesh->UpdateMeshSection(1, DrawWaterVertices, Normals, TextureCoordinates,
			HexTextureCoordinates, UV2, UV3, VertexColors, Tangents);
	}

	for (auto &Item : HexGrids)
	{
		if (Item.Value.Level > MaxDepth - 6)
		{
			for (int Index = 0 ;Index < Item.Value.FoliageDatas.Num(); Index++)
			{
				for (auto &Value :Item.Value.FoliageDatas[Index])
				{
					//int Idx = PlanetFoliages[Index].Foliage()->AddInstance(Value);
					//Item.Value.FoliageInstanceIndex[Index].Add(Idx);
				}
			}
		}
	}
}
void APlanetActor::CreateProceduralMesh()
{
	TArray<FVector> Normals;
	TArray<FVector2D> UV2;
	TArray<FVector2D> UV3;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	PlanetMesh->ClearMeshSection(0);

	if (DrawOcean)
	{
		PlanetMesh->ClearMeshSection(1);
	}

	PlanetMesh->CreateMeshSection(0, DrawTerrainVertices, TerrainTriangles, Normals, TextureCoordinates,
		HexTextureCoordinates, UV2, UV3, VertexColors, Tangents, false);

	if (PlanetMesh->GetMaterial(0) == nullptr)
	{
		PlanetMesh->SetMaterial(0, MeshMaterialLand);
	}

	if (DrawOcean)
	{
		PlanetMesh->CreateMeshSection(1, DrawTerrainVertices, TerrainTriangles, Normals, TextureCoordinates,
			HexTextureCoordinates, UV2, UV3, VertexColors, Tangents, false);

		if (PlanetMesh->GetMaterial(0) == nullptr)
		{
			PlanetMesh->SetMaterial(1, MeshMaterialOcean);
		}
	}
}

void APlanetActor::CreateTerrain()
{
	VertexHash.Empty();
	TerrainVertices.Empty();
	TerrainTriangles.Empty();
	HexTextureCoordinates.Empty();
	SphericalCoordinateVertices.Empty();
	TextureCoordinates.Empty();
	DrawTerrainVertices.Empty();
	HexGrids.Empty();

	UWorld* World = GetWorld();
	APawn *Pawn = GEngine->GetFirstLocalPlayerController(World)->GetPawn();

	CenterNormal = GetPlayerHexCenterNormal();

	if (CenterNormal.X == 0 && CenterNormal.Y == 0 && CenterNormal.Z == 0)
	{
		return;
	}

	CreateElevation = Pawn->GetActorLocation().Size();

	FVectorDouble3D Pos = CenterNormal * CreateElevation;

	FVectorDouble3D Center = (Pos - this->GetActorLocation()) / SphereRadius;

	RatioSizeScale = Center.Size();
	RatioSizeScale = FMath::Clamp(RatioSizeScale, (float)1.0, (float)3.0);
	MaxDist = 1.0 / Center.Size();
	MaxAngle = acos(MaxDist);
	MaxDist = FMath::Tan(MaxAngle) * 4.5;

	for (auto &Item : RegularPentagonTriangleAreas)
	{
		CreateVerticesRecursive(
			{Item.GetPos0(), Item.GetCenterPos(), Item.GetPos1()}, 
			{ {1, 0}, {0, 0}, {0, 1}},	Center, Item.GetIndex());
	}

	TextureCoordinates.SetNum(SphericalCoordinateVertices.Num());
	VerticesHeight.SetNum(SphericalCoordinateVertices.Num());
	DrawWaterVertices.SetNum(TerrainVertices.Num());
	DrawTerrainVertices.SetNum(TerrainVertices.Num());

	ParallelFor(TerrainVertices.Num(),
		[&](int32 Index)
	{
		HexTextureCoordinates[Index].Y = acos(TerrainVertices[Index] | CenterNormal);
	});

	struct FCompareAngle
	{
		FCompareAngle(const APlanetActor& InPlanetActor, int InCenter) : 
			PlanetActor(InPlanetActor),Center(InCenter)
		{}
		FORCEINLINE bool operator()(const int& A, const int& B) const
		{
			float A1 = (PlanetActor.TerrainVertices[A] ^ PlanetActor.TerrainVertices[Center]) | FVectorDouble3D(0, 0, 1);
			float A2 = (PlanetActor.TerrainVertices[B] ^ PlanetActor.TerrainVertices[Center]) | FVectorDouble3D(0, 0, 1);
			return A1 > A2;
		}
		const APlanetActor& PlanetActor;
		int Center;
	};

	for (auto &Item :HexGrids)
	{
		Item.Value.SidesVerticeIndex.Sort(FCompareAngle(*this, Item.Value.VerticeIndex));
	}

	UpdateTerrain();

	ReqCreateProceduralMesh = true;
}

void APlanetActor::CreateVerticesRecursive(
	const TArray<FVectorDouble3D> &Points,
	const TArray<FIntPoint> &HexPoints,
	const FVectorDouble3D &Center,
	int AreaIndex,
	double Size,
	int Depth
)
{
	const FVectorDouble3D &P1 = Points[0];
	const FVectorDouble3D &P2 = Points[1];
	const FVectorDouble3D &P3 = Points[2];

	FVectorDouble3D EdgeCenter[3] = { (P1 + P2) / 2, (P2 + P3) / 2, (P3 + P1) / 2 };

	EdgeCenter[0].Normalize();
	EdgeCenter[1].Normalize();
	EdgeCenter[2].Normalize();

	bool EdgeTest[3];
	double LoRatioSize = Size * RatioSize * RatioSizeScale;

	for (int i = 0; i < 3; i++)
	{
		FVectorDouble3D D = Center - EdgeCenter[i];
		double Dist = D.Size();
		if (Dist > LoRatioSize)
		{
			EdgeTest[i] = true;
		}
		else
		{
			EdgeTest[i] = false;
		}
	}

	double CP1P2 = PointToSegDist(P1, P2, Center);
	double CP2P3 = PointToSegDist(P2, P3, Center);
	double CP3P1 = PointToSegDist(P3, P1, Center);
	double MinDist = FMath::Min(CP1P2, FMath::Min(CP2P3, CP3P1));
	const FVector VP1(P1.X, P1.Y, P1.Z) ;
	const FVector VP2(P2.X, P2.Y, P2.Z) ;
	const FVector VP3(P3.X, P3.Y, P3.Z) ;
	const FVector VCenter(Center.X, Center.Y, Center.Z);
	if (!FGeomTools::PointInTriangle(VP1, VP2, VP3, VCenter) && MaxDist < MinDist)
	{
		return;//culling
	}

	if ((EdgeTest[0] && EdgeTest[1] && EdgeTest[2]) || Depth > MaxDepth)
	{
		const TArray<FVector2D> UVPs = { {1,1}, {0,0}, {1,1} };
		TArray<FVectorDouble3D> PitchVertices;
		TArray<FVector2D> PitchVerticesUV;
		TArray<int32>   PitchTriangles;

		FVectorDouble3D d2 = P1 - P2;
		FVectorDouble3D d3 = P3 - P2;

		FVector2D t2 = UVPs[0] - UVPs[1];
		FVector2D t3 = UVPs[2] - UVPs[1];

		for (int i = 0; i < PatchResolution + 1; i++)
		{
			for (int j = 0; j < i + 1; j++)
			{
				FVectorDouble3D P(double(double(1.0 - double(i) / double(PatchResolution))),
					double(double(double(j) / double(PatchResolution))), 0);

				PitchVerticesUV.Add(UVPs[1] + (t2 * P.X + t3 * P.Y));
				P = P2 + (d2 * P.X + d3 * P.Y);

				//P.Normalize();

				PitchVertices.Add(P);

				if (i > 0 && j > 0)
				{
					PitchTriangles.Add(uint8(PitchVertices.Num() - 1));
					PitchTriangles.Add(uint8(PitchVertices.Num() - 2));
					PitchTriangles.Add(uint8(PitchVertices.Num() - 2 - i));

					if (j < i)
					{
						PitchTriangles.Add(uint8(PitchVertices.Num() - 2 - i));
						PitchTriangles.Add(uint8(PitchVertices.Num() - 1 - i));
						PitchTriangles.Add(uint8(PitchVertices.Num() - 1));
					}
				}
			}
		}
		uint32 hash = 0;
		hash = GetTypeHash(P2);
		int TriIdx = AddTerrainPoint(P2, UVPs[1]);
		if (!HexGrids.Contains(hash))
		{
			FHexGridInfo GridInfo;
			GridInfo.VerticeIndex = TriIdx;
			GridInfo.Level = Depth;
			GridInfo.Point = P1;
			GridInfo.CenterHash = hash;
			FIntVector Location = { HexPoints[1].X, HexPoints[1].Y, AreaIndex };
			Location = Location * pow(2, MaxDepth > Depth ? MaxDepth - Depth : 0);
			Location.Z = AreaIndex;
			GridInfo.HexLocations.Add(Location);
			HexGrids.Add(hash, GridInfo);
		}
		else
		{

			FHexGridInfo &GridInfo = HexGrids[hash];
			FIntVector Location = { HexPoints[1].X, HexPoints[1].Y, AreaIndex };
			Location = Location * pow(2, MaxDepth > Depth ? MaxDepth - Depth : 0);
			Location.Z = AreaIndex;

			GridInfo.HexLocations.Add(Location);

			int TriIdx1 = AddTerrainPoint(P1, UVPs[0]);
			uint32 hash1 = GetTypeHash(P1);
			GridInfo.AddSide(hash1, TriIdx1);

			int TriIdx3 = AddTerrainPoint(P3, UVPs[2]);
			uint32 hash3 = GetTypeHash(P3);
			GridInfo.AddSide(hash3, TriIdx3);
		}

		for (int Idx : PitchTriangles)
		{
			int TriangleIdx = AddTerrainPoint(PitchVertices[Idx], PitchVerticesUV[Idx]);

			TerrainTriangles.Add(TriangleIdx);
		}
		return;
	}

	FVectorDouble3D P[6] = { P1, P2, P3, EdgeCenter[0], EdgeCenter[1], EdgeCenter[2] };
	FIntPoint H[6] = { HexPoints[0] * 2, HexPoints[1] * 2, HexPoints[2] * 2,
		(HexPoints[0] + HexPoints[1]), (HexPoints[1] + HexPoints[2]), (HexPoints[2] + HexPoints[0]) };

	int Idx[12] = { 3, 5, 0,   4, 5, 3,     3, 1, 4,   2, 5, 4 };
	bool Valid[4] = { 1, 1, 1, 1 };

	if (EdgeTest[0])
	{
		P[3] = P1;
		Valid[0] = 0;
	}
	if (EdgeTest[1])
	{
		P[4] = P2;
		Valid[2] = 0;
	}
	if (EdgeTest[2])
	{
		P[5] = P3;
		Valid[3] = 0;
	}

	for (int i = 0; i < 4; i++)
	{
		if (Valid[i])
		{
			int i1 = Idx[3 * i + 0];
			int	i2 = Idx[3 * i + 1];
			int i3 = Idx[3 * i + 2];

			CreateVerticesRecursive(
				{P[i1], P[i2], P[i3]}, 
				{ H[i1], H[i2], H[i3] },
				Center, AreaIndex, Size / 2, Depth + 1);
		}
	}
}
FVectorDouble3D APlanetActor::Vector3dFromHexXY(const FIntVector &Point, int Level)
{
	return RegularPentagonTriangleAreas[Point.Z].LocalXYToWorldPosition({Point.X, Point.Y}, Level, false);
}
FIntVector APlanetActor::GetHexXY(const FVector& Point, int Level)
{
	for (FRegularPentagonTriangleArea &Item : RegularPentagonTriangleAreas)
	{
		FIntVector Ret = Item.GetHexXY(Point, Level);

		if (Ret != FIntVector(-1, -1, -1))
		{
			return Ret;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("GetHexXY not found %s ..!"), *Point.ToString());

	return FIntVector(-1, -1, -1);

}
FHexGridInfo APlanetActor::GetTerrainPosition(const FVector &Point)
{
	FIntVector Location = GetHexXY(Point, MaxDepth + 1);
	if (Location.X == -1)
	{
		return FHexGridInfo();

	}

	FVectorDouble3D Pos = Vector3dFromHexXY(Location, MaxDepth + 1);

	uint32 hash = GetTypeHash(Pos);
	FHexGridInfo *HexGridInfo = HexGrids.Find(hash);
	if (HexGridInfo != nullptr)
	{
		if (HexGridInfo->SphericalCoordinate.X == 0.0 &&  HexGridInfo->SphericalCoordinate.Y == 0.0)
		{
			HexGridInfo->SphericalCoordinate = SphericalFromCartesian(HexGridInfo->Point);
			HexGridInfo->TerrainHeight = VerticesHeight[HexGridInfo->VerticeIndex];
		}
		return *HexGridInfo;
	}
	
	for (auto &Item :HexGrids)
	{
		if (Item.Value.HexLocations[0] == Location)
		{
			FVector P = DrawTerrainVertices[Item.Value.VerticeIndex];
			UE_LOG(LogTemp, Warning, TEXT("GetTerrainPosition %s ..!"), *P.ToString());
			return Item.Value;
		}
	}

	return FHexGridInfo();

}
void APlanetActor::SetActorGroundPointAndRotator(const AActor *Actor, const FVector &Point, bool FacePlayer)
{
	FVector Pl = Point;
	AActor *A = (AActor *)Actor;

	FIntVector Location = GetHexXY(Pl, MaxDepth + 1);
	if (Location.X == -1)
	{
		return;
	}

	for (auto &Item : HexGrids)
	{
		if (Item.Value.HexLocations[0] == Location)
		{
			FVector P = DrawTerrainVertices[Item.Value.VerticeIndex];
			FRotator Rotator = FRotationMatrix::MakeFromX(P * -1.0).Rotator();
			if (FacePlayer)
			{
				FVector Pi = GEngine->GetFirstLocalPlayerController(GetWorld())->GetPawn()->GetActorLocation();
				Rotator = FRotationMatrix::MakeFromXZ(FVector::CrossProduct(Pi, P), P).Rotator();

			}
			else
			{
				Rotator.Pitch = Rotator.Pitch + 90;
			}
			A->SetActorRotation(Rotator);
			A->SetActorLocation(P);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SetActorGroundPointAndRotator not found ..!"));

}
TArray<FHexGridInfo> APlanetActor::TempShowRangePos(const FVector &Pos, float V)
{
	TArray<FHexGridInfo> R;

	for (auto &Item : HexGrids)
	{
		float A = acos(Item.Value.Point | Pos);
		if (A < V)
		{
			R.Add(Item.Value);
		}
	}
	return R;
}
TArray<FHexGridInfo> APlanetActor::TempShowRangePosA(const FIntVector &Pos, float V)
{

	TArray<FHexGridInfo> R;
	FVectorDouble3D P = Vector3dFromHexXY(Pos, MaxDepth + 1);

	uint32 hash = GetTypeHash(P);
	FHexGridInfo *HexGridInfo = HexGrids.Find(hash);
	
	for (auto &Item :HexGrids)
	{
		if (Item.Value.HexLocations[0] == Pos)
		{
			R.Add(Item.Value);
			return R;
		}
	}
	return R;
}
TArray<FHexGridInfo> APlanetActor::TempGetHexGridInfo(const int Begin, const int End)
{
	TArray<FHexGridInfo> R;
	for (auto &Item : HexGrids)
	{
		if (Item.Value.VerticeIndex == 1)
		{
			R.Add(Item.Value);
		}
	}
	return R;
}
TArray<FVector>  APlanetActor::GetHexRandomPoint(const FHexGridInfo& Hex, int Density, int DensityIndex)
{
	const char *P0 = "758193175231676967119430603117310424686683842516767481802071468428272271125569054809383400705332093458353404753123677177";
	const char *P1 = "889852119830415017602402534964020946568562492484687788842558918821191886871317160511712629954680682350519798388386857021";
	const char *P2 = "331683296567306135970220695003348904733292463265748034956442991160014068536265928030910270953335508755146052797290050301";
	const char *P3 = "334443297411081618935085097501713552626818879279620793858969261641903520901172942351929924952409102625576395549120361747";
	const char *P4 = "404674934273614271582425309988314245103954928270751827852572646759752382459172893747756098980655817221038192987045019103";
	const char *P5 = "538590938481041997294456284313218934657704699686693057641591003839397067858661965940388789479201209850146961545256402083";
	const char *PK[6] = { P0, P1, P2, P3, P4, P4 };
	static int ID[6] = {2, 0, 1, 3, 5, 4};

	TArray<FVector> R;
	if (Hex.SidesVerticeIndex.Num() != 6) return R;

	if (Hex.Level != 13) return R;

	FVector2D Noise2D = SphericalFromCartesian(Hex.Point).ToVector2D()* NoiseArgs[DensityIndex].X;
	float H = USimplexNoiseBPLibrary::SimplexNoise2D(Noise2D.X, Noise2D.Y);
	if (H < 0) return R;

	Density = H * Density;
	int K = H * 10;
	K = (H * 100 - K * 10);
	K = K % 6;
	const char *P = PK[K];;

	if (Density > 39) Density = 39;

	for (int i = 0; i < Density; i++)
	{
		int S1 = ID[(P[i * 3] - 48) % 6];
		int S2 = ID[(P[i * 3] - 47) % 6];

		float X = (P[3 * i + 1] - 48) * 0.1;
		float Y = (P[3 * i + 2] - 48) * 0.1;

		FQuatDouble QuatD = GetQuatDouble(FVectorDouble3D(0, 0, 1), TerrainVertices[Hex.VerticeIndex], 1.0);
		if (X == 0.0) X = 0.5;
		X = X * LevelAngle[Hex.Level];
		Y = 2.0 * Y * PI;
		FVectorDouble3D HP = SphericalToCartesian(FVectorDouble2D(X, Y));
		HP.Normalize();
		FVectorDouble3D NDP = QuatD.RotateVector(HP);
		float Height = GetTerrainHeight(NDP.ToVector());
		R.Add(NDP.ToVector() * Height);
	}
	return R;
}

FVectorDouble3D APlanetActor::GetPlayerHexCenterNormal()
{
	UWorld* World = GetWorld();
	APawn *Pawn = GEngine->GetFirstLocalPlayerController(World)->GetPawn();
	FIntVector Location = GetHexXY(Pawn->GetActorLocation(), MaxDepth + 1);
	if (Location.X == -1)
	{
		return { 0,0,0 };
	}
	return Vector3dFromHexXY(Location, MaxDepth + 1);
}
void APlanetActor::UpdateFoliage()
{
	FVector Pl = GEngine->GetFirstLocalPlayerController(GetWorld())->GetPawn()->GetActorLocation();

	for (auto &Item : HexGrids)
	{
		TArray<int> HexObject = UPlanetDatasLibrary::GetHexObject(Item.Value.HexLocations[0]);
		if (HexObject.Num() > 0)
		{
			FVector Point = DrawTerrainVertices[Item.Value.VerticeIndex];
			FRotator Rotator = FRotationMatrix::MakeFromXZ(FVector::CrossProduct(Pl, Point), Point).Rotator();
			OnCreateHexObject(Point, Rotator);
			continue;
		}
		if (Item.Value.Level > MaxDepth - 6)
		{
			for (auto &PFItem : PlanetFoliages)
			{
				int Index = Item.Value.FoliageDatas.Add(TArray<FTransform>());
				Item.Value.FoliageInstanceIndex.Add(TArray<int>());

				if (VerticesHeight[Item.Value.VerticeIndex] > PFItem.PlaceMaxHeight ||
					VerticesHeight[Item.Value.VerticeIndex] < PFItem.PlaceMinHeight)
				{
					continue;
				}

				TArray<FVector> PS = GetHexRandomPoint(Item.Value, PFItem.Density, PFItem.DensityNoiseIndex);
				for (FVector &PItem : PS)
				{
					FTransform InstanceTransform;
					InstanceTransform.SetLocation(PItem);

					FRotator Rotator = FRotationMatrix::MakeFromXZ(FVector::CrossProduct(Pl, PItem), PItem).Rotator();

					InstanceTransform.SetRotation(Rotator.Quaternion());
					InstanceTransform.SetScale3D(PFItem.Scale);
					//PFItem.Foliage()->AddInstance(InstanceTransform);
					Item.Value.FoliageDatas[Index].Add(InstanceTransform);
				}
			}
		}
	}
}
#pragma optimize("", on)