// Fill out your copyright notice in the Description page of Project Settings.

#include "ACPP_RuntimeTerrain.h"
#include "RealtimeMeshSimple.h"

using namespace RealtimeMesh; 

// Sets default values
ACPP_RuntimeTerrain::ACPP_RuntimeTerrain()
{
	PrimaryActorTick.bCanEverTick = fals// Fill out your copyright notice in the Description page of Project Settings.

#include "ACPP_RuntimeTerrain.h"
#include "RealtimeMeshSimple.h"
#include "Misc/AssertionMacros.h"

using namespace RealtimeMesh; 

// Sets default values
ACPP_RuntimeTerrain::ACPP_RuntimeTerrain()
{
	PrimaryActorTick.bCanEverTick = false; //only generate mesh once
	M_fileLines = TArray<FString, FDefaultAllocator>(); 
	M_pointData      = TMap<double, FInternalPointData>(); 
	M_pointBuilderID = TMap<double, uint32>();
	M_pointLogged    = TMap<double, bool>(); 
	M_triangles      = TArray<FInternalTriangleData>(); 

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
	M_triangles.Reset(); 

	auto vertices = Resolution * Resolution; 
	M_fileLines.Reserve(vertices +1);
	M_pointData.Reserve(vertices);
	M_pointBuilderID.Reserve(vertices);
	M_pointLogged.Reserve(vertices);
	M_triangles.Reserve(vertices *4);

	GetPointDataFromCSV();
	MakeTriangles();

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			auto message = FString(TEXT("Created [Triangesls | Points] : [ "));
			auto triangleCount = M_triangles.Num(); 
			auto pointCount = M_pointData.Num(); 
			message += FString::FromInt(triangleCount); 
			message += TEXT(" | "); 
			message += FString::FromInt(pointCount);
			message += TEXT(" ]"); 
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Purple, message);

		    message = FString(TEXT("Memory for structs in bytes [Triangesls | Points] : [ "));
		    triangleCount = M_triangles.GetAllocatedSize();
		    pointCount = M_pointData.GetAllocatedSize();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, message);
		}
	}

	//Using open source thrid party plugin that works similar to procedurcal mesh component, but is more memory efficent
	URealtimeMeshSimple* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>(); 
	FRealtimeMeshStreamSet StreamSet; 

	RealtimeMesh->SetupMaterialSlot(0, "PrimaryMaterial");

	// We iterate over triangles to get the data we need for the mesh as the context is important
	int quadCount = 0; 
	for (int triangs = 0; triangs < M_triangles.Num(); triangs++)
	{
		MeshBuilder Builder(StreamSet);

		Builder.EnableTangents();
		Builder.EnableTexCoords();
		Builder.EnableColors();
		Builder.EnablePolyGroups();

		// We have 2 triangles in the poly group of a quad
		const auto triangle1 = M_triangles[triangs++];
		const auto triangle2 = M_triangles[triangs];

		AddDataToMeshBuilder(Builder, triangle1, quadCount);
		AddDataToMeshBuilder(Builder, triangle2, quadCount);

		FString quadName = TEXT("Quad_"); 
		quadName += FString::FromInt(quadCount); 
		const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(quadCount, FName(quadName));
		const FRealtimeMeshSectionKey PolyGroupKey= FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, quadCount);
		RealtimeMesh->CreateSectionGroup(GroupKey, StreamSet);
		RealtimeMesh->UpdateSectionConfig(PolyGroupKey, FRealtimeMeshSectionConfig(0));

		if (OnScreenDebugMessages)
		{
			if (GEngine)
			{
				quadName += FString(TEXT(" created...")); 
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, quadName);
			}
		}

		quadCount++; 
	}

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			auto info = FString(TEXT("INFORMATION: "));
			info += FString::FromInt(quadCount); 
			info += TEXT(" Quads were created");
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, info);
		}
	}
}

