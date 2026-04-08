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
	FVector point; 
};

USTRUCT(BlueprintType, Category="InternalTriangleData")
struct FInternalTriangleData
{
	//Struct to hold triangles for TArray
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Triangle")
	FVector triangleVertexIDs;
};

UCLASS(Blueprintable)
class FANTASYWORLDPCG_API ACPP_RuntimeTerrain : public ARealtimeMeshActor
{
	GENERATED_BODY()
	
	/*
	* This class loads a csv file consisting of xyz point data and then creates a RealtimeMeshComponent from said data 
	* The Realtime Mesh Component is an open source plugin developed by TriAxis Games and available at https://github.com/TriAxis-Games/RealtimeMeshComponent/tree/master
	* -> Liscended under "Runtime Mesh Component Community MIT License" - https://github.com/TriAxis-Games/RealtimeMeshComponent/blob/master/LICENSE.txt
	* 
	* Sources:
	* * runtime file loading - https://forums.unrealengine.com/t/ffilehelper-loadfiletostring-does-not-work-on-packaged-project/468567/2
	* * loading csv files with structs -  https://forums.unrealengine.com/t/idiots-guide-to-importing-csv-files/298580/6 
	* * how runtime meshes are set up: video guide on generating procedural mesh in c++ by fettis GameDev - https://www.youtube.com/playlist?list=PLyL5ZNukfVqskz_OkMdrLamiYg1sITyic
	* * computing tangents and normals - https://forums.unrealengine.com/t/recompute-tangents-and-normal-maps-i-have-found-a-solution/132852
	* * example of RealTimeMesh single  triangle - https://github.com/TriAxis-Games/RealtimeMeshComponent/blob/master/Source/RealtimeMeshExamples/Private/RealtimeMeshBasic.cpp
	*/

public:
	ACPP_RuntimeTerrain();

	UFUNCTION(BlueprintCallable)
	virtual void GetPointDataFromCSV();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="CSV_POINTS_FILEPATH")
	FString csvFilePath{ TEXT("")};

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Show_DEBUG_MESSAGES")
	bool OnScreenDebugMessages = false; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Log_DEBUG_MESSAGES")
	bool LogDebugMessages = true;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "PointData");
	TMap<double, FInternalPointData> M_pointData{};

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Tranform) override; 

	// Function constructing the simple runtime mesh data with Builder helper using FInternalTriangleData
    void AddDataToMeshBuilder(
		RealtimeMesh::TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1>&  BuilderRef, 
		const FInternalTriangleData&                                                       triangleWithPointIDs,
		int8                                                                               polygroupIndex
	);

	void AddPointToBuilder(
		const double& pointKey, 
		FVector3f&    vertex, 
		FVector2f&    uv, 
		FVector&      Normals, 
		FVector3f&    Tangents, 
		FColor        Color,
		RealtimeMesh::TRealtimeMeshBuilderLocal<uint16, FPackedNormal, FVector2DHalf, 1>& BuilderRef, 
		int32&        builderID
	);

	virtual void GetPointDataFromCsvParseArray();
	virtual void MakeTriangles();
	virtual void PopulateRealtimeMeshSection(); 

	//---------------------------------------------
	//The data dor the csv file will not be visible in a blueprint, as it only gets populated in onBeginPlay()
	// UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "CSVFileLines");
    // TArray<FString> m_fileLines{};

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "DataAsString");
	FString M_dataString{};

	//---------------------------------------------

	UPROPERTY();
	TMap<double, bool> M_pointLogged{};

	UPROPERTY();
	TMap<double, int32> M_pointBuilderID{};

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "TrinangleVertexis");
	TArray<FInternalTriangleData> M_triangles{};

public:	
	virtual void Tick(float DeltaTime) override;
};
