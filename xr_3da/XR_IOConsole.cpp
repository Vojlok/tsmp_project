// XR_IOConsole.cpp: implementation of the CConsole class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "igame_level.h"
#include "igame_persistent.h"

#include "x_ray.h"
#include "xr_ioconsole.h"
#include "xr_input.h"
#include "xr_ioc_cmd.h"
#include "GameFont.h"
#include "xr_trims.h"
#include "CustomHUD.h"
#pragma warning(push)
#pragma warning(disable:4995)
//#include <locale>
#pragma warning(pop)

#define  LDIST .05f

ENGINE_API CConsole*	Console = NULL;
const char *			ioc_prompt = ">>> ";

static char RusSymbols[] = { '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
'�','�','�','�','�','�','�','�','�','�','�','�','�', '�','�','�','.' };

static char RusSymbolsBig[] = { '�','�','�','�','�','�','�','�','�','�','�','�','�','�','�','�',
'�','�','�','�','�','�','�','�','�','�','�','�','�', '�','�','�',',' };

static char EngSymbols[] = { 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p',
'q','r','s','t','u','v','w','x','y','z' ,'[',']' ,';', '\'',',','.','/' };

static char EngSymbolsBig[] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
'Q','R','S','T','U','V','W','X','Y','Z' ,'{','}' ,':', '"','<','>','?' };

static u32 DIKS[] = { DIK_A, DIK_B, DIK_C, DIK_D, DIK_E, DIK_F, DIK_G, DIK_H, DIK_I,
DIK_J, DIK_K, DIK_L, DIK_M, DIK_N, DIK_O, DIK_P, DIK_Q, DIK_R, DIK_S, DIK_T, DIK_U, DIK_V,
DIK_W, DIK_X, DIK_Y, DIK_Z, DIK_LBRACKET, DIK_RBRACKET, DIK_SEMICOLON, DIK_APOSTROPHE,
DIK_COMMA, DIK_PERIOD, DIK_SLASH };


