/*
Name: Ju-ve Chankasemporn
E-mail: juvereturn@gmail.com
@2026 MyLoyalFans Productions
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AmmoRefillComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup = (Pickup), meta = (BlueprintSpawnableComponent))
class GOLDENEYEFINAL_API UAmmoRefillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAmmoRefillComponent();

	UFUNCTION(BlueprintCallable, Category = "Pickup|Ammo")
	bool TryRefillAmmo(AActor* PickupActor);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Ammo", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 AmmoAmount = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Ammo", meta = (AllowPrivateAccess = "true"))
	bool bDestroyOwnerAfterPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Ammo", meta = (AllowPrivateAccess = "true"))
	bool bDisableAfterPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Ammo", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPrimitiveComponent> OverlapComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Pickup|Ammo", meta = (AllowPrivateAccess = "true"))
	bool bHasBeenPickedUp = false;

	UFUNCTION()
	void HandleComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPrimitiveComponent* ResolveOverlapComponent() const;
	void CompletePickup();
};
