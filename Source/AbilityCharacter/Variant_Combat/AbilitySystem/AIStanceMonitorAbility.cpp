// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIStanceMonitorAbility.h"

UAIStanceMonitorAbility::UAIStanceMonitorAbility()
{
	// AI characters watch for the player
	HostileActorTag = FName("Player");
}
