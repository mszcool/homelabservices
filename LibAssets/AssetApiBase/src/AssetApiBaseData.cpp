#include "AssetApiBaseData.h"

#include <SPIFFS.h>

AssetBaseRepository::AssetBaseRepository()
{
    Serial.println("AssetBaseRepository::AssetBaseRepository - enter");

    if (!SPIFFS.begin())
    {
        Serial.println("Failed to mount file system, formatting...");
        SPIFFS.format();
        if(!SPIFFS.begin())
        {
            Serial.println("Failed to mount file system, aborting...");
            return;
        }
    }

    Serial.println("AssetBaseRepository::AssetBaseRepository - exit");
}

AssetBaseRepository::~AssetBaseRepository()
{
    SPIFFS.end();
}

AssetMetadataParams AssetBaseRepository::loadMetadata()
{
    Serial.println("AssetBaseRepository::loadMetadata - enter");

    AssetMetadataParams metadata;
    File file = SPIFFS.open(ASSET_METADATA_FILENAME, "r");
    if (file)
    {
        file.readBytes((char *)&metadata, sizeof(metadata));
        file.close();
    }
    else
    {
        Serial.println("AssetBaseRepository::loadMetadata - failed to open file - returning defaults");
        metadata.sensorName[0] = '\0';
        metadata.sensorLocation[0] = '\0';
        metadata.sensorMqttServer[0] = '\0';
        metadata.sensorMqttUsername[0] = '\0';
        metadata.sensorMqttPassword[0] = '\0';
        metadata.sensorMqttPort = 0;
    }

    Serial.println("AssetBaseRepository::loadMetadata - sensorName = " + String(metadata.sensorName));
    Serial.println("AssetBaseRepository::loadMetadata - sensorLocation = " + String(metadata.sensorLocation));
    Serial.println("AssetBaseRepository::loadMetadata - sensorMqttServer = " + String(metadata.sensorMqttServer));
    Serial.println("AssetBaseRepository::loadMetadata - exit");
    return metadata;
}

bool AssetBaseRepository::saveMetadata(AssetMetadataParams metadata)
{
    Serial.println("AssetBaseRepository::saveMetadata - enter");

    bool succeeded = false;
    File file = SPIFFS.open(ASSET_METADATA_FILENAME, "w");
    if (file)
    {
        file.write((const uint8_t *)&metadata, sizeof(metadata));
        file.close();
        succeeded = true;
    }
    else
    {
        Serial.println("AssetBaseRepository::saveMetadata - failed to open file");
    }

    Serial.println("AssetBaseRepository::saveMetadata - exit");
    return succeeded;
}