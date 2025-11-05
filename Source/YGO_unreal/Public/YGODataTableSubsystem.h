#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "YGODataTableSubsystem.generated.h"

UCLASS(Blueprintable, BlueprintType)
class YGO_UNREAL_API UYGODataTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DataTable")
	UDataTable *GetDataTable(FName TableName) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DataTable")
	TArray<FName> GetAllDataTableNames() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DataTable")
	bool HasDataTable(FName TableName) const;

private:
	UPROPERTY()
	TMap<FName, UDataTable *> LoadedDataTables;

	void LoadAllDataTablesFromFolder();
};