/*
//Assumes data from csv file is 1 row per point, coordinates comma sperated
void ACPP_RuntimeTerrain::GetPointDataFromCSVParseArray()
{
	// read .cvs files to get xyz point data to a comma seperated string
	auto filehelper = FFileHelper();
	filehelper.LoadFileToStringArray(M_fileLines, *csvFilePath);
	FString deliminator = TEXT(",");

	int indexX = 0;
	int indexY = 0;
	int pointID = 0;

	// the CV application parses out the z values to a .csv file only for a 400 x 400 px image
	for (const FString line : M_fileLines)
	{
		FInternalPointData point;
		TArray<FString, FDefaultAllocator> xyz;
		line.ParseIntoArray(xyz, TEXT(","), true);

		for (const auto& coordinateAsString : xyz)
		{
			pointID = indexX + indexY;
			point.point[0] = indexX * 10; //100 unreal units is rougly 1m 
			point.point[1] = indexY * 10;
			point.point[2] = (FCString::Atoi(*line) * 10);

			if (indexX < 399)
			{
				indexX++;
			}
			else
			{
				indexX = 0;
				indexY++;
			}
		}
		M_pointData.EmplaceByHash(pointID, point);
		M_pointLogged.EmplaceByHash(pointID, false);
	}
}
*/


