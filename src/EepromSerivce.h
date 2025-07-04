#ifndef EEPROMSERVICE_H
#define EEPROMSERVICE_H

#include <Arduino.h>
#include <EEPROM.h>

/**
 * @class EepromService
 * @brief A generic, static template-based class for handling EEPROM writes and reads
 * with a wear-leveling algorithm.
 */
class EepromService
{
private:
    /**
     * @struct Record
     * @brief A template struct for a single record in EEPROM.
     * @tparam T The data type to be stored.
     */
    template <typename T>
    struct Record
    {
        uint32_t writeCounter; // The number of writes performed
        T value; // The stored value of type T
    };

public:
    /**
     * @brief Writes a value of any type T to the EEPROM using wear leveling.
     * @tparam T The data type of the value to write.
     * @param startAddress The starting physical address in EEPROM for this data's block.
     * @param value The value to write.
     * @param slots The number of wear-leveling slots to use for this data block.
     */
    template <typename T>
    static void write(uint16_t startAddress, T value, uint8_t slots = 10)
    {
        Record<T> record;
        const uint16_t recordSize = sizeof(record);

        uint32_t latestCounter = 0;
        uint8_t latestIndex = 0;
        bool foundAny = false;

        // 1. Find the last written slot
        for (uint8_t i = 0; i < slots; ++i)
        {
            EEPROM.get(startAddress + i * recordSize, record);

            if (record.writeCounter != 0xFFFFFFFF)
            {
                if (!foundAny || (int32_t)(record.writeCounter - latestCounter) > 0)
                {
                    latestCounter = record.writeCounter;
                    latestIndex = i;
                }
                foundAny = true;
            }
        }

        // 2. Prepare and write the new record in the next slot
        uint8_t nextIndex = foundAny ? (latestIndex + 1) % slots : 0;

        Record<T> newRecord = {
            .writeCounter = latestCounter + 1,
            .value = value
        };

        EEPROM.put(startAddress + nextIndex * recordSize, newRecord);
    }

    /**
     * @brief Reads a value of any type T from the EEPROM.
     * @tparam T The data type of the value to read.
     * @param startAddress The starting physical address in EEPROM for this data's block.
     * @param defaultValue The value to return if no valid record is found.
     * @param slots The number of wear-leveling slots configured for this data block.
     * @return The read value or the defaultValue.
     */
    template <typename T>
    static T read(uint16_t startAddress, T defaultValue = 0, uint8_t slots = 10)
    {
        Record<T> record;
        const uint16_t recordSize = sizeof(record);

        uint32_t latestCounter = 0;
        T latestValue = defaultValue;
        bool foundAny = false;

        // Find the slot with the highest counter and return its value
        for (uint8_t i = 0; i < slots; ++i)
        {
            EEPROM.get(startAddress + i * recordSize, record);

            if (record.writeCounter != 0xFFFFFFFF)
            {
                if (!foundAny || (int32_t)(record.writeCounter - latestCounter) > 0)
                {
                    latestCounter = record.writeCounter;
                    latestValue = record.value;
                }
                foundAny = true;
            }
        }

        return latestValue;
    }
};

#endif //EEPROMSERVICE_H
