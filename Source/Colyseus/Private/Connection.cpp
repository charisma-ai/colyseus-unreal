#include "Connection.h"

#include <WebSocketsModule.h>

void Connection::Connect(const FString& Url)
{
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

			if (this->OnClose)
			{
				this->OnClose();
			}
		});

	Socket->OnRawMessage().AddLambda(
		[this](const void* Data, SIZE_T Size, SIZE_T BytesRemaining) -> void
		{
			if (this->OnMessage)
			{
				this->OnMessage(Data, Size, BytesRemaining);
			}
		});

	Socket->OnClosed().AddLambda(
		[this](int32 StatusCode, const FString& Reason, bool bWasClean) -> void
		{
			if (this->OnClose)
			{
				this->OnClose();
			}
		});

	Socket->Connect();
}

void Connection::Close(int32 Code, const FString& Reason)
{
	Socket->Close(Code, Reason);

	Socket = nullptr;
}
