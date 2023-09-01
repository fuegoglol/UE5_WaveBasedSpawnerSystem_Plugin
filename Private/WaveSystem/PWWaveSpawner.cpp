// Fill out your copyright notice in the Description page of Project Settings.


#include "WaveSystem/PWWaveSpawner.h"

//#include "FunctionalUIScreenshotTest.h"
#include "Bars/HB_HealthBar.h"
#include "Characters/Player/PWPlayerCharacter.h"
#include "Destructor/Pumpkin/PWDestructiblePumpkin.h"
#include "GameInstance/PWGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "WaveSystem/Data/PWEnemyData.h"
#include "WaveSystem/Data/PWWaveData.h"


// Sets default values
APWWaveSpawner::APWWaveSpawner()
{
	// Deactivate Tick for obvious reasons
	PrimaryActorTick.bCanEverTick = false;

	// bind delegates
	OnNightmareStartDelegate.AddDynamic(this,&ThisClass::OnNightmareStart);
	OnNightmareEndDelegate.AddDynamic(this,&ThisClass::OnNightmareEnd);
}

void APWWaveSpawner::RemoveCurrentSpawnedActor(int amount)
{
	currentSpawnedActors -= amount;
}

void APWWaveSpawner::OnPumpkinDestroyed(float AmountOfBossHealth)
{
	PumpkinsBossHealthRetrieve += AmountOfBossHealth;
	OnPumpkinDestroyedDelegate.Broadcast();
}

// Called when the game starts or when spawned
void APWWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	// start the waves
	SpawnWave(CurrentWave);

	Players.SetNum(GetWorld()->GetNumPlayerControllers());
	for(int i=0; i<Players.Num();i++)
	{
		APWPlayerCharacter* Player = Cast<APWPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(),i));
		if(Player)
			Players.EmplaceAt(i,Player);
	}

	SpiderSpawner = GetWorld()->SpawnActor<APWGridSpawner>(SpawnerClass,FVector::ZeroVector,FRotator::ZeroRotator);

	SpiderSpawner->OnActorsSpawnedDelegate.AddDynamic(this,&ThisClass::OnSpiderEggSpawned);

	PumpkinSpawner = GetWorld()->SpawnActor<APWGridSpawner>(SpawnerClass,FVector::ZeroVector,FRotator::ZeroRotator);

	PumpkinSpawner->OnActorsSpawnedDelegate.AddDynamic(this,&ThisClass::OnPumpkinSpawned);

	GridSpawner = GetWorld()->SpawnActor<APWGridSpawner>(SpawnerClass,FVector::ZeroVector,FRotator::ZeroRotator);

	GridSpawner->OnActorsSpawnedDelegate.AddDynamic(this,&ThisClass::OnGridSpawned);


	FountainQueryRequest = FEnvQueryRequest(FountainQuery, this);
	FountainQueryRequest.Execute(EEnvQueryRunMode::AllMatching, this, &ThisClass::FountainQueryFinished);

	BarMaterialInstance = UMaterialInstanceDynamic::Create(BarMaterialInterface,this);
	BarMaterialInstance->SetScalarParameterValue(FName("Percentage"),(float)(0.f));
}

