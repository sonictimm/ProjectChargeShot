// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IKillableEntity.h"
#include "Survios_2019_01Character.generated.h"


	/*
	Enums for input modes.
	FullControl: Full normal control of character
	ChargeMode: For use while charging a charge attack
	Cameraonly: Player can move camera, but not character.  May be used in cutscenes
	NoJumping: Character can run around, but not jump, as in some games while in towns
	NoControl: Player cannot affect character with input.
	*/
UENUM(BluePrintType)
enum class InputMode : uint8
{
	FullControl UMETA(DisplayName = "Full Control"),
	ChargeMode UMETA(DisplayName = "Charge Mode"),
	CameraOnly UMETA(DisplayName = "Camera Only"),
	NoJumping UMETA(DisplayName = "No Jumping"),
	NoControl UMETA(DisplayName = "No Control")
};

UCLASS(config=Game)
class ASurvios_2019_01Character : public ACharacter, public IIKillableEntity
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	//Original running speed of character.  It's far easier & less invasive to set it here than to mess with the engine's settings in Character.cpp
	float originalRunSpeed;

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
	float LStickX;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
	float LStickY;

	//health and dmg stuff
	UPROPERTY(VisibleAnywhere, Category=HealthDamage)
	float health;

	//high score data
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Score)
	float score;
	//high score data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Score)
	int combo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Score)
	float pointsPerKill;

	//invincibility timer for when damaged
	FTimerHandle invincibilityTimer;
	//Reflector timer
	FTimerHandle reflTimer;
	//Handle respawning
	FTimerHandle respawnTimer;

	//Reflecting Projectiles
	UPROPERTY(VisibleAnywhere, Category = Abilities)
	bool refl;

public:
	ASurvios_2019_01Character();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	//Turn Rate Modifier
	UPROPERTY(EditAnywhere, Category = Camera)
	float TurnRateMultiplier;

	//Z-Velocities to tilt camera at its max:  (NOT IMPLEMENTED) 
	UPROPERTY(EditAnywhere, Category = Camera)
	float MaxCamTiltRiseSpeed;
	UPROPERTY(EditAnywhere, Category = Camera)
	float MaxCamTiltFallSpeed;

	//How fast should character be going up when jump button is released?
	UPROPERTY(EditAnywhere, Category = Input)
	float hatZVelocity;

	//How high should they bounce when they hit an enemy head?
	UPROPERTY(EditAnywhere, Category = Input)
	float bounceZVelocity;

	//Should Camera automagically rotate on X axis?
	UPROPERTY(EditAnywhere, Category = Camera)
	bool bCamAutoRotateX;

	//When running for a while, what is the max character movement speed?
	UPROPERTY(EditAnywhere, Category = Input)
	float maxRunSpeed;

	//input mode: How much control of character does player have?
	UPROPERTY(EditAnywhere, Category = Input)
	InputMode iMode;

	//health and dmg stuff
	UPROPERTY(EditAnywhere, Category = HealthDamage)
	float maxHealth;
	UPROPERTY(BluePrintReadWrite, Category = HealthDamage)
	bool invincible;
	UPROPERTY(EditAnywhere, Category = HealthDamage)
	float invincibilityTime;
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = HealthDamage)
	float respawnTime;
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = HealthDamage)
	AActor *spawnPoint;

	//Abilities
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = Abilities)
	bool reflEnable;
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = Abilities)
	float reflCooldown;
	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = Abilities)
	float reflDuration;


protected:
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/*
	Manage input for jumping and ending jump
	*/
	void JumpInput();
	void StopJumpingInput();
	void CapJumping();

	//input for abillities
	void StartReflInput();

	//called for Forward/Backward and Side-to-side input
	void MoveForwardInput(float Value);
	void MoveRightInput(float Value);

	/** Called for forwards/backward movement of character*/
	void MoveForward(float Value);

	/** Called for side to side movement of character*/
	void MoveRight(float Value);

	//Handle below turn and lookup functions:
	void TurnAtRateInput(float Rate);
	void LookUpAtRateInput(float Rate);

	//Toggle camera mode
	void CameraModeInput();

	//handle Pause
	void SetGamePausedInput();

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	//for IIkillableEntity
	UFUNCTION()
	void EndInvincibility();


public:

	//Start of game:	
	UFUNCTION(BluePrintCallable, Category="BasicFunctions")
	virtual void BeginPlay() override;

	//Tick events:
	UFUNCTION(BluePrintCallable, Category = "BasicFunctions")
	virtual void Tick(float DeltaTime) override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	//Toggles or returns camera auto-rotation
	bool CameraMode();
	bool CameraMode(bool mode);

	//set pause
	void SetGamePaused(bool bIsPaused);
	//Handles hit/collision events. 
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	//Overlaps are handles like with impulse of 0
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	//Handle Collision with other characters
	UFUNCTION(BlueprintCallable, Category="Interactions")
	void CharacterHit(AActor* actor);

	//Handle Collision with Colliders
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void DamagerHit(AActor* actor);

	//Bounce off an enemy when we touch it
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void EnemyBounce();
	
	//Get and set score
	UFUNCTION(BlueprintCallable, Category = "Interactions")
		float GetScore() { return score; };
	//for editing value
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	float AddPoints(float points);
	//For an enemy jump:
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void AddPointsForJump();

	//Handle Damage & Death.   From Interface
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	void TakeDamage(float damage);
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	void Respawn();
	void DeathSequence();
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	float GetHealth() { return health; };
	UFUNCTION(BlueprintCallable, Category = "HealthDamage")
	void SetHealth(float newHealth);

	//Reflector Functions.  Reflector makes character immune to damage for a short time, and sends projectiles (damagers with displacement) backwards.
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void StartRefl();
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void EndRefl();
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void ResetRefl();
	//Is reflector activated now
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool GetRefl() { return refl; };
};

