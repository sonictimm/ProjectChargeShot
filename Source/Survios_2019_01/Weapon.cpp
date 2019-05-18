// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

// Sets default values for this component's properties
UWeapon::UWeapon()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//default mesh is empty
	//mesh = CreateDefaultSubobject<UStaticMesh>(TEXT("StaticMeshComponent"));	//Let's just not mess with this, waaay easier to add in BP as a component

	//Type of weapon:
	wType = WeaponType::None;
	
	//set default attributes
	fireEnabled = false;
	damage = 10;
	spawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPointComponent"));		//this is supposed to be a scene component where the thing spawns, but I can't find it...
			//Maybe make it a reference to a local component, and then set the component per BP?
	spawnPoint->SetRelativeTransform(FTransform());
	friendlyFireEnabled = false;

	//Attributes for ranged weapons
	rPersistence = 1;
	rRate = 1.f;
	rDisplacement = FVector(0.f,0.f,30.f);


	//for later:
	//GetOwner();	//gets ownign actor of this component
}


// Called when the game starts
void UWeapon::BeginPlay()
{
	Super::BeginPlay();

	
	// ...
	
}

void UWeapon::Fire()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("FIYER! function called in Weapon.h")));	//for debug

	if (damager != NULL)
	{
		//Old code:
		{
		/*
		FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FTransform transform = FTransform();
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Item should be spawned...")));	//for debug
		*/

		//GetWorld()->SpawnActor<ADamager>(damager->GetClass(), transform, spawnParams);
		//LogSpawn: Warning: SpawnActor failed because BlueprintGeneratedClass is not an actor class
		/*
		Syntax:
		 AActor* SpawnActor( UClass* InClass, FVector const* Location, FRotator const* Rotation, const FActorSpawnParameters& SpawnParameters );
		 AActor* SpawnActor( UClass* Class, FTransform const* Transform, const FActorSpawnParameters& SpawnParameters));
		*/
		//Still no dice, switching to this method:
		//https://docs.unrealengine.com/en-us/Programming/UnrealArchitecture/Actors/Spawning
		/*	
		AActor* UWorld::SpawnActor
		(
			UClass*         Class,
			FName           InName,
			FVector const*  Location,
			FRotator const* Rotation,
			AActor*         Template,
			bool            bNoCollisionFail,
			bool            bRemoteOwned,
			AActor*         Owner,
			APawn*          Instigator,
			bool            bNoFail,
			ULevel*         OverrideLevel,
			bool            bDeferConstruction
		)
		*/
		}	
		AActor *newActor  =  GetWorld()->SpawnActor(damager);	//Wow, is that all it takes?		//YEET, all other params have defaults/
		//Set attributes for new actor
		newActor->SetActorTransform(spawnPoint->GetComponentToWorld());

		ADamager *newDamager = Cast<ADamager>(newActor);
		if (newDamager != nullptr)
		{
			newDamager->damage = damage;
			newDamager->damagerOwner = GetOwner();
			newDamager->persistence = rPersistence;
			newDamager->displacement = rDisplacement;
			
			//&newDamager = ADamager(GetOwner(), damage, rPersistence, rDisplacement, friendlyFireEnabled);
			//Unfortunately I can't dereference it, so default constructor can't simply be called.  There's definitely gotta be a better way.
			//Maybe I can make a new one and change the pointer to it, then delete the old one...  But this works fine, so I'll keep it for now.
		}
	}
	
}


// Called every frame
void UWeapon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
void UWeapon::FireEnable()
{
	fireEnabled = true;
	if (wType == WeaponType::Melee)
		Fire();
	else if (wType == WeaponType::Ranged)
	{
		Fire();
		//Looping timer to keep firing
		GetWorld()->GetTimerManager().SetTimer(wTimerHandle, this, &UWeapon::Fire, rRate, true, -1.f);
	}

}
void UWeapon::FireDisable()
{
	fireEnabled = false;
	//end the looping timer
	GetWorld()->GetTimerManager().ClearTimer(wTimerHandle);
}
void UWeapon::FireOnce()
{
	Fire();
}
bool UWeapon::IsFiring()
{
	return fireEnabled;
}
//just gonna put this here: spawnDamager
// 	ADamager(AActor* owner, float damage = 10.f, int persistence = 1, FVector displacement = FVector(0.f, 0.f, 0.f), bool friendlyFire = false);
//should spawn once ever if melee, should spawn forever so long as S