void APWWaveSpawner::SpawnWave(int Wave)
{
	bIsNightmareWave = false;
	
	//check if this wave have nightmares
	for(const auto& Spawner : Spawns)
	{
		if(!Spawner) break;
		const auto WavesInfos = Spawner->SpawnInfos->Spawns.WavesInfos;

		bool bIsThereANightmare = false;

		FWaveInfo WaveInfo;

		// Common Waves
		if(WavesInfos.IsValidIndex(Wave))
		{
			// Get the infos about the wave
			WaveInfo = WavesInfos[Wave];

			// If the Wave Info is override, take the overriding Data
			if(WaveInfo.bOverrideData) WaveInfo = WaveInfo.WaveData->Wave;

			if(WaveInfo.bIsNightmare)
			{
				bIsThereANightmare = true;
				DelayBeforeNightmareStarts = WaveInfo.DelayBeforeNightmareStarts;
			}
			
		}
		else
		{
			const auto EndWavesInfos = Spawner->SpawnInfos->Spawns.EndGameWavesInfos;

			checkf(!EndWavesInfos.IsEmpty(),TEXT("End Game Waves is Ill formed, it should implement at least one wave!"));

			const int WaveNum = (Wave-WavesInfos.Num())%EndWavesInfos.Num();
			
			// Get the infos about the wave
			WaveInfo = EndWavesInfos[WaveNum];

			// If the Wave Info is override, take the overriding Data
			if(WaveInfo.bOverrideData) WaveInfo = WaveInfo.WaveData->Wave;

			if(WaveInfo.bIsNightmare)
			{
				bIsThereANightmare = true;
				DelayBeforeNightmareStarts = WaveInfo.DelayBeforeNightmareStarts;
			}
		}


		if(bIsThereANightmare)
		{
			bIsNightmareWave = true;
			if(!bIsNightmareWaveRunning)
			{
				BossHealth = WaveInfo.BossHealth;
				// Pumpkins
				float PumpkinTotalPercentage = WaveInfo.PumpkinBossHealthPercentage;
				PumpkinsBossHealth = (BossHealth*(PumpkinTotalPercentage/100))/WaveInfo.NbPumpkin;
				PumpkinSpawner->Execute(WaveInfo.PumpkinToSpawn,WaveInfo.NbPumpkin,true);

				//Spiders
				SpiderSpawner->Execute(WaveInfo.EggToSpawn,WaveInfo.NbEggs,true);
				OnNightmareStartDelegate.Broadcast();
				bIsNightmareWaveRunning = true;

				// Broadcast global event for NPC reaction
				if (GlobalEventDispatcher != nullptr)
					GlobalEventDispatcher->OnNightmareWaveStarted.Broadcast();
			}
			break;
		}
	}
	
	// iterate through all spawners
	for(const auto& Spawner : Spawns)
	{
		if(!Spawner) break;
		const auto WavesInfos = Spawner->SpawnInfos->Spawns.WavesInfos;

		// Common Waves
		if(WavesInfos.IsValidIndex(Wave))
		{
			// Get the infos about the wave
			auto WaveInfo = WavesInfos[Wave];

			// If the Wave Info is override, take the overriding Data
			if(WaveInfo.bOverrideData) WaveInfo = WaveInfo.WaveData->Wave;

			// Set the progress bar capacity of the spawning wave
			EnemyMaxInWave+=WaveInfo.EnemiesInfos.Num();
			EnemyStillInWave+=WaveInfo.EnemiesInfos.Num();
		
			// Get the delay to wait before Starting The first Wave
			float DelayBeforeSpawn = WaveInfo.DelayBeforeStarts;
			if(bOverrideDelayBetweenWaves && !WaveInfo.bForceUseTheDelayBeforeStarts)
				DelayBeforeSpawn = DelayBetweenWaves;

			// Override the delay if nightmare
			if(bIsNightmareWaveRunning /*&& !WaveInfo.bIsNightmare*/)
				DelayBeforeSpawn = DelayBeforeNightmareStarts;


			// create Timer to Start the Wave
			FTimerHandle InTimerHandle = AddTimerHandle();
			
			if(DelayBeforeSpawn == 0)
			{
				StartWave(InTimerHandle, Spawner, WaveInfo);
			}
			else
			{
				FTimerDelegate Delegate;
				Delegate.BindUFunction(this, "StartWave",InTimerHandle, Spawner, WaveInfo);

				// Start Timer
				GetWorld()->GetTimerManager().SetTimer(InTimerHandle, Delegate, DelayBeforeSpawn, false);
			}
			
		}
		else
		// End Game Waves
		{
			const auto EndWavesInfos = Spawner->SpawnInfos->Spawns.EndGameWavesInfos;

			checkf(!EndWavesInfos.IsEmpty(),TEXT("End Game Waves is Ill formed, it should implement at least one wave!"));

			const int WaveNum = (Wave-WavesInfos.Num())%EndWavesInfos.Num();
			
			// Get the infos about the wave
			auto WaveInfo = EndWavesInfos[WaveNum];

			// If the Wave Info is override, take the overriding Data
			if(WaveInfo.bOverrideData) WaveInfo = WaveInfo.WaveData->Wave;

			// Set the progress bar capacity of the spawning wave
			EnemyMaxInWave+=WaveInfo.EnemiesInfos.Num();
			EnemyStillInWave+=WaveInfo.EnemiesInfos.Num();

			// update wave info to match with EnemiesNumberIncreasePerWave infos
			const int IncreaseAmount = (Wave-WavesInfos.Num())*Spawner->SpawnInfos->Spawns.EnemiesNumberIncreasePerWave;
			for(auto &EnemyInfo : WaveInfo.EnemiesInfos)
			{
				EnemyInfo.MinSpawnAmount += IncreaseAmount;
				EnemyInfo.MaxSpawnAmount += IncreaseAmount;
			}
				
		
			// Get the delay to wait before Starting The first Wave
			float DelayBeforeSpawn = WaveInfo.DelayBeforeStarts;
			if(bOverrideDelayBetweenWaves && !WaveInfo.bForceUseTheDelayBeforeStarts)
				DelayBeforeSpawn = DelayBetweenWaves;

			// Override the delay if nightmare
			if(bIsNightmareWaveRunning /*&& !WaveInfo.bIsNightmare*/)
				DelayBeforeSpawn = DelayBeforeNightmareStarts;

			// create Timer to Start the Wave
			FTimerHandle InTimerHandle = AddTimerHandle();
			
			if(DelayBeforeSpawn == 0)
			{
				StartWave(InTimerHandle, Spawner, WaveInfo);
			}
			else
			{
				FTimerDelegate Delegate;
				Delegate.BindUFunction(this, "StartWave",InTimerHandle, Spawner, WaveInfo);

				// Start Timer
				GetWorld()->GetTimerManager().SetTimer(InTimerHandle, Delegate, DelayBeforeSpawn, false);
			}
		}
	}
	// Give free water storage before the beginning of a new wave for all players
	// RefillWaterForAllPlayer();
}

