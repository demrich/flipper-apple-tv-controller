#include "apple_tv_remote_view.h"

#include <infrared_worker.h>
#include <infrared_transmit.h>

struct AppleTVRemoteView {
    View* view;
    NotificationApp* notifications;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool back_pressed;
    bool connected;
} AppleTVRemoteViewModel;

static void
    apple_tv_remote_view_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
    if(dir == CanvasDirectionBottomToTop) {
        canvas_draw_triangle(canvas, x - 2, y - 2, 5, 3, dir);
        canvas_draw_line(canvas, x - 2, y - 3, x - 2, y + 4);
    } else if(dir == CanvasDirectionTopToBottom) {
        canvas_draw_triangle(canvas, x - 2, y + 2, 5, 3, dir);
        canvas_draw_line(canvas, x - 2, y - 4, x - 2, y + 3);
    } else if(dir == CanvasDirectionRightToLeft) {
        canvas_draw_triangle(canvas, x - 4, y, 5, 3, dir);
        canvas_draw_line(canvas, x + 2, y, x - 5, y);
    } else if(dir == CanvasDirectionLeftToRight) {
        canvas_draw_triangle(canvas, x, y, 5, 3, dir);
        canvas_draw_line(canvas, x - 6, y, x + 1, y);
    }
}

static void apple_tv_remote_view_draw_arrow_button(
    Canvas* canvas,
    bool pressed,
    uint8_t x,
    uint8_t y,
    CanvasDirection direction) {
    canvas_draw_icon(canvas, x, y, &I_Button_18x18);
    if(pressed) {
        elements_slightly_rounded_box(canvas, x + 3, y + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    apple_tv_remote_view_draw_arrow(canvas, x + 11, y + 8, direction);
    canvas_set_color(canvas, ColorBlack);
}

static void apple_tv_remote_draw_wide_button(
    Canvas* canvas,
    bool pressed,
    uint8_t x,
    uint8_t y,
    char* text,
    const Icon* icon) {
    // canvas_draw_icon(canvas, 0, 25, &I_Space_65x18);
    elements_slightly_rounded_frame(canvas, x, y, 64, 17);
    if(pressed) {
        elements_slightly_rounded_box(canvas, x + 2, y + 2, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, x + 11, y + 4, icon);
    elements_multiline_text_aligned(canvas, x + 28, y + 12, AlignLeft, AlignBottom, text);
    canvas_set_color(canvas, ColorBlack);
}

static void apple_tv_remote_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    AppleTVRemoteViewModel* model = context;

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Apple TV");

    canvas_set_font(canvas, FontSecondary);

    canvas_draw_icon(canvas, 0, 12, &I_Pin_back_arrow_10x8);
    canvas_draw_str(canvas, 12, 20, "Hold");

    apple_tv_remote_view_draw_arrow_button(
        canvas, model->up_pressed, 23, 74, CanvasDirectionBottomToTop);
    apple_tv_remote_view_draw_arrow_button(
        canvas, model->down_pressed, 23, 110, CanvasDirectionTopToBottom);
    apple_tv_remote_view_draw_arrow_button(
        canvas, model->left_pressed, 0, 92, CanvasDirectionRightToLeft);
    apple_tv_remote_view_draw_arrow_button(
        canvas, model->right_pressed, 46, 92, CanvasDirectionLeftToRight);

    int buttons_post = 30;
    // Ok
    apple_tv_remote_draw_wide_button(
        canvas, model->ok_pressed, 0, buttons_post, "Center", &I_Ok_btn_9x9);
    // Back
    apple_tv_remote_draw_wide_button(
        canvas, model->back_pressed, 0, buttons_post + 19, "Back", &I_Pin_back_arrow_10x8);
}

const NotificationSequence sequence_blink_purple_50 = {
    &message_red_255,
    &message_blue_255,
    &message_delay_50,
    NULL,
};

void send_apple_tv_ir(uint32_t command, NotificationApp* notifications) {
    InfraredMessage* message = malloc(sizeof(InfraredMessage));
    message->protocol = InfraredProtocolNECext;
    message->address = 0xEE87;
    message->command = command;
    message->repeat = false;
    notification_message(notifications, &sequence_blink_purple_50);
    infrared_send(message, 2);
    free(message);
}

static void
    apple_tv_remote_view_process(AppleTVRemoteView* apple_tv_remote_view, InputEvent* event) {
    with_view_model(
        apple_tv_remote_view->view,
        AppleTVRemoteViewModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    send_apple_tv_ir(0x0B2E, apple_tv_remote_view->notifications);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    send_apple_tv_ir(0x0D2E, apple_tv_remote_view->notifications);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                    send_apple_tv_ir(0x082E, apple_tv_remote_view->notifications);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    send_apple_tv_ir(0x072E, apple_tv_remote_view->notifications);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                    send_apple_tv_ir(0x5D2E, apple_tv_remote_view->notifications);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                    send_apple_tv_ir(0x022E, apple_tv_remote_view->notifications);
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = false;
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                    // furi_hal_hid_kb_press(HID_KEYBOARD_DELETE);
                    // furi_hal_hid_kb_release(HID_KEYBOARD_DELETE);
                    // furi_hal_hid_consumer_key_press(HID_CONSUMER_AC_BACK);
                    // furi_hal_hid_consumer_key_release(HID_CONSUMER_AC_BACK);
                }
            }
        },
        true);
}

static bool apple_tv_remote_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    AppleTVRemoteView* apple_tv_remote_view = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        // LONG KEY BACK PRESS HANDLER
    } else {
        apple_tv_remote_view_process(apple_tv_remote_view, event);
        consumed = true;
    }

    return consumed;
}

AppleTVRemoteView* apple_tv_remote_view_alloc(NotificationApp* notifications) {
    AppleTVRemoteView* apple_tv_remote_view = malloc(sizeof(AppleTVRemoteView));
    apple_tv_remote_view->view = view_alloc();
    apple_tv_remote_view->notifications = notifications;
    view_set_orientation(apple_tv_remote_view->view, ViewOrientationVertical);
    view_set_context(apple_tv_remote_view->view, apple_tv_remote_view);
    view_allocate_model(
        apple_tv_remote_view->view, ViewModelTypeLocking, sizeof(AppleTVRemoteViewModel));
    view_set_draw_callback(apple_tv_remote_view->view, apple_tv_remote_view_draw_callback);
    view_set_input_callback(apple_tv_remote_view->view, apple_tv_remote_view_input_callback);

    return apple_tv_remote_view;
}

void apple_tv_remote_view_free(AppleTVRemoteView* apple_tv_remote_view) {
    furi_assert(apple_tv_remote_view);
    view_free(apple_tv_remote_view->view);
    free(apple_tv_remote_view);
}

View* apple_tv_remote_view_get_view(AppleTVRemoteView* apple_tv_remote_view) {
    furi_assert(apple_tv_remote_view);
    return apple_tv_remote_view->view;
}

void apple_tv_remote_view_set_connected_status(
    AppleTVRemoteView* apple_tv_remote_view,
    bool connected) {
    furi_assert(apple_tv_remote_view);
    with_view_model(
        apple_tv_remote_view->view,
        AppleTVRemoteViewModel * model,
        { model->connected = connected; },
        true);
}
