// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealtimeMeshActor.h"
#include "RealtimeMeshSimple.h"

#include "ACPP_RuntimeTerrain.generated.h"

class FRealtimeMeshSectionSimple;

USTRUCT(BlueprintType, Category="InternalPointData")
struct FInternalPointData
{
	//We need a struct to hold the point data bc of TArray required typetraits 
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Point")
	FVector3f point; 
};

USTRUCT(BlueprintType, Category="InternalTriangleData")
struct FInternalTriangleData
{
	//Struct to hold triangles for TArray
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Triangle")
	FVector3f triangleVertexIDs;
};

UCLASS(Blueprintable)
class FANTASYWORLDPCG_API ACPP_RuntimeTerrain : public ARealtimeMeshActor
{
	GENERATED_BODY()
	
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

public:
	using MeshBuilder = RealtimeMesh::TRealtimeMeshBuilderLocal<uint32, FPackedNormal, FVector2DHalf, 1, uint32>; 

	ACPP_RuntimeTerrain();

	UFUNCTION(BlueprintCallable)
	virtual void GetPointDataFromCSV();

	//UFUNCTION(BlueprintCallable)
	//virtual void GetPointDataFromCSVParseArray();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="CSV_POINTS_FILEPATH")
	FString csvFilePath{ TEXT("C:/Users/evali/Pictures/heightMapPoints.csv") };

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "ScaleBetweenPoints")
	double Scale{1.0}; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Heightmap_Resolution_XY")
	double Resolution{ 40.0 };

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Show_DEBUG_MESSAGES")
	bool OnScreenDebugMessages = true; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Log_DEBUG_MESSAGES")
	bool LogDebugMessages = true;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "DebugSpheres")
	bool DebugSpheres = true;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "PointData");
	TMap<double, FInternalPointData> M_pointData{};

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Tranform) override;


	// Function constructing the simple runtime mesh data with Builder helper using FInternalTriangleData
    void AddDataToMeshBuilder(
		MeshBuilder&                 BuilderRef, 
		const FInternalTriangleData& triangleWithPointIDs,
		uint32                       polygroupIndex
	);

	void AddPointToBuilder(
		const double& pointKey, 
		FVector3f&    vertex, 
		FVector2f&    uv, 
		FVector&      Normals, 
		FVector3f&    Tangents, 
		FColor        Color,
		MeshBuilder&  BuilderRef, 
		uint32&       builderID
	);

	//virtual void GetPointDataFromCsvParseArray();
	virtual void MakeTriangles();
	virtual void PopulateRealtimeMeshSection(); 

	//---------------------------------------------
	//The data dor the csv file will not be visible in a blueprint, as it only gets populated in onBeginPlay()
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "CSVFileLines");
    TArray<FString> M_fileLines{};

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "DataAsString");
	FString M_dataString{};

	//---------------------------------------------

	UPROPERTY();
	TMap<double, bool> M_pointLogged{};

	UPROPERTY();
	TMap<double, uint32> M_pointBuilderID{};

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "TrinangleVertexis");
	TArray<FInternalTriangleData> M_triangles{};

public:	
	virtual void Tick(float DeltaTime) override;
};
