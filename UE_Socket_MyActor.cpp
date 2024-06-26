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
    //printf("开始");
    //UE_LOG(LogTemp, Warning, TEXT("---------------------------------开始"));
    while (bIsRunning) {
        a++;
        float randNum1 = FMath::FRandRange(0.f, 1.f); // 生成 0 到 1 之间的小数
        float randNum2 = FMath::FRandRange(0.f, 1.f); // 再次生成 0 到 1 之间的小数
        randNum1 = FMath::RoundToFloat(randNum1 * 1000000) / 1000000.f; // 取小数点后六位
        randNum2 = FMath::RoundToFloat(randNum2 * 1000000) / 1000000.f;
        //UE_LOG(LogTemp, Warning, TEXT("a = %f, b = %f"), randNum1, randNum2); // 打印结果
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

    TArray<uint8> Buffer; // 定义缓冲区
    const int32 BufferSize = 1024; // 设置缓冲区大小
    FString message0 = "C\n";
    const TCHAR* serializedChar0 = *message0;
    int32 sent0 = 0;
    Buffer.Append((const uint8*)serializedChar0, message0.Len() * sizeof(TCHAR));
    if (SendSocket != nullptr) {
        bool successful = SendSocket->Send(Buffer.GetData(), Buffer.Num(), sent0);
    }
    Buffer.Empty();
    //UE_LOG(LogTemp, Warning, TEXT("---------------------------------send successed"));
    // 定义互斥锁
    //FCriticalSection Mutex;
    int a = 0;
    while (bIsRunning)
    {
        FPlatformProcess::Sleep(0.001);
        a++;
        //UE_LOG(LogTemp, Error, TEXT("%d"), a);*/
        // 加锁
       // Mutex.Lock();
        result = Thread1Param + Thread1Param1 + Thread1Param2;
        // 解锁
       // Mutex.Unlock();
        FString message = *result;
        message += "\n"; // 在消息结尾添加分隔符
        const TCHAR* serializedChar = *message;
        int32 size = FCString::Strlen(serializedChar);
        int32 sent = 0;
        for (int32 i = 0; i < size; ++i)
        {
            Buffer.Add(serializedChar[i]);
            // 如果添加的字符是分隔符，或者缓冲区已满，则发送缓冲区中的数据
            if (serializedChar[i] == '\n' || Buffer.Num() >= BufferSize)
            {
                
                if (SendSocket != nullptr) {
                    bool successful = SendSocket->Send(Buffer.GetData(), Buffer.Num(), sent);
                    //UE_LOG(LogTemp, Warning, TEXT("---------------------------------send successed"));
                }
                else {
                    break;
                }
                Buffer.Empty(); // 清空缓冲区
            }
        }

    }

    UE_LOG(LogTemp, Warning, TEXT("-------------一共执行send%d次--------------------SENDTheard stop!"),a);
    Send_SocketBIsStop = true;
}


