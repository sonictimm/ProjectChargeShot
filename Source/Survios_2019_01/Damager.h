// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Damager.generated.h"

UCLASS()
class SURVIOS_2019_01_API ADamager : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	bool friendlyFire;

	//If we hit something while ticking
	FHitResult tickHit;

public:	
	// Sets default values for this actor's properties
	ADamager();
	//add another constructor with more: owner, damage, persistence, displacement = 0, friendly fire = false
	ADamager(AActor* dOwner, float damage = 10.f, int persistence = 1, FVector displacement = FVector(0.f, 0.f, 0.f), bool friendlyFire = false);

	//root
	USceneComponent* RootComp;
	//collisionArea
	UCapsuleComponent* CapsuleComponent;

public:	//since there seems tobe an issue assigning this:
	//who fired the shot?
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Damage)
	AActor* damagerOwner;
	//how much damage it deals
	UPROPERTY(BluePrintReadWrite, EditAnywhere, Category=Damage)
	float damage;

	//How many objects should it hit before despawning?  -1 for forever.
	UPROPERTY(BluePrintReadWrite, EditAnywhere, Category = Damage)
	int persistence;

	//How far it should move each tick
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Movement)
	FVector displacement;

	//"Array" of actors it can or can't hit, as a blacklist or a whitelist???
	
private:
	//set up common components
	void InitializeComponents();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//when we hit something
	void HitObject(AActor *other);

	//When we overlap
	//UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//ensure it's not our friend.  False if FF is disabled.
	bool CheckFriendlyFire();

	//when it expires
	UFUNCTION(BlueprintCallable)
	void DeathSequence();

	//Assign stuff:
	UFUNCTION(BlueprintCallable)
	void AssignAttributes(AActor* dOwner, float setDamage = 10.f, int newPersistence = 1, FVector newDisplacement = FVector(0.f, 0.f, 0.f), bool fFire = false);
};
