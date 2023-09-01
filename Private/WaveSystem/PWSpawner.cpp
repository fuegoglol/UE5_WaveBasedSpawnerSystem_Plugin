// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveSystem/PWSpawner.h"

#include "Kismet/GameplayStatics.h"


// Sets default values
APWSpawner::APWSpawner()
{
	// deactivate tick for obvious reasons
	PrimaryActorTick.bCanEverTick = false;

	MassSpawner = CreateDefaultSubobject<AMassSpawner>(TEXT("Mass Spawner"));
}

APWEnemyCharacter* APWSpawner::SpawnAEnemy(TSubclassOf<AActor> Enemy)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	// Spawn the Enemy
	UNiagaraComponent * comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SpawnNiagaraSystem, GetActorLocation()+FVector3d(0,0,ZOffsetSpawnParticle));
	return GetWorld()->SpawnActor<APWEnemyCharacter>(Enemy,GetActorLocation(),GetActorRotation(),SpawnParameters);
}

void APWSpawner::SpawnMassEnemy(TSubclassOf<UDataAsset> EnemyMassDA)
{
	
	//MassSpawner->DoSpawning();
}

// Called when the game starts or when spawned
void APWSpawner::BeginPlay()
{
	Super::BeginPlay();

	checkf(SpawnInfos,TEXT("Spawn Infos Data Asset must be set up before playing"));

	// check if an enemy use Mass
	for(auto& WavesInfo : SpawnInfos->Spawns.WavesInfos)
	{
		for(auto& EnemyInfo : WavesInfo.EnemiesInfos)
		{
			if(EnemyInfo.bUseMass)
			{
				SetupMass();
				return;
			}
		}
	}
	for(auto& EndGameWavesInfo : SpawnInfos->Spawns.EndGameWavesInfos)
	{
		for(auto& EnemyInfo : EndGameWavesInfo.EnemiesInfos)
		{
			if(EnemyInfo.bUseMass)
			{
				SetupMass();
				return;
			}
		}
	}
	

	// if we reach here, it means that there's no need of Mass Spawn
	//MassSpawner->Destroy();
	
}

void APWSpawner::SetupMass()
{
	// nothing to specify yet...
}



