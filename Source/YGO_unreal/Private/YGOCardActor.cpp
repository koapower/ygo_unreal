// YGO_unreal - Card Actor Implementation

#include "YGOCardActor.h"
#include "YGODataTableSubsystem.h"
#include "YGOFieldZone.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "YGOPlayerState.h"

AYGOCardActor::AYGOCardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 啟用網路複製
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// 創建卡片網格組件
	CardFrontMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardFrontMesh"));
	CardFrontMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		CardFrontMesh->SetStaticMesh(PlaneMesh.Object);
		CardFrontMesh->SetRelativeScale3D(FVector(0.484f, 0.7f, 1.0f));
	}

	CardBackMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardBackMesh"));
	CardBackMesh->SetupAttachment(RootComponent);

	if (PlaneMesh.Succeeded())
	{
		CardBackMesh->SetStaticMesh(PlaneMesh.Object);
		CardBackMesh->SetRelativeScale3D(FVector(0.484f, 0.7f, 1.0f));

		// 翻轉180度 (讓它背對正面)
		CardBackMesh->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
		CardBackMesh->SetRelativeLocation(FVector(0.f, 0.f, -0.01f)); // 微微偏移避免 Z-fighting
	}

	// 啟用點擊事件
	CardFrontMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CardFrontMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CardFrontMesh->SetGenerateOverlapEvents(true);
	CardFrontMesh->OnClicked.AddDynamic(this, &AYGOCardActor::OnClicked);
	CardBackMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CardBackMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CardBackMesh->SetGenerateOverlapEvents(true);
	CardBackMesh->OnClicked.AddDynamic(this, &AYGOCardActor::OnClicked);

	// 預設狀態
	bFaceUp = false;
	CardFrontMaterialTemplate = nullptr;
	CardBackMaterialTemplate = nullptr;
	DynamicFrontMaterial = nullptr;
	DynamicBackMaterial = nullptr;

	// 移動和旋轉系統初始化
	CurrentZone = nullptr;
	StackIndex = 0;
	TargetLocation = FVector::ZeroVector;
	TargetRotation = FRotator::ZeroRotator;
	bIsMoving = false;
	MovementSpeed = 800.0f;
	RotationSpeed = 360.0f;
}

void AYGOCardActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYGOCardActor, CardInstance);
	DOREPLIFETIME(AYGOCardActor, bFaceUp);
}

void AYGOCardActor::BeginPlay()
{
	Super::BeginPlay();

	// 創建動態材質實例
	if (CardFrontMaterialTemplate)
	{
		DynamicFrontMaterial = UMaterialInstanceDynamic::Create(CardFrontMaterialTemplate, this);
		if (DynamicFrontMaterial)
		{
			CardFrontMesh->SetMaterial(0, DynamicFrontMaterial);
		}
	}

	if (CardBackMaterialTemplate)
	{
		DynamicBackMaterial = UMaterialInstanceDynamic::Create(CardBackMaterialTemplate, this);
		if (DynamicBackMaterial)
		{
			CardBackMesh->SetMaterial(0, DynamicBackMaterial);
		}
	}

	// 初始化視覺效果
	UpdateCardVisual();
}

void AYGOCardActor::OnRep_CardInstance()
{
	// 當卡片資料複製到客戶端時更新視覺效果
	UpdateCardVisual();
}

void AYGOCardActor::OnRep_FaceUp()
{
	// 當正反面狀態改變時更新視覺效果
	UpdateCardVisual();
}

void AYGOCardActor::SetCardData(const FYGOCardInstance &NewCardData)
{
	if (!HasAuthority())
	{
		return;
	}

	CardInstance = NewCardData;
	UpdateCardVisual();
}

