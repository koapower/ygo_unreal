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

	/** 是否被佔用 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Zone")
	bool bOccupied;

	/** 當前佔用此位置的卡片 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Zone")
	class AYGOCardActor* OccupyingCard;

	/** 視覺化組件 (在編輯器中顯示) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YGO|Zone")
	class UStaticMeshComponent* VisualMesh;

	/** 放置卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Zone")
	void PlaceCard(class AYGOCardActor* Card);

	/** 移除卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Zone")
	void RemoveCard();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif
};
