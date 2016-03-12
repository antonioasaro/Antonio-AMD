#include <pebble.h>

#define FRAMES 20
#define BKGD_FRAME 99
#define FIFO_DEPTH 4	
	
Window *my_window;
TextLayer *time_text_layer;
TextLayer *date_text_layer;
BitmapLayer *layer_conn_img;
BitmapLayer *layer_bkgd_img;
GBitmap *bt_connect_img;
GBitmap *bt_disconnect_img;
GBitmap *bkgd_img;
GBitmap *fifo_img[FIFO_DEPTH];


const int AMD_LOGO_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_AMD_LOGO_00,
  RESOURCE_ID_IMAGE_AMD_LOGO_01,
  RESOURCE_ID_IMAGE_AMD_LOGO_02,
  RESOURCE_ID_IMAGE_AMD_LOGO_03,
  RESOURCE_ID_IMAGE_AMD_LOGO_04,
  RESOURCE_ID_IMAGE_AMD_LOGO_05,
  RESOURCE_ID_IMAGE_AMD_LOGO_06,
  RESOURCE_ID_IMAGE_AMD_LOGO_07,
  RESOURCE_ID_IMAGE_AMD_LOGO_08,
  RESOURCE_ID_IMAGE_AMD_LOGO_09,
  RESOURCE_ID_IMAGE_AMD_LOGO_10,
  RESOURCE_ID_IMAGE_AMD_LOGO_11,
  RESOURCE_ID_IMAGE_AMD_LOGO_12,
  RESOURCE_ID_IMAGE_AMD_LOGO_13,
  RESOURCE_ID_IMAGE_AMD_LOGO_14,
  RESOURCE_ID_IMAGE_AMD_LOGO_15,
  RESOURCE_ID_IMAGE_AMD_LOGO_16,
  RESOURCE_ID_IMAGE_AMD_LOGO_17,
  RESOURCE_ID_IMAGE_AMD_LOGO_18,
  RESOURCE_ID_IMAGE_AMD_LOGO_19
};

static AppTimer *timer;
static bool is_animating;

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x2,         // TUPLE_CSTRING
};

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN,
  RESOURCE_ID_IMAGE_CLOUD,
  RESOURCE_ID_IMAGE_RAIN,
  RESOURCE_ID_IMAGE_SNOW
};

static TextLayer *temp_text_layer;
static TextLayer *s_city_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;
static AppSync s_sync;
static uint8_t s_sync_buffer[64];

//// prototypes
static void handle_timer(void *data);

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "sync_tuple_changed_callback with key: %d", (int) key);	
  switch (key) {
    case WEATHER_ICON_KEY:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }
      s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
      break;

    case WEATHER_TEMPERATURE_KEY:
      text_layer_set_text(temp_text_layer, new_tuple->value->cstring);
      break;
/*
    case WEATHER_CITY_KEY:
      text_layer_set_text(s_city_layer, new_tuple->value->cstring);
      break;
*/
  }
}

static void request_weather(void) {
  DictionaryIterator *iter;
	
  APP_LOG(APP_LOG_LEVEL_INFO, "requesting weather");
  app_message_outbox_begin(&iter);
	if (!iter) return; 		// Error creating outbound message

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);
  app_message_outbox_send();
}

void handle_bluetooth(bool connected) {
    if (connected) {
        bitmap_layer_set_bitmap(layer_conn_img, bt_connect_img);
    } else {
        bitmap_layer_set_bitmap(layer_conn_img, bt_disconnect_img);
        vibes_long_pulse();
    }
}

void setup_gbitmaps() {
	for (int i=0; i<FIFO_DEPTH; i++) { 
		gbitmap_destroy(fifo_img[i]);
		fifo_img[i] = gbitmap_create_with_resource(AMD_LOGO_RESOURCE_IDS[i]); 
	}
}

void animate_amd_logo() {
	is_animating = true;
	setup_gbitmaps();
	timer = app_timer_register(100, handle_timer, (int *) 0);
}

