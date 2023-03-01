#include "DynamicMeshActor.h"

#include "Generators/SphereGenerator.h"
#include "Generators/GridBoxMeshGenerator.h"
#include "MeshQueries.h"
#include "MeshSimplification.h"
#include "Implicit/Solidify.h"

#include "MeshDescriptionToDynamicMesh.h"

void FBooleanWithMeshAsyncTask::DoWork()
{
	SCOPE_CYCLE_COUNTER(STAT_BooleanWithMeshAsyncTask);
	MeshTransforms::ApplyTransform(OtherMesh, OtherToWorld);
	MeshTransforms::ApplyTransformInverse(OtherMesh, ActorToWorld);

	FDynamicMesh3 ResultMesh;

	FMeshBoolean::EBooleanOp ApplyOp = FMeshBoolean::EBooleanOp::Union;
	switch (BooleanOperation)
	{
	case EDynamicMeshActorBooleanOperation::Cut:
		ApplyOp = FMeshBoolean::EBooleanOp::DifferenceNoPatch;
		break;
	case EDynamicMeshActorBooleanOperation::Subtraction:
		ApplyOp = FMeshBoolean::EBooleanOp::Difference;
		break;
	case EDynamicMeshActorBooleanOperation::Intersection:
		ApplyOp = FMeshBoolean::EBooleanOp::Intersect;
		break;
	default:
		break;
	}

	FMeshBoolean Boolean(
		&Mesh, FTransform3d::Identity(),
		&OtherMesh, FTransform3d::Identity(),
		&ResultMesh,
		ApplyOp);
	Boolean.bPutResultInInputSpace = true;
	Boolean.bSimplifyAlongNewEdges = BooleanOptions.bSimplifyOutput;
	Boolean.Compute();

	if (NormalsMode == EDynamicMeshActorNormalsMode::PerVertexNormals)
	{
		ResultMesh.EnableAttributes();
		FMeshNormals::InitializeOverlayToPerVertexNormals(ResultMesh.Attributes()->PrimaryNormals(), false);
	}
	else if (NormalsMode == EDynamicMeshActorNormalsMode::FaceNormals)
	{
		ResultMesh.EnableAttributes();
		FMeshNormals::InitializeOverlayToPerTriangleNormals(ResultMesh.Attributes()->PrimaryNormals());
	}

	Mesh = MoveTemp(ResultMesh);
}

// Sets default values
ADynamicMeshActor::ADynamicMeshActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AccumulatedTime = 0;
	MeshAABBTree.SetMesh(&SourceMesh);

	FastWinding = MakeUnique<TFastWindingTree<FDynamicMesh3>>(&MeshAABBTree, false);

	//BooleanWithMeshAsyncTask = new FAsyncTask<FBooleanWithMeshAsyncTask>();
}

ADynamicMeshActor::~ADynamicMeshActor()
{

}

void ADynamicMeshActor::PostLoad()
{
	Super::PostLoad();

	OnMeshGenerationSettingsModified();
}

void ADynamicMeshActor::PostActorCreated()
{
	Super::PostActorCreated();
	OnMeshGenerationSettingsModified();
}

// Called when the game starts or when spawned
void ADynamicMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
	AccumulatedTime = 0;
	OnMeshGenerationSettingsModified();
}

// Called every frame
void ADynamicMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AccumulatedTime += DeltaTime;
	if (bRegenerateOnTick && SourceType == EDynamicMeshActorSourceType::Primitive)
	{
		OnMeshGenerationSettingsModified();
	}

	if (BooleanWithMeshAsyncTask && BooleanWithMeshAsyncTask->IsDone())
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_ResetMeshData);
		SourceMesh = BooleanWithMeshAsyncTask->GetTask().Mesh;
		MeshAABBTree.SetMesh(&SourceMesh);

		OnMeshEditedInternal();
		BooleanWithMeshAsyncTask->GetTask().CanBeReuse = true;
	}
}

#if WITH_EDITOR
void ADynamicMeshActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnMeshGenerationSettingsModified();
}
#endif


void ADynamicMeshActor::EditMesh(TFunctionRef<void(FDynamicMesh3&)> EditFunc)
{
	EditFunc(SourceMesh);

	// update spatial data structures
	if (bEnableSpatialQueries || bEnableInsideQueries)
	{
		MeshAABBTree.Build();
		if (bEnableInsideQueries)
		{
			FastWinding->Build();
		}
	}

	OnMeshEditedInternal();
}

