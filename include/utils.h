#include <Arduino.h>

void printHeapInfo() {
    Serial.print("Free heap: ");
    Serial.println(esp_get_free_heap_size()); // Total free heap in bytes

    Serial.print("Minimum free heap: ");
    Serial.println(esp_get_minimum_free_heap_size()); // Lowest heap size recorded since boot

    Serial.print("Largest free block: ");
    Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT)); // Largest allocatable block
}

void printTaskStackInfo() {
    TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
    Serial.print("Task: ");
    Serial.println(pcTaskGetName(currentTask));

    Serial.print("Free stack (bytes): ");
    Serial.println(uxTaskGetStackHighWaterMark(currentTask) * sizeof(StackType_t));
}