FString AMyActor::ReceiveMessage()
{
    if (!ReceiveSocket) // 如果Socket为空，直接返回空字符串
    {
        return "";
    }

    uint32 size = 0;

    TArray<uint8> Buffer; // 定义缓冲区
    const int32 BufferSize = 1024; // 设置缓冲区大小

    FString message0 = "W\n"; // 发送一个消息以获取服务器响应
    const TCHAR* serializedChar0 = *message0;
    int32 sent0 = 0;
    Buffer.Append((const uint8*)serializedChar0, message0.Len() * sizeof(TCHAR));
    if (ReceiveSocket != nullptr) {
        bool successful = ReceiveSocket->Send(Buffer.GetData(), Buffer.Num(), sent0); // 发送消息到服务器
    }
    Buffer.Empty(); // 清空缓冲区
    FPlatformProcess::Sleep(2); // 等待服务器响应
    UE_LOG(LogTemp, Error, TEXT("进入while"));
    int a = 0;
    while (RecvRunning) // 循环接收数据
    {
        a++;
        FString receivedData = ""; // 用于保存接收到的数据
        FPlatformProcess::Sleep(0.0001); // 等待1毫秒
        //UE_LOG(LogTemp, Error, TEXT("开始接收数据**********************"));
        char headerData[4]; // 头部数据为4个字节，用于存储消息体的长度
        int32 headerRead = 0;
       // bool headerReceived = false;
        //UE_LOG(LogTemp, Error, TEXT("while二"));
        while (headerRead < 4) { // 循环接收头部数据，直到接收到4个字节为止
            // UE_LOG(LogTemp, Error, TEXT("进入第一个循环**********************"));
            int32 read = 0;
            if (ReceiveSocket != nullptr) {
                bool success = ReceiveSocket->Recv((uint8*)(headerData + headerRead), 4 - headerRead, read);
                if (success) {
                    headerRead += read;
                }
                else {
                    // 接收数据失败，处理错误情况
                    //break;
                }
            }
        }
        if (headerRead == 4) {
            uint32 bodyLength = static_cast<uint32>(static_cast<uint8>(headerData[0])) << 24 |
                static_cast<uint32>(static_cast<uint8>(headerData[1])) << 16 |
                static_cast<uint32>(static_cast<uint8>(headerData[2])) << 8 |
                static_cast<uint32>(static_cast<uint8>(headerData[3]));

            char* bodyData = new char[bodyLength + 1]; // 创建消息体数据缓冲区
            char* startbodyData = bodyData;
            uint32 bodyRead = 0;
            bool bodyReceived = false;
            while (bodyRead < bodyLength) { // 循环接收消息体数据，直到接收到指定长度的数据为止
                //UE_LOG(LogTemp, Error, TEXT("进入第二个循环**********************"));
                int32 read = 0;
                if (ReceiveSocket != nullptr) {
                    bool success = ReceiveSocket->Recv((uint8*)(bodyData + bodyRead), bodyLength - bodyRead, read);
                    if (success) {
                        bodyRead += read;
                    }
                    else {
                        // 接收数据失败，处理错误情况
                        //break;
                    }
                }
            }
            // 接收到完整的消息体数据，添加到已接收的数据中
            startbodyData[bodyRead] = '\0'; // 在消息体末尾添加字符串结束符
            receivedData += FString(startbodyData); // 将消息体数据转换为FString 类型
            TArray<FString> stringArray;
            receivedData.ParseIntoArray(stringArray, TEXT("#"));
            // 将分割后的字符串保存到对应的FString变量中
            //UE_LOG(LogTemp, Warning, TEXT("---***************************************** stop!"), a);
            if (stringArray.Num() >= 2)
            {
                Thread1Param3 = stringArray[0];
                Thread1Param4 = stringArray[1];
               // UE_LOG(LogTemp, Warning, TEXT("----------------一共执行%d次recv-----------------RECVTheard stop!"), a);
            }
            // Thread1Param3 = receivedData;
             //UE_LOG(LogTemp, Error, TEXT("%s****%d"), *receivedData,a);
            delete[] startbodyData;
            startbodyData = NULL;
        }
    }


    UE_LOG(LogTemp, Warning, TEXT("----------------一共执行%d次recv-----------------RECVTheard stop!"),a);
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
    // 在需要停止线程时设置 bIsRunning 为 false
    bIsRunning = false;
    RecvRunning = false;
    while (1) {
        if (Send_SocketBIsStop == true && Recv_SocketBIsStop == true) {
            break;
        }
    }
    FPlatformProcess::Sleep(2); // 等待1毫秒
    CloseConnection();
    UE_LOG(LogTemp, Warning, TEXT("-----------------------------socket stop"));
}

void AMyActor::MyPrint(FString meaasge) {
    FColor Color = FColor::Green;
    UE_LOG(LogTemp, Error, TEXT("%s"),*meaasge);
}




