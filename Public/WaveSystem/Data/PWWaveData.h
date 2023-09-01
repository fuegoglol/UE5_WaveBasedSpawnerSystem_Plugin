// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PWEnemyData.h"
#include "Creator/PWSpiderEgg.h"
//#include "Destructor/Pumpkin/PWDestructiblePumpkin.h"
#include "PWWaveData.generated.h"

class APWDestructiblePumpkin;
class UPWWaveData;

USTRUCT(BlueprintType)
struct FWaveInfo
{
	GENERATED_USTRUCT_BODY()

	/**
	 * Should use a prefab Data Asset
	 */
	UPROPERTY(EditAnywhere)
	bool bOverrideData = false;

	/**
	 * prefab Data to use
	 */
	UPROPERTY(EditAnywhere,meta=(EditCondition="bOverrideData"))
	TObjectPtr<UPWWaveData> WaveData;

	/*
	 * Is this wave a nightmare?
	 */
	UPROPERTY(EditAnywhere, meta=(EditCondition="!bOverrideData"))
	bool bIsNightmare = false;
	
	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(Units="Seconds",EditCondition="!bOverrideData && bIsNightmare"))
	float DelayBeforeNightmareStarts = 10;

	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(EditCondition="!bOverrideData && bIsNightmare"))
	int NbPumpkin = 1;

	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(Units="Percent",EditCondition="!bOverrideData && bIsNightmare"))
	int PumpkinBossHealthPercentage = 50;

	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(EditCondition="!bOverrideData && bIsNightmare"))
	TSubclassOf<APWDestructiblePumpkin> PumpkinToSpawn;

	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(EditCondition="!bOverrideData && bIsNightmare"))
	int NbEggs = 1;

	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(EditCondition="!bOverrideData && bIsNightmare"))
	TSubclassOf<APWSpiderEgg> EggToSpawn;

	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(EditCondition="!bOverrideData && bIsNightmare"))
	float BossHealth = 1000;

	UPROPERTY(EditAnywhere,Category="Nightmare", meta=(EditCondition="!bOverrideData && bIsNightmare"))
	TSubclassOf<APWEnemyCharacter> BossToSPawn = nullptr;

	/*
	 * Wave Number
	 */
	UPROPERTY(EditAnywhere, meta=(EditCondition="!bOverrideData"))
	int Wave = 1;

	/*
	 * If true, even if the Wave Spawner manger override the delay between waves, this wave will start after DelayBeforeStarts
	 */
	UPROPERTY(EditAnywhere, meta=(EditCondition="!bOverrideData"))
	bool bForceUseTheDelayBeforeStarts = false;

	/*
	 * Delay before the Wave starts
	 */
	UPROPERTY(EditAnywhere,meta=(ClampMin=0,Units="Seconds",EditCondition="!bOverrideData"))
	float DelayBeforeStarts = 1;

	/*
	 * Enemies Spawn Info for the Wave
	 */
	UPROPERTY(EditAnywhere, meta=(EditFixedOrder,EditCondition="!bOverrideData"))
	TArray<FEnemyInfo> EnemiesInfos;
	
};

/**
 * 
 */
UCLASS()
class PROJECTWATER_API UPWWaveData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FWaveInfo Wave;
};
