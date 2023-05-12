#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleDynamicMeshComponent.h"
#include "SimpleDynamicMeshActor.h"
#include "MeshBooleanSlicerActor.generated.h"

UENUM()
enum class ESlicerType : uint8
{
	ST_OnceSlice UMETA(DisplayName = "Once Slice"),
	ST_AlwaySlice UMETA(DisplayName = "Always Slice"),
};

UCLASS()
class RUNTIMEGEOMETRYUTILS_API AMeshBooleanSlicerActor : public ASimpleDynamicMeshActor
{
	GENERATED_BODY()

public:
	AMeshBooleanSlicerActor();

	/** Slicer type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DynamicMeshOptions, meta = (DisplayName = "Slicer Type"))
	ESlicerType SlicerType = ESlicerType::ST_OnceSlice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DynamicMeshOptions, meta = (DisplayName = "Boolean Operation"))
	EDynamicMeshActorBooleanOperation BooleanOperation = EDynamicMeshActorBooleanOperation::Cut;

	virtual EDynamicMeshActorBooleanOperation GetBooleanOperation() const override
	{
		return BooleanOperation;
	}

	virtual bool IsChangeMaintainMesh() const override;

};
