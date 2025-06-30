/**
 * Control points on the walls (wall switches) contains Ethernet ("RJ45") cables with 8 wires.
 * It is possible that multiple RJ45 cables are connected with control point.
 * Naming convention: CP_[controlPointNumber]_[cableNumber]_[wireNumber]
 * E.g.: CP11x2x6 means: control point 11, cable no.2 and wire no.6
 *
 * Ethernet wire colors (T568B), numbering and reserved wires:
 * 1. Orange/White - free to use
 * 2. Orange       - free to use
 * 3. Green/White  - free to use
 * 4. Blue         - free to use (optionally GND) // can be connected if there is no more free wires and 5V Brown/White wire is already used
 * 5. Blue/White   - free to use
 * 6. Green        - free to use
 * 7. Brown/White  - free to use (optionally 5V DC) // can be connected if there is no more free wires (before using Blue wire)
 * 8. Brown        - reserved for 24V DC // excluded from ControlPoint enum
 */

#ifndef AHA_DEVICES_CONTROLPOINT_H
#define AHA_DEVICES_CONTROLPOINT_H

#include <Arduino.h>

// Naming convention: CP_[controlPointNumber]_[cableNumber]_[wireNumber]
enum ControlPoint : uint16_t {
    CP_1_1_1,
    CP_1_1_2,
    CP_1_1_3,
    CP_1_1_4,
    CP_1_1_5,
    CP_1_1_6,
    CP_1_1_7,

    CP_1_2_1,
    CP_1_2_2,
    CP_1_2_3,
    CP_1_2_4,
    CP_1_2_5,
    CP_1_2_6,
    CP_1_2_7,


    CP_2_1_1,
    CP_2_1_2,
    CP_2_1_3,
    CP_2_1_4,
    CP_2_1_5,
    CP_2_1_6,
    CP_2_1_7,


    CP_3_1_1,
    CP_3_1_2,
    CP_3_1_3,
    CP_3_1_4,
    CP_3_1_5,
    CP_3_1_6,
    CP_3_1_7,


    CP_4_1_1,
    CP_4_1_2,
    CP_4_1_3,
    CP_4_1_4,
    CP_4_1_5,
    CP_4_1_6,
    CP_4_1_7,


    CP_5_1_1,
    CP_5_1_2,
    CP_5_1_3,
    CP_5_1_4,
    CP_5_1_5,
    CP_5_1_6,
    CP_5_1_7,


    CP_6_1_1,
    CP_6_1_2,
    CP_6_1_3,
    CP_6_1_4,
    CP_6_1_5,
    CP_6_1_6,
    CP_6_1_7,


    CP_7_1_1,
    CP_7_1_2,
    CP_7_1_3,
    CP_7_1_4,
    CP_7_1_5,
    CP_7_1_6,
    CP_7_1_7,

    CP_7_2_1,
    CP_7_2_2,
    CP_7_2_3,
    CP_7_2_4,
    CP_7_2_5,
    CP_7_2_6,
    CP_7_2_7,


    CP_8_1_1,
    CP_8_1_2,
    CP_8_1_3,
    CP_8_1_4,
    CP_8_1_5,
    CP_8_1_6,
    CP_8_1_7,

    CP_8_2_1,
    CP_8_2_2,
    CP_8_2_3,
    CP_8_2_4,
    CP_8_2_5,
    CP_8_2_6,
    CP_8_2_7,

    CP_8_3_1,
    CP_8_3_2,
    CP_8_3_3,
    CP_8_3_4,
    CP_8_3_5,
    CP_8_3_6,
    CP_8_3_7,


    CP_9_1_1,
    CP_9_1_2,
    CP_9_1_3,
    CP_9_1_4,
    CP_9_1_5,
    CP_9_1_6,
    CP_9_1_7,

    CP_9_2_1,
    CP_9_2_2,
    CP_9_2_3,
    CP_9_2_4,
    CP_9_2_5,
    CP_9_2_6,
    CP_9_2_7,


    CP_10_1_1,
    CP_10_1_2,
    CP_10_1_3,
    CP_10_1_4,
    CP_10_1_5,
    CP_10_1_6,
    CP_10_1_7,


    CP_11_1_1,
    CP_11_1_2,
    CP_11_1_3,
    CP_11_1_4,
    CP_11_1_5,
    CP_11_1_6,
    CP_11_1_7,


    CP_12_1_1,
    CP_12_1_2,
    CP_12_1_3,
    CP_12_1_4,
    CP_12_1_5,
    CP_12_1_6,
    CP_12_1_7,


    CP_13_1_1,
    CP_13_1_2,
    CP_13_1_3,
    CP_13_1_4,
    CP_13_1_5,
    CP_13_1_6,
    CP_13_1_7,


    CP_14_1_1,
    CP_14_1_2,
    CP_14_1_3,
    CP_14_1_4,
    CP_14_1_5,
    CP_14_1_6,
    CP_14_1_7,


    CP_15_1_1,
    CP_15_1_2,
    CP_15_1_3,
    CP_15_1_4,
    CP_15_1_5,
    CP_15_1_6,
    CP_15_1_7,


    CP_16_1_1,
    CP_16_1_2,
    CP_16_1_3,
    CP_16_1_4,
    CP_16_1_5,
    CP_16_1_6,
    CP_16_1_7,


