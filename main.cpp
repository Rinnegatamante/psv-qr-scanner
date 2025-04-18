#include <vitasdk.h>
#include <vita2d.h>
#include <quirc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dialogs.h"
#include "network.h"

#define CAM_WIDTH (640)
#define CAM_HEIGHT (360)

int _newlib_heap_size_user = 128 * 1024 * 1024;

SceCameraInfo cam_info;
SceCameraRead cam_info_read;
uint8_t *cam_buf;
struct quirc *qr;

enum {
	STATUS_SCANNING,
	STATUS_ASK_DOWNLOAD,
	STATUS_ASK_OPEN,
	STATUS_DOWNLOAD,
	STATUS_DOWNLOAD_END
};

int main(int argc, char *argv[]) {
	scePowerSetArmClockFrequency(444);
	
	SceAppUtilInitParam init_param;
	SceAppUtilBootParam boot_param;
	sceClibMemset(&init_param, 0, sizeof(SceAppUtilInitParam));
	sceClibMemset(&boot_param, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&init_param, &boot_param);

	generic_mem_buffer = (uint8_t *)malloc(MEM_BUFFER_SIZE);
	
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	int ret = sceNetShowNetstat();
	SceNetInitParam initparam;
	if (ret == SCE_NET_ERROR_ENOTINIT) {
		initparam.memory = malloc(141 * 1024);
		initparam.size = 141 * 1024;
		initparam.flags = 0;
		sceNetInit(&initparam);
	}
	
	vita2d_init_advanced(0x800000);
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	
	vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW);
	vita2d_texture *cam_tex = vita2d_create_empty_texture(CAM_WIDTH, CAM_HEIGHT);
	cam_buf = (uint8_t *)vita2d_texture_get_datap(cam_tex);
	
	cam_info.size = sizeof(SceCameraInfo);
	cam_info.format = SCE_CAMERA_FORMAT_ABGR;
	cam_info.resolution = SCE_CAMERA_RESOLUTION_640_360;
	cam_info.pitch = vita2d_texture_get_stride(cam_tex) - (CAM_WIDTH << 2);
	cam_info.sizeIBase = (CAM_WIDTH * CAM_HEIGHT) << 2;
	cam_info.pIBase = cam_buf;
	cam_info.framerate = 30;
	sceCameraOpen(SCE_CAMERA_DEVICE_BACK, &cam_info);
	sceCameraStart(SCE_CAMERA_DEVICE_BACK);
	sceCameraSetEffect(SCE_CAMERA_DEVICE_BACK, SCE_CAMERA_EFFECT_BLACKWHITE);
	
	cam_info_read.size = sizeof(SceCameraRead);
	cam_info_read.mode = 0;

	qr = quirc_new();
	if (quirc_resize(qr, CAM_WIDTH, CAM_HEIGHT) < 0) {
		sceClibPrintf("quirc failed to allocate the required buffer\n");
	}
	
	uint8_t status = STATUS_SCANNING;
	char *payload = nullptr;
	for (;;) {
		sceCameraRead(SCE_CAMERA_DEVICE_BACK, &cam_info_read);
		
		vita2d_start_drawing();
		vita2d_clear_screen();
		vita2d_draw_texture(cam_tex, 160, 92);
		vita2d_end_drawing();
		
		if (payload) {
			if (status == STATUS_DOWNLOAD) {
				if (sceImeDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
					vita2d_common_dialog_update();
				} else {
					SceImeDialogResult res;
					sceClibMemset(&res, 0, sizeof(SceImeDialogResult));
					sceImeDialogGetResult(&res);
					if (res.button == SCE_IME_DIALOG_BUTTON_ENTER) {
						char file_name[128] = {0};
						getDialogTextResult(file_name);
						char full_name[256];
						sprintf(full_name, "ux0:data/%s", file_name);
						sceIoRename(TEMP_DOWNLOAD_NAME, full_name);
						sceImeDialogTerm();
						init_msg_dialog("Download finished!\nYou can find the file in %s", full_name);
					} else {
						sceImeDialogTerm();
						init_msg_dialog("Download aborted");
						sceIoRemove(TEMP_DOWNLOAD_NAME);
					}
					status = STATUS_DOWNLOAD_END;
				}
			} else {
				if (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
					vita2d_common_dialog_update();
				} else {
					if (status != STATUS_DOWNLOAD_END) {
						SceMsgDialogResult msg_res;
						sceClibMemset(&msg_res, 0, sizeof(SceMsgDialogResult));
						sceMsgDialogGetResult(&msg_res);
						sceMsgDialogTerm();
						if (msg_res.buttonId == SCE_MSG_DIALOG_BUTTON_ID_YES) {
							if (status == STATUS_ASK_DOWNLOAD) {
								early_download_file(payload, "Downloading file in ux0:data");
								init_interactive_ime_dialog("Insert file name", "file.zip");
								status = STATUS_DOWNLOAD;
							} else if (status == STATUS_ASK_OPEN) {
								SceAppUtilWebBrowserParam param;
								sceClibMemset(&param, 0, sizeof(SceAppUtilWebBrowserParam));
								param.str = payload;
								param.strlen = strlen(payload);
								sceAppUtilLaunchWebBrowser(&param);
								free(payload);
								payload = nullptr;
								status = STATUS_SCANNING;
							}
						} else {
							if (status == STATUS_ASK_DOWNLOAD) {
								init_interactive_msg_dialog("Do you want to open %s?", payload);
								status = STATUS_ASK_OPEN;
							} else {
								free(payload);
								payload = nullptr;
								status = STATUS_SCANNING;
							}
						}
					} else {
						free(payload);
						payload = nullptr;
						status = STATUS_SCANNING;
					}
				}
			}
			
		} else {
			int w, h;
			uint8_t *qr_buf = quirc_begin(qr, &w, &h);
			for (int i = 0; i < w * h; i++) {
				qr_buf[i] = cam_buf[i * 4];
			}
			quirc_end(qr);
		
			if (quirc_count(qr) > 0) {
				struct quirc_code code;
				struct quirc_data data;
				quirc_extract(qr, 0, &code);
				quirc_decode_error_t err = quirc_decode(&code, &data);
				if (err) {
					sceClibPrintf("Failed to decode QR\n");
				} else {
					payload = (char *)malloc(data.payload_len + 1);
					sceClibMemcpy(payload, data.payload, data.payload_len);
					payload[data.payload_len] = 0;
					sceClibPrintf("New QR: %s\n", payload);
					init_interactive_msg_dialog("Do you want to download %s?", payload);
					status = STATUS_ASK_DOWNLOAD;
				}
			}
		}
		
		vita2d_swap_buffers();
	}
	
	return 0;
}