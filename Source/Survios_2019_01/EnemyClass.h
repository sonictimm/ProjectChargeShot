// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IKillableEntity.h"
#include "EnemyClass.generated.h"

UCLASS()
class SURVIOS_2019_01_API AEnemyClass : public ACharacter, public IIKillableEntity
{
	GENERATED_BODY()

	//keep track of hit points / health
	UPROPERTY(VisibleAnywhere, Category = HealthDamage)
	float health;

protected:

	//invincibility timer
	FTimerHandle invincibilityTimer;

public:
	// Sets default values for this character's properties
	AEnemyClass();

	//How Much damage to player on contact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HealthDamage)
	float dmgContact;
	
	//How much damage should fired projectiles do to player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HealthDamage)
	float dmgProjectile;

	//Enemy max health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HealthDamage)
	float maxHealth;
	
	//finish up IKillable Interface:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HealthDamage)
	bool invincible;
	UPROPERTY(EditAnywhere, Category = HealthDamage)
	float invincibilityTime;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//for IKillable interface
	UFUNCTION()
	void EndInvincibility();
public:	

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Killable Entity Interface:
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	void SetHealth(float newHealth);
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	float GetHealth() { return health; };
	//Handle Damage
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	void TakeDamage(float damage);
	//Initiate Death Sequence
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	void DeathSequence();

	//Hit stuff
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	//Overlaps are handles like with impulse of 0
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);


};
