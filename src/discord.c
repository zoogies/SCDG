#include "discord.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "discordSDK/discord_game_sdk.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <string.h>
#endif

#define DISCORD_REQUIRE(x) assert(x == DiscordResult_Ok)

struct Application {
    struct IDiscordCore* core;
    struct IDiscordActivityManager* activities;
};

struct Application app;

void DISCORD_CALLBACK UpdateActivityCallback(void* data, enum EDiscordResult result)
{
    DISCORD_REQUIRE(result);
}

void init_discord_rich_presence()
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

    DISCORD_REQUIRE(DiscordCreate(DISCORD_VERSION, &params, &app.core));

    app.activities = app.core->get_activity_manager(app.core);
}

void update_discord_activity(const char* details, const char* state, const char* large_image, const char* large_text)
{
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
    DISCORD_REQUIRE(app.core->run_callbacks(app.core));
}

void shutdown_discord_rich_presence()
{
    app.core->destroy(app.core);
}

// this whole file was refactored by gpt4 from the awful discord example
// TODO: comment and double check this code