#pragma once

#include <Containers/UnrealString.h>

#include <string>

std::string COLYSEUSCLIENT_API FStringToStdString(const FString& UEString);
FString COLYSEUSCLIENT_API StdStringToFString(const std::string& StdString);
