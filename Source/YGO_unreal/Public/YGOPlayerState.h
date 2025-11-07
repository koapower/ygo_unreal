// YGO_unreal - Player State
// 管理單個玩家的狀態資訊(生命值、手牌、墓地等)
// 這個類別會透過網路同步,但手牌等私密資訊只會同步給擁有者

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "YGOCoreTypes.h"
#include "YGOPlayerState.generated.h"

class AYGOCardActor;

/**
 * YGO 玩家狀態
 * 儲存每個玩家的遊戲狀態資訊
 */
UCLASS()
class YGO_UNREAL_API AYGOPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AYGOPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, Category = "YGO|Spawn")
	TSubclassOf<AYGOCardActor> CardActorClass;

	// ========================================================================
	// 基本資訊
	// ========================================================================

	/** 玩家 ID (0 或 1) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Player")
	uint8 YGOPlayerID;

	/** 生命值 (預設 8000) */
	UPROPERTY(ReplicatedUsing = OnRep_LifePoints, BlueprintReadOnly, Category = "YGO|Player")
	int32 LifePoints;

	UFUNCTION()
	void OnRep_LifePoints();

	/** 設定生命值 (只在 Server 執行) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Player", meta = (DisplayName = "Set Life Points (Server)"))
	void SetLifePoints(int32 NewLP);

	/** 扣血 (重新命名避免與 AActor::TakeDamage 衝突) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Player")
	void ReduceLifePoints(int32 Amount);

	/** 回復 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Player")
	void RecoverLife(int32 Amount);

	// ========================================================================
	// 卡片區域 (只有 Server 知道完整資訊)
	// ========================================================================

	/** 主卡組 (Server Only) */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Cards")
	TArray<FYGOCardInstance> MainDeck;

	/** 額外卡組 (Server Only) */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Cards")
	TArray<FYGOCardInstance> ExtraDeck;

	/** 手牌 (Server Only - 只有擁有者可見) */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Cards")
	TArray<FYGOCardInstance> Hand;

	/** 墓地 (所有人可見) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Cards")
	TArray<FYGOCardInstance> Graveyard;

	/** 除外區 (所有人可見) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Cards")
	TArray<FYGOCardInstance> Banished;

	// ========================================================================
	// 卡片數量資訊 (Client 只知道數量,不知道具體內容)
	// ========================================================================

	/** 主卡組剩餘卡片數 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Info")
	int32 DeckCount;

	/** 額外卡組剩餘卡片數 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Info")
	int32 ExtraDeckCount;

	/** 手牌數量 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Info")
	int32 HandCount;

	// ========================================================================
	// 遊戲狀態
	// ========================================================================

	/** 本回合是否已經通常召喚 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|State")
	bool bHasNormalSummoned;

	/** 本回合通常召喚次數 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|State")
	int32 NormalSummonCount;

	/** 是否已進入戰鬥階段 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|State")
	bool bHasEnteredBattlePhase;

	// ========================================================================
	// 卡組管理
	// ========================================================================

	/** 載入卡組 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Deck")
	void LoadDeck(const TArray<int32> &CardCodes);

	/** 抽牌 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Deck")
	void DrawCards(int32 Count);

	/** 洗牌 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Deck")
	void ShuffleDeck();

	/** 將卡片加入手牌 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Hand")
	void AddCardToHand(const FYGOCardInstance &Card);

	/** 將卡片從手牌移除 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Hand")
	void RemoveCardFromHand(int32 InstanceID);

	/** 將卡片送往墓地 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Graveyard")
	void SendToGraveyard(const FYGOCardInstance &Card);

	// ========================================================================
	// 客戶端手牌查詢 (只有擁有者可以看到完整手牌)
	// ========================================================================

	/** 取得手牌 (Client) - 僅擁有者可見 */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "YGO|Hand")
	void Client_ReceiveHandCards(const TArray<FYGOCardInstance> &HandCards);

	/** 儲存客戶端手牌 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Hand")
	TArray<FYGOCardInstance> ClientHand;

	// ========================================================================
	// 卡片視覺化
	// ========================================================================

	/** 手牌卡片 Actor 陣列 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Visual")
	TArray<AYGOCardActor *> HandCardActors;

	/** 牌組所有卡片 Actor 陣列 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Visual")
	TArray<AYGOCardActor *> DeckCardActors;

	/** 額外牌組所有卡片 Actor 陣列 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Visual")
	TArray<AYGOCardActor *> ExtraDeckCardActors;

	/** 生成手牌視覺化 (Client) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void SpawnHandCards();

	/** 生成所有牌組卡片 (Client) 生成所有真實卡片並堆疊 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void SpawnAllDeckCards();

	/** 生成所有額外牌組卡片 (Client) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void SpawnAllExtraDeckCards();

	/** 清除所有牌組視覺化 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void ClearDeckCardActors();

	/** 更新手牌位置 (排列成弧形) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void UpdateHandPositions();

	/** 從手牌移除視覺化卡片 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Visual")
	void RemoveHandCardActor(AYGOCardActor *CardActor);

	// ========================================================================
	// 事件
	// ========================================================================

	// 委託宣告必須在 UPROPERTY 之前
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLifePointsChanged, int32, OldLP, int32, NewLP);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardDrawn, const FYGOCardInstance &, DrawnCard);

	/** 當生命值改變 */
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnLifePointsChanged OnLifePointsChanged;

	/** 當抽牌 */
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnCardDrawn OnCardDrawn;

protected:
	virtual void BeginPlay() override;

	/** 更新卡片數量資訊 (Server) */
	void UpdateCardCounts();

	/** 下一個卡片實例 ID */
	int32 NextInstanceID = 1;
};
