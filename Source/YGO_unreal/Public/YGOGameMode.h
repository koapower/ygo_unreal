// YGO_unreal - Game Mode
// 管理整場決鬥的流程和規則

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "YGOCoreTypes.h"
#include "YGOGameMode.generated.h"

/**
 * YGO 遊戲模式
 * 負責決鬥的整體流程控制
 */
UCLASS()
class YGO_UNREAL_API AYGOGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AYGOGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController *NewPlayer) override;
	virtual void Logout(AController *Exiting) override;

	// ========================================================================
	// 遊戲設置
	// ========================================================================

	/** 每位玩家的起始生命值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "YGO|Rules")
	int32 StartingLifePoints = 8000;

	/** 起始手牌數 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YGO|Rules")
	int32 StartingHandSize = 5;

	/** 先攻玩家在第一回合是否可以抽牌 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "YGO|Rules")
	bool bFirstPlayerDraws = false;

	// ========================================================================
	// 玩家管理
	// ========================================================================

	/** 玩家 0 的 Controller */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Players")
	class AYGOPlayerController *Player0;

	/** 玩家 1 的 Controller */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Players")
	class AYGOPlayerController *Player1;

	/** 已連線的玩家數量 */
	UPROPERTY(BlueprintReadOnly, Category = "YGO|Players")
	int32 ConnectedPlayers = 0;

	// ========================================================================
	// 遊戲流程
	// ========================================================================

	/** 初始化決鬥 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Game")
	void InitializeDuel();

	/** 開始決鬥 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Game")
	void StartDuel();

	/** 結束決鬥 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Game")
	void EndDuel(uint8 WinnerPlayerID, const FString &Reason);

	// ========================================================================
	// 回合管理
	// ========================================================================

	/** 開始新回合 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Turn")
	void StartNewTurn();

	/** 進入階段 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Turn")
	void EnterPhase(EYGOPhase Phase);

	/** 抽牌階段處理 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Turn")
	void ProcessDrawPhase();

	/** 待機階段處理 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Turn")
	void ProcessStandbyPhase();

	// ========================================================================
	// 卡組載入
	// ========================================================================

	/** 為玩家載入預設卡組 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Deck")
	void LoadDefaultDeck(uint8 PlayerID);

	/** 從陣列載入卡組 */
	UFUNCTION(BlueprintCallable, Category = "YGO|Deck")
	void LoadDeckFromArray(uint8 PlayerID, const TArray<int32> &CardCodes);

	// ========================================================================
	// 事件
	// ========================================================================

	/** 當所有玩家準備完成 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllPlayersReady);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnAllPlayersReady OnAllPlayersReady;

	/** 當決鬥開始 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDuelStarted);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnDuelStarted OnDuelStarted;

	/** 當決鬥結束 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDuelEnded, uint8, WinnerID, const FString &, Reason);
	UPROPERTY(BlueprintAssignable, Category = "YGO|Events")
	FOnDuelEnded OnDuelEnded;

protected:
	/** 分配玩家 ID */
	void AssignPlayerID(class AYGOPlayerController *PlayerController);

	/** 檢查是否所有玩家已準備 */
	bool AreAllPlayersReady() const;
};
