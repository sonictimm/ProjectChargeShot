//WARNING: This code is unstable and may crash the editor.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Damager.h"
#include "TimerManager.h"
#include "Weapon.generated.h"

//enun for weapon type
UENUM(BluePrintType)
enum class WeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	Melee UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged")
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIOS_2019_01_API UWeapon : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = HealthDamage)
	bool fireEnabled;
	//needs a get and a set.  Nah, just use public functions whiich can be called by enemy AI
	//Function: fireEnable
	/*
	if melee: fires once
	if ranged:
	Starts a timer handle that loops and runs the fire function
	
	Fire function only runs if fireActivated is true
	makes a projectile in the place with desired stuff (melee: displacement = 0)		//do ranged first.
	*/
	//function: fireDisable
	/*
	Ends the timer handle
	disables fireActivated
	if melee: destroy damager?  
	*/

public:	
	// Sets default values for this component's properties
	UWeapon();

	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh *mesh;	//set this in the blueprint to whatever, don't set it here.
	*/

	//melee ranged or none?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	WeaponType wType;

	//Type of damager to shoot / use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass *damager;
	//TSubclassOf<class ADamager> damager;

	//for both ranged and melee
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HealthDamage)
	float damage;
	UPROPERTY(EditAnywhere, BluePrintReadWrite)
	USceneComponent* spawnPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HealthDamage)
	bool friendlyFireEnabled;
	
	//Attributes for ranged weapons
	//Persistence
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=RangedWeapons)
	float rPersistence;
	//Rate of Fire: Seconds between shots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RangedWeapons)
	float rRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RangedWeapons)
	FVector rDisplacement;
	UPROPERTY()
	FTimerHandle wTimerHandle;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//actovate weapon by spawning a damager
	UFUNCTION()
	void Fire();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//enable firing
	UFUNCTION(BlueprintCallable)
	void FireEnable();
	UFUNCTION(BlueprintCallable)
	void FireDisable();
	//Fire one shot without enabling repeat fire
	UFUNCTION(BlueprintCallable)
	void FireOnce();
	UFUNCTION(BlueprintCallable)
	bool IsFiring();
};