//We iterate over the triangle data so we can set the vertexes and their colour in context
void ACPP_RuntimeTerrain::AddDataToMeshBuilder(MeshBuilder& BuilderRef, const FInternalTriangleData& triangleWithPointIDs, uint32 polygroupIndex)
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

	//Get points
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

	//Process points by adding position to vertices if not logged yet
	//If points were logged, then we already added them as vertex to builder and know their new ID
	if (*pAlogged != true)
	{
		AddPointToBuilder(pointKeyA, vertexA, positionA, NormalVector, Tangents, FColor::Red, BuilderRef, builderA); 
		UE_LOG(LogTemp, Warning, TEXT("[Log] Point A with key %f logged"), pointKeyA);
	}
	else
	{
		builderA = (*(M_pointBuilderID.Find(pointKeyA))) - 1;
	}

	if (*pBlogged != true)
	{
		AddPointToBuilder(pointKeyB, vertexB, positionB, NormalVector, Tangents, FColor::Green, BuilderRef, builderB);
		UE_LOG(LogTemp, Warning, TEXT("[Log] Point B with key %f logged"), pointKeyB);
	}
	else
	{
		builderB = (*(M_pointBuilderID.Find(pointKeyB))) - 1;
	}

	if (*pClogged != true)
	{
		AddPointToBuilder(pointKeyC, vertexC, positionC, NormalVector, Tangents, FColor::Blue, BuilderRef, builderC);
		UE_LOG(LogTemp, Warning, TEXT("[Log] Point C with key %f logged"), pointKeyC);
	}
	else
	{
		builderC = (*(M_pointBuilderID.Find(pointKeyC))) - 1;
	}

	auto builderIDAInvalid = builderA == invalidID; 
	auto builderIDBInvalid = builderB == invalidID;
	auto builderIDCInvalid = builderC == invalidID;
	if (OnScreenDebugMessages && (builderIDAInvalid || builderIDBInvalid || builderIDCInvalid)) //sanity check 
	{
		if (GEngine)
		{
			auto error = FString(TEXT("ERROR: one of the points in the triangle was not associated with a valid a builder IDs [Builder IDs]"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
			UE_LOG(LogTemp, Warning, TEXT("[Builder IDs] A %d B %d, C %d"), builderA, builderB, builderC); 
		}
	}

	//Add triangle to builder 
	BuilderRef.AddTriangle(builderA, builderB, builderC, polygroupIndex);
}

//Adds the data for a point to the builder reference 
void ACPP_RuntimeTerrain::AddPointToBuilder(const double& pointKey, FVector3f& vertex, FVector2f& uv, FVector& Normals, FVector3f& Tangents, FColor Color, MeshBuilder& BuilderRef, uint32& builderID)
{
	//sanity check 
	if (!M_pointBuilderID.Contains(pointKey))
	{
		auto point = M_pointData.Find(pointKey);
		auto coord = point->point;
		vertex = FVector3f(coord);
		uv = FVector2f(vertex[0], vertex[1]); //Investigate: Add world offset
		FVector3f normals{ (float)Normals[0], (float)Normals[1], (float)Normals[2]};

		builderID = BuilderRef.AddVertex(vertex)
			.SetNormalAndTangent(normals, Tangents)
			.SetColor(Color)
			.SetTexCoord(uv);

		UE_LOG(LogTemp, Warning, TEXT("[Pt ID] Builder ID %d was emplaced"), builderID);

		//let us know we added this vertex to the builder
		M_pointBuilderID.Emplace(pointKey, builderID+1);
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
		//values should be chars representing -127 to 128; 
		TArray<FString, FDefaultAllocator> xyz;
		xyz.Reserve(400); 
		inputLine.ParseIntoArray(xyz, TEXT(","), true);
		auto size = xyz.Num();
		UE_LOG(LogTemp, Warning, TEXT("[RowData] Row %d with %d points"), (lineIndex + 1), size);

		for (int indexOfYValue = 0; indexOfYValue < xyz.Num(); indexOfYValue++)
		{
			int ZValue = (FCString::Atoi(*xyz[indexOfYValue]) * Scale);

			if (LogDebugMessages)
			{
				auto input = *xyz[indexOfYValue];

				UE_LOG(LogTemp, Warning, TEXT("[CV Point Char] Read as: %s"), input);
				UE_LOG(LogTemp, Warning, TEXT("[CV Point int] Converted to: %d"), ZValue);
			}

			FInternalPointData point;
			pointID = (lineIndex * 40) + (indexOfYValue +1);
			point.point[0] = indexOfYValue * Scale * 100;
			point.point[1] = lineIndex * Scale * 100;
			point.point[2] = (ZValue * Scale * 1);

			FVector draw{ point.point[0], point.point[1], point.point[2]};

			if (OnScreenDebugMessages)
			{
				auto xVal = point.point[0]; 
				auto yVal = point.point[1]; 
				auto zVal = point.point[2];
				UE_LOG(LogTemp, Warning, TEXT("[Point] ID %f, X %f, Y %f, Z %f"), pointID, xVal, yVal, zVal);
				DrawDebugSphere(GetWorld(), draw, 10, 16, FColor::Red, true); 
			}

			M_pointData.Emplace(pointID, point);
			M_pointLogged.Emplace(pointID, false);
		}
		lineIndex++;
	}

	auto lineArraySize = M_fileLines.Num(); 
	auto pointDataSize = M_pointData.Num(); 
	UE_LOG(LogTemp, Warning, TEXT("[Lines Read] %d"), lineIndex);
	UE_LOG(LogTemp, Warning, TEXT("[Input string array size] %d"), lineArraySize);
	UE_LOG(LogTemp, Warning, TEXT("[Point Num] Total point after CSV complleted reading: %d"), pointDataSize);
	ensureMsgf(pointDataSize == 1600, TEXT("Unexpected size for xyz coordinate vector, size was %d"), pointDataSize);
}

// This function maps the Point IDs to triangles in m_triangles
void ACPP_RuntimeTerrain::MakeTriangles()
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
	int index4 = Resolution +1;

	if (M_pointData.Num() != 1600)
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
				index1 = (col) + (row*Resolution); 
				if (index1 < ((Resolution *Resolution) - Resolution))
				{
					index2 = index1 + 1;
					index3 = index1 + 40;
					index4 = index3 + 1;

					if (index1 < (39 + (40*row)))
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

							M_triangles.Add(Triangle1);
							M_triangles.Add(Triangle2);

							if (LogDebugMessages)
							{
								UE_LOG(LogTemp, Warning, TEXT("[Triangle] IDs %d %d %d"), index3, index2, index1);
								UE_LOG(LogTemp, Warning, TEXT("[Triangle] IDs %d %d %d"), index3, index4, index2);
								if (DebugSpheres)
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

									DrawDebugSphere(GetWorld(), pt3V, 10, 16, FColor::Blue, true);
									DrawDebugSphere(GetWorld(), pt2V, 10, 16, FColor::Blue, true);
									DrawDebugSphere(GetWorld(), pt1V, 10, 16, FColor::Blue, true);

									DrawDebugSphere(GetWorld(), pt3V, 10, 16, FColor::Emerald, true);
									DrawDebugSphere(GetWorld(), pt4V, 10, 16, FColor::Emerald, true);
									DrawDebugSphere(GetWorld(), pt2V, 10, 16, FColor::Emerald, true);
								}

								auto point1 = M_pointData.Find(index1);
								auto point2 = M_pointData.Find(index2);
								auto point3 = M_pointData.Find(index3);
								auto point4 = M_pointData.Find(index4);

								FVector pt1{ point1->point[0],point1->point[1], point1->point[2] };
								FVector pt2{ point2->point[0],point2->point[1], point2->point[2] };
								FVector pt3{ point3->point[0],point3->point[1], point3->point[2] };
								FVector pt4{ point4->point[0],point4->point[1], point4->point[2] };

								DrawDebugPoint(GetWorld(), pt1, 10, FColor::Blue);
								DrawDebugPoint(GetWorld(), pt2, 10, FColor::Blue);
								DrawDebugPoint(GetWorld(), pt3, 10, FColor::Blue);

								DrawDebugPoint(GetWorld(), pt3, 10, FColor::Purple);
								DrawDebugPoint(GetWorld(), pt4, 10, FColor::Purple);
								DrawDebugPoint(GetWorld(), pt2, 10, FColor::Purple);
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
}

void ACPP_RuntimeTerrain::PopulateRealtimeMeshSection()
{

}

// Called every frame
void ACPP_RuntimeTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

e; //only generate mesh once
	//m_fileLines = TArray<FString, FDefaultAllocator>(); 
	M_pointData      = TMap<double, FInternalPointData>(); 
	M_pointBuilderID = TMap<double, int32>();
	M_pointLogged    = TMap<double, bool>(); 
	M_triangles      = TArray<FInternalTriangleData>(); 

}