void ADynamicMeshActor::GetMeshCopy(FDynamicMesh3& MeshOut)
{
	MeshOut = SourceMesh;
}

const FDynamicMesh3& ADynamicMeshActor::GetMeshRef() const
{
	return SourceMesh;
}

void ADynamicMeshActor::SetMesh(FDynamicMesh3& Mesh)
{
	SourceMesh = Mesh;
}

void ADynamicMeshActor::CreateDynamicMeshFromStaticMesh(const UStaticMesh* StaticMesh)
{
	if (StaticMesh)
	{
		SourceMesh.Clear();

		FMeshDescriptionToDynamicMesh Converter;
		Converter.Convert(StaticMesh->GetMeshDescription(0), SourceMesh);
	}
}

void ADynamicMeshActor::OnMeshEditedInternal()
{
	OnMeshModified.Broadcast(this);
}

void ADynamicMeshActor::OnMeshGenerationSettingsModified()
{
	EditMesh([this](FDynamicMesh3& MeshToUpdate) {
		RegenerateSourceMesh(MeshToUpdate);
	});
}

void ADynamicMeshActor::RegenerateSourceMesh(FDynamicMesh3& MeshOut)
{
	if (SourceType == EDynamicMeshActorSourceType::Primitive)
	{
		double UseRadius = (this->MinimumRadius + this->VariableRadius)
			+ (this->VariableRadius) * FMathd::Sin(PulseSpeed * AccumulatedTime);

		// generate new mesh
		if (this->PrimitiveType == EDynamicMeshActorPrimitiveType::Sphere)
		{
			FSphereGenerator SphereGen;
			SphereGen.NumPhi = SphereGen.NumTheta = FMath::Clamp(this->TessellationLevel, 3, 50);
			SphereGen.Radius = UseRadius;
			MeshOut.Copy(&SphereGen.Generate());
		}
		else
		{
			FGridBoxMeshGenerator BoxGen;
			int TessLevel = FMath::Clamp(this->TessellationLevel, 2, 50);
			BoxGen.EdgeVertices = FIndex3i(TessLevel, TessLevel, TessLevel);
			FVector3d BoxExtents = UseRadius * FVector3d::One();
			BoxExtents.Z *= BoxDepthRatio;
			BoxGen.Box = FOrientedBox3d(FVector3d::Zero(), BoxExtents);
			MeshOut.Copy(&BoxGen.Generate());
		}

	}
	else if (SourceType == EDynamicMeshActorSourceType::StaticMeshAsset)
	{
		CreateDynamicMeshFromStaticMesh(StaticMeshAssets);
	}

	RecomputeNormals(MeshOut);
}

void ADynamicMeshActor::RecomputeNormals(FDynamicMesh3& MeshOut)
{
	if (this->NormalsMode == EDynamicMeshActorNormalsMode::PerVertexNormals)
	{
		MeshOut.EnableAttributes();
		FMeshNormals::InitializeOverlayToPerVertexNormals(MeshOut.Attributes()->PrimaryNormals(), false);
	}
	else if (this->NormalsMode == EDynamicMeshActorNormalsMode::FaceNormals)
	{
		MeshOut.EnableAttributes();
		FMeshNormals::InitializeOverlayToPerTriangleNormals(MeshOut.Attributes()->PrimaryNormals());
	}
}

int ADynamicMeshActor::GetTriangleCount()
{
	return SourceMesh.TriangleCount();
}

float ADynamicMeshActor::DistanceToPoint(FVector WorldPoint, FVector& NearestWorldPoint, int& NearestTriangle, FVector& TriBaryCoords)
{
	NearestWorldPoint = WorldPoint;
	NearestTriangle = -1;
	if (bEnableSpatialQueries == false)
	{
		return TNumericLimits<float>::Max();
	}

	FTransform3d ActorToWorld(GetActorTransform());
	FVector3d LocalPoint = ActorToWorld.InverseTransformPosition((FVector3d)WorldPoint);

	double NearDistSqr;
	NearestTriangle = MeshAABBTree.FindNearestTriangle(LocalPoint, NearDistSqr);
	if (NearestTriangle < 0)
	{
		return TNumericLimits<float>::Max();
	}

	FDistPoint3Triangle3d DistQuery = TMeshQueries<FDynamicMesh3>::TriangleDistance(SourceMesh, NearestTriangle, LocalPoint);
	NearestWorldPoint = (FVector)ActorToWorld.TransformPosition(DistQuery.ClosestTrianglePoint);
	TriBaryCoords = (FVector)DistQuery.TriangleBaryCoords;
	return (float)FMathd::Sqrt(NearDistSqr);
}

