// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "CPP_ProceduralMeshTerrain.generated.h"

USTRUCT(BlueprintType, Category = "PointDataInternal")
struct FPointDataInternal
{
	//We need a struct to hold the point data bc of TArray required typetraits 
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Point")
	FVector3f point;
	FVector2d UVcoordinates; 
};

UCLASS()
class FANTASYWORLDV1_API ACPP_ProceduralMeshTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_ProceduralMeshTerrain();

	UPROPERTY(BlueprintReadWrite)
	UProceduralMeshComponent* M_ProceduralMesh = nullptr;

	UFUNCTION(Blueprintable)
	UProceduralMeshComponent* getProceduralMeshTerrain() { return  M_ProceduralMesh; };

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "0_CSV_POINTS_FILEPATH")
	FString csvFilePath{ TEXT("C:/Users/evali/Pictures/heightMapPoints.csv") };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Tranform) override;

	UPROPERTY()
	TArray<FString> M_fileLines{};

	UPROPERTY(BlueprintReadWrite, Category = "Point_XYZ_UV_Map")
	TMap<double, FPointDataInternal> M_pointData{};

	UPROPERTY(BlueprintReadWrite, Category = "Tangent_Array")
	TArray<FProcMeshTangent> M_Tangents{};

	UPROPERTY(BlueprintReadWrite, Category = "Normal_Array")
	TArray<FVector> M_Normals{};

	UPROPERTY(BlueprintReadWrite, Category = "Triangle_Array")
	TArray<int32> M_triangles{};

	UPROPERTY(BlueprintReadWrite, Category = "Vertex_Color_Array")
	TArray<FLinearColor> M_Colors{};

	UPROPERTY()
	TMap<double, bool> M_pointLogged{};

	UPROPERTY()
	TMap<double, uint32> M_pointBuilderID{};

	UPROPERTY(BlueprintReadWrite, Category = "CornerPointCoordinates")
	TArray<FVector3f> CornerPointCoordinates{};


	void GetPointDataFromCSV(); 
	void SetUpContainersInternal(); 
	void CreateDataPerQuad();
	void CalculateNormalAndTangent(TArray<int32> triangle, TArray<FProcMeshTangent>& Tang, TArray<FVector>& Norm);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "2_Heightmap_Resolution_XY")
	double Resolution{ 40.0 }; //pixels per row/col of heightmap

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "1_Log_Debug_Messages")
	bool LogDebugMessages = true; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "1_MeshSectionMaxID")
	int32 MeshSectionID = 0; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "4_MeshBoundingBox_ForPCG")
	FBoxSphereBounds MeshBounds{};

	UFUNCTION(BlueprintCallable)
	FBoxSphereBounds getBoundingBox() { return MeshBounds; }

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "1_Draw_Debug_Spheres")
	bool DrawDebugSpheres = true; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "1_Debug_Messages_OnScreen")
	bool OnScreenDebugMessages = true; 

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "1_ScaleBetweenPoints")
	double Scale{ 10000.0 }; //UE defaults to 100 units = 1 m 

	//Control Var, not editable in Blueprint, only indicator if logic is executing correctly internally
	UPROPERTY(BlueprintReadOnly, Category = "0_Data_State_Valid")
	bool DataStateValid = true; 
};
