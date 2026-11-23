#pragma once
#include <cstdint>
#include <string>

namespace pocket_soul {

enum class Mood : uint8_t { Calm, Happy, Sleepy, Hungry, Curious, Shy };
enum Interaction : uint8_t { Pet = 0, Feed = 1, Play = 2 };

struct PetState {
    char name[16] = "Xingzai";
    uint32_t personality_seed = 0x9A724C81;
    uint32_t age_minutes = 0;
    uint16_t hunger = 70;      // 0..100; higher = satiated
    uint16_t energy = 75;     // 0..100
    uint16_t mood = 70;       // 0..100
    uint16_t affection = 10;  // 0..100
    uint16_t trust = 10;      // 0..100
    uint16_t cleanliness = 90;// 0..100
    uint32_t last_epoch = 0;
    uint8_t animation = 0;
};

struct PetEvent {
    const char* text;
    Mood mood;
    uint8_t animation;
};

void tick(PetState& pet, uint32_t elapsed_seconds);
PetEvent interact(PetState& pet, Interaction action);
Mood current_mood(const PetState& pet);
}