// Called when the game starts or when spawned
void ACPP_RuntimeTerrain::BeginPlay()
{
	Super::BeginPlay();
}

void ACPP_RuntimeTerrain::OnConstruction(const FTransform& Tranform)
{
	//M_fileLines.Reset(); 
	M_pointData.Reset();
	M_pointBuilderID.Reset();
	M_pointLogged.Reset();
	M_triangles.Reset(); 

	GetPointDataFromCSV();
	MakeTriangles();

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			auto message = FString(TEXT("Created [Triangesls | Points] : [ "));
			auto triangleCount = M_triangles.Num(); 
			auto pointCount = M_pointData.Num(); 
			message += FString::FromInt(triangleCount); 
			message += TEXT(" | "); 
			message += FString::FromInt(pointCount);
			message += TEXT(" ]"); 
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Purple, message);

		    message = FString(TEXT("Memory for structs in bytes [Triangesls | Points] : [ "));
		    triangleCount = M_triangles.GetAllocatedSize();
		    pointCount = M_pointData.GetAllocatedSize();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, message);
		}
	}

	//Using open source thrid party plugin that works similar to procedurcal mesh component, but is more memory efficent
	URealtimeMeshSimple* RealtimeMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>(); 
	FRealtimeMeshStreamSet StreamSet; 

	// We iterate over triangles to get the data we need for the mesh as the context is important
	int quadCount = 0; 
	for (int triangs = 0; triangs < M_triangles.Num(); triangs++)
	{
		TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

		Builder.EnableTangents();
		Builder.EnableTexCoords();
		Builder.EnableColors();
		Builder.EnablePolyGroups();

		// We have 2 triangles in the poly group of a quad
		const auto triangle1 = M_triangles[triangs++];
		const auto triangle2 = M_triangles[triangs];

		AddDataToMeshBuilder(Builder, triangle1, quadCount);
		AddDataToMeshBuilder(Builder, triangle2, quadCount);

		FString quadName = TEXT("Quad_"); 
		quadName += FString::FromInt(quadCount); 
		const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(quadCount, FName(quadName));

		if (OnScreenDebugMessages)
		{
			if (GEngine)
			{
				quadName += FString(TEXT(" created...")); 
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, quadName);
			}
		}

		quadCount++; 
	}

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			auto info = FString(TEXT("INFORMATION: "));
			info += FString::FromInt(quadCount); 
			info += TEXT(" Quads were created");
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, info);
		}
	}
}

