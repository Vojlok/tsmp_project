#include "stdafx.h"

#include "level.h"
#include "xrServer.h"


#include "xrMessages.h"
#include "xrGameSpyServer.h"
#include "../igame_persistent.h"

#include "GameSpy/GameSpy_Base_Defs.h"
#include "GameSpy/GameSpy_Available.h"

#include "..\TSMP_BuildConfig.h"
//#define DEMO_BUILD

xrGameSpyServer::xrGameSpyServer()
{
	m_iReportToMasterServer = 0;
	m_bQR2_Initialized = FALSE;
	m_bCDKey_Initialized = FALSE;
	m_bCheckCDKey = false;
	if (0 != strstr(Core.Params, "-check_cd_key")) m_bCheckCDKey = true;
	ServerFlags.set( server_flag_all, 0 );
	iGameSpyBasePort = 0;
}

xrGameSpyServer::~xrGameSpyServer()
{
	CDKey_ShutDown();
	QR2_ShutDown();
}

bool	xrGameSpyServer::HasPassword()	{ return !!ServerFlags.test(server_flag_password); }
bool	xrGameSpyServer::HasProtected()	{ return !!ServerFlags.test(server_flag_protected); }

//----------- xrGameSpyClientData -----------------------
IClient*		xrGameSpyServer::client_Create		()
{
	return xr_new<xrGameSpyClientData> ();
}
xrGameSpyClientData::xrGameSpyClientData	():xrClientData()
{
	m_bCDKeyAuth = false;
	m_iCDKeyReauthHint = 0;
}
void	xrGameSpyClientData::Clear()
{
	inherited::Clear();

	m_pChallengeString[0] = 0;
	m_bCDKeyAuth = false;
	m_iCDKeyReauthHint = 0;
};

xrGameSpyClientData::~xrGameSpyClientData()
{
	m_pChallengeString[0] = 0;
	m_bCDKeyAuth = false;
	m_iCDKeyReauthHint = 0;
}
//-------------------------------------------------------
xrGameSpyServer::EConnect xrGameSpyServer::Connect(shared_str &session_name)
{
	EConnect res = inherited::Connect(session_name);
	if (res!=ErrNoError) return res;

	if ( 0 == *(game->get_option_s		(*session_name,"hname",NULL)))
	{

		string1024	CompName;
		DWORD		CompNameSize = 1024;
		if (GetComputerName(CompName, &CompNameSize)) HostName._set(CompName);
	}
	else
		HostName._set(game->get_option_s		(*session_name,"hname",NULL));
	
	if (0 != *(game->get_option_s		(*session_name,"psw",NULL)))
		Password._set(game->get_option_s		(*session_name,"psw",NULL));


	string4096	tMapName = "";
	const char* SName = *session_name;
	strncpy(tMapName, *session_name, strchr(SName, '/') - SName);
	MapName._set(tMapName);// = (session_name);

	MapNameRus._set(MapName);

	{
		string_path						cfg_name="level.name";

		string_path cfg_full_name;

		FS.update_path(cfg_full_name, "$level$", cfg_name);

		if (NULL == FS.exist(cfg_full_name))
			strcpy_s(cfg_full_name, cfg_name);

		IReader* F = FS.r_open(cfg_full_name);

		string1024						str;

		if (F != NULL)
		{
			F->r_string(str, sizeof(str));
		    if (sizeof(str)!=NULL) MapNameRus._set(str);		
			FS.r_close(F);
			Msg("[%s] successfully loaded.", cfg_full_name);
		}
		else
		{
			Msg("! Cannot open script file [%s]", cfg_full_name);
		}
	}
	

	m_iReportToMasterServer = game->get_option_i		(*session_name,"public",0);
	if (m_iReportToMasterServer == 2) m_iReportToMasterServer = 0;

	m_iMaxPlayers	= game->get_option_i		(*session_name,"maxplayers",40);
//	m_bCheckCDKey = game->get_option_i		(*session_name,"cdkey",0) != 0;

//	m_bCheckCDKey = game->get_option_i		(*session_name,"public",0) != 0;

	if (0 != strstr(Core.Params, "-check_cd_key"))	m_bCheckCDKey = true ;
	else m_bCheckCDKey = false;

	//--------------------------------------------//
	if (game->Type() != GAME_SINGLE) 
	{
		//----- Check for Backend Services ---
		CGameSpy_Available GSA;
		shared_str result_string;
		if (!GSA.CheckAvailableServices(result_string))
		{
			Msg(*result_string);
		};

		//------ Init of QR2 SDK -------------
		iGameSpyBasePort = game->get_option_i(*session_name, "portgs", -1);
		QR2_Init(iGameSpyBasePort);

		//------ Init of CDKey SDK -----------

#ifndef DEMO_BUILD
		if(m_bCheckCDKey) 
#endif

			CDKey_Init();
	};

	return res;
}

