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

#ifndef _DIALOGS_H
#define _DIALOGS_H

int init_interactive_msg_dialog(const char *msg, ...);
int init_msg_dialog(const char *msg, ...);
int init_progressbar_dialog(const char *msg, ...);
int init_interactive_ime_dialog(const char *msg, const char *start_text);
int init_warning(const char *fmt, ...);

void getDialogTextResult(char *text);

extern bool is_ime_active;

#endif
