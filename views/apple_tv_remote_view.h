#pragma once

#include <gui/view.h>
#include <furi.h>
#include <gui/elements.h>
#include <xc_icons.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>


typedef struct AppleTVRemoteView AppleTVRemoteView;

AppleTVRemoteView* apple_tv_remote_view_alloc();

void apple_tv_remote_view_free(AppleTVRemoteView* apple_tv_remote_view);

View* apple_tv_remote_view_get_view(AppleTVRemoteView* apple_tv_remote_view);

void apple_tv_remote_view_set_connected_status(
    AppleTVRemoteView* apple_tv_remote_view,
    bool connected);
