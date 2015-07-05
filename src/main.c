#include <pebble.h>

#define FRAMES 11
#define BKGD_FRAME 99
	
Window *my_window;
TextLayer *time_text_layer;
TextLayer *date_text_layer;
BitmapLayer *layer_conn_img;
BitmapLayer *layer_bkgd_img;
GBitmap *bt_connect_img;
GBitmap *bt_disconnect_img;
GBitmap *bkgd_img;
static GBitmap *amd_img[FRAMES];

static AppTimer *timer;
static bool is_animating;

//// prototypes
static void handle_timer(void *data);

void handle_bluetooth(bool connected) {
    if (connected) {
        bitmap_layer_set_bitmap(layer_conn_img, bt_connect_img);
    } else {
        bitmap_layer_set_bitmap(layer_conn_img, bt_disconnect_img);
        vibes_long_pulse();
    }
}

void setup_gbitmaps() {
	amd_img[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_00);
	amd_img[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_01);
	amd_img[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_02);
	amd_img[3] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_03);
	amd_img[4] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_04);
	amd_img[5] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_05);
	amd_img[6] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_06);
	amd_img[7] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_07);
	amd_img[8] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_08);
	amd_img[9] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_09);
	amd_img[10] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_10);
////	amd_img[11] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_11);
////	amd_img[12] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_12);
////	amd_img[13] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_13);
////	amd_img[14] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_14);
////	amd_img[15] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_15);
////	amd_img[16] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_16);
////	amd_img[17] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_17);
////	amd_img[18] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_18);
////	amd_img[19] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_19);
////	amd_img[20] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_BLACK);
}

void animate_amd_logo() {
	is_animating = true;
	timer = app_timer_register(250, handle_timer, (int *) 0);
}

void update_amd_logo(int current_frame) {
	app_log(APP_LOG_LEVEL_INFO, "main.c", 61, "update_amd_log - %d", current_frame);
	if ((current_frame >= 0) && (current_frame < FRAMES)) {
	    bitmap_layer_set_bitmap(layer_bkgd_img, amd_img[current_frame]);
	} else {
	    bitmap_layer_set_bitmap(layer_bkgd_img, bkgd_img);
	}
}

static void handle_timer(void *data) {
	static uint32_t next_frame;
	uint32_t current_frame = (uint32_t) data;

	if (is_animating == false) return;
	if (current_frame == (uint32_t) FRAMES) {
		is_animating = false;
		update_amd_logo(BKGD_FRAME);
		return;
	} else {
		update_amd_logo(current_frame);
		next_frame = current_frame + 1;
		timer = app_timer_register(250, &handle_timer, (void *) next_frame);
		return;
	}
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    // Need to be static because they're used by the system later.
    static char date_text[] = "Sun, Jan 00";
    static char wday_text[] = "Xxx";
    static char mnth_text[] = "Xxx";
    static char wdat_text[] = "00";
    static char time_text[] = "00:00";
    char *time_format;

    strftime(wday_text, sizeof(wday_text), "%A", tick_time);
    strftime(mnth_text, sizeof(mnth_text), "%B", tick_time);
	strftime(wdat_text, sizeof(wdat_text), "%e", tick_time);
	strcpy(date_text, wday_text); strcat(date_text, ", ");
	strcat(date_text, mnth_text); strcat(date_text, " ");
	strcat(date_text, wdat_text);
    text_layer_set_text(date_text_layer, date_text);

	if (clock_is_24h_style()) {
        time_format = "%R";
    } else {
        time_format = "%I:%M";
    }

    strftime(time_text, sizeof(time_text), time_format, tick_time);
    if (!clock_is_24h_style() && (time_text[0] == '0')) {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }
    text_layer_set_text(time_text_layer, time_text);
	animate_amd_logo();
}

void handle_init(void) {
    my_window = window_create();
	window_set_background_color(my_window, GColorBlack);
    window_stack_push(my_window, true);

    time_text_layer = text_layer_create(GRect(0, 46, 144, 80));
    text_layer_set_font(time_text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
    text_layer_set_text_color(time_text_layer, GColorWhite);	
    text_layer_set_background_color(time_text_layer, GColorBlack);
	text_layer_set_text(time_text_layer, "00:00");
    layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(time_text_layer));	
	
    date_text_layer = text_layer_create(GRect(8, 4, 144, 30));
    text_layer_set_font(date_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(date_text_layer, GTextAlignmentLeft);
    text_layer_set_text_color(date_text_layer, GColorWhite);	
    text_layer_set_background_color(date_text_layer, GColorBlack);
    text_layer_set_text(date_text_layer, "Sun, Jan 00");
    layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(date_text_layer));	
 
    setup_gbitmaps();
    bkgd_img        = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_FINAL);
    layer_bkgd_img  = bitmap_layer_create(GRect(2, 100, 144, 68));
    bitmap_layer_set_bitmap(layer_bkgd_img, bkgd_img);
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(layer_bkgd_img));	
 	
    bt_connect_img     = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECT);
    bt_disconnect_img  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECT);
    layer_conn_img     = bitmap_layer_create(GRect(118, 12, 20, 20));
    bitmap_layer_set_bitmap(layer_conn_img, bt_connect_img);
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(layer_conn_img));	

	bluetooth_connection_service_subscribe(&handle_bluetooth);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
 }

void handle_deinit(void) {
    gbitmap_destroy(bt_connect_img);
    gbitmap_destroy(bt_disconnect_img);
    gbitmap_destroy(bkgd_img);
 	for (int i=0; i<FRAMES; i++) gbitmap_destroy(amd_img[i]);
    bitmap_layer_destroy(layer_conn_img);
    bitmap_layer_destroy(layer_bkgd_img);
    text_layer_destroy(time_text_layer);
    text_layer_destroy(date_text_layer);
    window_destroy(my_window);

	bluetooth_connection_service_unsubscribe();	
}

int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}
