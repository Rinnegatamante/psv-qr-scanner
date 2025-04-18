/*
 * This file is part of VitaDB Downloader
 * Copyright 2022 Rinnegatamante
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vitasdk.h>
#include <vita2d.h>

#define SCR_WIDTH 960
#define SCR_HEIGHT 544

int init_interactive_msg_dialog(const char *fmt, ...) {
	va_list list;
	char msg[1024];

	va_start(list, fmt);
	vsnprintf(msg, sizeof(msg), fmt, list);
	va_end(list);
	
	SceMsgDialogUserMessageParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_YESNO;
	msg_param.msg = (SceChar8 *)msg;

	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	_sceCommonDialogSetMagicNumber(&param.commonParam);
	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
	param.userMsgParam = &msg_param;

	return sceMsgDialogInit(&param);
}

int init_msg_dialog(const char *fmt, ...) {
	va_list list;
	char msg[1024];

	va_start(list, fmt);
	vsnprintf(msg, sizeof(msg), fmt, list);
	va_end(list);
  
	SceMsgDialogUserMessageParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
	msg_param.msg = (SceChar8 *)msg;

	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	_sceCommonDialogSetMagicNumber(&param.commonParam);
	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
	param.userMsgParam = &msg_param;

	return sceMsgDialogInit(&param);
}

int init_warning(const char *fmt, ...) {
	va_list list;
	char msg[1024];

	va_start(list, fmt);
	vsnprintf(msg, sizeof(msg), fmt, list);
	va_end(list);
  
	SceMsgDialogUserMessageParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_NONE;
	msg_param.msg = (SceChar8 *)msg;

	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	_sceCommonDialogSetMagicNumber(&param.commonParam);
	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
	param.userMsgParam = &msg_param;

	return sceMsgDialogInit(&param);
}

int init_progressbar_dialog(const char *fmt, ...) {
	va_list list;
	char msg[1024];

	va_start(list, fmt);
	vsnprintf(msg, sizeof(msg), fmt, list);
	va_end(list);

	SceMsgDialogProgressBarParam msg_param;
	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	msg_param.barType = SCE_MSG_DIALOG_PROGRESSBAR_TYPE_PERCENTAGE;
	msg_param.msg = (const SceChar8*)msg;
	
	SceMsgDialogParam param;
	sceMsgDialogParamInit(&param);
	param.mode = SCE_MSG_DIALOG_MODE_PROGRESS_BAR;
	param.progBarParam = &msg_param;
	
	return sceMsgDialogInit(&param);
}

static uint16_t dialog_res_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
bool is_ime_active = false;
void getDialogTextResult(char *text) {
	// Converting text from UTF16 to UTF8
	std::u16string utf16_str = (char16_t*)dialog_res_text;
	std::string utf8_str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(utf16_str.data());
	strcpy(text, utf8_str.c_str());
}
void init_interactive_ime_dialog(const char *msg, const char *start_text) {
	SceImeDialogParam params;
	
	sceImeDialogParamInit(&params);
	params.type = SCE_IME_TYPE_BASIC_LATIN;
			
	// Converting texts from UTF8 to UTF16
	std::string utf8_str = msg;
	std::u16string utf16_str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8_str.data());
	std::string utf8_arg = start_text;
	std::u16string utf16_arg = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(utf8_arg.data());
			
	params.title = (const SceWChar16*)utf16_str.c_str();
	sceClibMemset(dialog_res_text, 0, sizeof(dialog_res_text));
	sceClibMemcpy(dialog_res_text, utf16_arg.c_str(), utf16_arg.length() * 2);
	params.initialText = dialog_res_text;
	params.inputTextBuffer = dialog_res_text;
			
	params.maxTextLength = SCE_IME_DIALOG_MAX_TEXT_LENGTH;
			
	sceImeDialogInit(&params);
	is_ime_active = true;
}
