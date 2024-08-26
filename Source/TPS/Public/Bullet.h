// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class TPS_API ABullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 충돌체, 외관, 발사체, 이동
	UPROPERTY(EditAnywhere)
	class USphereComponent* CollisionComp;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* MoveComp;

	//총알 제거 함수
	void Die();

	//void SetTimer
	//{
	//	FTimerHandle & InOutHandle // 등록할 알람 시계
	//	UserClass * InObj         // 알림 처리를 갖고 있는 객체 
	//	typename FTimerDelegate::TUObjectMethodDelegate<UserClass>::FMethodPtr InTimerMethod, // 알림 처리 함수 
	//	float InRate			// 알람 시간
	//	bool InbLoop			// 반복 여부 
	//	float InFirstDelay		// 처음 호출되기 전 지연 시간
	//}
};
