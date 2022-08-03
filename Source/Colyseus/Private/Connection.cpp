#include "Connection.h"

#include <WebSocketsModule.h>

void Connection::Connect(const FString& Url)
{
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	Socket = FWebSocketsModule::Get().CreateWebSocket(Url, "wss");

	Socket->OnConnected().AddLambda(
		[this]() -> void
		{
			if (this->OnOpen)
			{
				this->OnOpen();
			}
		});

	Socket->OnConnectionError().AddLambda(
		[this](const FString& message) -> void
		{
			if (this->OnError)
			{
				this->OnError(message);
			}
		});

	Socket->OnRawMessage().AddLambda(
		[this](const void* Data, SIZE_T Size, SIZE_T BytesRemaining) -> void
		{
			if (this->OnMessage)
			{
				// If there are bytes remaining in the message, we need to buffer the received data
				// and wait until we receive a message with no bytes remaining.
				// This is to prevent MsgPack from trying to deserialize partial messages.
				if (BytesRemaining > 0)
				{
					this->ReceiveBuffer.Append(static_cast<const uint8*>(Data), Size);
					return;
				}

				// Otherwise, this message was either received as a complete message, or in buffered chunks.
				// If a complete message (else case), we can avoid the overhead of buffering.
				if (this->ReceiveBuffer.Num() > 0)
				{
					this->ReceiveBuffer.Append(static_cast<const uint8*>(Data), Size);
					this->OnMessage(static_cast<const void*>(this->ReceiveBuffer.GetData()), this->ReceiveBuffer.Num(), 0);
					this->ReceiveBuffer.Empty();
				}
				else
				{
					this->OnMessage(Data, Size, 0);
				}
			}
		});

	Socket->OnClosed().AddLambda(
		[this](int32 StatusCode, const FString& Reason, bool bWasClean) -> void
		{
			if (this->OnClose)
			{
				this->OnClose(StatusCode, Reason, bWasClean);
			}
		});

	Socket->Connect();
}

void Connection::Close(int32 Code, const FString& Reason)
{
	Socket->Close(Code, Reason);

	Socket = nullptr;
}
