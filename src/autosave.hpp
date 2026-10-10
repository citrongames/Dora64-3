#pragma once
namespace dora3::autosave {
enum class Notification { None, Saved, Failed };
void configure(bool enabled);
// Called after old guest threads stop, before a new session begins.
void reset();
Notification notification();
}
