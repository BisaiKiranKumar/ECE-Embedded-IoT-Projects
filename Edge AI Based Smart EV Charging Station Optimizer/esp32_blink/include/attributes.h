#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <ArduinoJson.h>

// Function declarations
void requestSharedAttributes();
void applySharedAttributes(JsonObject attrs);

#endif // ATTRIBUTES_H