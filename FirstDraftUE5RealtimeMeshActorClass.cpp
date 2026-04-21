// Fill out your copyright notice in the Description page of Project Settings.

#include "CPP_RuntimeTerrain.h"
#include "RealtimeMeshSimple.h"
#include "Misc/AssertionMacros.h"

using namespace RealtimeMesh;

// Sets default values
ACPP_RuntimeTerrain::ACPP_RuntimeTerrain()
{
	PrimaryActorTick.bCanEverTick = false; //only generate mesh once

	M_fileLines = TArray<FString, FDefaultAllocator>();
	M_pointData = TMap<double, FInternalPointData>();
	M_pointBuilderID = TMap<double, uint32>();
	M_pointLogged = TMap<double, bool>();
	M_trianglesLOD0 = TArray<FInternalTriangleData>();

}

// Called when the game starts or when spawned
void ACPP_RuntimeTerrain::BeginPlay()
{
	Super::BeginPlay();
}

void ACPP_RuntimeTerrain::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, TEXT("Construction of terrain is initiated. Reading CSV"));
		}
	}

	M_fileLines.Reset();
	M_pointData.Reset();
	M_pointBuilderID.Reset();
	M_pointLogged.Reset();
	M_trianglesLOD0.Reset();

	auto vertices = Resolution * Resolution;
	M_fileLines.Reserve(vertices + 1);
	M_pointData.Reserve(vertices);
	M_pointsLOD2.Reserve(vertices/2); 
	M_pointsLOD3.Reserve(vertices/4);
	M_pointsLOD4.Reserve(vertices/8);
	M_pointBuilderID.Reserve(vertices);

	M_pointLogged.Reserve(vertices);
	M_trianglesLOD0.Reserve(vertices * 4);
	M_trianglesLOD1.Reserve(vertices/2 * 4);
	M_trianglesLOD2.Reserve(vertices/4 * 4);
	M_trianglesLOD3.Reserve(vertices/8 * 4);

	GetPointDataFromCSV();

	const FColor colors[4] = {FColor::Red, FColor::Emerald, FColor::Cyan, FColor::Purple}; 

	//Using open source thrid party plugin that works similar to procedurcal mesh component, but is more memory efficent
	GetRealtimeMeshComponent()->SetWorldLocation(FVector(0.0, 0.0, 0.0));
	GetRealtimeMeshComponent()->SetGenerateOverlapEvents(true);
	GetRealtimeMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetRealtimeMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
	GetRealtimeMeshComponent()->SetCollisionObjectType(ECC_WorldDynamic);
	GetRealtimeMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	// We want to create 4 levels of detail, less detail the further away we are from a players camera. Therefor we create 4 meshes
	MakeTriangles(colors[0]);
	MakeTrianglesForLODs(2, M_pointsLOD2, colors[1]);
	MakeTrianglesForLODs(4, M_pointsLOD3, colors[2]);
	MakeTrianglesForLODs(8, M_pointsLOD4, colors[3]);

	URealtimeMeshSimple* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();
	RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial");

	CreateMeshForLODlevel(0, RealtimeMesh, colors[0]);
	if (GenerateLODs) CreateMeshForLODlevel(1, RealtimeMesh, colors[1]);
    if (GenerateLODs) CreateMeshForLODlevel(2, RealtimeMesh, colors[2]);
    if (GenerateLODs) CreateMeshForLODlevel(3, RealtimeMesh, colors[3]);

	MeshBounds = GetRealtimeMeshComponent()->CalcBounds(GetRealtimeMeshComponent()->GetComponentTransform());
	GetRealtimeMeshComponent()->RecreatePhysicsState();

	GetRealtimeMeshComponent()->RegisterComponent();
	RootComponent = GetRealtimeMeshComponent(); 
}

