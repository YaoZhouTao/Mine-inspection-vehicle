// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "Async/Async.h"



// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bIsRunning = true;
    Send_SocketBIsStop = false;
    Recv_SocketBIsStop = false;
    RecvRunning = true;
    Thread1Param3 = "xu";
    Thread1Param4 = "tao";
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AMyActor::StartThreads_Send() {
    Async(EAsyncExecution::Thread, [=] { SendMessage(); });
}

void AMyActor::StartThreads_Recv() {
    Async(EAsyncExecution::Thread, [=] { ReceiveMessage(); });
}

void AMyActor::Threads_Printf() {
    Async(EAsyncExecution::Thread, [=] { T_Myprint(); });
}

void AMyActor::T_Myprint() {
    int a = 0;
    //printf("��ʼ");
    //UE_LOG(LogTemp, Warning, TEXT("---------------------------------��ʼ"));
    while (bIsRunning) {
        a++;
        float randNum1 = FMath::FRandRange(0.f, 1.f); // ���� 0 �� 1 ֮���С��
        float randNum2 = FMath::FRandRange(0.f, 1.f); // �ٴ����� 0 �� 1 ֮���С��
        randNum1 = FMath::RoundToFloat(randNum1 * 1000000) / 1000000.f; // ȡС�������λ
        randNum2 = FMath::RoundToFloat(randNum2 * 1000000) / 1000000.f;
        //UE_LOG(LogTemp, Warning, TEXT("a = %f, b = %f"), randNum1, randNum2); // ��ӡ���
        FPlatformProcess::Sleep(0.0001);
    }
    UE_LOG(LogTemp, Warning, TEXT("-*********---xu---------%d--------tao---********"), a);
    //printf("**********%d***********",a);
}



bool AMyActor::S_Connect(FString IP, int32 Port)
{
    FString address = IP;
    int32 port = Port;
    TSharedRef<FInternetAddr> remoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    bool isValid;
    remoteAddr->SetIp(*address, isValid);
    remoteAddr->SetPort(port);

    SendSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
    if (SendSocket->Connect(*remoteAddr)) {
        bool connected = SendSocket->Connect(*remoteAddr);
        return true;
    }
    return false;
}

bool AMyActor::R_Connect(FString IP, int32 Port)
{
    FString address = IP;
    int32 port = Port;
    TSharedRef<FInternetAddr> remoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    bool isValid;
    remoteAddr->SetIp(*address, isValid);
    remoteAddr->SetPort(port);

    if (ReceiveSocket)
    {
        ReceiveSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ReceiveSocket);
        ReceiveSocket = nullptr;
    }

    ReceiveSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
    if (ReceiveSocket->Connect(*remoteAddr)) {
        bool connected1 = ReceiveSocket->Connect(*remoteAddr);
        return true;
    }
    return false;
}

void AMyActor::SendMessage()
{
    if (!SendSocket)
    {
        return;
    }

    TArray<uint8> Buffer; // ���建����
    const int32 BufferSize = 1024; // ���û�������С
    FString message0 = "C\n";
    const TCHAR* serializedChar0 = *message0;
    int32 sent0 = 0;
    Buffer.Append((const uint8*)serializedChar0, message0.Len() * sizeof(TCHAR));
    if (SendSocket != nullptr) {
        bool successful = SendSocket->Send(Buffer.GetData(), Buffer.Num(), sent0);
    }
    Buffer.Empty();
    //UE_LOG(LogTemp, Warning, TEXT("---------------------------------send successed"));
    // ���廥����
    //FCriticalSection Mutex;
    int a = 0;
    while (bIsRunning)
    {
        FPlatformProcess::Sleep(0.001);
        a++;
        //UE_LOG(LogTemp, Error, TEXT("%d"), a);*/
        // ����
       // Mutex.Lock();
        result = Thread1Param + Thread1Param1 + Thread1Param2;
        // ����
       // Mutex.Unlock();
        FString message = *result;
        message += "\n"; // ����Ϣ��β��ӷָ���
        const TCHAR* serializedChar = *message;
        int32 size = FCString::Strlen(serializedChar);
        int32 sent = 0;
        for (int32 i = 0; i < size; ++i)
        {
            Buffer.Add(serializedChar[i]);
            // �����ӵ��ַ��Ƿָ��������߻��������������ͻ������е�����
            if (serializedChar[i] == '\n' || Buffer.Num() >= BufferSize)
            {
                
                if (SendSocket != nullptr) {
                    bool successful = SendSocket->Send(Buffer.GetData(), Buffer.Num(), sent);
                    //UE_LOG(LogTemp, Warning, TEXT("---------------------------------send successed"));
                }
                else {
                    break;
                }
                Buffer.Empty(); // ��ջ�����
            }
        }

    }

    UE_LOG(LogTemp, Warning, TEXT("-------------һ��ִ��send%d��--------------------SENDTheard stop!"),a);
    Send_SocketBIsStop = true;
}


