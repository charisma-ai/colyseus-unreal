#pragma once

#include <IWebSocket.h>
#include <stdio.h>

#include <functional>

class COLYSEUSCLIENT_API Connection
{
private:
	TSharedPtr<IWebSocket> Socket;

public:
	~Connection()
	{
		Socket->OnRawMessage().Clear();
		Socket->OnClosed().Clear();
		Socket->OnConnected().Clear();
		Socket->OnConnectionError().Clear();
		Socket->Close();
		Socket = nullptr;
	}

	// Methods
	void Connect(const FString& Url);
	void Close(int32 Code = 1000, const FString& Reason = FString());

	inline void Send(const void* Buffer, SIZE_T Size) const
	{
		Socket->Send(Buffer, Size, true);
	}

	// Callbacks
	TFunction<void()> OnOpen;
	TFunction<void(int32 StatusCode, const FString& Reason, bool bWasClean)> OnClose;
	TFunction<void(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)> OnMessage;
	TFunction<void(const FString& message)> OnError;

	// Properties
	TArray<uint8> ReceiveBuffer;
};
