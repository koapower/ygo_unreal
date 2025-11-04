// YGO_unreal - Card Actor
// 卡片的 3D 可視化 Actor,繼承自你現有的 CardBase

#pragma once

#include "CoreMinimal.h"
#include "CardBase.h"
#include "YGOCoreTypes.h"
#include "YGOCardActor.generated.h"

/**
 * YGO 卡片 Actor
 * 代表場上的一張實體卡片
 */
UCLASS()
class YGO_UNREAL_API AYGOCardActor : public ACardBase
{
	GENERATED_BODY()

public:
	AYGOCardActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================================================================
	// 卡片資料
	// ========================================================================

	/** 卡片實例資料 (包含靜態資料和運行時狀態) */
	UPROPERTY(ReplicatedUsing = OnRep_CardInstance, BlueprintReadWrite, Category = "YGO|Card")
	FYGOCardInstance CardInstance;

	UFUNCTION()
	void OnRep_CardInstance();

	/** 設定卡片資料 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Card")
	void SetCardData(const FYGOCardInstance& NewCardData);

	// ========================================================================
	// 視覺化
	// ========================================================================

	/** 卡片網格 (平面或 3D 模型) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YGO|Visual")
	class UStaticMeshComponent* CardMesh;

	/** 卡片材質 (用於顯示卡圖) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Visual")
	class UMaterialInstanceDynamic* CardMaterial;

	/** 更新卡片外觀 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void UpdateCardVisual();

	/** 設定卡片位置 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void SetCardPosition(EYGOPosition NewPosition);

	/** 翻轉卡片 (翻面) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void FlipCard();

	// ========================================================================
	// 遊戲邏輯
	// ========================================================================

	/** 是否為怪獸卡 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Card")
	bool IsMonster() const;

	/** 是否為魔法卡 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Card")
	bool IsSpell() const;

	/** 是否為陷阱卡 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Card")
	bool IsTrap() const;

	/** 是否在場上 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Card")
	bool IsOnField() const;

	/** 是否表側表示 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Card")
	bool IsFaceUp() const;

	/** 是否為攻擊表示 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Card")
	bool IsAttackPosition() const;

	// ========================================================================
	// 戰鬥
	// ========================================================================

	/** 攻擊另一張卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Battle")
	void AttackCard(AYGOCardActor* Target);

	/** 直接攻擊 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Battle")
	void DirectAttack();

	/** 可否攻擊 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Battle")
	bool CanAttack() const;

	// ========================================================================
	// 事件
	// ========================================================================

	// 委託宣告必須在 UPROPERTY 之前
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardClicked, AYGOCardActor*, ClickedCard);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardSummoned, AYGOCardActor*, SummonedCard);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardDestroyed, AYGOCardActor*, DestroyedCard);

	/** 當卡片被點擊 */
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnCardClicked OnCardClicked;

	/** 當卡片被召喚 */
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnCardSummoned OnCardSummoned;

	/** 當卡片被破壞 */
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnCardDestroyed OnCardDestroyed;

protected:
	virtual void BeginPlay() override;

	/** 處理點擊事件 */
	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
};
