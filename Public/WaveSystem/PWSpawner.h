// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/PWSpawnDatas.h"
#include "GameFramework/Actor.h"
#include "MassSpawner.h"
#include "PWSpawner.generated.h"


UCLASS(Blueprintable)
class PROJECTWATER_API APWSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APWSpawner();

	APWEnemyCharacter* SpawnAEnemy(TSubclassOf<AActor> Enemy);
	
	void SpawnMassEnemy(TSubclassOf<UDataAsset> EnemyMassDA);

	/**
	 * Datas Used to Spawn Enemies
	 */
	UPROPERTY(EditAnywhere,Category="Spawn Datas")
	TObjectPtr<UPWSpawnDatas> SpawnInfos;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SetupMass();
	
	TObjectPtr<AMassSpawner> MassSpawner;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> SpawnNiagaraSystem;

	UPROPERTY(EditAnywhere)
	float ZOffsetSpawnParticle = 25.f;
	
};
