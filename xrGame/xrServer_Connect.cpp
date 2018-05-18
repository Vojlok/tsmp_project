#include "stdafx.h"


#include "..\..\sysmsgs.h"


#include "xrServer.h"
#include "game_sv_single.h"
#include "game_sv_deathmatch.h"
#include "game_sv_teamdeathmatch.h"
#include "game_sv_artefacthunt.h"
#include "xrMessages.h"
#include "game_cl_artefacthunt.h"
#include "game_cl_single.h"
#include "MainMenu.h"

std::string ConPlayer = "123";

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

xrServer::EConnect xrServer::Connect(shared_str &session_name)
{
#ifdef DEBUG
	Msg						("* sv_Connect: %s",	*session_name);
#endif

	// Parse options and create game
	if (0==strchr(*session_name,'/'))
		return				ErrConnect;

	string1024				options;
	R_ASSERT2(xr_strlen(session_name) <= sizeof(options), "session_name too BIIIGGG!!!");
	strcpy					(options,strchr(*session_name,'/')+1);
	
	// Parse game type
	string1024				type;
	R_ASSERT2(xr_strlen(options) <= sizeof(type), "session_name too BIIIGGG!!!");
	strcpy					(type,options);
	if (strchr(type,'/'))	*strchr(type,'/') = 0;
	game					= NULL;

	CLASS_ID clsid			= game_GameState::getCLASS_ID(type,true);
	game					= smart_cast<game_sv_GameState*> (NEW_INSTANCE(clsid));

	// Options
	if (0==game)			return ErrConnect;
	csPlayers.Enter			();
//	game->type				= type_id;
#ifdef DEBUG
	Msg("* Created server_game %s",game->type_name());
#endif

	game->Create			(session_name);
	csPlayers.Leave			();
	
#ifdef BATTLEYE
	if ( game->get_option_i( *session_name, "battleye", 1) != 0 ) // default => battleye enable (always)
	{
		// if level exist & if server in internet
		if ( g_pGameLevel && (game->get_option_i( *session_name, "public", 0) != 0)  )
		{
			if ( Level().battleye_system.server )
			{
				Msg( "Warning: BattlEye already loaded!" );
			}
			else
			{
				if ( !Level().battleye_system.LoadServer( this ) )
				{
					return ErrBELoad;
				}
			}
		}//g_pGameLevel
	}
/*	if ( g_pGameLevel && Level().battleye_system.server )
	{
		if ( game->get_option_i( *session_name, "battleye_update", 1) != 0 ) // default => battleye auto_update enable (always)
		{
			Level().battleye_system.auto_update = 1;
		}
		else
		{
			Level().battleye_system.auto_update = 0;
		}
	}*/
#endif // BATTLEYE
	
	return IPureServer::Connect(*session_name);
}


IClient* xrServer::new_client( SClientConnectData* cl_data )
{
	IClient* CL		= client_Find_Get( cl_data->clientID );
	VERIFY( CL );
	
	// copy entity
	CL->ID			= cl_data->clientID;
	CL->process_id	= cl_data->process_id;
	
	string1024 new_name;

	strcpy_s( new_name, cl_data->name );

	ConPlayer = new_name;
	Msg("new = %s", cl_data->new_code);

	std::string Name_new = new_name;
	Name_new.resize(70);
	std::string connecting = "connecting player "+Name_new;
	Msg(connecting.c_str());
	std::string messages = "! nickname of connecting player "+Name_new+" is >70 symbols";
	if (strlen(new_name) > 70) Msg(messages.c_str());
	messages = "! connecting player - " + Name_new + " tried to use percent symbol in nick";
	while (Name_new.find('%') != std::string::npos)
	{
		
		Name_new.replace(Name_new.find("%"), 1, " ");
		Msg(messages.c_str());
	}
	messages = "! connecting player - " + Name_new + " tried to use _ symbol in nick";
	while (Name_new.find('_') != std::string::npos)
	{

		Name_new.replace(Name_new.find("_"), 1, " ");
		Msg(messages.c_str());
	}
	strcpy ( new_name, Name_new.c_str());

	CL->name._set( new_name );
	
	if ( !HasProtected() && game->NewPlayerName_Exists( CL, new_name ) )
	{
		game->NewPlayerName_Generate( CL, new_name );
		game->NewPlayerName_Replace( CL, new_name );
	}
	CL->name._set( new_name );
	CL->pass._set( cl_data->pass );

	NET_Packet		P;
	P.B.count		= 0;
	P.r_pos			= 0;
	
	game->AddDelayedEvent( P, GAME_EVENT_CREATE_CLIENT, 0, CL->ID );
	if ( client_Count() == 1 )
	{
		Update();
	}
	return CL;
}

