// YGO_unreal - Game UI Widget
// 顯示玩家手牌和雙方 LP 的 UI Widget
// 可以在 UMG Blueprint 中繼承這個類別來自訂外觀

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "YGOCoreTypes.h"
#include "YGOGameWidget.generated.h"

class AYGOPlayerState;

/**
 * 遊戲主 UI Widget
 * 負責顯示玩家手牌、雙方 LP 和其他遊戲資訊
 */
UCLASS()
class YGO_UNREAL_API UYGOGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================================================
	// 初始化
	// ========================================================================

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 設定玩家狀態參考 */
	UFUNCTION(BlueprintCallable, Category = "YGO|UI")
	void SetPlayerStates(AYGOPlayerState* LocalPlayer, AYGOPlayerState* OpponentPlayer);

	// ========================================================================
	// 手牌顯示
	// ========================================================================

	/** 更新手牌顯示（當手牌改變時調用） */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "YGO|UI|Hand")
	void UpdateHandDisplay(const TArray<FYGOCardInstance>& HandCards);
	virtual void UpdateHandDisplay_Implementation(const TArray<FYGOCardInstance>& HandCards);

	/** 更新對手手牌顯示（當手牌改變時調用） */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "YGO|UI|Hand")
	void UpdateOpponentHandDisplay(int32 HandCount);
	virtual void UpdateOpponentHandDisplay_Implementation(int32 HandCount);

	/** 獲取當前玩家手牌 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Hand")
	TArray<FYGOCardInstance> GetLocalPlayerHand() const;

	/** 獲取對手手牌數量 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Hand")
	int32 GetOpponentHandCount() const;

	// ========================================================================
	// LP 顯示
	// ========================================================================

	/** 更新本地玩家 LP 顯示 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "YGO|UI|LP")
	void UpdateLocalPlayerLP(int32 OldLP, int32 NewLP);
	virtual void UpdateLocalPlayerLP_Implementation(int32 OldLP, int32 NewLP);

	/** 更新對手 LP 顯示 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "YGO|UI|LP")
	void UpdateOpponentLP(int32 OldLP, int32 NewLP);
	virtual void UpdateOpponentLP_Implementation(int32 OldLP, int32 NewLP);

	/** 獲取本地玩家當前 LP */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|LP")
	int32 GetLocalPlayerLP() const;

	/** 獲取對手當前 LP */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|LP")
	int32 GetOpponentLP() const;

	// ========================================================================
	// 其他遊戲資訊
	// ========================================================================

	/** 獲取本地玩家卡組剩餘數量 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Info")
	int32 GetLocalPlayerDeckCount() const;

	/** 獲取對手卡組剩餘數量 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Info")
	int32 GetOpponentDeckCount() const;

	/** 獲取本地玩家額外卡組剩餘數量 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Info")
	int32 GetLocalPlayerExtraDeckCount() const;

	/** 獲取對手額外卡組剩餘數量 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Info")
	int32 GetOpponentExtraDeckCount() const;

	/** 獲取本地玩家墓地卡片數量 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Info")
	int32 GetLocalPlayerGraveyardCount() const;

	/** 獲取對手墓地卡片數量 */
	UFUNCTION(BlueprintPure, Category = "YGO|UI|Info")
	int32 GetOpponentGraveyardCount() const;

	// ========================================================================
	// Blueprint 可實作的事件
	// ========================================================================

	/** 當手牌改變時觸發（可在 Blueprint 實作） */
	UFUNCTION(BlueprintImplementableEvent, Category = "YGO|UI|Events")
	void OnHandChanged(const TArray<FYGOCardInstance>& NewHand);

	/** 當本地玩家 LP 改變時觸發（可在 Blueprint 實作） */
	UFUNCTION(BlueprintImplementableEvent, Category = "YGO|UI|Events")
	void OnLocalPlayerLPChanged(int32 OldLP, int32 NewLP);

	/** 當對手 LP 改變時觸發（可在 Blueprint 實作） */
	UFUNCTION(BlueprintImplementableEvent, Category = "YGO|UI|Events")
	void OnOpponentLPChanged(int32 OldLP, int32 NewLP);

	/** 當對手手牌張數改變時觸發（可在 Blueprint 實作） */
	UFUNCTION(BlueprintImplementableEvent, Category = "YGO|UI|Events")
	void OnOpponentHandCountChanged(int32 HandCount);

	/** 當玩家抽牌時觸發（可在 Blueprint 實作，帶動畫效果） */
	UFUNCTION(BlueprintImplementableEvent, Category = "YGO|UI|Events")
	void OnCardDrawn(const FYGOCardInstance& DrawnCard);

protected:
	// ========================================================================
	// 玩家狀態參考
	// ========================================================================

	/** 本地玩家狀態 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|UI")
	AYGOPlayerState* LocalPlayerState;

	/** 對手玩家狀態 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|UI")
	AYGOPlayerState* OpponentPlayerState;

	// ========================================================================
	// 內部狀態
	// ========================================================================

	/** 本地玩家上次 LP（用於檢測變化） */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|UI")
	int32 CachedLocalPlayerLP;

	/** 對手上次 LP（用於檢測變化） */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|UI")
	int32 CachedOpponentLP;

	/** 本地玩家上次手牌數量（用於檢測變化） */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|UI")
	int32 CachedLocalPlayerHandCount;

	// ========================================================================
	// 內部方法
	// ========================================================================

	/** 綁定玩家狀態事件 */
	void BindPlayerStateEvents();

	/** 解除綁定玩家狀態事件 */
	void UnbindPlayerStateEvents();

	/** 處理本地玩家 LP 改變 */
	UFUNCTION()
	void HandleLocalPlayerLPChanged(int32 OldLP, int32 NewLP);

	/** 處理對手 LP 改變 */
	UFUNCTION()
	void HandleOpponentLPChanged(int32 OldLP, int32 NewLP);

	/** 處理本地玩家抽牌 */
	UFUNCTION()
	void HandleLocalPlayerCardDrawn(const FYGOCardInstance& DrawnCard);

	/** 處理對手手牌張數改變 */
	UFUNCTION()
	void HandleOpponentHandCountChanged(int32 HandCount);

	/** 檢查並更新手牌顯示 */
	void CheckAndUpdateHandDisplay();
};
