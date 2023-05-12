#include "MeshBooleanSlicerActor.h"

AMeshBooleanSlicerActor::AMeshBooleanSlicerActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AMeshBooleanSlicerActor::IsChangeMaintainMesh() const
{
	return SlicerType == ESlicerType::ST_OnceSlice;
}