void			xrGameSpyServer::Update				()
{
	inherited::Update();

	if (m_bQR2_Initialized)
	{
		m_QR2.Think(NULL);
	};

	if (m_bCDKey_Initialized)
	{
		m_GCDServer.Think();
	};
	static u32 next_send_time = Device.dwTimeGlobal+10000;
	if(Device.dwTimeGlobal >= next_send_time)
	{
		next_send_time					= Device.dwTimeGlobal+5000;
		NET_Packet						Packet;
		Packet.w_begin					(M_MAP_SYNC);
		Packet.w_stringZ				(MapName);
		SendBroadcast					(BroadcastCID, Packet, net_flags(TRUE,TRUE));
	}
}

int	xrGameSpyServer::GetPlayersCount() 
{
	int NumPlayers = client_Count();
	if (!g_dedicated_server || NumPlayers < 1)
	{
	     return NumPlayers;		
	}
	if (0 != strstr(Core.Params, "-survival"))
	{
		return NumPlayers + 1;
	}
	else
	{ 
		return NumPlayers - 1; // players count
	}
};

bool			xrGameSpyServer::NeedToCheckClient_GameSpy_CDKey	(IClient* CL)
{
	if (0 != strstr(Core.Params, "-check_cd_key"))
	{
		if (!m_bCDKey_Initialized || (CL == GetServerClient() && g_dedicated_server))
		{
			return false;
		};


		SendChallengeString_2_Client(CL);

		return true;
	}
	else return false;
};

void			xrGameSpyServer::OnCL_Disconnected	(IClient* _CL)
{
	inherited::OnCL_Disconnected(_CL);

	csPlayers.Enter			();

	if (m_bCDKey_Initialized)
	{
		Msg("xrGS::CDKey::Server : Disconnecting Client [ %s ]", _CL->name.c_str());
		m_GCDServer.DisconnectUser(int(_CL->ID.value()));
	};

	csPlayers.Leave			();
}

u32				xrGameSpyServer::OnMessage(NET_Packet& P, ClientID sender)			// Non-Zero means broadcasting with "flags" as returned
{
	u16			type;
	P.r_begin	(type);

	xrGameSpyClientData* CL		= (xrGameSpyClientData*)ID_to_client(sender);

	switch (type)
	{
	case M_GAMESPY_CDKEY_VALIDATION_CHALLENGE_RESPOND:
		{
			string4096 ResponseStr;
			P.r_stringZ(ResponseStr);

			if (strlen(ResponseStr) > 100)
			{		
				xrClientData *ClData = (xrClientData*)ID_to_client(sender);
				ip_address			ipSender;
				DWORD dwPort = 0;
				u32 uBanFor = 99999999999999999;

				Level().Server->GetClientAddress(ClData->ID, ipSender, &dwPort);
				Level().Server->clients_Lock();
				Level().Server->BanAddress(ipSender, uBanFor);
				Level().Server->DisconnectAddress(ipSender);
				Level().Server->clients_Unlock();

				Msg("! stalkazz attack detected, %i characters string was in packet, blocked ip %s"
					, strlen(ResponseStr)
					, ipSender.to_string().c_str());
				
				return 0;
			}

			if (!CL->m_bCDKeyAuth)
			{

				Msg("xrGS::CDKey::Server : Respond accepted, Authenticate client.");
				m_GCDServer.AuthUser(int(CL->ID.value()), CL->m_cAddress.m_data.data, CL->m_pChallengeString, ResponseStr, this);
				strcpy_s(CL->m_guid,128,this->GCD_Server()->GetKeyHash(CL->ID.value()));
			}
			else
			{
				Msg("xrGS::CDKey::Server : Respond accepted, ReAuthenticate client.");
				m_GCDServer.ReAuthUser(int(CL->ID.value()), CL->m_iCDKeyReauthHint, ResponseStr);
			}

			return 0;
		}break;
	}

	return	inherited::OnMessage(P, sender);
};

