#include "SimpleDynamicMeshActor.h"
#include "DynamicMesh3.h"

DECLARE_CYCLE_STAT(TEXT("UpdateSDMCMesh"), STAT_UpdateDynamicMesh, STATGROUP_Game);

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

void ASimpleDynamicMeshActor::OnMeshEditedInternal()
{
	UpdateDynamicMesh();
	Super::OnMeshEditedInternal();
}

void ASimpleDynamicMeshActor::UpdateDynamicMesh()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDynamicMesh);

	if (MeshComponent)
	{
		*(MeshComponent->GetMesh()) = SourceMesh;
		MeshComponent->NotifyMeshUpdated();

		// update material on new section
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);
	}
}