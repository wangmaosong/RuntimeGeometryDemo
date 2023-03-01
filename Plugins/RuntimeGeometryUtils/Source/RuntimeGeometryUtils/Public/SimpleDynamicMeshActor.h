#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleDynamicMeshComponent.h"
#include "DynamicMeshActor.h"
#include "SimpleDynamicMeshActor.generated.h"

class FDynamicMesh3;

UCLASS()
class RUNTIMEGEOMETRYUTILS_API ASimpleDynamicMeshActor : public ADynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASimpleDynamicMeshActor();

public:
	UPROPERTY(VisibleAnywhere)
	USimpleDynamicMeshComponent* MeshComponent = nullptr;

	FDynamicMesh3 OriginMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ResetMeshData() override;

protected:
	/**
	 * ADynamicBaseActor API
	 */
	virtual void OnMeshEditedInternal() override;

protected:
	virtual void UpdateDynamicMesh();
};