#ifndef _SYSMSGS_H_
#define _SYSMSGS_H_

#include "windows.h"

#pragma pack(push, 1)

typedef void* FZSysmsgPayloadWriter;
typedef void* FZSysMsgSender;

//��� ����������, �������������� � ����������� �� ������� �����   
typedef DWORD FZArchiveCompressionType;
    const FZArchiveCompressionType FZ_COMPRESSION_NO_COMPRESSION = 0;    //���������� ���, ���� �� ������� �� ����
    const FZArchiveCompressionType FZ_COMPRESSION_LZO_COMPRESSION = 1;   //���� ���� ������������� LZO-������������
    const FZArchiveCompressionType FZ_COMPRESSION_CAB_COMPRESSION = 2;   //���� ���� ����������� �������� MAKECAB �� ������� Windows   

//���������� ������, � ������� ������ ���������� ������� ����
typedef DWORD FZModdingPolicy;

    //������� �� DLL ���� ����� ���������� � ����� �����. �������������� ��������� �� ������������.
    const FZModdingPolicy FZ_MODDING_ANYTIME = 0;
    
    //������� �� DLL ���� ������ ���������� ������ � �������� �������� � ������� (�.�. ��� ������������� ������������� ������������ ��������� ����� ������)
    //������ ��� �����, �� ��������� �������� �������
    const FZModdingPolicy FZ_MODDING_WHEN_CONNECTING = 1;
    
    //������� �� DLL ���� ������ ���������� ������ ����� ������ �� � ��������� �������� (�.�. ���� ���� �� ������� ��� ���� - ��� ����� ����� ���������� ����������)
    //��������������� ���������� �� ������������, ��� ��� �������� �� ��������� � ������ ������������� ��������
    //������ � ����: � ������ ������ ����� ����� ����� ���� �� ������ ������! � ���� ������ ��������������� ����������� � ������� ������� ����������� �� �����!
    const FZModdingPolicy FZ_MODDING_WHEN_NOT_CONNECTING = 2;

//��������� ������������ �����
struct FZFileDownloadInfo
{
    //��� ����� (������ � �����) �� �������, �� �������� �� ������ ���� ��������
    char* filename;
    //URL, � �������� ����� ������������� �������� �����
    char* url;
    //����������� ����� CRC32 ��� ����� (� ������������� ����)
    DWORD crc32;
    //������������ ��� ����������
    FZArchiveCompressionType compression;
    //���������, ��������� ������������ �� ����� �������
    char* progress_msg;
    //���������, ��������� ������������ ��� ������������� ������ �� ����� �������
    char* error_already_has_dl_msg;
};

//��������� ���������� ������� � �������
struct FZReconnectInetAddrData
{
    //IPv4-����� ������� (��������, 127.0.0.1)
    char* ip;
    //���� �������
    DWORD port;
};

typedef DWORD FZMapLoadingFlags;
    const FZMapLoadingFlags FZ_MAPLOAD_MANDATORY_RECONNECT = 1; //������������ ��������� ����� �������� ��������� ��������� �����

//��������� �������� �������� db-������ � ������
struct FZMapInfo
{
    //��������� �����
    FZFileDownloadInfo fileinfo;
    //IP-����� � ���� ��� ���������� ����� ���������� �������. ���� IP ������, �� ��������� ���������� ������������� ������� ����� �� ���, �� ����� ������� ��������� ����������.
    FZReconnectInetAddrData reconnect_addr;
    //������������� ��� ����������� ����� (��������, mp_pool)
    char* mapname;
    //������ ����������� ����� (������ 1.0)
    char* mapver;
    //�������� xml-����� � ��������������� ��������� � ��������� ����� (nil, ���� ����� �� ���������)
    char* xmlname;
    //����� ��� ��������� ������������ ���������� �����
    FZMapLoadingFlags flags;
};

typedef DWORD FZDllModFunResult;
    const FZDllModFunResult FZ_DLL_MOD_FUN_SUCCESS_LOCK = 0;    //��� ������� ����������, ��������� �������� ������� �� name_lock
    const FZDllModFunResult FZ_DLL_MOD_FUN_SUCCESS_NOLOCK = 1;  //�����, ������ ������� (� �������������� name_lock) ���� �� ����
    const FZDllModFunResult FZ_DLL_MOD_FUN_FAILURE = 2;         //������ �������� ����

typedef FZDllModFunResult (__stdcall *FZDllModFun) (char* procarg1, char* procarg2);
    
//��������� �������� �������� DLL-���� ����������� ProcessClientModDll
struct FZDllDownloadInfo
{
    //��������� ����� ��� dll ����
    FZFileDownloadInfo fileinfo;

    //��� ��������� � dll ����, ������� ������ ���� �������; ������ ����� ��� FZDllModFun
    char* procname;

    //��������� ��� �������� � ���������
    char* procarg1;
    char* procarg2;

    //�������� ������� ��� ����������� DLL - ����������� ����� ���, ��� �������� ���������� � ������� ����
    char* dsign;

    //IP-����� � ���� ��� ����������. ���� IP �������, �� ��������� ���������� ������������� ������� ����� �� ���, �� ����� ������� ��������� ����������.
    FZReconnectInetAddrData reconnect_addr;

    //��������� ������� ��������� ����
    FZModdingPolicy modding_policy;

    //�������� ��� �������� � ��������� -fzmod.
    //���� �������� nil - ������ �� �����������.
    //���� �������� ��������� � ������� � ��������� ������ - ��� ��������� ��� �������������, ������� ������������
    //���� �������� �� ��������� � ��������� � ��������� ������ - ���������� ���������� ������
    //���� � ��������� ������ ��������� ��� - ���������� ��������� ����.
    char* name_lock;

    //������, ��������� ��� ����������� �������������� ����
    char* incompatible_mod_message;

    //������, ��������� ��� ���������� ���� ����� ����������� (���� ������� FZ_MODDING_WHEN_NOT_CONNECTING)
    char* mod_is_applying_message;
};

//��������� ��������� ����� ��� ���������� � ������ �����������
struct FZClientVotingElement
{
    //������������� ��� ����� (��������, mp_pool). � ������ ���� nil - ������������ ������� ������ ����!
    //������������� ���������� nil ������ ��������� � ������
    char* mapname;
    //������ �����
    char* mapver;
    //�������������� �������� �����; ���� nil, ����� ����������� ��������� ������ ������������ �������������� ����������� �� �������
    char* description;
};

//��������� ���������� ���� � ������, ��������� ��� �����������, ������������ � ProcessClientVotingMaplist
struct FZClientVotingMapList
{
  //��������� �� ������ �� FZClientVotingElement. ������ ������� ������� �������� ��������� ����� �����,
  //������� ��������� �������� � ������ ����, ��������� ��� �����������
  FZClientVotingElement* maps;

  //����� ��������� � ������� maps
  DWORD count;

  //������������� ���� ����, ��� �������� ��������� �������� ������ ����. � ������ ������� ��� ���� ���������� � � game_GameState.m_type
  DWORD gametype;

  //�������� ��������, ����������, ������� ���� �� ������� ���� ���������� �������, ������ ���� � ������ �������.
  DWORD was_sent;
};

#pragma pack(pop)

#endif