void APWWaveSpawner::OnNightmareStart_Implementation()
{
	ActivateWebOnFountains();
}

void APWWaveSpawner::OnNightmareEnd_Implementation()
{
	DesactivateWebOnFountains();
}

void APWWaveSpawner::StartWave(FTimerHandle& InTimerHandle, APWSpawner* Spawner, FWaveInfo WaveInfo)
{
	UpdateBarMaterial();
	
	// If the Wave Info is override, take the overriding Data
	if(WaveInfo.bOverrideData) WaveInfo = WaveInfo.WaveData->Wave;

	// broadcast nightmare wave
	if(WaveInfo.bIsNightmare)
	{
		//spawn boss
		checkf(WaveInfo.BossToSPawn,TEXT("ENZO!! The nightmare wave must have a boss to spawn referenced!"));
		auto* EnemySpawned = Spawner->SpawnAEnemy(WaveInfo.BossToSPawn);
		if(EnemySpawned)
		{
			OnBossSpawnDelegate.Broadcast(EnemySpawned);
			currentSpawnedActors++;
			EnemySpawned->OnDeathDelegate.AddDynamic(this,&ThisClass::OnActorDestroyed);
			// setup boss health
			EnemySpawned->HealthComp->SetBarValue(WaveInfo.BossHealth-PumpkinsBossHealthRetrieve);
		}
	}
		
	// iterate through all enemies for the current wave
	for(const auto& EnemyInfo : WaveInfo.EnemiesInfos)
	{
		FEnemyInfo CurrentEnemyInfo = EnemyInfo;
		if(EnemyInfo.bOverrideData) CurrentEnemyInfo = CurrentEnemyInfo.EnemyData->Enemy;
			
		// Get the delay to wait before Spawning the Enemy
		const float DelayBeforeSpawn = CurrentEnemyInfo.InitialSpawnDelay;

		// create Timer to Spawn the Enemy
		FTimerHandle TimerHandle = AddTimerHandle();

		ActiveSpawners ++;

		if(DelayBeforeSpawn == 0)
		{
			SpawnEnemy(TimerHandle, Spawner, CurrentEnemyInfo);
		}
		else
		{
			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, "SpawnEnemy", TimerHandle, Spawner, CurrentEnemyInfo,1);

			// Start Timer
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, DelayBeforeSpawn, false);
		}
			
	}

	// clear the current Timer Handle
	RemoveTimerHandle(InTimerHandle);
}