FVector ADynamicMeshActor::NearestPoint(FVector WorldPoint)
{
	if (bEnableSpatialQueries)
	{
		FTransform3d ActorToWorld(GetActorTransform());
		FVector3d LocalPoint = ActorToWorld.InverseTransformPosition((FVector3d)WorldPoint);
		return (FVector)ActorToWorld.TransformPosition(MeshAABBTree.FindNearestPoint(LocalPoint));
	}
	return WorldPoint;
}

bool ADynamicMeshActor::ContainsPoint(FVector WorldPoint, float WindingThreshold)
{
	if (bEnableInsideQueries)
	{
		FTransform3d ActorToWorld(GetActorTransform());
		FVector3d LocalPoint = ActorToWorld.InverseTransformPosition((FVector3d)WorldPoint);
		return FastWinding->IsInside(LocalPoint, WindingThreshold);
	}
	return false;
}

bool ADynamicMeshActor::IntersectRay(FVector RayOrigin, FVector RayDirection, 
	FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords,
	float MaxDistance)
{
	if (bEnableSpatialQueries)
	{
		FTransform3d ActorToWorld(GetActorTransform());
		FVector3d WorldDirection(RayDirection); WorldDirection.Normalize();
		FRay3d LocalRay(ActorToWorld.InverseTransformPosition((FVector3d)RayOrigin),
			ActorToWorld.InverseTransformNormal(WorldDirection));
		IMeshSpatial::FQueryOptions QueryOptions;
		if (MaxDistance > 0)
		{
			QueryOptions.MaxDistance = MaxDistance;
		}
		NearestTriangle = MeshAABBTree.FindNearestHitTriangle(LocalRay, QueryOptions);
		if (SourceMesh.IsTriangle(NearestTriangle))
		{
			FIntrRay3Triangle3d IntrQuery = TMeshQueries<FDynamicMesh3>::TriangleIntersection(SourceMesh, NearestTriangle, LocalRay);
			if (IntrQuery.IntersectionType == EIntersectionType::Point)
			{
				HitDistance = IntrQuery.RayParameter;
				WorldHitPoint = (FVector)ActorToWorld.TransformPosition(LocalRay.PointAt(IntrQuery.RayParameter));
				TriBaryCoords = (FVector)IntrQuery.TriangleBaryCoords;
				return true;
			}
		}
	}
	return false;
}

void ADynamicMeshActor::CutMesh(ADynamicMeshActor* OtherMeshActor)
{
	FMeshBooleanOptions Options;
	BooleanWithMeshAsync(OtherMeshActor, EDynamicMeshActorBooleanOperation::Cut, Options);
}

void ADynamicMeshActor::SubtractMesh(ADynamicMeshActor* OtherMeshActor)
{
	FMeshBooleanOptions Options;
	BooleanWithMeshAsync(OtherMeshActor, EDynamicMeshActorBooleanOperation::Subtraction, Options);
}
void ADynamicMeshActor::UnionWithMesh(ADynamicMeshActor* OtherMeshActor)
{
	FMeshBooleanOptions Options;
	BooleanWithMeshAsync(OtherMeshActor, EDynamicMeshActorBooleanOperation::Union, Options);
}
void ADynamicMeshActor::IntersectWithMesh(ADynamicMeshActor* OtherMeshActor)
{
	FMeshBooleanOptions Options;
	BooleanWithMeshAsync(OtherMeshActor, EDynamicMeshActorBooleanOperation::Intersection, Options);
}

void ADynamicMeshActor::ResetMesh()
{
	RegenerateSourceMesh(SourceMesh);
}

