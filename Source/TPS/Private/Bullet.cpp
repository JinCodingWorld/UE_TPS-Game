// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//1.충돌체를 Root Component로 만들고 반지름은 12.5f로 설정한다.
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->SetSphereRadius(12.5f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAll"));

	//2. 외관을 충돌체에 붙이고 크기는 0.5f로 설정한다.
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetRelativeScale3D(FVector(0.05f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//3. Move Component를 만들고 속력과 바운스를 설정한다. 
	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MoveComp"));
	MoveComp->InitialSpeed = 3000.f;
	MoveComp->MaxSpeed = 3000.f;
	MoveComp->bShouldBounce = true;
	MoveComp->Bounciness = .3f;
	// 총알 인스턴스 생명 주기
	//InitialLifeSpan = 2.0f;

	MoveComp->SetUpdatedComponent(CollisionComp);
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle DeathTimer;
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &ABullet::Die, 2.0f, false);
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABullet::Die()
{
	Destroy();
}