void AYGOCardActor::UpdateCardVisual()
{
	if (!CardFrontMesh || !CardBackMesh)
	{
		return;
	}

	UDataTable *SpriteSheetDataTable = GetGameInstance()->GetSubsystem<UYGODataTableSubsystem>()->GetDataTable(TEXT("ygo04_-_spritesheetindex"));

	if (!SpriteSheetDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpriteSheetDataTable is null"));
		return;
	}

	int32 cardId = CardInstance.CardData.CardCode == 0 ? 4354 : CardInstance.CardData.CardCode; // 用隨便一個valid cardid避免他報warning。
	FName RowName = FName(*FString::FromInt(cardId));
	const FYGOSpriteSheetIndex *Row = SpriteSheetDataTable->FindRow<FYGOSpriteSheetIndex>(
		RowName, TEXT("CardVisualLookup"));

	bool bShouldShowFront = bFaceUp || IsOwnedByLocalPlayer();
	if (Row)
	{
		if (DynamicFrontMaterial && bShouldShowFront)
		{
			DynamicFrontMaterial->SetScalarParameterValue(TEXT("Index"), Row->Index);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No SpriteSheetIndex found for CardCode: %d"), CardInstance.CardData.CardCode);
		return;
	}
}

void AYGOCardActor::FlipFaceUp()
{
	if (!HasAuthority())
	{
		return;
	}

	SetCardPosition(EYGOPosition::FaceUpAttack);
	UpdateTargetTransform();
}

void AYGOCardActor::FlipFaceDown()
{
	if (!HasAuthority())
	{
		return;
	}

	SetCardPosition(EYGOPosition::FaceDownDefense);
	UpdateTargetTransform();
}

void AYGOCardActor::SetCardPosition(EYGOPosition NewPosition)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentZone)
	{
		bool isDefensePos = NewPosition == EYGOPosition::FaceUpDefense ||
							NewPosition == EYGOPosition::FaceDownDefense;
		bool isMonsterZone = CurrentZone->ZoneType == EYGOLocation::MonsterZone;
		if (isDefensePos && !isMonsterZone)
		{
			if (NewPosition == EYGOPosition::FaceUpDefense)
			{
				NewPosition = EYGOPosition::FaceUpAttack;
			}
			else if (NewPosition == EYGOPosition::FaceDownDefense)
			{
				NewPosition = EYGOPosition::FaceDownAttack;
			}
		}
	}

	if (CardInstance.Position == NewPosition)
	{
		return;
	}

	CardInstance.Position = NewPosition;

	// 更新正反面狀態
	switch (NewPosition)
	{
	case EYGOPosition::FaceUpAttack:
	case EYGOPosition::FaceUpDefense:
		bFaceUp = true;
		break;
	case EYGOPosition::FaceDownAttack:
	case EYGOPosition::FaceDownDefense:
		bFaceUp = false;
		break;

	default:
		break;
	}
}

bool AYGOCardActor::IsMonster() const
{
	return (CardInstance.CardData.Type & YGOCardType::Monster) != 0;
}

bool AYGOCardActor::IsSpell() const
{
	return (CardInstance.CardData.Type & YGOCardType::Spell) != 0;
}

bool AYGOCardActor::IsTrap() const
{
	return (CardInstance.CardData.Type & YGOCardType::Trap) != 0;
}

bool AYGOCardActor::IsOnField() const
{
	return CardInstance.Location == EYGOLocation::MonsterZone ||
		   CardInstance.Location == EYGOLocation::SpellTrapZone ||
		   CardInstance.Location == EYGOLocation::FieldZone;
}

void AYGOCardActor::AttackCard(AYGOCardActor *Target)
{
	if (!HasAuthority() || !Target)
	{
		return;
	}

	// TODO: 實作戰鬥邏輯
	UE_LOG(LogTemp, Log, TEXT("[Card] %s attacks %s"),
		   *CardInstance.CardData.CardName,
		   *Target->CardInstance.CardData.CardName);
}

void AYGOCardActor::DirectAttack()
{
	if (!HasAuthority())
	{
		return;
	}

	// TODO: 實作直接攻擊邏輯
	UE_LOG(LogTemp, Log, TEXT("[Card] %s direct attack!"),
		   *CardInstance.CardData.CardName);
}

bool AYGOCardActor::CanAttack() const
{
	// TODO: 檢查是否可以攻擊
	return IsOnField() &&
		   bFaceUp &&
		   (CardInstance.Position == EYGOPosition::FaceUpAttack);
}

void AYGOCardActor::OnClicked(UPrimitiveComponent *TouchedComponent, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Log, TEXT("[Card] Clicked: %s"), *CardInstance.CardData.CardName);

	if (OnCardClicked.IsBound())
	{
		OnCardClicked.Broadcast(this);
	}
}

bool AYGOCardActor::IsOwnedByLocalPlayer() const
{
	// 檢查這張卡是否屬於本地玩家
	APlayerController *LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC)
	{
		return false;
	}

	AYGOPlayerState *LocalPlayerState = LocalPC->GetPlayerState<AYGOPlayerState>();
	if (!LocalPlayerState)
	{
		return false;
	}

	return CardInstance.OwnerPlayerID == LocalPlayerState->YGOPlayerID;
}

