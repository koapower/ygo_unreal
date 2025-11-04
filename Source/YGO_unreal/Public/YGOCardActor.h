// YGO_unreal - Card Actor
// 卡片的 3D 可視化 Actor

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YGOCoreTypes.h"
#include "YGOCardActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

/**
 * YGO 卡片 Actor
 * 代表場上的一張實體卡片
 */
UCLASS()
class YGO_UNREAL_API AYGOCardActor : public AActor
{
	GENERATED_BODY()

public:
	AYGOCardActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	// ========================================================================
	// 卡片資料
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YGO|Data")
	UDataTable *SpriteSheetDataTable;

	/** 卡片實例資料 (包含靜態資料和運行時狀態) */
	UPROPERTY(ReplicatedUsing = OnRep_CardInstance, BlueprintReadWrite, Category = "YGO|Card")
	FYGOCardInstance CardInstance;

	UFUNCTION()
	void OnRep_CardInstance();

	/** 設定卡片資料 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Card")
	void SetCardData(const FYGOCardInstance &NewCardData);

	// ========================================================================
	// 視覺化 - 雙面卡片系統
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YGO|Visual")
	USceneComponent *Root;

	/** 卡片網格組件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YGO|Visual")
	UStaticMeshComponent *CardFrontMesh;

	/** 卡片網格組件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YGO|Visual")
	UStaticMeshComponent *CardBackMesh;

	/** 卡片正面材質模板 (M_CardAtlas_Inst) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Visual")
	UMaterialInterface *CardFrontMaterialTemplate;

	/** 卡片背面材質模板 (M_CardBack_Inst) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Visual")
	UMaterialInterface *CardBackMaterialTemplate;

	/** 運行時動態材質 (正面) */
	UPROPERTY()
	UMaterialInstanceDynamic *DynamicFrontMaterial;

	/** 運行時動態材質 (背面) */
	UPROPERTY()
	UMaterialInstanceDynamic *DynamicBackMaterial;

	/** 是否正面朝上 (同步到所有 Client) */
	UPROPERTY(ReplicatedUsing = OnRep_FaceUp, BlueprintReadWrite, Category = "YGO|State")
	bool bFaceUp;

	UFUNCTION()
	void OnRep_FaceUp();

	/** 更新卡片視覺效果 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void UpdateCardVisual();

	/** 設定卡圖 Index (用於 6x6 SpriteSheet) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void SetCardArtIndex(int32 Index);

	/** 翻轉到正面 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Card")
	void FlipFaceUp();

	/** 翻轉到背面 (蓋放) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Card")
	void FlipFaceDown();

	/** 設定卡片位置 (表示/裡側,攻擊/守備) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Card")
	void SetCardPosition(EYGOPosition NewPosition);

	// ========================================================================
	// 卡片類型判斷
	// ========================================================================

	/** 是否為怪獸卡 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Type")
	bool IsMonster() const;

	/** 是否為魔法卡 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Type")
	bool IsSpell() const;

	/** 是否為陷阱卡 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Type")
	bool IsTrap() const;

	/** 是否在場上 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|State")
	bool IsOnField() const;

	// ========================================================================
	// 戰鬥相關
	// ========================================================================

	/** 攻擊另一張卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Battle")
	void AttackCard(AYGOCardActor *Target);

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
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardClicked, AYGOCardActor *, ClickedCard);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardSummoned, AYGOCardActor *, SummonedCard);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardDestroyed, AYGOCardActor *, DestroyedCard);

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
	/** 處理點擊事件 */
	UFUNCTION()
	void OnClicked(UPrimitiveComponent *TouchedComponent, FKey ButtonPressed);

	/** 檢查是否為本地玩家擁有 */
	bool IsOwnedByLocalPlayer() const;
};