/*
//Assumes data from csv file is 1 row per point, coordinates comma sperated
void ACPP_RuntimeTerrain::GetPointDataFromCSVParseArray()
{
	// read .cvs files to get xyz point data to a comma seperated string
	auto filehelper = FFileHelper();
	filehelper.LoadFileToStringArray(m_fileLines, TEXT("C:/Users/evali/FinalProjectFiles/OpenCVModel/OpenCVLegoMapScannerV1/out/build/x64-debug/heightMapPointCloudData.csv"));
	FString deliminator = TEXT(",");

	int indexX = 0;
	int indexY = 0;
	int pointID = 0;

	// the CV application parses out the z values to a .csv file only for a 400 x 400 px image
	for (const FString line : m_fileLines)
	{
		FInternalPointData point;
		TArray<FString, FDefaultAllocator> xyz;
		line.ParseIntoArray(xyz, TEXT(","), true);

		for (const auto& coordinateAsString : xyz)
		{
			pointID = indexX + indexY;
			point.point[0] = indexX * 10; //100 unreal units is rougly 1m 
			point.point[1] = indexY * 10;
			point.point[2] = (FCString::Atoi(*line) * 10);

			if (indexX < 399)
			{
				indexX++;
			}
			else
			{
				indexX = 0;
				indexY++;
			}
		}
		M_pointData.EmplaceByHash(pointID, point);
		M_pointLogged.EmplaceByHash(pointID, false);
	}
}
*/

//We iterate over the triangle data so we can set the vertexes and their colour in context
void ACPP_RuntimeTerrain::AddDataToMeshBuilder(TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1>& BuilderRef, const FInternalTriangleData& triangleWithPointIDs, int8 polygroupIndex)
{
	//Get PointData keys
	const auto pointKeyA = triangleWithPointIDs.triangleVertexIDs[0];
	const auto pointKeyB = triangleWithPointIDs.triangleVertexIDs[1];
	const auto pointKeyC = triangleWithPointIDs.triangleVertexIDs[2];

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

	//Get points
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

	int32 builderA = -1, builderB = -1, builderC = -1;
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
		AddPointToBuilder(pointKeyA, vertexA, positionA, NormalVector, Tangents, FColor::Red, BuilderRef, builderA); 
	}
	else
	{
		builderA = *(M_pointBuilderID.Find(pointKeyA));
	}

	if (*pBlogged != true)
	{
		AddPointToBuilder(pointKeyB, vertexB, positionB, NormalVector, Tangents, FColor::Green, BuilderRef, builderB);
	}
	else
	{
		builderB = *(M_pointBuilderID.Find(pointKeyB));
	}

	if (*pClogged != true)
	{
		AddPointToBuilder(pointKeyC, vertexC, positionC, NormalVector, Tangents, FColor::Blue, BuilderRef, builderC);
	}
	else
	{
		builderC = *(M_pointBuilderID.Find(pointKeyC));
	}


	if (OnScreenDebugMessages && (builderA == -1 || builderB == -1 || builderC == -1)) //sanity check 
	{
		if (GEngine)
		{
			auto error = FString(TEXT("ERROR: one of the points has not got a builder ID"));
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, error);
		}
	}

	//Add triangle to builder 
	BuilderRef.AddTriangle(builderA, builderB, builderC, polygroupIndex);
}