// ============================================================================
// 移動和旋轉系統實現
// ============================================================================

FRotator AYGOCardActor::GetLocalRotationForPosition(EYGOPosition Position)
{
	switch (Position)
	{
	case EYGOPosition::FaceUpAttack:
		// 正面朝上，直立
		return FRotator(0, 0, 0);

	case EYGOPosition::FaceUpDefense:
		// 正面朝上，橫向 (守備表示)
		return FRotator(0, 0, 90);

	case EYGOPosition::FaceDownAttack:
		// 背面朝上，直立
		return FRotator(180, 0, 0);

	case EYGOPosition::FaceDownDefense:
		// 背面朝上，橫向 (守備表示)
		return FRotator(180, 0, 90);

	default:
		return FRotator::ZeroRotator;
	}
}

void AYGOCardActor::MoveToZone(AYGOFieldZone *TargetZone)
{
	if (!TargetZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CardActor] MoveToZone called with null TargetZone"));
		return;
	}

	// 從舊 Zone 移除
	if (CurrentZone && CurrentZone != TargetZone)
	{
		CurrentZone->RemoveCard(this);
	}

	// 加入新 Zone
	CurrentZone = TargetZone;
	SetCardPosition(CardInstance.Position); // 不是怪獸區都不可以打橫的放，要更新
	TargetZone->AddCard(this);				// AddCard 會自動調用 UpdateCardPositions，進而調用 UpdateTargetTransform

	bIsMoving = true;

	UE_LOG(LogTemp, Log, TEXT("[CardActor] %s moving to zone %s"),
		   *CardInstance.CardData.CardName,
		   *UEnum::GetValueAsString(TargetZone->ZoneType));
}

void AYGOCardActor::UpdateTargetTransform()
{
	if (!CurrentZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CardActor] UpdateTargetTransform called but CurrentZone is null"));
		return;
	}

	// 1. 獲取 Zone 的 Transform
	FTransform ZoneTransform = CurrentZone->GetActorTransform();

	// 2. 獲取本地旋轉 (基於 EYGOPosition - 包含正反面和攻守表示)
	FRotator LocalRotation = GetLocalRotationForPosition(CardInstance.Position);

	// 3. 獲取堆疊偏移 (基於 Zone 類型和 StackIndex)
	FVector LocalOffset = CurrentZone->GetCardLocalOffset(StackIndex);

	// 4. 組合成最終 Transform
	// 旋轉 = Zone 旋轉 + 本地旋轉
	TargetRotation = (ZoneTransform.GetRotation() * LocalRotation.Quaternion()).Rotator();

	// 位置 = Zone 位置 + (Zone 旋轉後的本地偏移)
	TargetLocation = ZoneTransform.TransformPosition(LocalOffset);

	bIsMoving = true;

	// UE_LOG(LogTemp, Log, TEXT("[CardActor] %s\n  StackIndex: %d\n  ZoneRot: %s\n  LocalRot: %s\n  FinalRot: %s\n  LocalOffset: %s\n  FinalLoc: %s"),
	// 	   *CardInstance.CardData.CardName,
	// 	   StackIndex,
	// 	   *ZoneTransform.GetRotation().Rotator().ToString(),
	// 	   *LocalRotation.ToString(),
	// 	   *TargetRotation.ToString(),
	// 	   *LocalOffset.ToString(),
	// 	   *TargetLocation.ToString());
}

void AYGOCardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving)
	{
		return;
	}

	// 插值移動到目標位置
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = FMath::VInterpConstantTo(
		CurrentLocation, TargetLocation, DeltaTime, MovementSpeed);
	SetActorLocation(NewLocation);

	// 插值旋轉到目標旋轉
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpConstantTo(
		CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
	SetActorRotation(NewRotation);

	// 檢查是否到達目標
	bool bReachedLocation = (NewLocation - TargetLocation).IsNearlyZero(1.0f);
	bool bReachedRotation = NewRotation.Equals(TargetRotation, 1.0f);

	if (bReachedLocation && bReachedRotation)
	{
		bIsMoving = false;
		//UE_LOG(LogTemp, Log, TEXT("[CardActor] %s reached target position"), *CardInstance.CardData.CardName);
	}
}
