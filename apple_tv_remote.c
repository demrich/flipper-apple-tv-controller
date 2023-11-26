#include <infrared_worker.h>
#include <infrared_transmit.h>

#include "apple_tv_remote.h"

#define TAG "AppleTVRemoteApp"

enum AppleTVRemoteSubmenuIndex {
    AppleTVRemoteSubmenuIndexAppleTV,
    AppleTVRemoteSubmenuIndexPower
};

uint32_t usb_hid_exit_confirm_view(void* context) {
    UNUSED(context);
    return UsbHidViewSubmenu;
}

uint32_t usb_hid_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

void usb_hid_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    AppleTVRemote* app = context;
    if(index == AppleTVRemoteSubmenuIndexAppleTV) {
        app->view_id = UsbHidViewAppleTVRemote;
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewAppleTVRemote);
    } else if(index == AppleTVRemoteSubmenuIndexPower) {
        InfraredMessage* message = malloc(sizeof(InfraredMessage));
        message->protocol = InfraredProtocolNECext;
        message->address = 0xEE87;
        message->command = 0xD02F; // Set this to your televisions power button
        message->repeat = false;
        infrared_send(message, 2);
        free(message);
    }
}

void usb_hid_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    AppleTVRemote* app = context;
    if(result == DialogExResultLeft) {
        view_dispatcher_stop(app->view_dispatcher);
    } else if(result == DialogExResultRight) {
        view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id); // Show last view
    } else if(result == DialogExResultCenter) {
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewSubmenu);
    }
}

AppleTVRemote* apple_tv_remote_app_alloc() {
    AppleTVRemote* app = malloc(sizeof(AppleTVRemote));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->view_dispatcher = view_dispatcher_alloc();

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Submenu view
    app->submenu = submenu_alloc();
    view_set_orientation(submenu_get_view(app->submenu), ViewOrientationVertical);

    submenu_add_item(
        app->submenu, "Apple TV", AppleTVRemoteSubmenuIndexAppleTV, usb_hid_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Power", AppleTVRemoteSubmenuIndexPower, usb_hid_submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), usb_hid_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, UsbHidViewSubmenu, submenu_get_view(app->submenu));

    // Apple TV Controller View
    app->apple_tv_remote_view = apple_tv_remote_view_alloc(app->notifications);
    view_set_previous_callback(
        apple_tv_remote_view_get_view(app->apple_tv_remote_view), usb_hid_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        UsbHidViewAppleTVRemote,
        apple_tv_remote_view_get_view(app->apple_tv_remote_view));

    // Switch to Apple TV Controller by default
    app->view_id = UsbHidViewSubmenu;
    view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id);

    return app;
}

void apple_tv_remote_app_free(AppleTVRemote* app) {
    furi_assert(app);

    // Reset notification
    notification_internal_message(app->notifications, &sequence_reset_blue);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewAppleTVRemote);
    apple_tv_remote_view_free(app->apple_tv_remote_view);
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);
    app->gui = NULL;
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Free rest
    free(app);
}

int32_t apple_tv_remote_app(void* p) {
    UNUSED(p);

    AppleTVRemote* app = apple_tv_remote_app_alloc();
    view_dispatcher_run(app->view_dispatcher);

    apple_tv_remote_app_free(app);

    return 0;
}
