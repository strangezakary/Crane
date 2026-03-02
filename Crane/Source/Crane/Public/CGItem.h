#pragma once

#include "CoreMinimal.h"
#include "CGItem.generated.h"

USTRUCT()
struct FCGItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere)
	FString Description;

	UPROPERTY(EditAnywhere)
	float Weight;

	UPROPERTY(EditAnywhere)
	FSoftObjectPath Asset;
};