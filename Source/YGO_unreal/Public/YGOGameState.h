// YGO_unreal - Game State
// 管理整場決鬥的全局狀態(回合、階段、場地卡片等)
// 這個類別在 Server 權威,所有資訊會同步到所有 Client

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "YGOCoreTypes.h"
#include "YGOGameState.generated.h"

class AYGOCardActor;
class AYGOFieldZone;

/**
 * YGO 遊戲狀態
 * 管理決鬥的全局資訊
 */
UCLASS()
class YGO_UNREAL_API AYGOGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AYGOGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================================================================
	// 回合系統
	// ========================================================================

	/** 當前回合數 */
	UPROPERTY(ReplicatedUsing = OnRep_TurnCount, BlueprintReadOnly, Category = "YGO|Turn")
	int32 TurnCount;

	UFUNCTION()
	void OnRep_TurnCount();

	/** 當前回合玩家 ID (0 或 1) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayer, BlueprintReadOnly, Category = "YGO|Turn")
	uint8 CurrentPlayer;

	UFUNCTION()
	void OnRep_CurrentPlayer();

	/** 當前階段 */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase, BlueprintReadOnly, Category = "YGO|Turn")
	EYGOPhase CurrentPhase;

	UFUNCTION()
	void OnRep_CurrentPhase();

	/** 前往下一回合 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Turn")
	void GoToNextTurn();

	/** 前往下一階段 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Turn")
	void GoToNextPhase();

	/** 設定階段 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Turn")
	void SetPhase(EYGOPhase NewPhase);

	// ========================================================================
	// 場地區域 (Field Zones)
	// ========================================================================

	/** 玩家 0 的怪獸區 (5 格) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Field")
	TArray<AYGOCardActor*> Player0_MonsterZone;

	/** 玩家 0 的魔法陷阱區 (5 格) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Field")
	TArray<AYGOCardActor*> Player0_SpellTrapZone;

	/** 玩家 1 的怪獸區 (5 格) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Field")
	TArray<AYGOCardActor*> Player1_MonsterZone;

	/** 玩家 1 的魔法陷阱區 (5 格) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Field")
	TArray<AYGOCardActor*> Player1_SpellTrapZone;

	/** 場地魔法區 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|Field")
	TArray<AYGOCardActor*> FieldSpellZone;

	// ========================================================================
	// 場地位置 Actor 參考 (在 Level 中手動放置的位置標記)
	// ========================================================================

	/** 玩家 0 怪獸區位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Field Positions")
	TArray<AYGOFieldZone*> Player0_MonsterZonePositions;

	/** 玩家 0 魔法陷阱區位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Field Positions")
	TArray<AYGOFieldZone*> Player0_SpellTrapZonePositions;

	/** 玩家 1 怪獸區位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Field Positions")
	TArray<AYGOFieldZone*> Player1_MonsterZonePositions;

	/** 玩家 1 魔法陷阱區位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Field Positions")
	TArray<AYGOFieldZone*> Player1_SpellTrapZonePositions;

	/** 玩家 0 牌組位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Field Positions")
	AYGOFieldZone* Player0_DeckPosition;

	/** 玩家 1 牌組位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YGO|Field Positions")
	AYGOFieldZone* Player1_DeckPosition;

	/** 自動註冊 FieldZone (由 FieldZone BeginPlay 呼叫) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Field")
	void RegisterFieldZone(AYGOFieldZone* Zone);

	// ========================================================================
	// 場地卡片管理
	// ========================================================================

	/** 在場地上放置卡片 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Field")
	bool PlaceCardOnField(AYGOCardActor* Card, uint8 PlayerID, EYGOLocation Zone, uint8 Sequence);

	/** 從場地移除卡片 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Field")
	bool RemoveCardFromField(AYGOCardActor* Card);

	/** 取得場地上的卡片 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Field")
	AYGOCardActor* GetCardAt(uint8 PlayerID, EYGOLocation Zone, uint8 Sequence) const;

	/** 檢查位置是否可用 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "YGO|Field")
	bool IsZoneAvailable(uint8 PlayerID, EYGOLocation Zone, uint8 Sequence) const;

	// ========================================================================
	// 遊戲狀態
	// ========================================================================

	/** 遊戲是否已開始 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|State")
	bool bGameStarted;

	/** 遊戲是否已結束 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|State")
	bool bGameEnded;

	/** 獲勝玩家 ID */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "YGO|State")
	uint8 WinnerPlayerID;

	/** 開始遊戲 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Game")
	void StartGame();

	/** 結束遊戲 (Server) */
	UFUNCTION(BlueprintCallable, Category = "YGO|Game")
	void EndGame(uint8 WinnerID);

	// ========================================================================
	// 事件
	// ========================================================================

	/** 當回合改變 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTurnChanged, int32, NewTurn, uint8, NewPlayer);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnTurnChanged OnTurnChanged;

	/** 當階段改變 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EYGOPhase, NewPhase);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnPhaseChanged OnPhaseChanged;

	/** 當遊戲開始 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnGameStarted OnGameStarted;

	/** 當遊戲結束 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEnded, uint8, WinnerID);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnGameEnded OnGameEnded;

protected:
	virtual void BeginPlay() override;

	/** 初始化場地區域陣列 */
	void InitializeFieldZones();

	/** 取得場地區域陣列參考 */
	TArray<AYGOCardActor*>* GetZoneArray(uint8 PlayerID, EYGOLocation Zone);
};
