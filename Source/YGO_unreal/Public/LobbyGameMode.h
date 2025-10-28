#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 *  Lobby GameMode: ����a�n�J�B�ж��H�ơB�ڵ��޿�C
 */
UCLASS()
class YGO_UNREAL_API ALobbyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALobbyGameMode();

	/** �U�@���a�ϦW�١]�i�b BP �]�w�^ */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    FName NextLevelName;

    /** �̤j���a�H�ơ]�w�] 2�A�i�b BP ��^ */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    int32 MaxPlayers = 2;

    /** �ж��O�_�����]�}����i��С^ */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    bool bIsLobbyClosed = false;

    /** �O�_�F��̤j�H�� */
    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool IsLobbyFull() const;

    /** ��������ζ}�� Lobby */
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void SetLobbyClosed(bool bClosed);

    /** ��X���a */
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void KickPlayer(APlayerController* PlayerToKick);

    UFUNCTION(BlueprintImplementableEvent, Category = "Game")
    void OnPrepareTravel(int32 PlayerCount);

protected:
    /** �b���a�n�J�e�ˬd�i�_�n�J */
    virtual void PreLogin(
        const FString& Options,
        const FString& Address,
        const FUniqueNetIdRepl& UniqueId,
        FString& ErrorMessage) override;

    /** ���a���\�n�J�� */
    virtual void PostLogin(APlayerController* NewPlayer) override;

	/** ���a�n�X�� */
    virtual void Logout(AController* Exiting) override;

    /** ���a�ڵ��ƥ�]�� BP ��@�^ */
    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void OnPlayerRejected(const FString& Reason);
};
