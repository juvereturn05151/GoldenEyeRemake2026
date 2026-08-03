#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWATProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class GOLDENEYEFINAL_API ASWATProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASWATProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SWAT|Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SWAT|Projectile")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SWAT|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Projectile", meta = (ClampMin = "0.0"))
	float Damage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Projectile", meta = (ClampMin = "0.0"))
	float InitialSpeed = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Projectile", meta = (ClampMin = "0.0"))
	float MaxSpeed = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Projectile", meta = (ClampMin = "0.0"))
	float LifeSpanSeconds = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Projectile", meta = (ClampMin = "0.0"))
	float CollisionRadius = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SWAT|Debug")
	bool bDebugProjectileLogs = false;

private:
	UFUNCTION()
	void HandleImpact(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	void ApplyProjectileSettings();
	void IgnoreOwnerAndInstigator();
	void ApplyDamageToActor(AActor* OtherActor);

	bool bHasAppliedDamage = false;
};
