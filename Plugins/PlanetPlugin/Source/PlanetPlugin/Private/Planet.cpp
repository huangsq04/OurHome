// Fill out your copyright notice in the Description page of Project Settings.


#include "Planet.h"
#include "Engine/Public/GeomTools.h"
#include "Math/UnrealMathUtility.h"
#include "SimplexNoiseBPLibrary.h"
#include "Components/SkyAtmosphereComponent.h"

TArray<FTriangleArea> APlanet::RegularPentagonTriangleAreas;

// Sets default values
APlanet::APlanet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RuntimeMeshComponent = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("RuntimeMeshComponent"));
	RootComponent = RuntimeMeshComponent;

	RuntimeMeshComponent->Mobility = EComponentMobility::Static;

	FTriangleArea::CreateBaseDodecahedron(RegularPentagonTriangleAreas);

	TerrainLoad = nullptr;
}

void APlanet::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	PlanetProvider = NewObject<UPlanetMeshProvider>(this, TEXT("PlanetMeshProvider"));
	PlanetProvider->SetSphereRadius(SphereRadius);

	RuntimeMeshComponent->Initialize(PlanetProvider);
}
// Called when the game starts or when spawned
void APlanet::BeginPlay()
{
	Super::BeginPlay();

	RuntimeMeshComponent->SetMaterial(0, MeshMaterialLand);
	if (bDrawOcean)
	{
		RuntimeMeshComponent->SetMaterial(1, MeshMaterialOcean);
	}
	PlanetProvider->SectionMeshForLODDelegate.BindUObject(this, &APlanet::GetSectionMeshForLOD);

	if (TerrainLoad == nullptr)
	{
		TerrainLoad = new FTerrainUpdateThread();
	}
}
void APlanet::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	TerrainLoad->Exit();
}
void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsNeedTerrainUpdate)
	{
		PlanetProvider->SetTime(DeltaTime);
		bIsNeedTerrainUpdate = false;
		bUpdateTerrain = false;
	}
	else
	{
		if (!bUpdateTerrain && TerrainUpdateCheck())
		{
			bUpdateTerrain = true;
			AsyncCreateTerrain();
		}
	}
}

float APlanet::GetNoiseTerrainHeight(const FVector2D &Point) const
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

	if(bGenHeight)
		return Height;
	else
		return 0.0;
}
double APlanet::GetTerrainHeight(const FVectorDouble3D &Point) const
{
	FVector2D XY = UPlanetFunctionLib::SphericalFromCartesian(Point).ToVector2D();
	return SphereRadius + GetNoiseTerrainHeight(XY) * NoiseScale.Z;
}
float APlanet::GetNoiseTerrainHeight(const FVector &Point)
{
	FVector2D XY = UPlanetFunctionLib::SphericalFromCartesian(Point).ToVector2D();
	return GetNoiseTerrainHeight(XY);
}
int APlanet::AddTerrainPoint(const FVectorDouble3D &Point, const FVector2D &UV)
{
	uint32 *Index;
	FVectorDouble2D XY = UPlanetFunctionLib::SphericalFromCartesian(Point);
	uint64 Hash = UPlanetFunctionLib::GetTypeHash(XY);
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

		FColor Color(UV.X * 255, UV.X * 255, UV.X * 255);
		TerrainColors.Add(Color);
		TerrainVertices.Add(Point);
		FVector P(Point.X, Point.Y, Point.Z);
		TerrainNormals.Add(P);
		return  VerticeIndex;
	}
}


FVectorDouble3D ScaleVector(const FVectorDouble3D &V, const FVectorDouble3D &A, const FVectorDouble3D &B, double AngleRad)
{
	FVectorDouble3D D = B - A;
	FVectorDouble3D R = V + D * AngleRad;
	R.Normalize();
	return R;
}