//Adds the data for a point to the builder reference 
void ACPP_RuntimeTerrain::AddPointToBuilder(const double& pointKey, FVector3f& vertex, FVector2f& uv, FVector& Normals, FVector3f& Tangents, FColor Color, TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1>& BuilderRef, int32& builderID)
{
	//sanity check 
	if (!M_pointBuilderID.Contains(pointKey))
	{
		auto point = M_pointData.Find(pointKey);
		auto coord = point->point;
		vertex = FVector3f(coord);
		uv = FVector2f(vertex[0], vertex[1]); //Investigate: Add world offset
		FVector3f normals{ (float)Normals[0], (float)Normals[1], (float)Normals[2]};

		builderID = BuilderRef.AddVertex(vertex)
			.SetNormalAndTangent(normals, Tangents)
			.SetColor(Color)
			.SetTexCoord(uv);

		//let us know we added this vertex to the builder
		M_pointBuilderID.Emplace(pointKey, builderID);
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

//Assumes data in csv file is singel comma sperated line
void ACPP_RuntimeTerrain::GetPointDataFromCSV()
{
	// read .cvs files to get xyz point data to a comma seperated string
	auto filehelper = FFileHelper();
	filehelper.LoadFileToString(M_dataString, *(FPaths::ProjectConfigDir() + csvFilePath));
	FString deliminator = TEXT(",");

	int indexX  = 0.0;
	int indexY  = 0.0;
	double pointID = 0.0;

	// the CV application parses out the z values to a .csv file only for a 400 x 400 px image
	if (M_dataString.IsEmpty())
	{
		if (OnScreenDebugMessages) //sanity check 
		{
			if (GEngine)
			{
				auto error = FString(TEXT("ERROR: CSV File could not be loaded, string is empty"));
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, error);
			}
		}
		return; 
	}

	TArray<FString, FDefaultAllocator> xyz;
	M_dataString.ParseIntoArray(xyz, TEXT(","), true);
	for (int indexOfYValue = 0; indexOfYValue < xyz.Num(); indexOfYValue++)
	{
		FInternalPointData point;
		pointID = indexX + indexY;
		point.point[0] = indexX * Scale;
		point.point[1] = indexY * Scale;
		point.point[2] = (FCString::Atoi(*xyz[indexOfYValue]) * Scale);

		if (OnScreenDebugMessages)
		{
			FVector center{ point.point[0], point.point[1], point.point[2] };
			DrawDebugSphere(GetWorld(), center, 25.0f, 16, FColor::Red, true);
		}

		// Recreate the image dimensions 
		if (indexX < 399)
		{
			indexX++;
		}
		else
		{
			indexX = 0;
			indexY++;
		}

		M_pointData.Emplace(pointID, point);
		M_pointLogged.Emplace(pointID, false);
	}
}

// This function maps the Point IDs to triangles in m_triangles
void ACPP_RuntimeTerrain::MakeTriangles()
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
	int index3 = 400; 
	int index4 = 401;

	if (M_pointData.Num() != 160000)
	{
		if (GEngine)
		{
			auto error = FString(TEXT("IMPORTANT ERROR: The number of points allocated during the reading of the CSV is incorrect!"));
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, error);
		}
	}

	if (!M_pointData.IsEmpty())
	{
		for (int col = 0; col < 398; col++)
		{
			for (int row = 0; row < 398; row++)
			{
				index1 = row + col;
				if (index1 < (160000 - 400))
				{
					index2 = index1 + 1;
					index3 = (400 * row) + col;
					index4 = index3 + 1;

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

						M_triangles.Add(Triangle1);
						M_triangles.Add(Triangle2);
					}
				}
			}
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

