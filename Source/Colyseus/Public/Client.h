#pragma once

#include "ColyseusUtils.h"
#include "Connection.h"
#include "Room.h"

#include <Containers/UnrealString.h>
#include <Http.h>

#include <nlohmann/json.hpp>

typedef nlohmann::json JoinOptions;

class MatchMakeError
{
public:
	MatchMakeError(int Code, const FString& Message) : Code(Code), Message(Message)
	{
	}

	int Code;
	FString Message;
};

class Client
{
public:
	FString Endpoint;

	Client(const FString& Endpoint) : Endpoint(Endpoint)
	{
	}

	template <typename S>
	inline void JoinOrCreate(const FString& RoomName, const JoinOptions& Options,
		const TFunction<void(TSharedPtr<MatchMakeError>, TSharedPtr<Room<S>>)>& Callback)
	{
		CreateMatchMakeRequest<S>("joinOrCreate", RoomName, Options, Callback);
	}

	template <typename S>
	inline void Join(const FString& RoomName, const JoinOptions& Options,
		const TFunction<void(TSharedPtr<MatchMakeError>, TSharedPtr<Room<S>>)>& Callback)
	{
		CreateMatchMakeRequest<S>("join", RoomName, Options, Callback);
	}

	template <typename S>
	inline void Create(const FString& RoomName, const JoinOptions& Options,
		const TFunction<void(TSharedPtr<MatchMakeError>, TSharedPtr<Room<S>>)>& Callback)
	{
		CreateMatchMakeRequest<S>("create", RoomName, Options, Callback);
	}

	template <typename S>
	inline void JoinById(const FString& roomId, const JoinOptions& Options,
		const TFunction<void(TSharedPtr<MatchMakeError>, TSharedPtr<Room<S>>)>& Callback)
	{
		CreateMatchMakeRequest<S>("joinById", roomId, Options, Callback);
	}

	template <typename S>
	inline void Reconnect(const FString& roomId, const FString& sessionId,
		const TFunction<void(TSharedPtr<MatchMakeError>, TSharedPtr<Room<S>>)>& Callback)
	{
		CreateMatchMakeRequest<S>("joinById", roomId, {{"sessionId", sessionId}}, Callback);
	}

private:
	template <typename S>
	void CreateMatchMakeRequest(const FString& Method, const FString& RoomName, const JoinOptions& Options,
		TFunction<void(TSharedPtr<MatchMakeError>, TSharedPtr<Room<S>>)> Callback)
	{
		FHttpModule* HttpModule = &FHttpModule::Get();
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule->CreateRequest();

		HttpRequest->SetURL(this->Endpoint.Replace(TEXT("ws"), TEXT("http")) + "/matchmake/" + Method + "/" + RoomName);
		HttpRequest->SetVerb("POST");

		FString Body = StdStringToFString(Options.dump());
		if (Body == "null")
		{
			Body = "{}";
		}
		HttpRequest->SetContentAsString(Body);

		HttpRequest->SetHeader("Accept", "application/json");
		HttpRequest->SetHeader("Content-Type", "application/json");

		HttpRequest->OnProcessRequestComplete().BindLambda(
			[this, RoomName, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
				if (!bWasSuccessful || !Response.IsValid())
				{
					TSharedPtr<MatchMakeError> Error(new MatchMakeError(0, "HttpRequest => no/invalid response"));
					Callback(Error, nullptr);
					return;
				}

				if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
				{
					TSharedPtr<MatchMakeError> Error(new MatchMakeError((int) Response->GetResponseCode(), TEXT("Server Error")));
					Callback(Error, nullptr);
					return;
				}

				FString Data = Response->GetContentAsString();
				nlohmann::json json = nlohmann::json::parse(FStringToStdString(Data));

				// Server responded with error.
				if (json["error"].is_string())
				{
					int Code = json["code"].get<int>();
					FString Message = StdStringToFString(json["error"].get<std::string>());

					TSharedPtr<MatchMakeError> Error(new MatchMakeError(Code, Message));
					Callback(Error, nullptr);
					return;
				}

				TSharedPtr<Room<S>> RoomInstance(new Room<S>(RoomName));
				RoomInstance->Id = StdStringToFString(json["room"]["roomId"].get<std::string>());
				RoomInstance->SessionId = StdStringToFString(json["sessionId"].get<std::string>());

				FString ProcessId = StdStringToFString(json["room"]["processId"].get<std::string>());

				RoomInstance->OnError = [RoomInstance, Callback](const int& Code, const FString& Message) {
					RoomInstance->OnJoin = nullptr;
					TSharedPtr<MatchMakeError> Error(new MatchMakeError(Code, Message));
					Callback(Error, nullptr);
					RoomInstance->OnError = nullptr;
				};

				RoomInstance->OnJoin = [RoomInstance, Callback]() {
					RoomInstance->OnError = nullptr;
					Callback(nullptr, RoomInstance);
					RoomInstance->OnJoin = nullptr;
				};

				RoomInstance->Connect(
					this->Endpoint + "/" + ProcessId + "/" + RoomInstance->Id + "?sessionId=" + RoomInstance->SessionId);
			});

		HttpRequest->ProcessRequest();
	}
};
