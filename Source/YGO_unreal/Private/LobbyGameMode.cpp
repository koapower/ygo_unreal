#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ALobbyGameMode::ALobbyGameMode()
{
	// 設定 GameState 類型（可選）
	// GameStateClass = ALobbyGameState::StaticClass();
}

bool ALobbyGameMode::IsLobbyFull() const
{
	if (!GameState) return false;
	return GameState->PlayerArray.Num() >= MaxPlayers;
}

void ALobbyGameMode::SetLobbyClosed(bool bClosed)
{
	bIsLobbyClosed = bClosed;
}

void ALobbyGameMode::KickPlayer(APlayerController* PlayerToKick)
{
	// 永遠不要在伺服器上對 PlayerController 呼叫 Destroy()，這非常危險。
	if (PlayerToKick)
	{
		UE_LOG(LogTemp, Log, TEXT("Kicking Player..."));
		PlayerToKick->ClientTravel(TEXT("?closed"), ETravelType::TRAVEL_Absolute);
	}
}

void ALobbyGameMode::PreLogin(
	const FString& Options,
	const FString& Address,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	// 若房間已關閉
	if (bIsLobbyClosed)
	{
		ErrorMessage = TEXT("LobbyClosed");
		OnPlayerRejected(TEXT("Lobby is closed"));
		return;
	}

	// 若人數已滿
	if (GameState && GameState->PlayerArray.Num() >= MaxPlayers)
	{
		ErrorMessage = TEXT("ServerFull");
		OnPlayerRejected(TEXT("Lobby is full"));
		return;
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	UE_LOG(LogTemp, Log, TEXT("Player joined. Current count: %d"), GameState->PlayerArray.Num());

	// 如果達到人數上限，可自動鎖房
	if (GameState && GameState->PlayerArray.Num() >= MaxPlayers)
	{
		bIsLobbyClosed = true;
		UE_LOG(LogTemp, Log, TEXT("Lobby is now full, closed to new players."));
		//UE_LOG(LogTemp, Log, TEXT("Lobby is now full, preparing travel."));

		OnPrepareTravel(GameState->PlayerArray.Num());

		FString TravelCommand = FString::Printf(TEXT("%s?listen"), *NextLevelName.ToString());
		GetWorld()->ServerTravel(TravelCommand);
	}

}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GameState)
	{
		int32 CurrentPlayers = GameState->PlayerArray.Num();
		UE_LOG(LogTemp, Log, TEXT("Player Left. Current Players: %d/%d"), CurrentPlayers, MaxPlayers);
	}
}
