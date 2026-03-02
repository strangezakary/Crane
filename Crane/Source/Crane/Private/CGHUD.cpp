// Fill out your copyright notice in the Description page of Project Settings.


#include "CGHUD.h"
#include "GameFramework/Controller.h"

void ACGHUD::BeginPlay()
{
	check(HotbarWidgetClass);
	APlayerController* PC = PlayerOwner.Get();
	if (IsValid(PC))
	{
		HotbarWidget = CreateWidget<UUserWidget>(PC, HotbarWidgetClass);
		if (HotbarWidget)
		{
			HotbarWidget->AddToViewport();
		}
	}
}