void ACPP_RuntimeTerrain::CreateMeshForLODlevel(int32 LODlevel, URealtimeMeshSimple* RealtimeMesh, FColor color)
{
	// LODlevelFactor = 2^LODlevel
	// sanity check
	if (!(LODlevel == 0 || LODlevel == 1 || LODlevel == 2 || LODlevel == 3))
	{
		UE_LOG(LogTemp, Error, TEXT("[Mesh Creation] Invalid LOD level %d . Has to be between 0 - 3"), LODlevel);
		return; 
	}

	FRealtimeMeshStreamSet StreamSet;

	auto trianglesRef = &M_trianglesLOD0;
	if (LODlevel == 1)      trianglesRef = &M_trianglesLOD1; 
	else if (LODlevel == 2) trianglesRef = &M_trianglesLOD2;
	else if (LODlevel == 3) trianglesRef = &M_trianglesLOD3;

	// automatically created level 0 so we only add subsequent LOD levels
	// also view https://github.com/TriAxis-Games/RealtimeMeshComponent/blob/master/Source/RealtimeMeshExamples/Private/RealtimeMeshLODExample.cpp
	if (LODlevel > 0)
	{
		auto config = FRealtimeMeshLODConfig(FMath::Pow(0.5f, LODlevel));
		RealtimeMesh->AddLOD(config);
	}

	// We iterate over triangles to get the data we need for the mesh as the context is important
	int quadCount = 0;
	for (int triangs = 0; triangs < trianglesRef->Num(); triangs++)
	{
		auto Builder = TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>(StreamSet);

		Builder.EnableTangents();
		Builder.EnableTexCoords();
		Builder.EnableColors();
		Builder.EnablePolyGroups();

		const auto triangle1 = (*trianglesRef)[triangs++];
		const auto triangle2 = (*trianglesRef)[triangs];

		AddDataToMeshBuilder(Builder, triangle1, LODlevel, color);
		AddDataToMeshBuilder(Builder, triangle2, LODlevel, color);

		quadCount++;
	}

	// reset logged for next mesh
	for (auto [_, loggedBool] : M_pointLogged)
	{
		loggedBool = false;
	}

	if (GEngine && OnScreenDebugMessages)
	{
		auto info = FString(TEXT("INFORMATION: "));
		info += FString::FromInt(quadCount);
		info += TEXT(" Quads were created for mesh LOD level ");
		info += FString::FromInt(LODlevel);
		GEngine->AddOnScreenDebugMessage(-1, 10.f, color, info);
	}

	FString quadName = TEXT("Mesh_");
	quadName += FString::FromInt(LODlevel);
	quadName += FString(TEXT("_Quad_Count_"));
	quadName += FString::FromInt(quadCount); 

	FString logMsg = TEXT("[Mesh] created with Name: ");
	logMsg += quadName; 
	UE_LOG(LogTemp, Warning, TEXT("%s"), *logMsg);

	// Create Section group based on LOD level and update configs  
	FString lodName = "LOD_"; 
	lodName += FString::FromInt(LODlevel);
	const auto GroupKey = FRealtimeMeshSectionGroupKey::Create(LODlevel, FName(lodName));
	const FRealtimeMeshSectionKey PolyGroup0SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);
	RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
	RealtimeMesh->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(0));
}