void APlanet::UpdateVerticesPoint(int Index, FQuatDouble &QuatD, FVectorDouble3D &CNormal)
{
	FVectorDouble3D P = QuatD.RotateVector(TerrainVertices[Index]);
	if (FMath::IsNaN(QuatD.X))
	{
		P = TerrainVertices[Index];
	}

	FVectorDouble2D XY = UPlanetFunctionLib::SphericalFromCartesian(P);
	SphericalCoordinateVertices[Index] = XY;

	float Height = GetNoiseTerrainHeight(XY.ToVector2D());
	VerticesHeight[Index] = Height;

	DrawTerrainVertices[Index] = (P * (SphereRadius + Height * NoiseScale.Z)).ToVector();

	TextureCoordinates[Index] = { 0, Height };
}

void APlanet::UpdateVerticesTangent(int Index)
{
	FVector V1 = DrawTerrainVertices[TerrainTriangles[Index * 3]];
	FVector V2 = DrawTerrainVertices[TerrainTriangles[Index * 3 + 1]];
	FVector V3 = DrawTerrainVertices[TerrainTriangles[Index * 3 + 2]];
	FVector C = (V1 + V2 + V3) / 3.0;

	C.Normalize();
	FVector B = FVector::CrossProduct(V2 - V1, V2 - V3);
	B.Normalize();

	float Angle = acos(FVector::DotProduct(B, C));

	if (FMath::IsNaN(Angle) || !FMath::IsFinite(Angle))
	{
		Angle = 0.0;
	}
	TextureCoordinates[TerrainTriangles[Index * 3]].X = (TextureCoordinates[TerrainTriangles[Index * 3]].X + Angle) / 2.0;
	TextureCoordinates[TerrainTriangles[Index * 3 + 1]].X = (TextureCoordinates[TerrainTriangles[Index * 3 + 1]].X + Angle) / 2.0;
	TextureCoordinates[TerrainTriangles[Index * 3 + 2]].X = (TextureCoordinates[TerrainTriangles[Index * 3 + 2]].X + Angle) / 2.0;
}

void APlanet::UpdateTerrain()
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

	FQuatDouble QuatD = UPlanetFunctionLib::GetQuatDouble(CenterNormal, CNormal, 1.0);

	ParallelFor(TerrainVertices.Num(),
		[&](int32 Index)
	{
		UpdateVerticesPoint(Index, QuatD, CNormal);
	});

	//计算斜率
	ParallelFor(TerrainTriangles.Num() / 3,
		[&](int32 Index)
	{
		UpdateVerticesTangent(Index);
	});

	bIsNeedTerrainUpdate = true;
}

void APlanet::AsyncUpdateTerrain()
{
	if (!TerrainLoad->TerrainUpdateFun.IsBound())
	{
		TerrainLoad->TerrainUpdateFun.BindUObject(this, &APlanet::UpdateTerrain);
	}
}

