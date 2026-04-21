// New class created because lack of time and documentation made using the Procedural Mesh Component a better choice. 
// The mesh generation logic mirrors the first draft using the Realtime Mesh Component
// The code for this was tested with 3 example heightmap .csv files and successfully sampled by a graph using the WorldRayHitQuery node and a surface sampler. 

#include "CPP_ProceduralMeshTerrain.h"

#include "KismetProceduralMeshLibrary.h"

ACPP_ProceduralMeshTerrain::ACPP_ProceduralMeshTerrain()
{
	PrimaryActorTick.bCanEverTick = false;

	M_ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GenerateMesh"));
	M_ProceduralMesh->bUseComplexAsSimpleCollision = true;
	M_ProceduralMesh->ContainsPhysicsTriMeshData(true); 
	RootComponent = M_ProceduralMesh;
}

void ACPP_ProceduralMeshTerrain::BeginPlay()
{
	Super::BeginPlay();
}

void ACPP_ProceduralMeshTerrain::OnConstruction(const FTransform& Tranform)
{
	DrawDebugSpheres = false;
	UE_LOG(LogTemp, Display, TEXT("[PMC] Begin Play called on procedural mesh actor"));

	// Get the point data from csv and 
	// create vertices, triangles, UVs, tangents and normals for PMC
	SetUpContainersInternal();
	GetPointDataFromCSV();
	if (DataStateValid) // Not valid if CSV file violates format
	{
		CreateDataPerQuad();
		TArray<FVector> vertices{};
		TArray<FVector2D> UVs{};

		for (const auto& [id, data] : M_pointData)
		{
			UE_LOG(LogTemp, Display, TEXT("[PMC] [Point Data epmplaced] ID %f"), id);
			vertices.Add(FVector(data.point[0], data.point[1], data.point[2]));
			UVs.Add(FVector2D(data.point[0], data.point[1]));
		}

		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(vertices, M_triangles, UVs, M_Normals, M_Tangents);
		if (M_ProceduralMesh)
		{
			M_ProceduralMesh->CreateMeshSection_LinearColor(MeshSectionID, vertices, M_triangles, M_Normals, UVs, M_Colors, M_Tangents, true); MeshSectionID++;
			M_ProceduralMesh->UpdateBounds();
			MeshBounds = M_ProceduralMesh->Bounds;
		}
		else { UE_LOG(LogTemp, Error, TEXT("[PMC] Data was read right but Procedural Mesh Component in Actor was null-pointer")); return; }
	}
	else return;

	Super::OnConstruction(Tranform); 
}

void ACPP_ProceduralMeshTerrain::GetPointDataFromCSV()
{
	// read .cvs files to get xyz point data to a comma seperated string
	// we are assuming that each line in the file corresponds to the Z values in the XYZ coordinates 
	// so a resolution 40 heightmap should result in 40 lines with 40 characters for example 
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
		UE_LOG(LogTemp, Display, TEXT("[PMC] [RowData] Row %d with %d points"), (lineIndex), size);

		if (size != Resolution) //Reset resolution to length of first line if it doesn't fit, or break once we know lines are different sizes
		{
			if (lineIndex == 0) { UE_LOG(LogTemp, Warning, TEXT("[PMC] [RowData] [ID 0] Expected Resolution of heightmap data %f x %f is not give. First line has %d entries"), Resolution, Resolution, size); Resolution = size; }
			else { UE_LOG(LogTemp, Error, TEXT("[PMC] [CSV File Format Error] Every line has different number of values. We can not process heightmap. Please inspect output csv.")); return; };
		}

		for (int indexOfYValue = 0; indexOfYValue < xyz.Num(); indexOfYValue++)
		{
			int ZValue = (FCString::Atoi(*xyz[indexOfYValue]));

			if (LogDebugMessages)
			{
				auto input = *xyz[indexOfYValue];

				UE_LOG(LogTemp, Display, TEXT("[CV Point Char] Read as: %s"), input);
				UE_LOG(LogTemp, Display, TEXT("[CV Point int] Converted to: %d"), ZValue);
			}

			FPointDataInternal point;
			pointID = (lineIndex * Resolution) + (indexOfYValue);
			point.point[0] = (indexOfYValue)*Scale;
			point.point[1] = (lineIndex)*Scale;
			point.point[2] = (ZValue) * (Scale / 100);
			point.UVcoordinates[0] = (indexOfYValue)*Scale;
			point.UVcoordinates[1] = (lineIndex)*Scale;

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
			auto isCorner = isFirstPoint || isFirstLineLast || isLastLineFirst || isLastLast; 

			if (isCorner)
			{
				CornerPointCoordinates.Add(point.point);
				if (DrawDebugSpheres) { FVector pt{ point.point[0], point.point[1], point.point[2] }; DrawDebugSphere(GetWorld(), pt, 15, 32, FColor::Purple, true); }
			}

			if (DrawDebugSpheres && !isCorner)
			{
				FVector draw{ point.point[0], point.point[1], point.point[2] };
				DrawDebugSphere(GetWorld(), draw, 10, 16, FColor::Blue, true);
			}

			M_pointLogged.Emplace(pointID, false);
			M_pointData.Emplace(pointID, point); 
			M_Colors.Add(FLinearColor::White);
		}
		lineIndex++;
	}

	auto lineArraySize = M_fileLines.Num();
	auto pointDataSize = M_pointData.Num();
	UE_LOG(LogTemp, Display, TEXT("[PMC] [Lines Read] %d"), lineIndex);
	UE_LOG(LogTemp, Display, TEXT("[PMC] [Input string array size] %d"), lineArraySize);
	UE_LOG(LogTemp, Display, TEXT("[PMC] [Point Num] Total point after CSV complleted reading: %d"), pointDataSize);
	ensureMsgf(pointDataSize == (Resolution * Resolution), TEXT("Unexpected size for xyz coordinate vector, size was %d"), pointDataSize);
}