    CP_17_1_1,
    CP_17_1_2,
    CP_17_1_3,
    CP_17_1_4,
    CP_17_1_5,
    CP_17_1_6,
    CP_17_1_7,


    CP_18_1_1,
    CP_18_1_2,
    CP_18_1_3,
    CP_18_1_4,
    CP_18_1_5,
    CP_18_1_6,
    CP_18_1_7,


    CP_19_1_1,
    CP_19_1_2,
    CP_19_1_3,
    CP_19_1_4,
    CP_19_1_5,
    CP_19_1_6,
    CP_19_1_7,


    CP_20_1_1,
    CP_20_1_2,
    CP_20_1_3,
    CP_20_1_4,
    CP_20_1_5,


    CP_30_1_1,
    CP_30_1_2,
    CP_30_1_3,
    CP_30_1_4,
    CP_30_1_5,
    CP_30_1_6,
    CP_30_1_7,


    CP_31_1_1,
    CP_31_1_2,
    CP_31_1_3,
    CP_31_1_4,
    CP_31_1_5,
    CP_31_1_6,
    CP_31_1_7,


    CP_32_1_1,
    CP_32_1_2,
    CP_32_1_3,
    CP_32_1_4,
    CP_32_1_5,
    CP_32_1_6,
    CP_32_1_7,


    CP_33_1_1,
    CP_33_1_2,
    CP_33_1_3,
    CP_33_1_4,
    CP_33_1_5,
    CP_33_1_6,
    CP_33_1_7,


    CP_34_1_1,
    CP_34_1_2,
    CP_34_1_3,
    CP_34_1_4,
    CP_34_1_5,
    CP_34_1_6,
    CP_34_1_7,


    CP_35_1_1,
    CP_35_1_2,
    CP_35_1_3,
    CP_35_1_4,
    CP_35_1_5,
    CP_35_1_6,
    CP_35_1_7,


    CP_36_1_1,
    CP_36_1_2,
    CP_36_1_3,
    CP_36_1_4,
    CP_36_1_5,
    CP_36_1_6,
    CP_36_1_7,


    CP_37_1_1,
    CP_37_1_2,
    CP_37_1_3,
    CP_37_1_4,
    CP_37_1_5,
    CP_37_1_6,
    CP_37_1_7,


    CP_38_1_1,
    CP_38_1_2,
    CP_38_1_3,
    CP_38_1_4,
    CP_38_1_5,
    CP_38_1_6,
    CP_38_1_7,


    CP_39_1_1,
    CP_39_1_2,
    CP_39_1_3,
    CP_39_1_4,
    CP_39_1_5,
    CP_39_1_6,
    CP_39_1_7,


    CP_40_1_1,
    CP_40_1_2,
    CP_40_1_3,
    CP_40_1_4,
    CP_40_1_5,
    CP_40_1_6,
    CP_40_1_7,


    CP_41_1_1,
    CP_41_1_2,
    CP_41_1_3,
    CP_41_1_4,
    CP_41_1_5,
    CP_41_1_6,
    CP_41_1_7,


    CP_42_1_1,
    CP_42_1_2,
    CP_42_1_3,
    CP_42_1_4,
    CP_42_1_5,
    CP_42_1_6,
    CP_42_1_7,


    CP_43_1_1,
    CP_43_1_2,
    CP_43_1_3,
    CP_43_1_4,
    CP_43_1_5,
    CP_43_1_6,
    CP_43_1_7,


    CP_44_1_1,
    CP_44_1_2,
    CP_44_1_3,
    CP_44_1_4,
    CP_44_1_5,
    CP_44_1_6,
    CP_44_1_7,


    CP_45_1_1,
    CP_45_1_2,
    CP_45_1_3,
    CP_45_1_4,
    CP_45_1_5,
    CP_45_1_6,
    CP_45_1_7,


    CP_46_1_1,
    CP_46_1_2,
    CP_46_1_3,
    CP_46_1_4,
    CP_46_1_5,
    CP_46_1_6,
    CP_46_1_7,


    CP_47_1_1,
    CP_47_1_2,
    CP_47_1_3,
    CP_47_1_4,
    CP_47_1_5,
    CP_47_1_6,
    CP_47_1_7,


    CP_48_1_1,
    CP_48_1_2,
    CP_48_1_3,
    CP_48_1_4,
    CP_48_1_5,
    CP_48_1_6,
    CP_48_1_7,

    CP_48_2_1,
    CP_48_2_2,
    CP_48_2_3,
    CP_48_2_4,
    CP_48_2_5,
    CP_48_2_6,
    CP_48_2_7,


    CP_49_1_1,
    CP_49_1_2,
    CP_49_1_3,
    CP_49_1_4,
    CP_49_1_5,
    CP_49_1_6,
    CP_49_1_7,

    CP_AGGREGATED_1,
    CP_AGGREGATED_2,
    CP_AGGREGATED_3,
    CP_AGGREGATED_4,
    CP_AGGREGATED_5,
    CP_AGGREGATED_6,
    CP_AGGREGATED_7,
    CP_AGGREGATED_8,
    CP_AGGREGATED_9,
    CP_AGGREGATED_10,
};


#endif // AHA_DEVICES_CONTROLPOINT_H
