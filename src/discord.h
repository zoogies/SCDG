#ifndef DISCORD_H
#define DISCORD_H

void init_discord_rich_presence();
void update_discord_activity(const char* details, const char* state, const char* large_image, const char* large_text);
void run_discord_callbacks();
void shutdown_discord_rich_presence();

#endif  // DISCORD_H