// Copyright 2025 Samantha Rubio-Campos (rubiocam@usc.edu)


#include "MultiOnlineSubsystem.h"

#include "Online/Lobbies.h"
#include "Online/OnlineAsyncOpHandle.h"
#include "Online/OnlineResult.h"
#include "Online/OnlineSessionNames.h"
using namespace UE::Online;

void UMultiOnlineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnlineServices = GetServices();

	if (OnlineServices)
	{
		AuthInterface = OnlineServices->GetAuthInterface();
		LobbiesInterface = OnlineServices->GetLobbiesInterface();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find a valid online service."));
	}

}

void UMultiOnlineSubsystem::Login()
{
	LoginHelper(LoginCredentialsType::PersistentAuth);
}

bool UMultiOnlineSubsystem::IsLoggedIn() const
{
	return AccountInfo.LoginStatus == ELoginStatus::LoggedIn;
}

bool UMultiOnlineSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if WITH_EDITOR
	return false;
#else
	return true;
#endif
}

void UMultiOnlineSubsystem::HostSession(bool Zombie)
{
	if (!LobbiesInterface && !IsLoggedIn())
	{
		return;
	}

	FCreateLobby::Params Params;
	Params.LocalAccountId = AccountInfo.AccountId;
	Params.LocalName = TEXT("MyGame");
	Params.SchemaId = FSchemaId(TEXT("GameLobby"));
	Params.bPresenceEnabled = true;
	Params.MaxMembers = 4;
	Params.JoinPolicy = ELobbyJoinPolicy::PublicAdvertised;

	if (!Zombie)
	{
		Params.Attributes.Emplace(SETTING_MAPNAME, TEXT("Lvl_Shooter"));
	}
	else
	{
		Params.Attributes.Emplace(SETTING_MAPNAME, TEXT("Lvl_Zombie"));
	}
	LobbiesInterface->CreateLobby(MoveTemp(Params)).OnComplete([this](const TOnlineResult<FCreateLobby>& Result)
	{
		if(Result.IsOk())
		{
			GetWorld()->ServerTravel("Lvl_Shooter?listen");
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create a lobby."));
		}
	});
}

void UMultiOnlineSubsystem::FindAndJoinSession()
{
	if (!LobbiesInterface || !IsLoggedIn())
	{
		return;
	}

	
	FFindLobbies::Params FLParam;
	FLParam.LocalAccountId = AccountInfo.AccountId;

	LobbiesInterface->FindLobbies(MoveTemp(FLParam)).OnComplete([this](const TOnlineResult<FFindLobbies>& Result)
	{
		if (Result.IsOk())
		{
			const FFindLobbies::Result& FindResults = Result.GetOkValue();
            
			for (auto& Lobby : FindResults.Lobbies)
			{
				if (Lobby->OwnerAccountId.IsValid() && Lobby->Members.Num() > 0)
				{
					FJoinLobby::Params JLParam;
					
					JLParam.LocalAccountId = AccountInfo.AccountId;
					JLParam.bPresenceEnabled = true;
					JLParam.LobbyId = Lobby->LobbyId; 
					JLParam.LocalName = TEXT("MyGame");
					
					LobbiesInterface->JoinLobby(MoveTemp(JLParam)).OnComplete([this](const TOnlineResult<FJoinLobby>& JLResult)
					{
						if (JLResult.IsOk())
						{
							auto& JLLobby = JLResult.GetOkValue().Lobby;
							TOnlineResult<FGetResolvedConnectString> ConnectResult = OnlineServices->GetResolvedConnectString({AccountInfo.AccountId, JLLobby->LobbyId});
							if (ConnectResult.IsOk())
							{
								APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
								PC->ClientTravel(ConnectResult.GetOkValue().ResolvedConnectString, TRAVEL_Absolute);
							}
							
						}
						
					});
					break;
				}
			}
		}
	});
}
void UMultiOnlineSubsystem::LoginHelper(FName CredentialsType)
{
	FAuthLogin::Params Params;
	if (ULocalPlayer* Player = GetGameInstance()->GetLocalPlayerByIndex(0))
	{
		Params.PlatformUserId = Player->GetPlatformUserId();
	}
	
	Params.CredentialsType = CredentialsType;

	AuthInterface->Login(MoveTemp(Params)).OnComplete([this, CredentialsType](const TOnlineResult<FAuthLogin>& Result)
	{
		if(Result.IsOk())
		{
			AccountInfo = Result.GetOkValue().AccountInfo.Get();
		}
		else
		{
			if (CredentialsType == TEXT("PersistentAuth"))
			{
				LoginHelper(LoginCredentialsType::AccountPortal);
			}
		}
	});
}