struct SMyUserData {
	xrServer* server; ClientID idOfPlayer;
};

void xrServer::SendCB(void* msg, unsigned int len, void* userdata)
{
	Msg("CD called from dll");
	//((SMyUserData*)userdata)->server->SendTo_LL(((SMyUserData*)userdata)->idOfPlayer, msg, len, net_flags(TRUE, TRUE, TRUE, TRUE));
	((SMyUserData*)userdata)->server->IPureServer::SendTo_LL(((SMyUserData*)userdata)->idOfPlayer, msg, len, net_flags(TRUE, TRUE, TRUE, TRUE));


	Msg("xrServer::SendCB");
};

void xrServer::AttachNewClient			(IClient* CL)
{
//	Msg("CL NAME = %s",ConPlayer.c_str());	

	if (g_dedicated_server)
	{

		if (ConPlayer == "server") Msg("connecting server");
		else
		{

				typedef bool(__stdcall *FZSysMsgsInit)();
				typedef bool(__stdcall *FZSysMsgsFlags)();
				typedef void* FZSysMsgsProcessClientModDll;
				typedef void(__stdcall *FZSysMsgSender) (void* msg, unsigned int len, void* userdata);
				typedef void(__stdcall *FZSysMsgsSendSysMessage_SOC)(void*, void*, FZSysMsgSender, void*);

				Msg("load dll");

				HMODULE dll = LoadLibrary("sysmsgs.dll");

				Msg("After load dll");
				if (dll == nullptr) Msg("1");

				FZSysMsgsSendSysMessage_SOC SendSysMessage = (FZSysMsgsSendSysMessage_SOC)GetProcAddress(dll, "FZSysMsgsSendSysMessage_SOC");
				if (SendSysMessage == nullptr) Msg("2");

				FZSysMsgsInit SysInit = (FZSysMsgsInit)GetProcAddress(dll, "FZSysMsgsInit");
				if (SysInit == nullptr) Msg("3");

				FZSysMsgsProcessClientModDll writer = (FZSysMsgsProcessClientModDll)GetProcAddress(dll, "FZSysMsgsProcessClientModDll");
				if (writer == nullptr) Msg("4");

				(*SysInit)();

				//flags
				typedef int FZSysmsgsCommonFlags;
				const FZSysmsgsCommonFlags FZ_SYSMSGS_ENABLE_LOGS = 1;
				const FZSysmsgsCommonFlags FZ_SYSMSGS_PATCH_UI_PROGRESSBAR = 2;

				typedef void(__stdcall *SetCommonSysmsgsFlags)(FZSysmsgsCommonFlags);

				SetCommonSysmsgsFlags SetFlags = (SetCommonSysmsgsFlags)GetProcAddress(dll, "FZSysMsgsSetCommonSysmsgsFlags");

				if (SetFlags == nullptr) Msg("5");
				else SetFlags(FZ_SYSMSGS_ENABLE_LOGS | FZ_SYSMSGS_PATCH_UI_PROGRESSBAR);
				//flags end


				SMyUserData userdata = {};
				userdata.idOfPlayer = CL->ID;
				userdata.server = this;

				FZDllDownloadInfo moddllinfo = {};

				moddllinfo.fileinfo.filename = "";
				moddllinfo.fileinfo.url = "";
				moddllinfo.fileinfo.crc32 = 0x274A4EBD;
				moddllinfo.fileinfo.progress_msg = "In Progress"; //Сообщение, выводимое пользователю во время закачки
				moddllinfo.fileinfo.error_already_has_dl_msg = "Error happens";  //Сообщение, выводимое пользователю при возникновении ошибки во время закачки
				moddllinfo.fileinfo.compression = FZ_COMPRESSION_NO_COMPRESSION; //Используемый тип компрессии
				moddllinfo.procname = "ModLoad";  //Имя процедуры в dll мода, которая должна быть вызвана; должна иметь тип FZDllModFun
				moddllinfo.procarg1 = "sace3"; //Аргументы для передачи в процедуру
				moddllinfo.procarg2 = "mod parameters"; //Аргументы для передачи в процедуру
				moddllinfo.dsign = "";
				moddllinfo.name_lock = "123";  //Цифровая подпись для загруженной DLL - проверяется перед тем, как передать управление в функцию мода
				moddllinfo.reconnect_addr.ip = "127.0.0.1";  //IP-адрес и порт для реконнекта. Если IP нулевой, то параметры реконнекта автоматически берутся игрой из тех, во время которых произошел дисконнект.
				moddllinfo.reconnect_addr.port = 5445; // Порт
			//	moddllinfo.is_reconnect_needed = 0;





				

	//-binlist <URL> - ссылка на адрес, по которому берется список файлов движка (для работы требуется запуск клиента с ключлм -fz_custom_bin)
	//-gamelist <URL> - ссылка на адрес, по которому берется список файлов мода (геймдатных\патчей)
	//-srv <IP> - IP-адрес сервера, к которому необходимо присоединиться после запуска мода
	//-srvname <domainname> - доменное имя, по которому располагается сервер. Можно использовать вместо параметра -srv в случае динамического IP сервера
	//-port <number> - порт сервера
	//-gamespymode - стараться использовать загрузку средствами GameSpy
	//-fullinstall - мод представляет собой самостоятельную копию игры, связь с файлами оригинальной не требуется
	//-sharedpatches - использовать общую с инсталляцией игры директорию патчей
	//-logsev <number> - уровень серьезности логируемых сообщений, по умолчанию FZ_LOG_ERROR
	//-configsdir <string> - директория конфигов
	//-exename <string> - имя исполняемого файла мода

				


				Msg("After this send sysmsgs");

				SendSysMessage(writer, &moddllinfo, xrServer::SendCB, &userdata);

				Msg("Before this send sysmsgs");

				FreeLibrary(dll);

				Msg("library free");
				Msg("0");
			}
		}



	MSYS_CONFIG	msgConfig;
	msgConfig.sign1 = 0x12071980;
	msgConfig.sign2 = 0x26111975;
	msgConfig.is_battleye = 0;


	
		



#ifdef BATTLEYE
		msgConfig.is_battleye = (g_pGameLevel && Level().battleye_system.server != 0)? 1 : 0;
#endif // BATTLEYE

	
			if (psNET_direct_connect)  //single_game
			{
				SV_Client = CL;
				CL->flags.bLocal = 1;
				SendTo_LL(SV_Client->ID, &msgConfig, sizeof(msgConfig), net_flags(TRUE, TRUE, TRUE, TRUE));
			}
			else


			{

				SendTo_LL(CL->ID, &msgConfig, sizeof(msgConfig), net_flags(TRUE, TRUE, TRUE, TRUE));
				Server_Client_Check(CL);

			}
		

	// gen message
	if (!NeedToCheckClient_GameSpy_CDKey(CL))
	{
	//-------------------------------------------------------------
	Check_GameSpy_CDKey_Success(CL);
 }

	//xrClientData * CL_D=(xrClientData*)(CL); 
	//ip_address				ClAddress;
	//GetClientAddress		(CL->ID, ClAddress);
	CL->m_guid[0]=0;
}


