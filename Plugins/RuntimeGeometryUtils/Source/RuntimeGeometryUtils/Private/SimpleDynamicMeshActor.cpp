#include "SimpleDynamicMeshActor.h"
#include "DynamicMesh3.h"


// Sets default values
ASimpleDynamicMeshActor::ASimpleDynamicMeshActor()
{
	MeshComponent = CreateDefaultSubobject<USimpleDynamicMeshComponent>(TEXT("MeshComponent"), false);
	SetRootComponent(MeshComponent);

	OriginMesh = *MeshComponent->GetMesh();
}

// Called when the game starts or when spawned
void ASimpleDynamicMeshActor::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void ASimpleDynamicMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASimpleDynamicMeshActor::ResetMeshData()
{
	SetMesh(OriginMesh);

	UpdateSDMCMesh();
	Super::OnMeshEditedInternal();
}



void ASimpleDynamicMeshActor::OnMeshEditedInternal()
{
	UpdateSDMCMesh();
	Super::OnMeshEditedInternal();
}

DECLARE_CYCLE_STAT(TEXT("UpdateSDMCMesh"), STAT_UpdateSDMCMesh, STATGROUP_Game);

void ASimpleDynamicMeshActor::UpdateSDMCMesh()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateSDMCMesh);

	if (MeshComponent)
	{
		*(MeshComponent->GetMesh()) = SourceMesh;
		MeshComponent->NotifyMeshUpdated();

		// update material on new section
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);
	}
}