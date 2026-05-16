#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealtimeMeshActor.h"
#include "RealtimeMeshSimple.h"
#include "Core/RealtimeMeshBuilder.h"

#include "CPP_RuntimeTerrain.generated.h"

class FRealtimeMeshSectionSimple;

USTRUCT(BlueprintType, Category = "InternalPointData")
struct FInternalPointData
{
	//We need a struct to hold the point data bc of TArray required typetraits 
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Point")
	FVector3f point;
};

USTRUCT(BlueprintType, Category = "InternalTriangleData")
struct FInternalTriangleData
{
	//Struct to hold triangles for TArray
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triangle")
	FVector3f triangleVertexIDs;
};

//using MeshBuilder = RealtimeMesh::TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>;

/*
* This class loads a csv file consisting of xyz point data and then creates a RealtimeMeshComponent from said data
* The Realtime Mesh Component is an open source plugin developed by TriAxis Games and available at https://github.com/TriAxis-Games/RealtimeMeshComponent/tree/master
* -> Liscended under "Runtime Mesh Component Community MIT License" - https://github.com/TriAxis-Games/RealtimeMeshComponent/blob/master/LICENSE.txt
* -> Runtime Mesh Component Documentation - https://rmc.triaxis.games/keyconcepts/meshes/
*
* Sources:
* * runtime file loading - https://forums.unrealengine.com/t/ffilehelper-loadfiletostring-does-not-work-on-packaged-project/468567/2
* * loading csv files with structs -  https://forums.unrealengine.com/t/idiots-guide-to-importing-csv-files/298580/6
* * how runtime meshes are set up: video guide on generating procedural mesh in c++ by fettis GameDev - https://www.youtube.com/playlist?list=PLyL5ZNukfVqskz_OkMdrLamiYg1sITyic
* * computing tangents and normals - https://forums.unrealengine.com/t/recompute-tangents-and-normal-maps-i-have-found-a-solution/132852
* * example of RealTimeMesh single triangle - https://github.com/TriAxis-Games/RealtimeMeshComponent/blob/master/Source/RealtimeMeshExamples/Private/RealtimeMeshBasic.cpp
* * documentation on assertions - https://dev.epicgames.com/documentation/unreal-engine/asserts-in-unreal-engine
*/
UCLASS(Blueprintable)
class FANTASYWORLDV1_API ACPP_RuntimeTerrain : public ARealtimeMeshActor
{
	GENERATED_BODY()

public:

	ACPP_RuntimeTerrain();

	UFUNCTION(BlueprintCallable)
	void GetPointDataFromCSV();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "0_CSV_POINTS_FILEPATH")
	FString csvFilePath{ TEXT("C:/Users/evali/Pictures/heightMapPoints.csv") };

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "1_ScaleBetweenPoints")
	double Scale{ 10000.0 }; //UE defaults to 100 units = 1 m 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "2_Heightmap_Resolution_XY")
	double Resolution{ 40.0 };

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "3_Show_Debug_Messages")
	bool OnScreenDebugMessages = false;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "3_Log_Debug_Messages")
	bool LogDebugMessages = true;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "3_DebugSpheres")
	bool DrawDebugSpheres = false;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "1_GenerateLODs")
	bool GenerateLODs = true;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "4_MeshBoundingBox_ForPCG")
	FBoxSphereBounds MeshBounds{};

	UFUNCTION(BlueprintCallable)
	FBoxSphereBounds getBoundingBox() { return MeshBounds; }

	UFUNCTION(BlueprintCallable)
	TArray<FVector3f> getAllPointsBaseMesh() 
	{
		TArray<FVector3f> pointdata{};
		pointdata.Reserve(M_pointData.Num()); 
		for (const auto [_,pt] : M_pointData)
		{
			pointdata.Add(pt.point); 
		}
		return pointdata; 
	}

	UPROPERTY(BlueprintReadWrite, Category = "OuterPoints")
	TArray<FVector3f> OuterPoints{};

	UFUNCTION(BlueprintCallable)
	TArray<FVector3f> getOuterPoints()
	{
		return OuterPoints;
	}

	UPROPERTY(BlueprintReadWrite, Category = "TransformCenter")
	FVector TransformCenter{0.0,0.0,0.0};

	UFUNCTION(BlueprintCallable)
	FVector GetTerrainCenterLocation() { return TransformCenter; }

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Tranform) override;

	UFUNCTION()
	void CreateMeshForLODlevel(
		int32                LODlevel, 
		URealtimeMeshSimple* RealtimeMesh, 
		FColor               color);


	// Function constructing the simple runtime mesh data with Builder helper using FInternalTriangleData
	void AddDataToMeshBuilder(
		RealtimeMesh::TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>& BuilderRef,
		const FInternalTriangleData& triangleWithPointIDs,
		uint32                       polygroupIndex,
		const FColor                 color
	);

	//void CreateNormalsAndTangentsLODs(const FInternalTriangleData& triangleWithPointIDs, int32 LODlevelFactor, RealtimeMesh::TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>& BuilderRef);

	void AddPointToBuilder(
		const double& pointKey,
		FVector3f& vertex,
		FVector2f& uv,
		FVector& Normals,
		FVector3f& Tangents,
		FColor        Color,
		RealtimeMesh::TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>& BuilderRef,
		uint32& builderID
	);

	UFUNCTION()
	virtual void MakeTriangles(const FColor color);

	UFUNCTION()
	void MakeTrianglesForLODs(const int32 LODlevelFactor, const TMap<double, FInternalPointData>& points, const FColor color);

	UFUNCTION()
	virtual void PopulateRealtimeMeshSection();

	//---------------------------------------------
	//The data dor the csv file will not be visible in a blueprint, as it only gets populated in onBeginPlay()
	// UPROPERTY macro ensures the garbage collector deos not destroy the values before we generated the mesh

	UPROPERTY()
	TArray<FString> M_fileLines{};

	UPROPERTY()
	TMap<double, FInternalPointData> M_pointData{};

	UPROPERTY()
	TMap<double, FInternalPointData> M_pointsLOD2{};

	UPROPERTY()
	TMap<double, FInternalPointData> M_pointsLOD3{};

	UPROPERTY()
	TMap<double, FInternalPointData> M_pointsLOD4{};

	UPROPERTY()
	TMap<double, bool> M_pointLogged{};

	UPROPERTY()
	TMap<double, uint32> M_pointBuilderID{};

	UPROPERTY()
	TArray<FInternalTriangleData> M_trianglesLOD0{};

	UPROPERTY()
	TArray<FInternalTriangleData> M_trianglesLOD1{};

	UPROPERTY()
	TArray<FInternalTriangleData> M_trianglesLOD2{};

	UPROPERTY()
	TArray<FInternalTriangleData> M_trianglesLOD3{};

	//---------------------------------------------
public:
	virtual void Tick(float DeltaTime) override;
};
