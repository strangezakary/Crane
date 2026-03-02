// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "CGHUD.generated.h"

class UUserWidget;

UCLASS()
class CRANE_API ACGHUD : public AHUD
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

private:
	UUserWidget* HotbarWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HotbarWidgetClass;
};