FString AMyActor::ReceiveMessage()
{
    if (!ReceiveSocket) // ���SocketΪ�գ�ֱ�ӷ��ؿ��ַ���
    {
        return "";
    }

    uint32 size = 0;

    TArray<uint8> Buffer; // ���建����
    const int32 BufferSize = 1024; // ���û�������С

    FString message0 = "W\n"; // ����һ����Ϣ�Ի�ȡ��������Ӧ
    const TCHAR* serializedChar0 = *message0;
    int32 sent0 = 0;
    Buffer.Append((const uint8*)serializedChar0, message0.Len() * sizeof(TCHAR));
    if (ReceiveSocket != nullptr) {
        bool successful = ReceiveSocket->Send(Buffer.GetData(), Buffer.Num(), sent0); // ������Ϣ��������
    }
    Buffer.Empty(); // ��ջ�����
    FPlatformProcess::Sleep(2); // �ȴ���������Ӧ
    UE_LOG(LogTemp, Error, TEXT("����while"));
    int a = 0;
    while (RecvRunning) // ѭ����������
    {
        a++;
        FString receivedData = ""; // ���ڱ�����յ�������
        FPlatformProcess::Sleep(0.0001); // �ȴ�1����
        //UE_LOG(LogTemp, Error, TEXT("��ʼ��������**********************"));
        char headerData[4]; // ͷ������Ϊ4���ֽڣ����ڴ洢��Ϣ��ĳ���
        int32 headerRead = 0;
       // bool headerReceived = false;
        //UE_LOG(LogTemp, Error, TEXT("while��"));
        while (headerRead < 4) { // ѭ������ͷ�����ݣ�ֱ�����յ�4���ֽ�Ϊֹ
            // UE_LOG(LogTemp, Error, TEXT("�����һ��ѭ��**********************"));
            int32 read = 0;
            if (ReceiveSocket != nullptr) {
                bool success = ReceiveSocket->Recv((uint8*)(headerData + headerRead), 4 - headerRead, read);
                if (success) {
                    headerRead += read;
                }
                else {
                    // ��������ʧ�ܣ�����������
                    //break;
                }
            }
        }
        if (headerRead == 4) {
            uint32 bodyLength = static_cast<uint32>(static_cast<uint8>(headerData[0])) << 24 |
                static_cast<uint32>(static_cast<uint8>(headerData[1])) << 16 |
                static_cast<uint32>(static_cast<uint8>(headerData[2])) << 8 |
                static_cast<uint32>(static_cast<uint8>(headerData[3]));

            char* bodyData = new char[bodyLength + 1]; // ������Ϣ�����ݻ�����
            char* startbodyData = bodyData;
            uint32 bodyRead = 0;
            bool bodyReceived = false;
            while (bodyRead < bodyLength) { // ѭ��������Ϣ�����ݣ�ֱ�����յ�ָ�����ȵ�����Ϊֹ
                //UE_LOG(LogTemp, Error, TEXT("����ڶ���ѭ��**********************"));
                int32 read = 0;
                if (ReceiveSocket != nullptr) {
                    bool success = ReceiveSocket->Recv((uint8*)(bodyData + bodyRead), bodyLength - bodyRead, read);
                    if (success) {
                        bodyRead += read;
                    }
                    else {
                        // ��������ʧ�ܣ�����������
                        //break;
                    }
                }
            }
            // ���յ���������Ϣ�����ݣ���ӵ��ѽ��յ�������
            startbodyData[bodyRead] = '\0'; // ����Ϣ��ĩβ����ַ���������
            receivedData += FString(startbodyData); // ����Ϣ������ת��ΪFString ����
            TArray<FString> stringArray;
            receivedData.ParseIntoArray(stringArray, TEXT("#"));
            // ���ָ����ַ������浽��Ӧ��FString������
            //UE_LOG(LogTemp, Warning, TEXT("---***************************************** stop!"), a);
            if (stringArray.Num() >= 2)
            {
                Thread1Param3 = stringArray[0];
                Thread1Param4 = stringArray[1];
               // UE_LOG(LogTemp, Warning, TEXT("----------------һ��ִ��%d��recv-----------------RECVTheard stop!"), a);
            }
            // Thread1Param3 = receivedData;
             //UE_LOG(LogTemp, Error, TEXT("%s****%d"), *receivedData,a);
            delete[] startbodyData;
            startbodyData = NULL;
        }
    }


    UE_LOG(LogTemp, Warning, TEXT("----------------һ��ִ��%d��recv-----------------RECVTheard stop!"),a);
    Recv_SocketBIsStop = true;
    return "as";
}


void AMyActor::CloseConnection()
{
    if (ReceiveSocket)
    {
        ReceiveSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ReceiveSocket);
        ReceiveSocket = nullptr;
    }

    if (SendSocket) {
        SendSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SendSocket);
        SendSocket = nullptr;
    }
}

void AMyActor::StopThreads()
{
    // ����Ҫֹͣ�߳�ʱ���� bIsRunning Ϊ false
    bIsRunning = false;
    RecvRunning = false;
    while (1) {
        if (Send_SocketBIsStop == true && Recv_SocketBIsStop == true) {
            break;
        }
    }
    FPlatformProcess::Sleep(2); // �ȴ�1����
    CloseConnection();
    UE_LOG(LogTemp, Warning, TEXT("-----------------------------socket stop"));
}

void AMyActor::MyPrint(FString meaasge) {
    FColor Color = FColor::Green;
    UE_LOG(LogTemp, Error, TEXT("%s"),*meaasge);
}




