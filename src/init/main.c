#include <stdio.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

#include "mouse.h" // mouse
#include "kbd.h"   // keyboard
#include "fb.h"    // framebuffer
#include "fs.h"    // filesystem

#include "shell.h"
#include "logo.h"

static void
AnimationTask(void *parameters) {
	FB_Color c, bg = { 0x50, 0x20, 0xa0, 0xff };
	FB_Surface *logo;
	FB_Rectangle upd;
	int dir, pos;

	fb_init();
	fb_mode(FB_MODE_1280x768, FB_FORMAT_BEST);
	fb_fill(NULL, NULL, &bg);
	fb_flip(NULL);

	logo = fb_create_surface(piratos_logo.width, piratos_logo.height, FB_FORMAT_BEST);

	for (int y=0; y<logo->height; y++) {
		for (int x=0; x<logo->width; x++) {
			int idx = y * logo->width * 4 + x * 4;

			c.r = piratos_logo.pixel_data[idx + 0];
			c.g = piratos_logo.pixel_data[idx + 1];
			c.b = piratos_logo.pixel_data[idx + 2];
			c.a = piratos_logo.pixel_data[idx + 3];

			fb_set_pixel(logo, x, y, &c);
		}
	}

	pos = (1280 - piratos_logo.width) / 2;
	upd.y = 768 - piratos_logo.height;
	upd.w = piratos_logo.width;
	upd.h = piratos_logo.height;
	dir = 2;

	while (1) {
		upd.x = pos;
		pos += dir;

		fb_fill(NULL, &upd, &bg);
		fb_blit(logo, NULL, upd.x, upd.y, NULL);
		fb_flip(&upd);

		vTaskDelay(40); // ~25fps

		if ((pos >= 1280 - upd.w) || (pos <= 0)) dir *= -1;
	}
}

static void
EventTask(void *parameters) {
	MOUSE_Event mouse_ev;
	KBD_Event kbd_ev;

	kbd_init();
	mouse_init();

	while (1) {
		if (mouse_poll(&mouse_ev)) {
			printf("mouse poll: %i, %i, %i, %i\n",
				mouse_ev.x, mouse_ev.y, mouse_ev.state, mouse_ev.button);
		}

		if (kbd_poll(&kbd_ev)) {
			printf("kbd poll: %i, %i\n", kbd_ev.state, kbd_ev.symbol);
		}

		vTaskDelay(10);
	}
}

static void
ShellTask(void *parameters) {
	fs_init();
	shell_init();
}

void
piratos(void) {
	portTickType rate = 500;

	printf("pir{A}tos Version " VERSION " (" PLATFORM ")\n");

	xTaskCreate(AnimationTask, "LOGOx",  configMINIMAL_STACK_SIZE, NULL, 7, NULL);
	xTaskCreate(EventTask,     "EVENTx", configMINIMAL_STACK_SIZE, NULL, 8, NULL);
	//xTaskCreate(ShellTask,     "SHELLx", configMINIMAL_STACK_SIZE, NULL, 9, NULL);

	vTaskStartScheduler();

	//shell_fini();
	fs_fini();
	mouse_fini();
	kbd_fini();
	fb_fini();
}
