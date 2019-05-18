// Fill out your copyright notice in the Description page of Project Settings.

#include "IKillableEntity.h"
/*
While it would be awesome if everything inherited this like in C#, C++ doesn't support that
https://stackoverflow.com/questions/9756893/how-to-implement-interfaces-in-c
You must re-implement this in each thing that inherits from it.
void IIKillableEntity::SetHealth(float newHealth)
{
	//Ensure health is above zero, or kill thing
	if (newHealth <= 0)
	{
		health = 0;
		DeathSequence();
	}
	//ensure health  isn't greater than max
	else if (newHealth > maxHealth)
		health = maxHealth;
	else
		health = newHealth;	//assign new health value
}
*/
