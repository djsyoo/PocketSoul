#include "pet_core.hpp"
#include <algorithm>

namespace pocket_soul {
static uint16_t clamp100(int value) { return static_cast<uint16_t>(std::clamp(value, 0, 100)); }
void tick(PetState& p, uint32_t seconds) {
    // Clamp offline catch-up to 72 hours to avoid pathological time jumps.
    seconds = std::min<uint32_t>(seconds, 72u * 60u * 60u);
    p.age_minutes += seconds / 60;
    // Slow, gentle needs: no death or harsh punishment while offline.
    p.hunger = clamp100(p.hunger - static_cast<int>(seconds / 900)); // 1 point / 15 min
    p.energy = clamp100(p.energy - static_cast<int>(seconds / 1800));
    p.cleanliness = clamp100(p.cleanliness - static_cast<int>(seconds / 3600));
    if (p.energy < 25) p.mood = clamp100(p.mood - 1);
    if (p.hunger < 30) p.mood = clamp100(p.mood - 1);
    if (p.hunger > 65 && p.energy > 60) p.mood = clamp100(p.mood + 1);
}
Mood current_mood(const PetState& p) {
    if (p.energy < 20) return Mood::Sleepy;
    if (p.hunger < 25) return Mood::Hungry;
    if (p.affection > 70 && p.mood > 70) return Mood::Happy;
    if ((p.personality_seed & 0xFF) > 180) return Mood::Curious;
    if (p.trust < 20) return Mood::Shy;
    return Mood::Calm;
}
PetEvent interact(PetState& p, Interaction action) {
    switch (action) {
    case Pet:
        p.affection = clamp100(p.affection + 2);
        p.trust = clamp100(p.trust + 1);
        p.mood = clamp100(p.mood + 3);
        p.animation = 1; // pet reaction
        return {"That feels nice!", Mood::Happy, p.animation};
    case Feed:
        p.hunger = clamp100(p.hunger + 25);
        p.mood = clamp100(p.mood + 2);
        p.animation = 2; // eating
        return {"Yummy!", Mood::Happy, p.animation};
    case Play:
        if (p.energy >= 12) {
            p.energy = clamp100(p.energy - 12);
            p.affection = clamp100(p.affection + 2);
            p.mood = clamp100(p.mood + 5);
            p.animation = 3; // playful
            return {"Let's play!", Mood::Happy, p.animation};
        }
        p.animation = 4; // tired
        return {"A little tired...", Mood::Sleepy, p.animation};
    }
    return {"", Mood::Calm, 0};
}
}
