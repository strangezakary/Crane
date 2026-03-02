#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CGManifest.generated.h"

UCLASS()
class UCrateManifest : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FCGItem> Items;
};