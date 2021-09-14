#pragma once

#include "ColyseusUtils.h"

std::string FStringToStdString(const FString& UEString)
{
	return std::string(TCHAR_TO_UTF8(*UEString));
}

FString StdStringToFString(const std::string& StdString)
{
	return FString(UTF8_TO_TCHAR(StdString.c_str()));
}
