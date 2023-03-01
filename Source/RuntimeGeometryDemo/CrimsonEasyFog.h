#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrimsonEasyFog.generated.h"

UCLASS()
class ACrimsonEasyFog : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY()
		UStaticMeshComponent* FogCardComponent = nullptr;

	UMaterialInstanceDynamic* EasyFogMaterialDynamic = nullptr;

public:
	ACrimsonEasyFog();

	/** Base Color Texture Map */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		UTexture2D* BaseColorMap;

	/** Opacity Map - Map determines the overall shape of EasyFog. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		UTexture2D* OpacityMap;
	/** Normal Maps will help give extra volume to the shape of the fog. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		UTexture2D* NormalMap;

	/** Add a tint to the color of the fog. Helps integration of the fog into the environment. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		FLinearColor BaseColorTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	/** Increase or reduce the contrast of the Base Color Texture. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float BaseColorConstrast = -0.3f;

	/** Adjusts the brightness of the Base Color */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float BaseColorIntensity = 1.0f;

	/** Adds an emissive glow control to the fog. Helps with integrating it with your environment. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float EmissiveIntensity = 0.5f;

	/** Adjusts the intensity of the normal map. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float NormalMapIntensity = 0.0f;

	/** Controls the overally density/opacity of EasyFog. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float FogDensity = 1.0f;

	/** Helps EasyFog blend into surrounding geometry. Higher values increase fade distance. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float GeometryFadingDistance = 1000.0f;

	/** Helps the camera prevent clipping/popping when moving through fog cards. Higher values smooth out the transition. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float CameraFadingDistance = 1000.0f;

	/** A fresnel-based masking which makes EasyFog fade at glancing angles. This can help make EasyFog feel less plane-like. */
	UPROPERTY(EditAnywhere, Category = "EasyFog Settings")
		float ViewAngleFade = 0.0f;

	/** Add wind to fog, or not */
	UPROPERTY(EditAnywhere, Category = "Wind Controls")
		float Wind = 0.0f;

	/** Speed of fog along the Y axis */
	UPROPERTY(EditAnywhere, Category = "Wind Controls")
		float WindSpeedY = 200.0f;

	/** Speed of the fog along the X axis */
	UPROPERTY(EditAnywhere, Category = "Wind Controls")
		float WindSpeedX = 200.0f;

	/** Adjusts the tiling of the wind's noise pattern. */
	UPROPERTY(EditAnywhere, Category = "Wind Controls")
		float WindNoiseTiling = 0.02f;

	/** Adjusts the contrast of the wind's noise pattern. */
	UPROPERTY(EditAnywhere, Category = "Wind Controls")
		float WindNoiseConstrast = 0.0f;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostLoad() override;

	void SetMaterialParamerters();
};