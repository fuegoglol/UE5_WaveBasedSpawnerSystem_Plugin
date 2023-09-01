// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PWWaveData.h"
#include "Characters/Enemies/PWEnemyCharacter.h"
#include "PWSpawnDatas.generated.h"

USTRUCT()
struct FSpawnInfo
{
 
	GENERATED_USTRUCT_BODY()

	/**
	 * Main Waves informations from Wave 1 to End Game Waves
	 */
	UPROPERTY(EditAnywhere, meta=(EditFixedOrder))
	TArray<FWaveInfo> WavesInfos;

	/**
	 * End Game Waves that looping after Main Waves are done
	 */
	UPROPERTY(EditAnywhere, meta=(EditFixedOrder), Category="EndGame")
	TArray<FWaveInfo> EndGameWavesInfos;

	/*
	 * How much the enemies should increase per end game wave
	 */
	UPROPERTY(EditAnywhere, Category="EndGame")
	int EnemiesNumberIncreasePerWave = 1;
	

};

/**
 * 
 */
UCLASS()
class PROJECTWATER_API UPWSpawnDatas : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FSpawnInfo Spawns;
};
