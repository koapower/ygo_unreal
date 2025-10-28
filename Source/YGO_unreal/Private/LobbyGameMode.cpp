#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ALobbyGameMode::ALobbyGameMode()
{
	// �]�w GameState �����]�i��^
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
	// �û����n�b���A���W�� PlayerController �I�s Destroy()�A�o�D�`�M�I�C
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

	// �Y�ж��w����
	if (bIsLobbyClosed)
	{
		ErrorMessage = TEXT("LobbyClosed");
		OnPlayerRejected(TEXT("Lobby is closed"));
		return;
	}

	// �Y�H�Ƥw��
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

	// �p�G�F��H�ƤW���A�i�۰����
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
