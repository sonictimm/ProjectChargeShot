#pragma once
/**
 * This interface is for objects which can be killed, or destroyed
 They all ought to have health, ways to take damage, and a way to handle running out of health
 More info on Interfaces: https://wiki.unrealengine.com/Interfaces_in_C++
 */
///*

#include "CoreMinimal.h"
#include "ObjectMacros.h"
#include "Components.h"
#include "IKillableEntity.generated.h"


UINTERFACE(BlueprintType)
class SURVIOS_2019_01_API UIKillableEntity : public UInterface
{
	GENERATED_BODY()
};
class SURVIOS_2019_01_API IIKillableEntity
{
	GENERATED_BODY()
protected:
	float health;
	//incinvibility timer
	FTimerHandle invincibilityTimer;
public:
	float maxHealth;
	bool invincible;
	float invincibilityTime;	//how long after a hit until it becomes vulnerable again?  Never set to zero, else overlapping for 1 second can cause 60 instances of damage

protected:
	//after invincibility timer runs out
	virtual void EndInvincibility() = 0;
public:


	//Set and get health
	virtual float GetHealth() = 0;
	virtual void SetHealth(float newHealth) = 0;

	//Handle Damage.  Usually calls SetHealth
	virtual void TakeDamage(float damage) = 0;

	//Initiate Death Sequence
	virtual void DeathSequence() = 0;

	//Handles hit/collision events
	UFUNCTION()
	virtual void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) = 0;
};
//*/