bool xrGameSpyServer::Check_ServerAccess( IClient* CL, string512& reason )
{
	if( !HasProtected() )
	{
		strcpy_s( reason, "Access successful by server. " );
		return true;
	}

	string_path		fn;
	FS.update_path( fn, "$app_data_root$", "server_users.ltx" );
	if( FS.exist(fn) == NULL )
	{
		strcpy_s( reason, "Access denied by server. " );
		return false;
	}

	CInifile inif( fn );
	if( inif.section_exist( "users" ) == FALSE )
	{
		strcpy_s( reason, "Access denied by server. " );
		return false;
	}

	if( inif.line_count( "users" ) == 0 )
	{
		strcpy_s( reason, "Access denied by server. " );
		return false;
	}
	
	if( CL != NULL && inif.line_exist( "users", CL->name ) )
	{
		if( game->NewPlayerName_Exists( CL, CL->name.c_str() ) )
		{
			strcpy_s( reason, "! Access denied by server. Login \"" );
			strcat_s( reason, CL->name.c_str() );
			strcat_s( reason, "\" exist already. " );
			return false;
		}

		shared_str pass1 = inif.r_string_wb( "users", CL->name.c_str() );
		if( xr_strcmp( pass1, CL->pass ) == 0 )
		{
			strcpy_s( reason, "- User \"" );
			strcat_s( reason, CL->name.c_str() );
			strcat_s( reason, "\" access successful by server. " );
			return true;
		}
	}
	strcpy_s( reason, "! Access denied by server. Wrong login/password. " );
	return false;
}

void xrGameSpyServer::Assign_ServerType( string512& res )
{
	string_path		fn;
	FS.update_path( fn, "$app_data_root$", "server_users.ltx" );
	if( FS.exist(fn) )
	{
		CInifile inif( fn );
		if( inif.section_exist( "users" ) )
		{
			if( inif.line_count( "users" ) != 0 )
			{
				ServerFlags.set( server_flag_protected, 1 );
				strcpy_s( res, "# Server started as protected, using users list." );
				Msg( res );
				return;
			}else{
				strcpy_s( res, "Users count in list is null." );
			}
		}else{
			strcpy_s( res, "Section [users] not found." );
		}
	}else{
		strcpy_s( res, "File <server_users.ltx> not found in folder <$app_data_root$>." );
	}// if FS.exist(fn)

	Msg( res );
	ServerFlags.set( server_flag_protected, 0 );
	strcpy_s( res, "# Server started without users list." );
	Msg( res );
}

void xrGameSpyServer::GetServerInfo( CServerInfo* si )
{
	string32 tmp, tmp2;

	//MapName="eee";

	std::string rus = MapNameRus.c_str();
	std::string eng = MapName.c_str();
	std::string inf,host,port,p2,p3;

	host = HostName.c_str();

	if (rus !=eng ) inf = rus + " ("+ eng+") ";
	else inf = rus;

	inf = inf + " ; ��� ������� = " + host;

//	si->AddItem( "��� �������", HostName.c_str(), RGB(128,128,255) );
	si->AddItem( "�����", inf.c_str(), RGB(255,0,128) );
	
	
	strcpy_s( tmp, itoa( GetPlayersCount(), tmp2, 10 ) );
	strcat_s( tmp, " / ");
	strcat_s( tmp, itoa( m_iMaxPlayers, tmp2, 10 ) );
	si->AddItem( "������", tmp, RGB(255,128,255) );

	string256 res;
	si->AddItem( "������ �������", TSMP_VERSION, RGB(0,158,255) );
	
	/*

	strcpy_s( res, "" );
	if ( HasProtected() || Password.size() > 0 || HasBattlEye() )
	{
		if ( HasProtected() )			strcat_s( res, "protected  " );
		if ( Password.size() > 0 )		strcat_s( res, "password  " );
		if ( HasBattlEye() )			strcat_s( res, "battleye  " );
	}
	else
	{
		if ( xr_strlen( res ) == 0 )	strcat_s( res, "free" );
	}
	si->AddItem( "Access to server", res, RGB(200,155,155) );

	*/
	p2 = std::to_string(iGameSpyBasePort);
	p3 = std::to_string(GetPort());
	port ="GameSpy:"+ p2 +" �������:"+ p3;

	si->AddItem( "�����", port.c_str(), RGB(200,5,155) );
	inherited::GetServerInfo( si );
}