void ADynamicMeshActor::BooleanWithMeshAsync(ADynamicMeshActor* OtherMeshActor, EDynamicMeshActorBooleanOperation Operation, FMeshBooleanOptions Options)
{
	check(IsInGameThread());
	if (ensure(OtherMeshActor) == false) return;

	QUICK_SCOPE_CYCLE_COUNTER(STAT_BooleanWithMeshAsync);

	FTransform3d ActorToWorld(GetActorTransform());
	FTransform3d OtherToWorld(OtherMeshActor->GetActorTransform());

	if (BooleanWithMeshAsyncTask)
	{
		BooleanWithMeshAsyncTask->GetTask().SetMeshData(SourceMesh, ActorToWorld, OtherMeshActor->SourceMesh, OtherToWorld, Operation, NormalsMode);
	}
	else
	{
		BooleanWithMeshAsyncTask = new FAsyncTask<FBooleanWithMeshAsyncTask>(SourceMesh, ActorToWorld, OtherMeshActor->SourceMesh, OtherToWorld, Operation, Options, NormalsMode);
	}
	
	BooleanWithMeshAsyncTask->GetTask().CanBeReuse = false;
	BooleanWithMeshAsyncTask->StartBackgroundTask();
}

void ADynamicMeshActor::CopyFromMesh(ADynamicMeshActor* OtherMesh, bool bRecomputeNormals)
{
	if (! ensure(OtherMesh) ) return;

	// the part where we generate a new mesh
	FDynamicMesh3 TmpMesh;
	OtherMesh->GetMeshCopy(TmpMesh);

	// apply our normals setting
	if (bRecomputeNormals)
	{
		RecomputeNormals(TmpMesh);
	}

	// update the mesh
	EditMesh([&](FDynamicMesh3& MeshToUpdate)
	{
		MeshToUpdate = MoveTemp(TmpMesh);
	});
}

void ADynamicMeshActor::SolidifyMesh(int VoxelResolution, float WindingThreshold)
{
	if (MeshAABBTree.IsValid() == false)
	{
		MeshAABBTree.Build();
	}
	if (FastWinding->IsBuilt() == false)
	{
		FastWinding->Build();
	}

	// ugh workaround for bug
	FDynamicMesh3 CompactMesh;
	CompactMesh.CompactCopy(SourceMesh, false, false, false, false);
	FDynamicMeshAABBTree3 AABBTree(&CompactMesh, true);
	TFastWindingTree<FDynamicMesh3> Winding(&AABBTree, true);

	double ExtendBounds = 2.0;
	//TImplicitSolidify<FDynamicMesh3> SolidifyCalc(&SourceMesh, &MeshAABBTree, FastWinding.Get());
	//SolidifyCalc.SetCellSizeAndExtendBounds(MeshAABBTree.GetBoundingBox(), ExtendBounds, VoxelResolution);
	TImplicitSolidify<FDynamicMesh3> SolidifyCalc(&CompactMesh, &AABBTree, &Winding);
	SolidifyCalc.SetCellSizeAndExtendBounds(AABBTree.GetBoundingBox(), ExtendBounds, VoxelResolution);
	SolidifyCalc.WindingThreshold = WindingThreshold;
	SolidifyCalc.SurfaceSearchSteps = 5;
	SolidifyCalc.bSolidAtBoundaries = true;
	SolidifyCalc.ExtendBounds = ExtendBounds;
	FDynamicMesh3 SolidMesh(&SolidifyCalc.Generate());

	SolidMesh.EnableAttributes();
	RecomputeNormals(SolidMesh);

	EditMesh([&](FDynamicMesh3& MeshToUpdate)
	{
		MeshToUpdate = MoveTemp(SolidMesh);
	});
}

void ADynamicMeshActor::SimplifyMeshToTriCount(int32 TargetTriangleCount)
{
	TargetTriangleCount = FMath::Max(1, TargetTriangleCount);
	if (TargetTriangleCount >= SourceMesh.TriangleCount()) return;

	// make compacted copy because it seems to change the results?
	FDynamicMesh3 SimplifyMesh;
	SimplifyMesh.CompactCopy(SourceMesh, false, false, false, false);
	SimplifyMesh.EnableTriangleGroups();			// workaround for failing check()
	FQEMSimplification Simplifier(&SimplifyMesh);
	Simplifier.SimplifyToTriangleCount(TargetTriangleCount);
	SimplifyMesh.EnableAttributes();
	RecomputeNormals(SimplifyMesh);

	EditMesh([&](FDynamicMesh3& MeshToUpdate)
	{
		MeshToUpdate.CompactCopy(SimplifyMesh);
	});
}