void APlanet::AsyncCreateTerrain()
{
	if (TerrainLoad == nullptr)
	{
		TerrainLoad = new FTerrainUpdateThread();
	}

	if (!TerrainLoad->TerrainUpdateFun.IsBound())
	{
		TerrainLoad->TerrainUpdateFun.BindUObject(this, &APlanet::CreateTerrain);
	}
}
void APlanet::CreateTerrain()
{
	FScopeLock Lock(&UpdateTerrainSync);
	VertexHash.Empty();
	TerrainVertices.Empty();
	TerrainTriangles.Empty();
	HexTextureCoordinates.Empty();
	SphericalCoordinateVertices.Empty();
	TextureCoordinates.Empty();
	DrawTerrainVertices.Empty();
	TerrainColors.Empty();
	DepthMaxAngle.Empty();
	CurrentDepthMax = 0;
	TerrainNormals.Empty();

	UWorld* World = GetWorld();
	APawn *Pawn = GEngine->GetFirstLocalPlayerController(World)->GetPawn();

	CenterNormal = GetPlayerHexCenterNormal();

	if (CenterNormal.X == 0 && CenterNormal.Y == 0 && CenterNormal.Z == 0)
	{
		return;
	}

	CreateElevation = Pawn->GetActorLocation().Size();

	if (CreateElevation > SphereRadius * 1.1)
	{
		CreateElevation = SphereRadius * 1.1;
	}

	//计算最大深度
	double EdLeng = RegularPentagonTriangleAreas[0].LenPos0Pos1 * SphereRadius;
	for (int i = 0; i < DefaultMaxDepth; i++)
	{
		DepthMaxAngle.Add(RegularPentagonTriangleAreas[0].LenPos0Pos1 / pow(2.0, i));
		if (EdLeng / pow(2.0, i) < (CreateElevation - SphereRadius )* EdgeHeightRatio)
		{
			MaxDepth = FMath::Clamp(i - 2, 2, DefaultMaxDepth);
			break;
		}
	}

	CullingAngleScale = (DefaultMaxDepth - MaxDepth + DefaultMaxDepth / 2.0) / (double)DefaultMaxDepth;
	CullingAngleScale = FMath::Clamp(CullingAngleScale, 0.5f, 1.0f);
	UpdatePlanetRatio = FMath::Clamp(1.0 - CullingAngleScale, 0.1, 0.5);


	CullingAngle = acos((double)SphereRadius / CreateElevation) * CullingAngleScale;
	CullingLength = sin(CullingAngle) * 2.0 ;

	FVectorDouble3D Pos = CenterNormal * CreateElevation;

	FVectorDouble3D Center = (Pos - this->GetActorLocation()) / SphereRadius;

	for (auto &Item : RegularPentagonTriangleAreas)
	{
		CreateVerticesRecursive(
			{Item.GetPos0(), Item.GetCenterPos(), Item.GetPos1()}, 
			{ {1, 0}, {0, 0}, {0, 1}}, CenterNormal, Item.GetIndex());
	}

	TextureCoordinates.SetNum(SphericalCoordinateVertices.Num());
	VerticesHeight.SetNum(SphericalCoordinateVertices.Num());
	DrawTerrainVertices.SetNum(TerrainVertices.Num());

	ParallelFor(TerrainVertices.Num(),
		[&](int32 Index)
	{
		HexTextureCoordinates[Index].Y = acos(TerrainVertices[Index] | CenterNormal);
	});

	struct FCompareAngle
	{
		FCompareAngle(const APlanet& InPlanetActor, int InCenter) :
			PlanetActor(InPlanetActor),Center(InCenter)
		{}
		FORCEINLINE bool operator()(const int& A, const int& B) const
		{
			float A1 = (PlanetActor.TerrainVertices[A] ^ PlanetActor.TerrainVertices[Center]) | FVectorDouble3D(0, 0, 1);
			float A2 = (PlanetActor.TerrainVertices[B] ^ PlanetActor.TerrainVertices[Center]) | FVectorDouble3D(0, 0, 1);
			return A1 > A2;
		}
		const APlanet& PlanetActor;
		int Center;
	};

	UpdateTerrain();

	bIsNeedTerrainUpdate = true;
}