//We iterate over the triangle data so we can set the vertexes, normals and tangents
void ACPP_RuntimeTerrain::AddDataToMeshBuilder(TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>& BuilderRef, const FInternalTriangleData& triangleWithPointIDs, uint32 polygroupIndex, FColor color)
{
	//Get PointData keys
	const auto pointKeyA = triangleWithPointIDs.triangleVertexIDs[0] + 1;
	const auto pointKeyB = triangleWithPointIDs.triangleVertexIDs[1] + 1;
	const auto pointKeyC = triangleWithPointIDs.triangleVertexIDs[2] + 1;

	//check which one's were logged already
	auto pAlogged = M_pointLogged.Find(pointKeyA);
	auto pBlogged = M_pointLogged.Find(pointKeyB);
	auto pClogged = M_pointLogged.Find(pointKeyC);

	if (pAlogged == nullptr || pBlogged == nullptr || pClogged == nullptr)
	{
		if (OnScreenDebugMessages)
		{
			if (GEngine)
			{
				auto error = FString(TEXT("ERROR: boolean flag for point logging not initialised!"));
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
			}
		}
		return;
	}

	//Get actual points
	auto pointA = M_pointData.Find(pointKeyA);
	auto pointB = M_pointData.Find(pointKeyB);
	auto pointC = M_pointData.Find(pointKeyC);

	if (pointA == nullptr || pointB == nullptr || pointC == nullptr)
	{
		if (OnScreenDebugMessages)
		{
			if (GEngine)
			{
				auto error = FString(TEXT("FATAL ERROR : point data for point not initialised!"));
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
			}
		}
		if (LogDebugMessages) UE_LOG(LogTemp, Error, TEXT("[Important Error] [Point] IDs %f %f %f resulted in a nullptr ref for base mesh, point data for point not initialised!"), pointKeyA, pointKeyB, pointKeyC);
		return;
	}

	auto invalidID = Resolution * Resolution + 1;
	uint32 builderA = invalidID, builderB = invalidID, builderC = invalidID;
	FVector3f vertexA, vertexB, vertexC;       // point data
	FVector2f positionA, positionB, positionC; // XY / UV coordinates

	//Get normals and tangents - https://forums.unrealengine.com/t/creating-the-tangentx-tangenty-tangentz-from-vertex-and-normal/308424/4
	FVector Edge1 = { (pointB->point[0] - pointA->point[0]), (pointB->point[1] - pointA->point[1]), (pointB->point[2] - pointA->point[2]) };
	FVector Edge2 = { (pointC->point[0] - pointA->point[0]), (pointC->point[1] - pointA->point[1]), (pointC->point[2] - pointA->point[2]) };
	auto dUV1 = FVector2f((pointB->point[0] - pointA->point[0]), (pointB->point[1] - pointA->point[1]));
	auto dUV2 = FVector2f((pointC->point[0] - pointA->point[0]), (pointC->point[1] - pointA->point[1]));
	float tangentFactor = 1.0 / (dUV1[0] * dUV2[1] - dUV1[1] * dUV2[0]);

	FVector   NormalVector = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
	FVector3f Tangents{
		(float)(tangentFactor * ((float)dUV2[1] * (float)Edge1[0] - (float)dUV1[1] * (float)Edge2[0])),
		(float)(tangentFactor * ((float)dUV2[1] * (float)Edge1[1] - (float)dUV1[1] * (float)Edge2[1])),
		(float)(tangentFactor * ((float)dUV2[1] * (float)Edge1[2] - (float)dUV1[1] * (float)Edge2[2]))
	};

	NormalVector.Normalize();
	Tangents.Normalize();

	//Process points by adding position to vertices if not logged yet
	//If points were logged, then we already added them as vertex to builder and know their new ID
	if (*pAlogged != true)
	{
		AddPointToBuilder(pointKeyA, vertexA, positionA, NormalVector, Tangents, color, BuilderRef, builderA);
		UE_LOG(LogTemp, Warning, TEXT("[Log] Point A with key %f logged"), pointKeyA);
	}
	else
	{
		builderA = (*(M_pointBuilderID.Find(pointKeyA))) - 1;
	}

	if (*pBlogged != true)
	{
		AddPointToBuilder(pointKeyB, vertexB, positionB, NormalVector, Tangents, color, BuilderRef, builderB);
		UE_LOG(LogTemp, Warning, TEXT("[Log] Point B with key %f logged"), pointKeyB);
	}
	else
	{
		builderB = (*(M_pointBuilderID.Find(pointKeyB))) - 1;
	}

	if (*pClogged != true)
	{
		AddPointToBuilder(pointKeyC, vertexC, positionC, NormalVector, Tangents, color, BuilderRef, builderC);
		UE_LOG(LogTemp, Warning, TEXT("[Log] Point C with key %f logged"), pointKeyC);
	}
	else
	{
		builderC = (*(M_pointBuilderID.Find(pointKeyC))) - 1;
	}

	auto builderIDAInvalid = builderA == invalidID;
	auto builderIDBInvalid = builderB == invalidID;
	auto builderIDCInvalid = builderC == invalidID;
	if ((builderIDAInvalid || builderIDBInvalid || builderIDCInvalid)) //sanity check 
	{
		if (OnScreenDebugMessages && GEngine)
		{
			auto error = FString(TEXT("ERROR: one of the points in the triangle was not associated with a valid a builder IDs [Builder IDs]"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
		}
		if (LogDebugMessages) UE_LOG(LogTemp, Display, TEXT("[Builder IDs] A %d B %d, C %d"), builderA, builderB, builderC);
	}

	//Add triangle to builder 
	BuilderRef.AddTriangle(builderA, builderB, builderC, 0);
}

/*
//We iterate over the triangle data so we can set the vertexes and their colour in context
void ACPP_RuntimeTerrain::CreateNormalsAndTangentsLODs(const FInternalTriangleData& triangleWithPointIDs, int32 LODlevelFactor, TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>& BuilderRef)
{
	bool validLODlvl = ((LODlevelFactor == 0) || (LODlevelFactor == 2) || (LODlevelFactor == 4) || (LODlevelFactor == 8));
	if (!validLODlvl)
	{
		UE_LOG(LogTemp, Error, TEXT("[LOD Gen] Could not deduce LOD level for factor %d"), LODlevelFactor);
		if (GEngine)
		{
			auto error = FString(TEXT("IMPORTANT ERROR: Could not deduce LOD level for factor: "));
			error += FString::FromInt(LODlevelFactor);
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, error);
		}
		return;
	}

	//Get PointData keys
	const auto pointKeyA = triangleWithPointIDs.triangleVertexIDs[0] + 1;
	const auto pointKeyB = triangleWithPointIDs.triangleVertexIDs[1] + 1;
	const auto pointKeyC = triangleWithPointIDs.triangleVertexIDs[2] + 1;

	//Get actual points
	auto pointA = M_pointData.Find(pointKeyA);
	auto pointB = M_pointData.Find(pointKeyB);
	auto pointC = M_pointData.Find(pointKeyC);

	if (pointA == nullptr || pointB == nullptr || pointC == nullptr)
	{
		if (OnScreenDebugMessages)
		{
			if (GEngine)
			{
				auto error = FString(TEXT("FATAL ERROR : point data for point not initialised!"));
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
			}
		}
		return;
	}

	auto invalidID = Resolution * Resolution + 1;
	uint32 builderA = invalidID, builderB = invalidID, builderC = invalidID;
	FVector3f vertexA, vertexB, vertexC;       // point data
	FVector2f positionA, positionB, positionC; // XY / UV coordinates

	//Get normals and tangents - https://forums.unrealengine.com/t/creating-the-tangentx-tangenty-tangentz-from-vertex-and-normal/308424/4
	FVector Edge1 = { (pointB->point[0] - pointA->point[0]), (pointB->point[1] - pointA->point[1]), (pointB->point[2] - pointA->point[2]) };
	FVector Edge2 = { (pointC->point[0] - pointA->point[0]), (pointC->point[1] - pointA->point[1]), (pointC->point[2] - pointA->point[2]) };
	auto dUV1 = FVector2f((pointB->point[0] - pointA->point[0]), (pointB->point[1] - pointA->point[1]));
	auto dUV2 = FVector2f((pointC->point[0] - pointA->point[0]), (pointC->point[1] - pointA->point[1]));
	float tangentFactor = 1.0 / (dUV1[0] * dUV2[1] - dUV1[1] * dUV2[0]);

	FVector   NormalVector = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
	FVector3f Tangents{
		(float)(tangentFactor * ((float)dUV2[1] * (float)Edge1[0] - (float)dUV1[1] * (float)Edge2[0])),
		(float)(tangentFactor * ((float)dUV2[1] * (float)Edge1[1] - (float)dUV1[1] * (float)Edge2[1])),
		(float)(tangentFactor * ((float)dUV2[1] * (float)Edge1[2] - (float)dUV1[1] * (float)Edge2[2]))
	};

	NormalVector.Normalize();
	Tangents.Normalize();

	auto builderIDAInvalid = builderA == invalidID;
	auto builderIDBInvalid = builderB == invalidID;
	auto builderIDCInvalid = builderC == invalidID;
	if ((builderIDAInvalid || builderIDBInvalid || builderIDCInvalid)) //sanity check 
	{
		if (OnScreenDebugMessages && GEngine)
		{
			auto error = FString(TEXT("ERROR: one of the points in the triangle was not associated with a valid a builder IDs [Builder IDs]"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
		}
		if (LogDebugMessages) UE_LOG(LogTemp, Error, TEXT("[LOD lvl factor %d ] [Builder IDs] A %d B %d, C %d"), LODlevelFactor, builderA, builderB, builderC);

		return; 
	}

	//Add triangle to builder 
	BuilderRef.AddTriangle(builderA, builderB, builderC, 0);
}
*/

//Adds the data for a point to the builder reference 
void ACPP_RuntimeTerrain::AddPointToBuilder(const double& pointKey, FVector3f& vertex, FVector2f& uv, FVector& Normals, FVector3f& Tangents, FColor Color, TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>& BuilderRef, uint32& builderID)
{
	//sanity check 
	if (!M_pointBuilderID.Contains(pointKey))
	{
		auto point = M_pointData.Find(pointKey);
		auto coord = point->point;
		vertex = FVector3f(coord);
		uv = FVector2f(vertex[0], vertex[1]); //Investigate: Add world offset
		FVector3f normals{ (float)Normals[0], (float)Normals[1], (float)Normals[2] };

		builderID = BuilderRef.AddVertex(vertex)
			.SetNormalAndTangent(normals, Tangents)
			.SetColor(Color)
			.SetTexCoord(uv);

		UE_LOG(LogTemp, Warning, TEXT("[Pt ID] Builder ID %d was emplaced"), builderID);

		//let us know we added this vertex to the builder
		M_pointBuilderID.Emplace(pointKey, builderID + 1);
		M_pointLogged.Remove(pointKey);
		M_pointLogged.Emplace(pointKey, true);
		return;
	}
	else
	{
		if (OnScreenDebugMessages) //fatal error
		{
			if (GEngine)
			{
				auto error = FString(TEXT("FATAL ERROR: pointKey was not found in builderID map"));
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
			}
		}
	}
}

void ACPP_RuntimeTerrain::GetPointDataFromCSV()
{
	// read .cvs files to get xyz point data to a comma seperated string
	auto filehelper = FFileHelper();
	filehelper.LoadFileToStringArray(M_fileLines, *csvFilePath);
	double pointID = 0.0;

	int lineIndex = 0;
	for (const auto inputLine : M_fileLines)
	{
		TArray<FString, FDefaultAllocator> xyz;
		xyz.Reserve(Resolution);
		inputLine.ParseIntoArray(xyz, TEXT(","), true);
		auto size = xyz.Num();
		UE_LOG(LogTemp, Warning, TEXT("[RowData] Row %d with %d points"), (lineIndex), size);

		for (int indexOfYValue = 0; indexOfYValue < xyz.Num(); indexOfYValue++)
		{
			int ZValue = (FCString::Atoi(*xyz[indexOfYValue]));

			if (LogDebugMessages)
			{
				auto input = *xyz[indexOfYValue];

				UE_LOG(LogTemp, Warning, TEXT("[CV Point Char] Read as: %s"), input);
				UE_LOG(LogTemp, Warning, TEXT("[CV Point int] Converted to: %d"), ZValue);
			}

			FInternalPointData point;
			pointID = (lineIndex * Resolution) + (indexOfYValue + 1);
			point.point[0] = (indexOfYValue) * Scale;
			point.point[1] = (lineIndex) * Scale;
			point.point[2] = (ZValue) * (Scale / 100); 

			if (DrawDebugSpheres)
			{
				FVector draw{ point.point[0], point.point[1], point.point[2] };
				DrawDebugSphere(GetWorld(), draw, 5, 16, FColor::Black, true);
			}

			if (LogDebugMessages)
			{
				auto xVal = point.point[0];
				auto yVal = point.point[1];
				auto zVal = point.point[2];
				UE_LOG(LogTemp, Warning, TEXT("[Point] ID %f, X %f, Y %f, Z %f"), pointID, xVal, yVal, zVal);
			}

			//We create 4 instances to accomodate the "Level of detail" rendering 
			bool isFirstPoint = lineIndex == 0 && indexOfYValue == 0;
			bool isFirstLineLast = lineIndex == 0 && ((indexOfYValue + 1) == xyz.Num());
			bool isLastLineFirst = ((lineIndex + 1) == xyz.Num()) && indexOfYValue == 0;
			bool isLastLast = ((lineIndex + 1) == xyz.Num()) && ((indexOfYValue + 1) == xyz.Num());

			M_pointData.Emplace(pointID, point);
			if (isFirstPoint || isFirstLineLast || isLastLineFirst || isLastLast)
			{
				// Technically this shouldn't be necessary, it's just to make double sure we always emplace the four corner vertexes
				M_pointsLOD2.Emplace(pointID, point);
				M_pointsLOD3.Emplace(pointID, point);
				M_pointsLOD4.Emplace(pointID, point);

				if (OuterPoints.Num() < 4)
				{
					auto pt1 = point.point[0];
					auto pt2 = point.point[1];
					auto pt3 = point.point[2];
					UE_LOG(LogTemp, Warning, TEXT("[Corner] Point %f %f %f added to corners"), pt1, pt2, pt3);
					OuterPoints.Add(point.point);
				}
			}
			else
			{
				if (((lineIndex % 2) == 0) && ((indexOfYValue % 2) == 0)) M_pointsLOD2.Emplace(pointID, point);
				if (((lineIndex % 4) == 0) && ((indexOfYValue % 4) == 0)) M_pointsLOD3.Emplace(pointID, point);
				if (((lineIndex % 8) == 0) && ((indexOfYValue % 8) == 0)) M_pointsLOD4.Emplace(pointID, point);
			}
		
			M_pointLogged.Emplace(pointID, false);
		}
		lineIndex++;
	}

	auto lineArraySize = M_fileLines.Num();
	auto pointDataSize = M_pointData.Num();
	UE_LOG(LogTemp, Warning, TEXT("[Lines Read] %d"), lineIndex);
	UE_LOG(LogTemp, Warning, TEXT("[Input string array size] %d"), lineArraySize);
	UE_LOG(LogTemp, Warning, TEXT("[Point Num] Total point after CSV complleted reading: %d"), pointDataSize);
	ensureMsgf(pointDataSize == (Resolution * Resolution), TEXT("Unexpected size for xyz coordinate vector, size was %d"), pointDataSize);

	TransformCenter = FVector(lineIndex/2, lineIndex/2, 1000); 
}

// This function maps the Point IDs to triangles in m_triangles
void ACPP_RuntimeTerrain::MakeTriangles(const FColor color)
{
	// We use the point IDs to infer pixel position and ierate over the "window" of four pixels at a time
	// Row 0 would have IDs 0-399, Row 1 400 - 799... So the following 4 coordinates make 1 Square/Quad in our mesh
	// One quad consists of 2 triangles
	// Imagine the matrix of points with our window like this: 
	// 
	// [index 1    index 2] ... points
	// [index 3    index 4] ... points
	// ...points

	int index1 = 0; //technically all could be intialised with 0, but values were put for clarity
	int index2 = 1;
	int index3 = Resolution;
	int index4 = Resolution + 1;

	if (M_pointData.Num() != (Resolution * Resolution))
	{
		if (GEngine)
		{
			auto error = FString(TEXT("IMPORTANT ERROR: The number of points allocated during the reading of the CSV is incorrect! Number :"));
			error += FString::FromInt(M_pointData.Num());
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, error);
		}
	}

	if (!M_pointData.IsEmpty())
	{
		for (int col = 0; col < Resolution; col++)
		{
			for (int row = 0; row < Resolution; row++)
			{
				index1 = (col)+(row * Resolution)+1;
				if (index1 < ((Resolution * Resolution) - (Resolution -1)))
				{
					index2 = index1 + 1;
					index3 = index1 + Resolution;
					index4 = index3 + 1;

					if (index1 < ((Resolution - 1) + (Resolution * row)))
					{
						if (M_pointData.Contains(index1) && M_pointData.Contains(index2) && M_pointData.Contains(index3) && M_pointData.Contains(index4))
						{
							// Triangles for meshes have to be created counter clockwise
							// We split the above mentioned quad into two by edge betweem vertices [index 3 - index 2]
							FInternalTriangleData Triangle1;
							FInternalTriangleData Triangle2;

							Triangle1.triangleVertexIDs[0] = index3;
							Triangle1.triangleVertexIDs[1] = index2;
							Triangle1.triangleVertexIDs[2] = index1;

							Triangle2.triangleVertexIDs[0] = index3;
							Triangle2.triangleVertexIDs[1] = index4;
							Triangle2.triangleVertexIDs[2] = index2;

							M_trianglesLOD0.Add(Triangle1);
							M_trianglesLOD0.Add(Triangle2);

							if (LogDebugMessages)
							{
								UE_LOG(LogTemp, Warning, TEXT("[Triangle] IDs %d %d %d"), index3, index2, index1);
								UE_LOG(LogTemp, Warning, TEXT("[Triangle] IDs %d %d %d"), index3, index4, index2);
								if (DrawDebugSpheres)
								{
									// Draw sphere at XY coordinates wjere Z = 0
									auto pt1 = M_pointData.Find(index1);
									auto pt2 = M_pointData.Find(index2);
									auto pt3 = M_pointData.Find(index3);
									auto pt4 = M_pointData.Find(index4);

									FVector pt1V{ pt1->point[0], pt1->point[1], 0.0 };
									FVector pt2V{ pt2->point[0], pt2->point[1], 0.0 };
									FVector pt3V{ pt3->point[0], pt3->point[1], 0.0 };
									FVector pt4V{ pt4->point[0], pt4->point[1], 0.0 };

									DrawDebugSphere(GetWorld(), pt3V, 10, 16, color, true);
									DrawDebugSphere(GetWorld(), pt2V, 10, 16, color, true);
									DrawDebugSphere(GetWorld(), pt1V, 10, 16, color, true);
								}
							}
						}
						else
						{
							if (!M_pointData.Contains(index1)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 1 does not exist in point data. ID %d"), index1);
							if (!M_pointData.Contains(index2)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 2 does not exist in point data. ID %d"), index2);
							if (!M_pointData.Contains(index3)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 3 does not exist in point data. ID %d"), index3);
							if (!M_pointData.Contains(index4)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 4 does not exist in point data. ID %d"), index4);
						}
					}
				}
			}
		}
	}

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			auto message = FString(TEXT("[Base Mesh] Created [Triangesls | Points] : [ "));
			auto triangleCount = M_trianglesLOD0.Num();
			auto pointCount = M_pointData.Num();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Purple, message);

			message = FString(TEXT("[Base Mesh] Memory for structs in bytes [Triangesls | Points] : [ "));
			triangleCount = M_trianglesLOD0.GetAllocatedSize();
			pointCount = M_pointData.GetAllocatedSize();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, message);
		}
	}
}

// This function maps the Point IDs to the LOD meshes
void ACPP_RuntimeTerrain::MakeTrianglesForLODs(const int32 LODlevelFactor, const TMap<double, FInternalPointData>& points, const FColor color)
{
	bool validLODlvl = ((LODlevelFactor == 2) || (LODlevelFactor == 4) || (LODlevelFactor == 8));
	if (!validLODlvl)
	{
		UE_LOG(LogTemp, Error, TEXT("[LOD Gen] Could not generate LOD level for mesh factord by %d. RT Mesh not generated"), LODlevelFactor);
		if (GEngine)
		{
			auto error = FString(TEXT("IMPORTANT ERROR: Could not generate runtime mesh with LOD level factor:"));
			error += FString::FromInt(LODlevelFactor);
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, error);
		}
		return;
	}

	auto* triangleArray = &M_trianglesLOD0;
	if (LODlevelFactor == 2)
	{
		triangleArray = &M_trianglesLOD1; 
	}
	else if (LODlevelFactor == 4)
	{
		triangleArray = &M_trianglesLOD2; 
	}
	else if (LODlevelFactor == 8)
	{
		triangleArray = &M_trianglesLOD3;
	}

	int index1 = 0; //technically all could be intialised with 0, but values were put for clarity
	int index2 = 1;
	int index3 = (Resolution/LODlevelFactor);
	int index4 = (Resolution/LODlevelFactor) + 1;

	if (!points.IsEmpty())
	{
		for (int col = 0; col < points.Num(); col++)
		{
			for (int row = 0; row < points.Num(); row++)
			{
				index1 = (col)+(row * (points.Num())) + 1;
				index2 = index1 + 1;
				index3 = index1 + (points.Num());
				index4 = index3 + 1;

				if (index1 < ((points.Num()) + ((points.Num()) * row)))
				{
					if (points.Contains(index1) && points.Contains(index2) && points.Contains(index3) && points.Contains(index4))
					{
						// Triangles for meshes have to be created counter clockwise
						// We split the above mentioned quad into two by edge betweem vertices [index 3 - index 2]
						FInternalTriangleData Triangle1;
						FInternalTriangleData Triangle2;

						Triangle1.triangleVertexIDs[0] = index3;
						Triangle1.triangleVertexIDs[1] = index2;
						Triangle1.triangleVertexIDs[2] = index1;

						Triangle2.triangleVertexIDs[0] = index3;
						Triangle2.triangleVertexIDs[1] = index4;
						Triangle2.triangleVertexIDs[2] = index2;

						triangleArray->Add(Triangle1);
						triangleArray->Add(Triangle2);

						if (LogDebugMessages)
						{
							UE_LOG(LogTemp, Warning, TEXT("[Triangle] IDs %d %d %d"), index3, index2, index1);
							UE_LOG(LogTemp, Warning, TEXT("[Triangle] IDs %d %d %d"), index3, index4, index2);
							if (DrawDebugSpheres)
							{
								// Draw sphere at XY coordinates wjere Z = 0
								auto pt1 = points.Find(index1);
								auto pt2 = points.Find(index2);
								auto pt3 = points.Find(index3);
								auto pt4 = points.Find(index4);

								FVector pt1V{ pt1->point[0], pt1->point[1], 0.0 };
								FVector pt2V{ pt2->point[0], pt2->point[1], 0.0 };
								FVector pt3V{ pt3->point[0], pt3->point[1], 0.0 };
								FVector pt4V{ pt4->point[0], pt4->point[1], 0.0 };

								DrawDebugSphere(GetWorld(), pt3V, 10 + LODlevelFactor, 16, color, true);
								DrawDebugSphere(GetWorld(), pt2V, 10 + LODlevelFactor, 16, color, true);
								DrawDebugSphere(GetWorld(), pt1V, 10 + LODlevelFactor, 16, color, true);
							}
						}
					}
					else
					{
						if (!M_pointData.Contains(index1)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 1 for LODlevelFactor %d does not exist in point data. ID %d"), index1, LODlevelFactor);
						if (!M_pointData.Contains(index2)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 2 for LODlevelFactor %d does not exist in point data. ID %d"), index2, LODlevelFactor);
						if (!M_pointData.Contains(index3)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 3 for LODlevelFactor %d does not exist in point data. ID %d"), index3, LODlevelFactor);
						if (!M_pointData.Contains(index4)) UE_LOG(LogTemp, Warning, TEXT("[Missing point ID] Index 4 for LODlevelFactor %d does not exist in point data. ID %d"), index4, LODlevelFactor);
					}
				}
			}
		}
	}

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			auto lodFactorString = FString(TEXT("[LOD "));
			lodFactorString += FString::FromInt(LODlevelFactor);
			lodFactorString += FString(TEXT("] "));

			auto message = lodFactorString; 
		    message += FString(TEXT("Created [Triangesls | Points] : [ "));
			auto triangleCount = triangleArray->Num();
			auto pointCount = points.Num();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, color, message);

			message = lodFactorString; 
			message += FString(TEXT("Memory for structs in bytes [Triangesls | Points] : [ "));
			triangleCount = triangleArray->GetAllocatedSize();
			pointCount = points.GetAllocatedSize();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, color, message);
		}
	}
}

void ACPP_RuntimeTerrain::PopulateRealtimeMeshSection()
{

}

// Called every frame
void ACPP_RuntimeTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