int CConsole::str_insert(LPSTR dest, LPSTR src, int m_char)
{
	int len = xr_strlen(dest);
	int lna = xr_strlen(src);
	if (len > m_char)
	{

		for (int i = len - 1; i >= m_char; i--)
			if (i + lna < CConsole::MAX_LEN)
				dest[i + lna] = dest[i]; // 

		for (int i = 0; i < lna; i++)
			if (i + n_char < CConsole::MAX_LEN)
				dest[i + m_char] = src[i]; // 

		dest[len + lna] = 0;
	}
	else
	{
		strcat_s(dest, CConsole::MAX_LEN, src);
	}

	m_char += lna;
	return m_char;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CConsole::AddCommand(IConsole_Command* C)
{
	Commands[C->Name()] = C;
}
void CConsole::RemoveCommand(IConsole_Command* C)
{
	vecCMD_IT it = Commands.find(C->Name());
	if (Commands.end() != it)
		Commands.erase(it);
}
void CConsole::Reset() { if (pFont) xr_delete(pFont); }

void CConsole::Initialize()
{
	scroll_delta = cmd_delta = old_cmd_delta = 0;
	editor[0] = 0;
	bShift = false;
	bCtrl = false;
	bIsRussian = false;
	RecordCommands = false;
	editor[0] = 0;
	cur_time = rep_time = 0;
	fAccel = 1.0f;
	bVisible = false;
	rep_time = 0;
	pFont = 0;
	last_mm_timer = 0;
	// Commands
	extern void CCC_Register();
	CCC_Register();
}

void CConsole::Destroy()
{
	Execute("cfg_save");
	xr_delete(pFont);
	Commands.clear();
}

void CConsole::OnFrame()
{
	u32 mm_timer = Device.dwTimeContinual;
	float fDelta = (mm_timer - last_mm_timer) / 1000.0f;
	if (fDelta>.06666f) fDelta = .06666f;
	last_mm_timer = mm_timer;

	cur_time += fDelta;
	rep_time += fDelta*fAccel;
	if (rep_time > 0.9f) { rep_time -= 0.9f; bRepeat = true;	fAccel += 0.2f; }
}

void out_font(CGameFont* pFont, LPCSTR text, float& pos_y)
{
	float str_length = pFont->SizeOf_(text);
	if (str_length>1024.0f)
	{
		float _l = 0.0f;
		int _sz = 0;
		int _ln = 0;
		string1024			_one_line;

		while (text[_sz])
		{
			_one_line[_ln + _sz] = text[_sz];
			_one_line[_ln + _sz + 1] = 0;
			float _t = pFont->SizeOf_(_one_line + _ln);
			if (_t > 1024.0f)
			{
				out_font(pFont, text + _sz, pos_y);
				pos_y -= LDIST;
				pFont->OutI(-1.0f, pos_y, "%s", _one_line + _ln);
				_l = 0.0f;
				_ln = _sz;
			}
			else
				_l = _t;

			++_sz;
		};
	}
	else
		pFont->OutI(-1.0f, pos_y, "%s", text);
}

void CConsole::OnRender()
{
	float			fMaxY;
	BOOL			bGame;

	if (!bVisible) return;
	if (0 == pFont)
		pFont = xr_new<CGameFont>("hud_font_di", CGameFont::fsDeviceIndependent);

	bGame = false;
	if ((g_pGameLevel && g_pGameLevel->bReady) || (g_pGamePersistent &&
		g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive()))
		bGame = true;

	if (g_dedicated_server)				bGame = false;

	VERIFY(HW.pDevice);

	//*** Shadow
	D3DRECT R = { 0,0,Device.dwWidth,Device.dwHeight };
	if (bGame) R.y2 /= 2;

	CHK_DX(HW.pDevice->Clear(1, &R, D3DCLEAR_TARGET, D3DCOLOR_XRGB(32, 32, 32), 1, 0));

	float dwMaxY = float(Device.dwHeight);
	// float dwMaxX=float(Device.dwWidth/2);
	if (bGame) { fMaxY = 0.f; dwMaxY /= 2; }
	else fMaxY = 1.f;

	char		buf[MAX_LEN + 5];
	strcpy_s(buf, ioc_prompt);
	strcat(buf, editor);

	str_insert(buf, "|", n_char + xr_strlen(ioc_prompt));

	pFont->SetColor(color_rgba(128, 128, 255, 255));
	pFont->SetHeightI(0.025f);
	pFont->OutI(-1.f, fMaxY - LDIST, "%s", buf);

	float ypos = fMaxY - LDIST - LDIST;
	for (int i = LogFile->size() - 1 - scroll_delta; i >= 0; i--)
	{
		ypos -= LDIST;
		if (ypos<-1.f)	break;
		LPCSTR			ls = *(*LogFile)[i];
		if (0 == ls)		continue;
		switch (ls[0]) {
		case '~':
			pFont->SetColor(color_rgba(255, 255, 0, 255));
			out_font(pFont, &ls[2], ypos);
			break;
		case '!':
			pFont->SetColor(color_rgba(255, 0, 0, 255));
			out_font(pFont, &ls[2], ypos);
			break;
		case '*':
			pFont->SetColor(color_rgba(128, 128, 128, 255));
			out_font(pFont, &ls[2], ypos);
			break;
		case '-':
			pFont->SetColor(color_rgba(0, 255, 0, 255));
			out_font(pFont, &ls[2], ypos);
			break;
		case '#':
			pFont->SetColor(color_rgba(0, 222, 205, 145));
			out_font(pFont, &ls[2], ypos);
			break;
		default:
			pFont->SetColor(color_rgba(255, 255, 255, 255));
			out_font(pFont, ls, ypos);
		}
	}
	pFont->OnRender();
}


void CConsole::OnPressKey(int dik, BOOL bHold)
{
	if (!bHold)	fAccel = 1.0f;

	char append[MAX_LEN];
	ZeroMemory(&append, MAX_LEN);

	switch (dik) 
	{
	case DIK_GRAVE:
		if (bShift) strcpy_s(append, "~");
		else		Hide();
		break;
	case DIK_ESCAPE:
		if (!bHold) 
		{
			if (g_pGameLevel ||(g_pGamePersistent && g_pGamePersistent->m_pMainMenu && 
				g_pGamePersistent->m_pMainMenu->IsActive()))
				Hide();
		}
		break;
	case DIK_PRIOR:
		scroll_delta++;
		if (scroll_delta > int(LogFile->size()) - 1) scroll_delta = LogFile->size() - 1;
		break;
	case DIK_NEXT:
		scroll_delta--;
		if (scroll_delta<0) scroll_delta = 0;
		break;
	case DIK_LEFT:
		if (n_char > 0) n_char--;
		break;
	case DIK_RIGHT:
		if (n_char < xr_strlen(editor)) n_char++;
		break;
	case DIK_HOME:
		n_char = 0;
		break;
	case DIK_END:
		n_char = xr_strlen(editor);
		break;
	case DIK_TAB:
	{
		LPCSTR radmin_cmd_name = "ra ";
		bool b_ra = (editor == strstr(editor, radmin_cmd_name));
		int offset = (b_ra) ? xr_strlen(radmin_cmd_name) : 0;
		vecCMD_IT I = Commands.lower_bound(editor + offset);
		if (I != Commands.end()) 
		{
			IConsole_Command &O = *(I->second);
			strcpy_s(editor + offset, sizeof(editor) - offset, O.Name());
			strcat(editor + offset, " ");
		}
	}
	break;
	case DIK_UP:
		cmd_delta--;
		SelectCommand();
		break;
	case DIK_DOWN:
		cmd_delta++;
		SelectCommand();
		break;
	case DIK_DELETE:
		if (n_char < xr_strlen(editor))
		{
			memcpy(&editor[n_char], &editor[n_char + 1], xr_strlen(editor) - n_char);
		}
		break;

	case DIK_BACK:
		if (xr_strlen(editor) > 0 && n_char > 0)
		{
			memcpy(&editor[n_char - 1], &editor[n_char], xr_strlen(editor) - n_char + 1);
			n_char--;
		}

		break;
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		bShift = true;

		if (bCtrl)
		{
			if (bIsRussian)
			{
				bIsRussian = false;
				Msg("Input language: eng");
			}
			else
			{
				bIsRussian = true;
				Msg("Input language: rus");
			}
		}
		break;
	
	case DIK_0:
		strcpy_s(append, bShift ? ")" : "0");
		break;
	case DIK_1:
		strcpy_s(append, bShift ? "!" : "1");
		break;
	case DIK_2:
		if (bIsRussian) strcpy_s(append, bShift ? "\"" : "2");
		else strcpy_s(append, bShift ? "@" : "2");
		break;
	case DIK_3:
		if (bIsRussian) strcpy_s(append, bShift ? "�" : "3");
		else strcpy_s(append, bShift ? "#" : "3");
		break;
	case DIK_4:
		if (bIsRussian) strcpy_s(append, bShift ? ";" : "4");
		else strcpy_s(append, bShift ? "$" : "4");
		break;
	case DIK_5:
		strcpy_s(append, bShift ? "%" : "5");
		break;
	case DIK_6:
		if (bIsRussian) strcpy_s(append, bShift ? ":" : "6");
		else strcpy_s(append, bShift ? "^" : "6");
		break;
	case DIK_7:
		if (bIsRussian) strcpy_s(append, bShift ? "?" : "7");
		else strcpy_s(append, bShift ? "&" : "7");
		break;
	case DIK_8:
		strcpy_s(append, bShift ? "*" : "8");
		break;
	case DIK_9:
		strcpy_s(append, bShift ? "(" : "9");
		break;
	case DIK_RCONTROL:
	case DIK_LCONTROL:
		bCtrl = true;
		break;

	case DIK_SPACE:	strcpy_s(append, " "); break;
	case DIK_BACKSLASH:
		if (bShift) strcpy_s(append, "|");
		else		strcpy_s(append, "\\");
		break;	
	case DIK_EQUALS:
		if (bShift) strcpy_s(append, "+");
		else		strcpy_s(append, "=");
		break;
	case DIK_MINUS:
		if (bShift) strcpy_s(append, "_");
		else		strcpy_s(append, "-");
		break;
	case DIK_RETURN:
		ExecuteCommand();
		break;
	case DIK_INSERT:
		if (OpenClipboard(0))
		{
			HGLOBAL hmem = GetClipboardData(CF_TEXT);
			if (hmem) 
			{
				LPCSTR	clipdata = (LPCSTR)GlobalLock(hmem);
				strncpy_s(append, clipdata, MAX_LEN - 1);

				for (u32 i = 0; i < xr_strlen(append); i++)
					if (!isprint(append[i])) append[i] = ' ';

					GlobalUnlock(hmem);
					CloseClipboard();
			}
		}
		break;
	default:
		int num = 0;
		bool found = false;
		for (int i = 0; i < 33; i++) if (dik == DIKS[i]) { num = i; found = true; }

		if (found)
		{
			std::string str;
			if (bIsRussian)	str= bShift ? RusSymbolsBig[num] : RusSymbols[num];
			else			str= bShift ? EngSymbolsBig[num] : EngSymbols[num];
			strcpy_s(append, str.c_str());
		}
		break;
	}

	char ch = append[0];
	if (ch)
	{
		n_char = str_insert(editor, append, n_char);
		editor[MAX_LEN - 1] = 0;
	}


	u32	clip = MAX_LEN - 8;
	if (xr_strlen(editor) >= clip) editor[clip - 1] = 0;

	if (n_char > xr_strlen(editor)) n_char = xr_strlen(editor);
	bRepeat = false;
	rep_time = 0;
}

void CConsole::IR_OnKeyboardPress(int dik) { OnPressKey(dik); }

void CConsole::IR_OnKeyboardRelease(int dik)
{
	fAccel = 1.0f;
	rep_time = 0;
	switch (dik)
	{
	case DIK_LSHIFT:
	case DIK_RSHIFT:
		bShift = false;
		break;
	case DIK_RCONTROL:
	case DIK_LCONTROL:
		bCtrl = false;
		break;
	}
}

void CConsole::IR_OnKeyboardHold(int dik)
{
	float fRep = rep_time;
	if (bRepeat) { OnPressKey(dik, true); bRepeat = false; }
	rep_time = fRep;
}

void CConsole::ExecuteCommand()
{
	char	first_word[MAX_LEN];
	char	last_word[MAX_LEN];
	char	converted[MAX_LEN];
	int		i, j, len;

	scroll_delta = 0;
	cmd_delta = 0;
	old_cmd_delta = 0;

	len = xr_strlen(editor);
	for (i = 0; i<len; i++) {
		if ((editor[i] == '\n') || (editor[i] == '\t')) editor[i] = ' ';
	}
	j = 0;
	for (i = 0; i<len; i++) {
		switch (editor[i]) {
		case ' ':
			if (editor[i + 1] == ' ') continue;
			if (i == len - 1) goto outloop;
			break;
		}
		converted[j++] = editor[i];
	}
outloop:
	converted[j] = 0;
	if (converted[0] == ' ')	strcpy_s(editor, &(converted[1]));
	else					strcpy_s(editor, converted);
	if (editor[0] == 0)		return;
	if (RecordCommands)		Log("~", editor);

	// split into cmd/params
	editor[j++] = ' ';
	editor[len = j] = 0;
	for (i = 0; i<len; i++) 
	{
		if (editor[i] != ' ') first_word[i] = editor[i];
		else 
		{
			// last 'word' - exit
			strcpy_s(last_word, editor + i + 1);
			break;
		}
	}
	first_word[i] = 0;
	if (last_word[xr_strlen(last_word) - 1] == ' ') last_word[xr_strlen(last_word) - 1] = 0;

	// search
	vecCMD_IT I = Commands.find(first_word);
	if (I != Commands.end()) 
	{
		IConsole_Command &C = *(I->second);
		if (C.bEnabled) {
			if (C.bLowerCaseArgs) strlwr(last_word);
			if (last_word[0] == 0)
			{
				if (C.bEmptyArgsHandled) C.Execute(last_word);
				else 
				{
					IConsole_Command::TStatus S; C.Status(S);
					Msg("- %s %s", C.Name(), S);
				}
			}
			else C.Execute(last_word);
		}
		else 
		{
			Log("! Command disabled.");
		}
	}
	else
		Log("! Unknown command: ", first_word);
	editor[0] = 0;
}

void CConsole::Show()
{
	if (bVisible)			return;
	bVisible = true;
	n_char = 0;
	editor[0] = 0;
	rep_time = 0;
	fAccel = 1.0f;

	IR_Capture();
	Device.seqRender.Add(this, 1);
	Device.seqFrame.Add(this);
}

void CConsole::Hide()
{
	if (!bVisible)													return;
	if (g_pGamePersistent && g_dedicated_server)	return;

	bVisible = false;
	Device.seqFrame.Remove(this);
	Device.seqRender.Remove(this);
	IR_Release();
}

void CConsole::SelectCommand()
{
	int		p, k;
	BOOL	found = false;
	for (p = LogFile->size() - 1, k = 0; p >= 0; p--) 
	{
		if (0 == *(*LogFile)[p])		continue;
		if ((*LogFile)[p][0] == '~') 
		{
			k--;
			if (k == cmd_delta) 
			{
				strcpy_s(editor, &(*(*LogFile)[p])[2]);
				found = true;
			}
		}
	}
	if (!found) 
	{
		if (cmd_delta>old_cmd_delta) editor[0] = 0;
		cmd_delta = old_cmd_delta;

	}
	else old_cmd_delta = cmd_delta;	
	n_char = xr_strlen(editor);
}

void CConsole::Execute(LPCSTR cmd)
{
	strncpy(editor, cmd, MAX_LEN - 1); editor[MAX_LEN - 1] = 0;
	RecordCommands = false;
	ExecuteCommand();
	RecordCommands = true;
}

void CConsole::ExecuteScript(LPCSTR N)
{
	string128		cmd;
	strconcat(sizeof(cmd), cmd, "cfg_load ", N);
	Execute(cmd);
}

BOOL CConsole::GetBool(LPCSTR cmd, BOOL& val)
{
	vecCMD_IT I = Commands.find(cmd);
	if (I != Commands.end()) 
	{
		IConsole_Command* C = I->second;
		CCC_Mask* cf = dynamic_cast<CCC_Mask*>(C);

		if (cf) val = cf->GetValue();
		else 
		{
			CCC_Integer* cf = dynamic_cast<CCC_Integer*>(C);
			val = !!cf->GetValue();
		}
	}
	return val;
}

float CConsole::GetFloat(LPCSTR cmd, float& val, float& min, float& max)
{
	vecCMD_IT I = Commands.find(cmd);
	if (I != Commands.end()) 
	{
		IConsole_Command* C = I->second;
		CCC_Float* cf = dynamic_cast<CCC_Float*>(C);
		val = cf->GetValue();
		min = cf->GetMin();
		max = cf->GetMax();
		return val;
	}
	return val;
}

int CConsole::GetInteger(LPCSTR cmd, int& val, int& min, int& max)
{
	vecCMD_IT I = Commands.find(cmd);
	if (I != Commands.end()) 
	{
		IConsole_Command* C = I->second;
		CCC_Integer* cf = dynamic_cast<CCC_Integer*>(C);
		if (cf)
		{
			val = cf->GetValue();
			min = cf->GetMin();
			max = cf->GetMax();
		}
		else 
		{
			CCC_Mask* cm = dynamic_cast<CCC_Mask*>(C);
			R_ASSERT(cm);
			val = (0 != cm->GetValue()) ? 1 : 0;
			min = 0;
			max = 1;
		}
		return val;
	}
	return val;
}


char * CConsole::GetString(LPCSTR cmd)
{
	static IConsole_Command::TStatus stat;
	vecCMD_IT I = Commands.find(cmd);
	if (I != Commands.end()) 
	{
		IConsole_Command* C = I->second;
		C->Status(stat);
		return stat;
	}
	return NULL;
}
char * CConsole::GetToken(LPCSTR cmd) {	return GetString(cmd);}

xr_token* CConsole::GetXRToken(LPCSTR cmd)
{
	vecCMD_IT I = Commands.find(cmd);
	if (I != Commands.end()) 
	{
		IConsole_Command* C = I->second;
		CCC_Token* cf = dynamic_cast<CCC_Token*>(C);
		return cf->GetToken();
	}
	return NULL;
}