void update_amd_logo(int current_frame) {
	// app_log(APP_LOG_LEVEL_INFO, "main.c", 68, "update_amd_log - %d", current_frame);
	if ((current_frame >= 0) && (current_frame < FRAMES)) {
		bitmap_layer_set_bitmap(layer_bkgd_img, fifo_img[(current_frame % FIFO_DEPTH)]);
		if ((current_frame > 1) && (current_frame < FRAMES-2)) {
		  	gbitmap_destroy(fifo_img[((current_frame-2) % FIFO_DEPTH)]);
			fifo_img[((current_frame-2) % FIFO_DEPTH)] = gbitmap_create_with_resource(AMD_LOGO_RESOURCE_IDS[current_frame + 2]);
		}
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
		if (current_frame == (uint32_t) (FRAMES-1)) {
	    	timer = app_timer_register(999, &handle_timer, (void *) next_frame);
		} else {
	    	timer = app_timer_register(166, &handle_timer, (void *) next_frame);
		}
		return;
	}
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	// Need to be static because they're used by the system later.
    static char date_text[] = "Sun Jan 00---------------------------";
    static char wday_text[] = "XxxXxxXxxXxxXxx";
    static char mnth_text[] = "YyyYyyYyyYyyYyy";
    static char wdat_text[] = "00";
    static char time_text[] = "00:00";
	static int show = 0;
    char *time_format;

    strftime(wday_text, sizeof(wday_text), "%a", tick_time);
    strftime(mnth_text, sizeof(mnth_text), "%b", tick_time);
	strftime(wdat_text, sizeof(wdat_text), "%e", tick_time);

	strcpy(date_text, wday_text); strcat(date_text, " ");
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
	if (show == 0) request_weather();
	show = (show + 1) % 30;
}

void handle_init(void) {
    my_window = window_create();
	window_set_background_color(my_window, GColorBlack);
    window_stack_push(my_window, true);

    date_text_layer = text_layer_create(GRect(8, 0, 144, 30));
    text_layer_set_font(date_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(date_text_layer, GTextAlignmentLeft);
    text_layer_set_text_color(date_text_layer, GColorSpringBud);	
    text_layer_set_background_color(date_text_layer, GColorBlack);
    text_layer_set_text(date_text_layer, "Sun Jan 1");
	layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(date_text_layer));	
 
    temp_text_layer = text_layer_create(GRect(8, 24, 144, 30));
    text_layer_set_font(temp_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(temp_text_layer, GTextAlignmentLeft);
    text_layer_set_text_color(temp_text_layer, GColorSpringBud);	
    text_layer_set_background_color(temp_text_layer, GColorBlack);
    text_layer_set_text(temp_text_layer, "0000\u00B0C");
    layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(temp_text_layer));	
	
    time_text_layer = text_layer_create(GRect(0, 46, 144, 80));
    text_layer_set_font(time_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_52)));
    text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
    text_layer_set_text_color(time_text_layer, GColorBrightGreen);	
    text_layer_set_background_color(time_text_layer, GColorBlack);
	text_layer_set_text(time_text_layer, "00:00");
	layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(time_text_layer));

	s_icon_layer = bitmap_layer_create(GRect(48, 27, 20, 20));
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_icon_layer));

    bkgd_img        = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO_FINAL);
    layer_bkgd_img  = bitmap_layer_create(GRect(0, 102, 144, 68));
    bitmap_layer_set_bitmap(layer_bkgd_img, bkgd_img);
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(layer_bkgd_img));	
 	
    bt_connect_img     = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECT);
    bt_disconnect_img  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECT);
    layer_conn_img     = bitmap_layer_create(GRect(118, 10, 20, 20));
    bitmap_layer_set_bitmap(layer_conn_img, bt_connect_img);
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(layer_conn_img));	

	bluetooth_connection_service_subscribe(&handle_bluetooth);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	
	app_message_open(64, 64);
    Tuplet initial_values[] = {
    	TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    	TupletCString(WEATHER_TEMPERATURE_KEY, "----\u00B0C"),
    	TupletCString(WEATHER_CITY_KEY, "St Pebblesburg"),
  	};
	app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_tuple_changed_callback, sync_error_callback, NULL);
 }

void handle_deinit(void) {
    gbitmap_destroy(bt_connect_img);
    gbitmap_destroy(bt_disconnect_img);
    gbitmap_destroy(bkgd_img);
 	for (int i=0; i<FIFO_DEPTH; i++) gbitmap_destroy(fifo_img[i]);
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