void APWWaveSpawner::SpawnEnemy(FTimerHandle& TimerHandle, APWSpawner* Spawner,FEnemyInfo EnemyInfo,int count)
{
	// Add a New Timer Handle
	FTimerHandle NewTimerHandle = AddTimerHandle();
	
	if(currentSpawnedActors>=MaxEnemiesSpawnedAtTheSameTime)
	{

		// create Timer for the next Spawn
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "SpawnEnemy", NewTimerHandle, Spawner, EnemyInfo, count);

		// Start Timer
		GetWorld()->GetTimerManager().SetTimer(NewTimerHandle, Delegate, 1, false);
		return;
	}
	
	
	APWEnemyCharacter* EnemySpawned = nullptr;
	
	// Depending on the Spawn Behavior, Spawn enemy
	if(!EnemyInfo.bUseMass && !EnemyInfo.bUseGridSpawn)
	{
		EnemySpawned = Spawner->SpawnAEnemy(EnemyInfo.EnemyActor);
		if(EnemySpawned)
			EnemySpawned->OnDeathDelegate.AddDynamic(this,&ThisClass::OnActorDestroyed);
	}
	else if(EnemyInfo.bUseGridSpawn)
	{
		GridSpawner->Execute(EnemyInfo.EnemyActor,1,true);
	}
	else
	{
		Spawner->SpawnMassEnemy(EnemyInfo.EnemyMassDA);
	}

	if(EnemySpawned || EnemyInfo.bUseGridSpawn)
	// increment the number of spawned actors for the current wave
	currentSpawnedActors++;

	float DelayBeforeSpawn;
	
	if(EnemyInfo.MinSpawnDelay == EnemyInfo.MaxSpawnDelay)
	{
		DelayBeforeSpawn = EnemyInfo.MinSpawnDelay;
	}
	else
	{
		DelayBeforeSpawn = FMath::RandRange(EnemyInfo.MinSpawnDelay,EnemyInfo.MaxSpawnDelay);
	}
	

	ActiveSpawners --;
	// clear the current Timer Handle
	RemoveTimerHandle(TimerHandle);

	float MaxCountSpawned;
	
	if(EnemyInfo.MinSpawnAmount == EnemyInfo.MaxSpawnAmount)
	{
		MaxCountSpawned = EnemyInfo.MinSpawnAmount;
	}
	else
	{
		MaxCountSpawned = FMath::RandRange(EnemyInfo.MinSpawnAmount,EnemyInfo.MaxSpawnAmount);
	}

	// if the max count of spawned entities is reached, exit
	if(count >= MaxCountSpawned) return;

	ActiveSpawners ++;

	int NewCount = count;
	if(EnemySpawned || EnemyInfo.bUseGridSpawn) ++NewCount;

	
	
	if(DelayBeforeSpawn == 0)
		SpawnEnemy(NewTimerHandle, Spawner, EnemyInfo, NewCount);
	else
	{
		// create Timer for the next Spawn
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "SpawnEnemy", NewTimerHandle, Spawner, EnemyInfo, NewCount);

		// Start Timer
		GetWorld()->GetTimerManager().SetTimer(NewTimerHandle, Delegate, DelayBeforeSpawn, false);
	}
	
}

void APWWaveSpawner::RefillWaterForAllPlayer()
{
	for(int i=0;i<Players.Num();i++)
	{
		APWPlayerCharacter* Player = Players[i];
		if(Player)
		{
			UPWWaterSystemComponent* WaterComp = Player->GetWaterComponent();
			if(WaterComp)
				WaterComp->StartFreeRefilling();
		}
	}
}

void APWWaveSpawner::FountainQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	TArray<AActor*> ResultArray;
	Result->GetAllAsActors(ResultArray);
	for(auto& ResultItem : ResultArray)
		// Try to get the Actor detected
	{
		// If there's no actors detected, return
		if(!ResultItem) return;
		APWFountain* Fountain = Cast<APWFountain>(ResultItem);
		if (!Fountain) return;
		Fountains.Add(Fountain);
	}
}

void APWWaveSpawner::ActivateWebOnFountains()
{
	for(int i=0;i<Fountains.Num();i++)
	{
		Fountains[i]->ActivateWeb();
	}
}

void APWWaveSpawner::DesactivateWebOnFountains()
{
	for(int i=0;i<Fountains.Num();i++)
	{
		Fountains[i]->DesactivateWeb();
	}
}


