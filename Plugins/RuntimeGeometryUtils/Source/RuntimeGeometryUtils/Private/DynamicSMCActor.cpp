#include "DynamicSMCActor.h"
#include "MeshComponentRuntimeUtils.h"

// Sets default values
ADynamicSMCActor::ADynamicSMCActor()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"), false);
	SetRootComponent(MeshComponent);
	StaticMesh = nullptr;
}

// Called when the game starts or when spawned
void ADynamicSMCActor::BeginPlay()
{
	StaticMesh = nullptr;
	Super::BeginPlay();
}

void ADynamicSMCActor::PostLoad()
{
	StaticMesh = nullptr;
	Super::PostLoad();
}

void ADynamicSMCActor::PostActorCreated()
{
	StaticMesh = nullptr;
	Super::PostActorCreated();
}

// Called every frame
void ADynamicSMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ADynamicSMCActor::OnMeshEditedInternal()
{
	UpdateSMCMesh();
	Super::OnMeshEditedInternal();
}

DECLARE_CYCLE_STAT(TEXT("UpdateSMCMesh"), STAT_UpdateSMCMesh, STATGROUP_Game);

void ADynamicSMCActor::UpdateSMCMesh()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateSMCMesh);
	if (StaticMesh == nullptr)
	{
		StaticMesh = NewObject<UStaticMesh>();
		MeshComponent->SetStaticMesh(StaticMesh);
		// add one material slot
		StaticMesh->StaticMaterials.Add(FStaticMaterial());
		StaticMesh->StaticMaterials[0].UVChannelData.bInitialized = true;
	}

	if (MeshComponent)
	{
		RTGUtils::UpdateStaticMeshFromDynamicMesh(StaticMesh, &SourceMesh);

		// update material on new section
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);
	}
}