#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 *  Lobby GameMode: 控制玩家登入、房間人數、拒絕邏輯。
 */
UCLASS()
class YGO_UNREAL_API ALobbyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALobbyGameMode();

	/** 下一關地圖名稱（可在 BP 設定） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    FName NextLevelName;

    /** 最大玩家人數（預設 2，可在 BP 改） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    int32 MaxPlayers = 2;

    /** 房間是否關閉（開局後可鎖房） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    bool bIsLobbyClosed = false;

    /** 是否達到最大人數 */
    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool IsLobbyFull() const;

    /** 手動關閉或開啟 Lobby */
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetLobbyClosed(bool bClosed);

    /** 踢出玩家 */
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void KickPlayer(APlayerController* PlayerToKick);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game")
    void OnPrepareTravel(int32 PlayerCount);

protected:
    /** 在玩家登入前檢查可否登入 */
    virtual void PreLogin(
        const FString& Options,
        const FString& Address,
        const FUniqueNetIdRepl& UniqueId,
        FString& ErrorMessage) override;

    /** 玩家成功登入後 */
    virtual void PostLogin(APlayerController* NewPlayer) override;

	/** 玩家登出後 */
    virtual void Logout(AController* Exiting) override;

    /** 玩家拒絕事件（給 BP 實作） */
    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void OnPlayerRejected(const FString& Reason);
};