void APWWaveSpawner::OnActorDestroyed_Implementation()
{
	// decremente the number of spawned actors for the current wave
	currentSpawnedActors--;
	EnemyStillInWave--;
	UpdateBarMaterial();

	// if there's no more enemies on the map and they have all already spawned
	// starts the next wave
	if(currentSpawnedActors <= 0 && !AreTimersValid())
	{
		// Broadcast global event for NPC reaction
		if (GlobalEventDispatcher != nullptr)
			GlobalEventDispatcher->OnWaveCleared.Broadcast();

		currentSpawnedActors = 0;
		//Calling the event for the widget of the current wave to show up.
		auto* PWPlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if(auto* Player = Cast<APWPlayerCharacter>(PWPlayerCharacter))
					Player->OnWaveChanged();

		if(bIsNightmareWaveRunning)
		{
			if (GlobalEventDispatcher != nullptr)
				GlobalEventDispatcher->OnNightmareWaveEnded.Broadcast();
			OnNightmareEndDelegate.Broadcast();
			bIsNightmareWaveRunning = false;
			PumpkinsBossHealthRetrieve = 0;
		}
		
		++CurrentWave;
		EnemyMaxInWave=0;
		EnemyStillInWave=0;
		SpawnWave(CurrentWave);
		OnNewWaveDelegate.Broadcast(CurrentWave);
		
	}
	
}

FTimerHandle APWWaveSpawner::AddTimerHandle()
{
	const FTimerHandle Timer;
	return Timer;
}

void APWWaveSpawner::RemoveTimerHandle(FTimerHandle& THandle)
{
	THandle.Invalidate();
}

bool APWWaveSpawner::AreTimersValid() const
{
	// check if empty
	return ActiveSpawners > 0;

}

void APWWaveSpawner::OnSpiderEggSpawned(TArray<AActor*> ActorsSpawned)
{
	for(auto * Actor : ActorsSpawned)
	{
		if(!Actor) continue;
		auto * Egg = Cast<APWSpiderEgg>(Actor);
		Egg->SetIncubationTime(DelayBeforeNightmareStarts);
		Egg->StartIncubation();
		Egg->OnSpiderEggHatchDelegate.AddDynamic(this,&ThisClass::OnEggHatch);
	}
}

void APWWaveSpawner::OnPumpkinSpawned(TArray<AActor*> ActorsSpawned)
{
	for(auto * Actor : ActorsSpawned)
	{
		auto * Pumpkin = Cast<APWDestructiblePumpkin>(Actor);
		Pumpkin->Init(this,PumpkinsBossHealth);
		OnNightmareEndDelegate.AddDynamic(Pumpkin,&APWDestructiblePumpkin::APWDestructiblePumpkin::OnEndWave);
	}
}

void APWWaveSpawner::OnGridSpawned(TArray<AActor*> ActorsSpawned)
{
	for(auto & Actor : ActorsSpawned)
	{
		// SpiderEgg
		auto* SpiderEgg = Cast<APWSpiderEgg>(Actor);
		if(SpiderEgg)
		{
			SpiderEgg->StartIncubation();
			SpiderEgg->OnKilledDelegate.AddDynamic(this,&ThisClass::OnEggDestroyed);
			SpiderEgg->OnSpiderEggHatchDelegate.AddDynamic(this,&ThisClass::OnGridEggHatch);
		}
	}
}

void APWWaveSpawner::OnEggDestroyed(AActor* Killer)
{
	OnActorDestroyed();
}

void APWWaveSpawner::OnGridEggHatch(AActor* ActorSpawned)
{
	auto * Spider = StaticCast<APWEnemyCharacter*>(ActorSpawned);
	Spider->OnDeathDelegate.AddDynamic(this,&ThisClass::OnActorDestroyed);
}

void APWWaveSpawner::OnEggHatch(AActor* ActorSpawned)
{
	currentSpawnedActors++;
	auto * Spider = StaticCast<APWEnemyCharacter*>(ActorSpawned);
	Spider->OnDeathDelegate.AddDynamic(this,&ThisClass::OnActorDestroyed);
}

void APWWaveSpawner::UpdateBarMaterial()
{
	BarMaterialInstance->SetScalarParameterValue(FName("Percentage"),(float)(EnemyStillInWave)/EnemyMaxInWave);
}
