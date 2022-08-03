#pragma once

#include "Connection.h"
#include "Protocol.h"
#include "Serializer/SchemaSerializer.hpp"
#include "Serializer/Serializer.hpp"
#include "Serializer/schema.h"

#include <stdio.h>

#pragma push_macro("check")
#undef check

#include <msgpack.hpp>

#pragma pop_macro("check")

template <typename S>
class Room
{
public:
	Room(const FString& Name) : Name(Name)
	{
	}

	// Methods
	void Connect(const FString& Endpoint)
	{
		ConnectionInstance = MakeShared<Connection>();
		ConnectionInstance->OnClose =
			std::bind(&Room::_onClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		ConnectionInstance->OnError = std::bind(&Room::_onError, this, std::placeholders::_1);
		ConnectionInstance->OnMessage =
			std::bind(&Room::_onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		ConnectionInstance->Connect(Endpoint);
	}

	void Leave(bool bConsented = true)
	{
		if (!Id.IsEmpty())
		{
			if (bConsented)
			{
				unsigned char bytes[1] = {(unsigned char) Protocol::LEAVE_ROOM};
				ConnectionInstance->Send(bytes, sizeof(bytes));
			}
			else
			{
				ConnectionInstance->Close();
			}
		}
		else
		{
			if (OnLeave)
			{
				OnLeave(4000);
			}
		}
	}

	inline void Send(unsigned char Type)
	{
		unsigned char Message[2] = {(unsigned char) Protocol::ROOM_DATA, Type};
		ConnectionInstance->Send(Message, sizeof(Message));
	}

	template <typename T>
	void Send(const int32_t& Type, T Message)
	{
		std::stringstream PackingStream;
		msgpack::pack(PackingStream, Message);
		std::string Encoded = PackingStream.str();
		size_t EncodedLength = Encoded.length();

		unsigned char* BytesToSend = new unsigned char[EncodedLength + 2];
		BytesToSend[0] = (unsigned char) Protocol::ROOM_DATA;
		BytesToSend[1] = Type;
		FMemory::Memcpy(BytesToSend + 2, Encoded.c_str(), EncodedLength);

		ConnectionInstance->Send(BytesToSend, EncodedLength + 2);

		delete[] BytesToSend;
	}

	void Send(const FString& Type)
	{
		const char* TypeBytes = TCHAR_TO_UTF8(*Type);
		size_t TypeLength = strlen(TypeBytes);

		unsigned char* BytesToSend = new unsigned char[2 + TypeLength];
		BytesToSend[0] = (unsigned char) Protocol::ROOM_DATA;
		BytesToSend[1] = Type.Len() | 0xa0;
		FMemory::Memcpy(BytesToSend + 2, TypeBytes, TypeLength);

		ConnectionInstance->Send(BytesToSend, 2 + TypeLength);

		delete[] BytesToSend;
	}

	template <typename T>
	void Send(const FString& Type, T Message)
	{
		const char* TypeBytes = TCHAR_TO_UTF8(*Type);
		size_t TypeLength = strlen(TypeBytes);

		std::stringstream PackingStream;
		msgpack::pack(PackingStream, Message);
		std::string Encoded = PackingStream.str();
		size_t EncodedLength = Encoded.length();

		unsigned char* BytesToSend = new unsigned char[2 + TypeLength + EncodedLength];
		BytesToSend[0] = (unsigned char) Protocol::ROOM_DATA;
		BytesToSend[1] = Type.Len() | 0xa0;
		FMemory::Memcpy(BytesToSend + 2, TypeBytes, TypeLength);
		FMemory::Memcpy(BytesToSend + 2 + TypeLength, Encoded.c_str(), EncodedLength);

		ConnectionInstance->Send(BytesToSend, 2 + TypeLength + EncodedLength);

		delete[] BytesToSend;
	}

	inline Room<S>* OnMessage(const int Type, const TFunction<void(const msgpack::object&)>& Callback)
	{
		OnMessageHandlers.Add(GetMessageHandlerKey(Type), Callback);
		return this;
	}

	inline Room<S>* OnMessage(const FString& Type, const TFunction<void(const msgpack::object&)>& Callback)
	{
		OnMessageHandlers.Add(GetMessageHandlerKey(Type), Callback);
		return this;
	}

	S* GetState()
	{
		return SerializerInstance->getState();
	}

	// Callbacks
	TFunction<void()> OnJoin;
	TFunction<void(int32 StatusCode)> OnLeave;
	TFunction<void(int32 StatusCode, const FString& Message)> OnError;
	TFunction<void(S*)> OnStateChange;
	TMap<const FString, TFunction<void(const msgpack::object&)>> OnMessageHandlers;

	// Properties
	TSharedPtr<Connection> ConnectionInstance;

	FString Id;
	FString Name;
	FString SessionId;
	FString SerializerId;

protected:
	bool bHasJoined = false;

	void _onClose(int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		if (!bHasJoined)
		{
			if (OnError)
			{
				OnError(StatusCode, Reason);
			}
		}

		if (OnLeave)
		{
			OnLeave(StatusCode);
		}
	}

	void _onError(const FString& Message)
	{
		if (OnError)
		{
			OnError(1005, Message);
		}
	}

	void _onMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
	{
		const unsigned char* Bytes = reinterpret_cast<const unsigned char*>(Data);

		colyseus::schema::Iterator* Iterator = new colyseus::schema::Iterator();
		Iterator->offset = 0;

#ifdef COLYSEUS_DEBUG
		std::cout << "onMessage bytes =>" << Bytes << std::endl;
#endif

		unsigned char Code = Bytes[Iterator->offset++];

		switch ((Protocol) Code)
		{
			case Protocol::JOIN_ROOM:
			{
#ifdef COLYSEUS_DEBUG
				std::cout << "Colyseus.Room: JOIN_ROOM" << std::endl;
#endif

				SerializerId = StdStringToFString(colyseus::schema::decodeString(Bytes, Iterator));

				// TODO: instantiate serializer by id
				if (Size > Iterator->offset)
				{
					SerializerInstance->handshake(Bytes, Iterator->offset);
				}

				bHasJoined = true;
				if (OnJoin)
				{
					OnJoin();
				}

				unsigned char Message[1] = {(int) Protocol::JOIN_ROOM};
				ConnectionInstance->Send(Message, sizeof(Message));
				break;
			}
			case Protocol::JOIN_ERROR:
			{
#ifdef COLYSEUS_DEBUG
				std::cout << "Colyseus.Room: ERROR" << std::endl;
#endif
				float ErrorCode = colyseus::schema::decodeNumber(Bytes, Iterator);
				std::string Message = colyseus::schema::decodeString(Bytes, Iterator);

				if (OnError)
				{
					OnError(Code, StdStringToFString(Message));
				}
				break;
			}
			case Protocol::LEAVE_ROOM:
			{
#ifdef COLYSEUS_DEBUG
				std::cout << "Colyseus.Room: LEAVE_ROOM" << std::endl;
#endif
				Leave();
				break;
			}
			case Protocol::ROOM_DATA:
			{
#ifdef COLYSEUS_DEBUG
				std::cout << "Colyseus.Room: ROOM_DATA" << std::endl;
#endif
				FString Type;

				if (colyseus::schema::numberCheck(Bytes, Iterator))
				{
					Type = GetMessageHandlerKey(colyseus::schema::decodeNumber(Bytes, Iterator));
				}
				else
				{
					Type = GetMessageHandlerKey(StdStringToFString(colyseus::schema::decodeString(Bytes, Iterator)));
				}

				TFunction<void(const msgpack::object&)>* Handler = OnMessageHandlers.Find(Type);

				if (Handler != nullptr)
				{
					if (Size > Iterator->offset)
					{
						const char* TheBytes = reinterpret_cast<const char*>(Data);
						msgpack::object_handle MsgpackObjectHandle = msgpack::unpack(TheBytes, Size, Iterator->offset);
						msgpack::object MsgpackObject = MsgpackObjectHandle.get();
						(*Handler)(MsgpackObject);
					}
					else
					{
						msgpack::object Empty;
						(*Handler)(Empty);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Room::onMessage() missing for type => %s"), *Type);
				}

				break;
			}
			case Protocol::ROOM_STATE:
			{
#ifdef COLYSEUS_DEBUG
				std::cout << "Colyseus.Room: ROOM_STATE" << std::endl;
#endif
				SetState(Bytes, Iterator->offset, Size);
				break;
			}
			case Protocol::ROOM_STATE_PATCH:
			{
#ifdef COLYSEUS_DEBUG
				std::cout << "Colyseus.Room: ROOM_STATE_PATCH" << std::endl;
#endif
				ApplyPatch(Bytes, Iterator->offset, Size);
				break;
			}
			default:
			{
				break;
			}
		}

		delete Iterator;
	}

	void SetState(unsigned const char* Bytes, int Offset, int Length)
	{
		SerializerInstance->setState(Bytes, Offset, Length);

		if (OnStateChange)
		{
			OnStateChange(GetState());
		}
	}

	void ApplyPatch(unsigned const char* Bytes, int Offset, int Length)
	{
		SerializerInstance->patch(Bytes, Offset, Length);

		if (OnStateChange)
		{
			OnStateChange(GetState());
		}
	}

	FString GetMessageHandlerKey(const int32 Type)
	{
		return "i" + FString::FromInt(Type);
	}

	FString GetMessageHandlerKey(const FString& Type)
	{
		return Type;
	}

	TSharedPtr<Serializer<S>> SerializerInstance;
};
