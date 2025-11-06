// YGO_unreal - Field Zone Actor
// 場地位置標記 Actor - 在 Level 中手動放置來標記卡片應該出現的 3D 位置

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YGOCoreTypes.h"
#include "YGOFieldZone.generated.h"

/**
 * 場地區域位置標記
 * 這是一個空的 Actor,用來在 3D 空間中標記卡片的位置
 */
UCLASS()
class YGO_UNREAL_API AYGOFieldZone : public AActor
{
	GENERATED_BODY()

public:
	AYGOFieldZone();

	/** 玩家 ID (0 或 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Zone")
	uint8 PlayerID;

	/** 區域類型 (怪獸區/魔陷區等) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Zone")
	EYGOLocation ZoneType;

	/** 序列 (該區域的索引 0-4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Zone")
	uint8 Sequence;

	/** 此 Zone 中的所有卡片 (堆疊) */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Zone")
	TArray<class AYGOCardActor*> CardsInZone;

	/** 視覺化組件 (在編輯器中顯示) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YGO|Zone")
	class UStaticMeshComponent* VisualMesh;

	/** 添加卡片到此 Zone */
	UFUNCTION(BlueprintCallable, Category = "YGO|Zone")
	void AddCard(class AYGOCardActor* Card);

	/** 從此 Zone 移除卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Zone")
	void RemoveCard(class AYGOCardActor* Card);

	/** 更新所有卡片的目標位置 (根據堆疊索引) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Zone")
	void UpdateCardPositions();

	/** 獲取指定索引的卡片應該在的位置 (Local Space) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Zone")
	FVector GetCardLocalOffset(int32 CardIndex) const;

	/** 檢查是否被佔用 (至少有一張卡) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Zone")
	bool IsOccupied() const { return CardsInZone.Num() > 0; }

	/** 獲取卡片數量 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Zone")
	int32 GetCardCount() const { return CardsInZone.Num(); }

	/** 舊的 API - 向後兼容 (已棄用) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Zone", meta = (DeprecatedFunction))
	void PlaceCard(class AYGOCardActor* Card);

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif
};
