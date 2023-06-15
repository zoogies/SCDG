#include "discord.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "discordSDK/discord_game_sdk.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <string.h>
#endif

struct Application {
    struct IDiscordCore* core;
    struct IDiscordActivityManager* activities;
};

struct Application app;

void DISCORD_CALLBACK UpdateActivityCallback(void* data, enum EDiscordResult result)
{
    (void)data; // Use the unused parameter to suppress warning
    if (result != DiscordResult_Ok) {
        printf("Error updating Discord activity: %d\n", result);
    }
}

int init_discord_rich_presence()
{
    memset(&app, 0, sizeof(app));

    struct IDiscordActivityEvents activities_events;
    memset(&activities_events, 0, sizeof(activities_events));

    struct DiscordCreateParams params;
    DiscordCreateParamsSetDefault(&params);
    params.client_id = 1114551124448510023;
    params.flags = DiscordCreateFlags_Default;
    params.event_data = &app;
    params.activity_events = &activities_events;

    if (DiscordCreate(DISCORD_VERSION, &params, &app.core) != DiscordResult_Ok) {
        app.core = NULL;
        return 0;
    }

    app.activities = app.core->get_activity_manager(app.core);
    return 1;
}

void update_discord_activity(const char* details, const char* state, const char* large_image, const char* large_text)
{
    if(app.core == NULL) return;
    struct DiscordActivity activity;
    memset(&activity, 0, sizeof(activity));
    sprintf(activity.details, "%s", details);
    sprintf(activity.state, "%s", state);
    sprintf(activity.assets.large_image, "%s", large_image);
    sprintf(activity.assets.large_text, "%s", large_text);

    app.activities->update_activity(app.activities, &activity, &app, UpdateActivityCallback);
}

void run_discord_callbacks()
{
    if (app.core != NULL) {
        enum EDiscordResult result = app.core->run_callbacks(app.core);
        if (result != DiscordResult_Ok) {
            printf("Error running Discord callbacks: %d\n", result);
        }
    }
}

void shutdown_discord_rich_presence()
{
    if(app.core == NULL) return;
    app.core->destroy(app.core);
}

// this whole file was refactored by gpt4 from the awful discord example
// TODO: comment and double check this code