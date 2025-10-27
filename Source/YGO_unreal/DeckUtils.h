// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DeckUtils.generated.h"

/**
 * 
 */
UCLASS()
class YGO_UNREAL_API UDeckUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Card|Utils")
	static void ShuffleIntArray(UPARAM(ref) TArray<int32>& Array);
};
