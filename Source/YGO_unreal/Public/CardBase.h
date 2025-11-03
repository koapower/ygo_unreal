#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardBase.generated.h"

UCLASS()
class YGO_UNREAL_API ACardBase : public AActor
{
    GENERATED_BODY()

public:
    ACardBase();

    // 讓 Blueprint 可以實作的函式
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Networking")
    bool BP_IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const;
    virtual bool BP_IsNetRelevantFor_Implementation(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const;

protected:
    virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
    
};