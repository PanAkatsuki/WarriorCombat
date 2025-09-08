#pragma once

namespace Debug
{
	static void Print(const FString& Msg, int32 InKey = -1, const float TimeToDisplay = 10.f, const FColor& Color = FColor::MakeRandomColor())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, TimeToDisplay, Color, Msg);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		}
	}

	static void Print(const FString& FloatTitle, float FloatValueToPrint, int32 InKey = -1, const float TimeToDisplay = 10.f, const FColor& Color = FColor::MakeRandomColor())
	{
		if (GEngine)
		{
			const FString Msg = FloatTitle + TEXT(": ") + FString::SanitizeFloat(FloatValueToPrint);
			GEngine->AddOnScreenDebugMessage(InKey, 10.f, Color, Msg);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		}
	}
} 