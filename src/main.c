#include <pebble.h>

Window *my_window;
TextLayer *text_layer;
BitmapLayer *layer_bkgd_img;
BitmapLayer *layer_conn_img;
GBitmap *img_bkgd;
GBitmap *img_bt_connect;
GBitmap *img_bt_disconnect;

void handle_bluetooth(bool connected) {
    if (connected) {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
    } else {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_disconnect);
        vibes_long_pulse();
    }
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    // Need to be static because they're used by the system later.
    static char time_text[] = "00:00";
    char *time_format;

    if (clock_is_24h_style()) {
        time_format = "%R";
    } else {
        time_format = "%I:%M";
    }

    strftime(time_text, sizeof(time_text), time_format, tick_time);
    if (!clock_is_24h_style() && (time_text[0] == '0')) {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }
    text_layer_set_text(text_layer, time_text);
}

void handle_init(void) {
  my_window = window_create();
  window_stack_push(my_window, true);

  text_layer = text_layer_create(GRect(0, 40, 144, 80));
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_text_color(text_layer, GColorBlack);	
  text_layer_set_text(text_layer, "00:00");
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(text_layer));	
	
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
 
  img_bkgd        = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AMD_LOGO);
  layer_bkgd_img  = bitmap_layer_create(GRect(2, 110, 144, 50));
  bitmap_layer_set_bitmap(layer_bkgd_img, img_bkgd);
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(layer_bkgd_img));	

  img_bt_connect     = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECT);
  img_bt_disconnect  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECT);
  layer_conn_img     = bitmap_layer_create(GRect(118, 12, 20, 20));
  bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(layer_conn_img));	

  bluetooth_connection_service_subscribe(&handle_bluetooth);
}

void handle_deinit(void) {
//
  bluetooth_connection_service_unsubscribe();	
  text_layer_destroy(text_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
