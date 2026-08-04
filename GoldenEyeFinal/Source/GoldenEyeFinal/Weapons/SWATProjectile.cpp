#include "SWATProjectile.h"

#include "../Components/BondHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASWATProjectile::ASWATProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->OnComponentHit.AddDynamic(
		this,
		&ASWATProjectile::HandleImpact
	);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bIsHomingProjectile = false;

	InitialLifeSpan = LifeSpanSeconds;

	ApplyProjectileSettings();
}

void ASWATProjectile::BeginPlay()
{
	Super::BeginPlay();

	ApplyProjectileSettings();
	IgnoreOwnerAndInstigator();
	SetLifeSpan(LifeSpanSeconds);
	CustomTimeDilation = 1.0f;

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity =
			GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	}

	const FString VelocityString = ProjectileMovement
		? ProjectileMovement->Velocity.ToCompactString()
		: FString(TEXT("None"));

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[SWAT Projectile Init] Projectile=%s Root=%s Movement=%s InitialSpeed=%.2f MaxSpeed=%.2f Collision=%s Hidden=%s LifeSpan=%.2f Velocity=%s"),
		*GetNameSafe(this),
		*GetNameSafe(GetRootComponent()),
		*GetNameSafe(ProjectileMovement),
		ProjectileMovement ? ProjectileMovement->InitialSpeed : 0.0f,
		ProjectileMovement ? ProjectileMovement->MaxSpeed : 0.0f,
		*GetNameSafe(CollisionComponent),
		IsHidden() ? TEXT("true") : TEXT("false"),
		GetLifeSpan(),
		*VelocityString
	);

	if (bDebugProjectileLogs)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[SWAT Projectile] Spawned: %s Owner=%s Instigator=%s"),
			*GetName(),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(GetInstigator())
		);
	}
}

void ASWATProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyProjectileSettings();
}

void ASWATProjectile::HandleImpact(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	if (bDebugProjectileLogs)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[SWAT Projectile] Impact: %s hit %s"),
			*GetName(),
			*GetNameSafe(OtherActor)
		);
	}

	ApplyDamageToActor(OtherActor);
	Destroy();
}

void ASWATProjectile::ApplyProjectileSettings()
{
	if (CollisionComponent)
	{
		CollisionComponent->SetSphereRadius(CollisionRadius);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InitialSpeed;
		ProjectileMovement->MaxSpeed = MaxSpeed;
		ProjectileMovement->ProjectileGravityScale = 0.0f;
		ProjectileMovement->bRotationFollowsVelocity = true;
		ProjectileMovement->bIsHomingProjectile = false;
	}
}

void ASWATProjectile::IgnoreOwnerAndInstigator()
{
	if (!CollisionComponent)
	{
		return;
	}

	if (AActor* ProjectileOwner = GetOwner())
	{
		CollisionComponent->IgnoreActorWhenMoving(ProjectileOwner, true);
	}

	if (APawn* InstigatorPawn = GetInstigator())
	{
		CollisionComponent->IgnoreActorWhenMoving(InstigatorPawn, true);
	}
}

void ASWATProjectile::ApplyDamageToActor(AActor* OtherActor)
{
	if (bHasAppliedDamage || !OtherActor || Damage <= 0.0f)
	{
		return;
	}

	UBondHealthComponent* BondHealthComponent =
		OtherActor->FindComponentByClass<UBondHealthComponent>();

	if (!BondHealthComponent)
	{
		return;
	}

	BondHealthComponent->ApplyDamage(Damage);
	bHasAppliedDamage = true;

	if (bDebugProjectileLogs)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[SWAT Projectile] Damage applied: %s Damage=%.2f"),
			*GetNameSafe(OtherActor),
			Damage
		);
	}
}