// Resets and reserves expected memory capacity for data 
void ACPP_ProceduralMeshTerrain::SetUpContainersInternal()
{
	auto vertices = Resolution * Resolution;
	M_fileLines.Reset();
	M_pointData.Reset();
	M_pointBuilderID.Reset();
	M_pointLogged.Reset();
	M_triangles.Reset();
	M_Normals.Reset(); 
	M_Tangents.Reset(); 
	M_Colors.Reset(); 

	M_fileLines.Reserve(vertices + 1);
	M_pointData.Reserve(vertices);
	M_pointBuilderID.Reserve(vertices);
	M_pointLogged.Reserve(vertices);
	M_triangles.Reserve(vertices * 4);
	M_Tangents.Reserve(vertices * 4);
	M_Normals.Reserve(vertices * 4); 
	M_Colors.Reserve(vertices); 
}

// Function calculates all data needed for a Quad in the Mesh: 4 vertexes create 2 triangles, with normals and tangents added to the corresponding vectors
void ACPP_ProceduralMeshTerrain::CreateDataPerQuad()
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
				index1 = (col)+(row * Resolution);
				index2 = index1 + 1;
				index3 = index1 + Resolution;
				index4 = index3 + 1;

				if (index1 < ((Resolution -1) + (Resolution * row)))
				{
					if (M_pointData.Contains(index1) && M_pointData.Contains(index2) && M_pointData.Contains(index3) && M_pointData.Contains(index4))
					{
						// Triangles for meshes have to be created counter clockwise due to UEs oriantation
						// We split the above mentioned quad into two by edge betweem vertices [index 3 - index 2]
						TArray<int32> Triangle1{};
						TArray<int32> Triangle2{};
						TArray<int32> both{};
						TArray<FProcMeshTangent> Tang{};
						TArray<FVector> Normals{};

						/*
						TArray<FVector> vertices{};
						TArray<FVector2D> UVs{};
						//TArray<FLinearColor> Colors{ FLinearColor::Red, FLinearColor::Blue, FLinearColor::Green, FLinearColor::Yellow };
						TArray<FLinearColor> Colors{ FLinearColor::White, FLinearColor::White, FLinearColor::White, FLinearColor::White };
						*/

						// Two triangles create 1 Quad
						Triangle1.Add(index3);
						Triangle1.Add(index2);
						Triangle1.Add(index1);

						Triangle2.Add(index3);
						Triangle2.Add(index4);
						Triangle2.Add(index2);

						for (const auto ptKey : Triangle1) M_triangles.Add(ptKey); 

						for (const auto ptKey2 : Triangle2) M_triangles.Add(ptKey2);

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

								DrawDebugSphere(GetWorld(), pt3V, 10, 16, FColor::Red, true);
								DrawDebugSphere(GetWorld(), pt2V, 10, 16, FColor::Red, true);
								DrawDebugSphere(GetWorld(), pt1V, 10, 16, FColor::Red, true);
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

	if (OnScreenDebugMessages)
	{
		if (GEngine)
		{
			auto message = FString(TEXT("[Base Mesh] Created [Triangesls | Points] : [ "));
			auto triangleCount = M_triangles.Num();
			auto pointCount = M_pointData.Num();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Purple, message);

			message = FString(TEXT("[Base Mesh] Memory for structs in bytes [Triangesls | Points] : [ "));
			triangleCount = M_triangles.GetAllocatedSize();
			pointCount = M_pointData.GetAllocatedSize();
			message += FString::FromInt(triangleCount);
			message += TEXT(" | ");
			message += FString::FromInt(pointCount);
			message += TEXT(" ]");
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, message);
		}
	}
}

void ACPP_ProceduralMeshTerrain::CalculateNormalAndTangent(TArray<int32> triangle, TArray<FProcMeshTangent>& Tang, TArray<FVector>& Norm)
{
	// Get normals and tangents - https://forums.unrealengine.com/t/creating-the-tangentx-tangenty-tangentz-from-vertex-and-normal/308424/4
	auto pointA = M_pointData.Find(triangle[0]);
	auto pointB = M_pointData.Find(triangle[1]);
	auto pointC = M_pointData.Find(triangle[2]);

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

	Tang.Add(FProcMeshTangent(Tangents[0], Tangents[1], Tangents[2]));
	Norm.Add(NormalVector);
	M_Normals.Add(NormalVector);
	M_Tangents.Add(FProcMeshTangent(Tangents[0], Tangents[1], Tangents[2]));
}