void APlanet::CreateVerticesRecursive(
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
	double LoRatioSize = Size * RatioSize;

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

	FVectorDouble3D Pc = (P1 + P2 + P3) / 3.0;

	bool Culling = false;
	double L1 = (P1 - Pc).Size();
	double L2 = (Center - Pc).Size();

	if (L2 > (L1 + CullingLength))
	{
		return;//culling
	}

	if ((EdgeTest[0] && EdgeTest[1] && EdgeTest[2]) || Depth > MaxDepth || Depth == HexDepth)
	{
		if (CurrentDepthMax < Depth)
		{
			CurrentDepthMax = Depth;
		}
		TArray<FVector2D> UVPs = { {1,0}, {0,0}, {1,0} };
		if (CurrentDepthMax != HexDepth)
		{
			UVPs = { {0,0}, {0,0}, {0,0} };
		}
		else
		{
			UVPs = { {1,0}, {0,0}, {1,0} };
		}
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
FVectorDouble3D APlanet::Vector3dFromHexXY(const FIntVector &Point, int Level)
{
	return RegularPentagonTriangleAreas[Point.Z].LocalXYToWorldPosition({Point.X, Point.Y}, Level, false);
}
FIntVector APlanet::GetHexXY(const FVector& Point, int Level)
{
	for (FTriangleArea &Item : RegularPentagonTriangleAreas)
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
void APlanet::SetActorGroundPointAndRotator(const AActor *Actor, const FVector &Point, bool FacePlayer)
{
	FVector Pl = Point;
	AActor *A = (AActor *)Actor;

	FIntVector Location = GetHexXY(Pl, HexDepth);
	if (Location.X == -1)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("SetActorGroundPointAndRotator not found ..!"));
}

FVectorDouble3D APlanet::GetPlayerHexCenterNormal()
{
	UWorld* World = GetWorld();
	APawn *Pawn = GEngine->GetFirstLocalPlayerController(World)->GetPawn();
	FIntVector Location = GetHexXY(Pawn->GetActorLocation(), HexDepth);
	if (Location.X == -1)
	{
		return { 0,0,0 };
	}
	FVectorDouble3D Ret = Vector3dFromHexXY(Location, HexDepth);
	Ret.Normalize();
	return Ret;
}

bool APlanet::TerrainUpdateCheck()
{
	UWorld* World = GetWorld();
	APawn *Pawn = GEngine->GetFirstLocalPlayerController(World)->GetPawn();

	FVectorDouble3D Pl = Pawn->GetActorLocation();
	//高度更新检查
	double Angle = acos((double)SphereRadius / Pl.Size()) *  CullingAngleScale; 
	if (abs(CullingAngle - Angle) > CullingAngle * UpdatePlanetRatio)
	{
		return true;
	}

	//创建中心点夹角检查
	Pl.Normalize();
	double PlAg = acos(CenterNormal | Pl);
	if (PlAg > DepthMaxAngle[(CurrentDepthMax + 1) / 2] * UpdatePlanetRatio)
	{
		return true;
	}

	return false;
}
void APlanet::CheckUpdateTerrain()
{
	if (!bUpdateTerrain && TerrainUpdateCheck())
	{
		bUpdateTerrain = true;
		AsyncCreateTerrain();
	}
}

FVector APlanet::GetHitPoint(const FVector &Src, const FVector &Dir)
{
	FVectorDouble3D S = -Src;
	S.Normalize();
	FVectorDouble3D D = Dir;
	D.Normalize();
	double a = acos(S| D);
	if (a > 1.5)
	{
		//return { 0,0,0 };
	}

	double l = cos(a) * Src.Size();
	FVectorDouble3D C = Src + Dir * l;
	double Cl = C.Size();
	if (Cl > SphereRadius * 0.9999)
	{
		//return { 0,0,0 };
	}

	double A = sqrt((double)SphereRadius * (double)SphereRadius  - Cl * Cl);

	FVectorDouble3D V =  C - Dir * A;
	V.Normalize();

	int T1 = 10000;
	FVectorDouble3D LastPos;
	for (int i = 0; i < T1; i++)
	{
		FVectorDouble3D P1 = Src + Dir * l * (1.0/T1) * i;
		double PH = P1.Size();
		P1.Normalize();
		LastPos = P1;
		double TH = GetTerrainHeight(P1);

		if (PH - TH < 0.0)
		{
			V = LastPos;
			break;
		}
	}

	FIntVector Location = GetHexXY(V.ToVector(), HexDepth);
	if (Location.X == -1)
	{
		return { 0,0,0 };
	}

	V = Vector3dFromHexXY(Location, HexDepth);
	V.Normalize();

	V = V * GetTerrainHeight(V);
	return V.ToVector();
}
void APlanet::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{
	FScopeLock Lock(&UpdateTerrainSync);

	FVector Tangent(1, 0, 1);
	Tangent.Normalize();

	for (int32 Idx : TerrainTriangles)
	{
		MeshData.Triangles.Add(Idx);
	}

	for (int i = 0; i < DrawTerrainVertices.Num(); i++)
	{
		MeshData.TexCoords.Add(TextureCoordinates[i]);

		if (SectionId == 0)
		{
			MeshData.Positions.Add(DrawTerrainVertices[i]);
		}
		else
		{
			if (bDrawOcean)
			{
				MeshData.Positions.Add(TerrainNormals[i] * SphereRadius);
			}
		}

		MeshData.Colors.Add(TerrainColors[i]);
		MeshData.Tangents.Add(TerrainNormals[i], Tangent);
	}
}
