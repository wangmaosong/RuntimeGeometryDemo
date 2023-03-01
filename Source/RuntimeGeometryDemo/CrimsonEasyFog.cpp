#include "CrimsonEasyFog.h"

static const FString EasyFogMaterialName = "MID_EasyFog";

ACrimsonEasyFog::ACrimsonEasyFog()
{
	SetReplicates(false);
	bSkipLoadOnServer = true;

	// construct plane mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh>PlaneMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	FogCardComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	FogCardComponent->SetupAttachment(GetRootComponent(), FName("PlaneMesh"));
	FogCardComponent->SetStaticMesh(PlaneMeshAsset.Object);
	FogCardComponent->CastShadow = false;
	FogCardComponent->ReceiveShadow = false;
	FogCardComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	RootComponent = FogCardComponent;

#if WITH_EDITOR
	// Try set default material, in project material directory
	UMaterialInstance* EasyFogMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/Game/Materials/MaterialInstance/MI_EasyFog.MI_EasyFog"));
	if (EasyFogMaterial)
	{
		FogCardComponent->SetMaterial(0, EasyFogMaterial);
	}
#endif
}

#if WITH_EDITOR
void ACrimsonEasyFog::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (EasyFogMaterialDynamic == nullptr)
	{
		EasyFogMaterialDynamic = FogCardComponent->CreateDynamicMaterialInstance(0, nullptr, *EasyFogMaterialName);
	}

	SetMaterialParamerters();
}
#endif

void ACrimsonEasyFog::PostLoad()
{
	Super::PostLoad();

	if (FogCardComponent)
	{
		EasyFogMaterialDynamic = FogCardComponent->CreateDynamicMaterialInstance(0, nullptr, *EasyFogMaterialName);
	}

	SetMaterialParamerters();
}

void ACrimsonEasyFog::SetMaterialParamerters()
{
	if (EasyFogMaterialDynamic)
	{
		if (BaseColorMap)
		{
			EasyFogMaterialDynamic->SetTextureParameterValue(FName("Base Color Map"), BaseColorMap);
		}
		if (OpacityMap)
		{
			EasyFogMaterialDynamic->SetTextureParameterValue(FName("Opacity Map"), OpacityMap);
		}
		if (NormalMap)
		{
			EasyFogMaterialDynamic->SetTextureParameterValue(FName("Normal Map"), NormalMap);
		}

		EasyFogMaterialDynamic->SetVectorParameterValue(FName("Base Color Tint"), BaseColorTint);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Base Color Contrast"), BaseColorConstrast);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Base Color Intensity"), BaseColorIntensity);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Fading Distance"), GeometryFadingDistance);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Camera Fading Distance"), CameraFadingDistance);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Fog Density"), FogDensity);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Normal Intensity"), NormalMapIntensity);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Emissive Intensity"), EmissiveIntensity);

		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Wind"), Wind);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Wind Speed Y"), WindSpeedY);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Wind Speed X"), WindSpeedX);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Wind Noise Tiling"), WindNoiseTiling);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("Wind Noise Contrast"), WindNoiseConstrast);
		EasyFogMaterialDynamic->SetScalarParameterValue(FName("View Angle Fade"), ViewAngleFade);
	}
}