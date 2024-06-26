// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Sockets.h"
#include "MyActor.generated.h"

UCLASS()
class KUANGMAIN_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		bool S_Connect(FString IP, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		bool R_Connect(FString IP, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		void SendMessage();

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		FString ReceiveMessage();

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		void MyPrint(FString meaasge);

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		void CloseConnection();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param4;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param6;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param7;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param8;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param9;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TCP Client")
		FString Thread1Param11;
public:
		UPROPERTY(BlueprintReadWrite, Category = "My Category")
		TArray<int32> MyIntArray;

		void T_Myprint();

private:
	FSocket* SendSocket;
	FSocket* ReceiveSocket;

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		void StopThreads();

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		void StartThreads_Send();

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		void StartThreads_Recv();

	UFUNCTION(BlueprintCallable, Category = "TCP Client")
		void Threads_Printf();

	bool bIsRunning;
	bool RecvRunning;
	bool Send_SocketBIsStop;
	bool Recv_SocketBIsStop;
	FString result;

};

