#include <pebble.h>

static Window *s_main_window;

static TextLayer *s_time_layer;

static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence;
static GBitmap *s_bitmap;

static void timer_handler(void *context) {
    uint32_t next_delay;
    
    // Advance to the next APNG frame, and get the delay for this frame
    if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
        // Set the new frame into the BitmapLayer
        bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
        layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
        
        // Timer for that frame's delay
        app_timer_register(next_delay, timer_handler, NULL);
    }
}

static void update_datetime() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    // Create a long-lived buffer
    static char time_buffer[] = "00:00";
    
    // Write the current hours and minutes into the buffer
    if(clock_is_24h_style() == true) {
        // Use 24 hour format
        strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
    } else {
        // Use 12 hour format
        strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
    }
    
    // Display on the TextLayer
    text_layer_set_text(s_time_layer, time_buffer);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_datetime();
}

static void load_sequence() {
    // Create sequence
    s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION4);
    
    // Create blank GBitmap using APNG frame size
    GSize frame_size = gbitmap_sequence_get_bitmap_size(s_sequence);
    s_bitmap = gbitmap_create_blank(frame_size, GBitmapFormat8Bit);
    
    uint32_t first_delay_ms = 0;
    
    // Schedule a timer to advance the first frame
    app_timer_register(first_delay_ms, timer_handler, NULL);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    
    // BitmapLayer - begin
    s_bitmap_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
    bitmap_layer_set_background_color(s_bitmap_layer, (GColor8){ .argb = GColorVividCeruleanARGB8 });
    bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
    load_sequence();
    // BitmapLayer - end
    
    // Time - begin
    s_time_layer = text_layer_create(GRect(0, 132, 35, 14));
    text_layer_set_background_color(s_time_layer, (GColor8){ .argb = GColorVividCeruleanARGB8 });
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");
    
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
    // Time - end
    update_datetime();
}

static void main_window_unload(Window *window) {
    bitmap_layer_destroy(s_bitmap_layer);
    gbitmap_sequence_destroy(s_sequence);
    gbitmap_destroy(s_bitmap);
    text_layer_destroy(s_time_layer);
}

static void init() {
    s_main_window = window_create();
    
    window_set_background_color(s_main_window, (GColor8){ .argb = GColorVividCeruleanARGB8 });
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
    });
    
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    
    window_stack_push(s_main_window, true);
}

static void deinit() {
    window_destroy(s_main_window);
}

int main() {
    init();
    app_event_loop();
    deinit();
}
