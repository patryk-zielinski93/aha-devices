//
// Created by zielq on 01.07.2025.
//

#ifndef AHA_DEVICES_BUTTONMAPPER_H
#define AHA_DEVICES_BUTTONMAPPER_H

#include <Arduino.h>

/**
 * @brief A generic mapping structure that links a technical control point ID
 * to a user-defined logical button ID enum.
 * @tparam T_ButtonIdEnum The controller-specific 'enum class' for button IDs.
 */
template <typename T_ButtonIdEnum>
struct ButtonMapping {
    uint16_t controlPointId;
    T_ButtonIdEnum buttonId;
};

/**
 * @brief A generic, compile-time function to find a logical button ID from a map.
 * @tparam T_Mapping The type of the mapping struct (e.g., ButtonMapping<MyEnum>).
 * @tparam N The size of the mapping array, deduced automatically by the compiler.
 * @param map A reference to the static C-style mapping array.
 * @param controlPointId The technical ID of the button to find.
 * @return The logical button ID enum value, or a default 'UNKNOWN' value if not found.
 */
template <typename T_Mapping, size_t N>
constexpr auto getButtonId(const T_Mapping(&map)[N], uint16_t controlPointId) {
    // The loop works identically for both std::array and C-style arrays.
    for (const auto& mapping : map) {
        if (mapping.controlPointId == controlPointId) {
            return mapping.buttonId;
        }
    }
    // Assumes that the user's enum will have an 'UNKNOWN' member with value 0.
    return static_cast<decltype(map[0].buttonId)>(0);
}

#endif // AHA_DEVICES_BUTTONMAPPER_H
