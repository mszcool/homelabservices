#include "AssetApiBase.h"

MszAssetApiBase::MszAssetApiBase()
{
}

MszAssetApiBase::MszAssetApiBase(short secretId, int serverPort)
{
    this->secretId = secretId;
    this->serverPort = serverPort;
}

MszAssetApiBase::~MszAssetApiBase()
{
}

void MszAssetApiBase::begin(MszSecretHandler *secretHandler)
{
    // First set protected members such as secretHandler.
    this->secretHandler = secretHandler;

    // Configure the endpoints that all assets should have.
    this->registerGetEndpoint(MszAssetApiBase::API_ENDPOINT_INFO, std::bind(&MszAssetApiBase::handleGetInfo, this));
    this->registerGetEndpoint(MszAssetApiBase::API_ENDPOINT_UPDATEINFO, std::bind(&MszAssetApiBase::handleUpdateInfo, this));

    // Then allow derived classes doing their configuration
    this->beginCfg();

    // Now begin the serving process
    this->beginServe();
}

void MszAssetApiBase::loop()
{
    if (!this->logLoopDone)
    {
        Serial.println("MszAssetApiBase::loop() - Starting loop.");
        this->logLoopDone = true;
    }
    this->handleClient();
}

bool MszAssetApiBase::authorize()
{
    bool authZResult = false;

    Serial.println("Asset API - authorize - enter");

    Serial.println("Checking if authorization is enabled...");
    char *secretResponse = this->secretHandler->getSecret(this->secretId);
    if (secretResponse == NULL)
    {
        Serial.println("No Secret found, authorization disabled!");
        authZResult = true;
    }
    else
    {
        String authHeader = this->getHttpHeader(MszAssetApiBase::HEADER_AUTHORIZATION);
        int firstPipe = authHeader.indexOf('|');
        int secondPipe = authHeader.indexOf('|', firstPipe + 1);

        if (firstPipe == -1 || secondPipe == -1)
        {
            // The authorization header does not contain two pipe characters
            // Return an empty string to indicate an error
            Serial.println("Switch API authorize FAILED - Invalid authorization token format - exit");
            authZResult = false;
        }
        else
        {
            // The timestamp is the substring before the first pipe character
            String timestampStr = authHeader.substring(0, firstPipe);
            String token = authHeader.substring(firstPipe + 1, secondPipe);
            String signature = authHeader.substring(secondPipe + 1);
            Serial.println("Switch API authorize - token: " + token);
            Serial.println("Switch API authorize - signature: " + signature);
            if (token == nullptr || token == "" || signature == nullptr || signature == "" || timestampStr == nullptr || timestampStr == "")
            {
                Serial.println("Switch API authorize FAILED NO TOKEN - exit");
                authZResult = false;
            }
            else
            {
                // First, convert the timestamp to an int, if possible
                int timestamp = 0;
                std::istringstream issTimeStamp(timestampStr.c_str());
                issTimeStamp >> std::noskipws >> timestamp;
                if (issTimeStamp.eof() || !issTimeStamp.fail())
                {
                    authZResult = this->validateAuthorizationToken(timestamp, token, signature);
                }
                else
                {
                    Serial.println("Switch API authorize FAILED - Invalid timestamp - exit");
                    authZResult = false;
                }
            }
        }
    }

    Serial.println("Asset API - authorize - exit");
    return authZResult;
}

void MszAssetApiBase::performAuthorizedAction(std::function<CoreHandlerResponse()> action)
{
    Serial.println("Asset API - performAuthorizedAction - enter");

    if (this->authorize())
    {
        Serial.println("Asset API - performAuthorizedAction - authorized, performing action");
        CoreHandlerResponse response = action();
        this->sendResponseData(response);
    }
    else
    {
        Serial.println("Asset API - performAuthorizedAction - not authorized, returning 401");
        CoreHandlerResponse response;
        response.statusCode = HTTP_UNAUTHORIZED_CODE;
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
        response.returnContent = "Unauthorized";
        this->sendResponseData(response);
    }

    Serial.println("Asset API - performAuthorizedAction - exit");
}

bool MszAssetApiBase::validateAuthorizationToken(int timestamp, String token, String signature)
{
    Serial.println("Asset API - validateAuthorizationToken - enter");

    // First, get the secret
    char *mySecret = this->secretHandler->getSecret(this->secretId);
    if (mySecret == NULL)
    {
        Serial.println("Switch API validateAuthorizationToken not activate because of empty secret - exit");
        return true;
    }

    // Next, check if the token is valid
    String secretKey = String(mySecret);
    bool validationResult = this->secretHandler->validateTokenSignature(token, timestamp, this->secretId, signature, TOKEN_EXPIRATION_SECONDS);
    if (validationResult)
    {
        Serial.println("Asset API - validateAuthorizationToken - token valid - exit");
        return true;
    }
    else
    {
        Serial.println("Asset API - validateAuthorizationToken - token invalid - exit");
        return false;
    }
}

void MszAssetApiBase::handleGetInfo()
{
    Serial.println("Asset API - handleGetInfo - enter");
    performAuthorizedAction([this]() -> CoreHandlerResponse {
        AssetBaseRepository switchRepository;
        AssetMetadataParams metadata = switchRepository.loadMetadata();

        CoreHandlerResponse response;
        response.statusCode = HTTP_OK_CODE;
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
        // TODO: update this to ArduinoJson as library before making additional API methods.
        response.returnContent = "{\n"
                                 "  \"status\": \"running\",\n"
                                 "  \"sensorName\": \"" + String(metadata.sensorName) + "\",\n"
                                 "  \"sensorLocation\": \"" + String(metadata.sensorLocation) + "\"\n"
                                 "}";
        return response;
    });
    Serial.println("Asset API - handleGetInfo - exit");
}

void MszAssetApiBase::handleUpdateInfo()
{
    Serial.println("Asset API - handleUpdateInfo - enter");
    performAuthorizedAction([this]() -> CoreHandlerResponse {
        AssetBaseRepository assetRepository;
        AssetMetadataParams metadataParams;

        if(!(this->getMetadataParams(metadataParams)))
        {
            Serial.println("Asset API - handleUpdateInfo - invalid metadata parameters - exit");

            CoreHandlerResponse response;
            response.statusCode = HTTP_BAD_REQUEST_CODE;
            response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
            response.returnContent = "Bad Request";
            
            Serial.println("Asset API - handleUpdateInfo - exit");
            return response;
        }

        // Validation succeeded, let's write the data to the repository.
        assetRepository.saveMetadata(metadataParams);

        CoreHandlerResponse response;
        response.statusCode = HTTP_OK_CODE;
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_TEXT_PLAIN;
        response.returnContent = "UPDATED";
        return response;
    });
    Serial.println("Asset API - handleUpdateInfo - exit");
}

bool MszAssetApiBase::getMetadataParams(AssetMetadataParams &metadataParams)
{
    Serial.println("Asset API - getMetadataParams - enter");

    metadataParams.sensorName[0] = '\0';
    metadataParams.sensorLocation[0] = '\0';

    String sensorName = this->getQueryStringParam(MszAssetApiBase::PARAM_SENSOR_NAME);
    String sensorLocation = this->getQueryStringParam(MszAssetApiBase::PARAM_SENSOR_LOCATION);

    if (sensorName == nullptr || sensorName == "" || sensorLocation == nullptr || sensorLocation == "")
    {
        Serial.println("Asset API - getMetadataParams - invalid metadata parameters - exit");
        return false;
    }

    sensorName.toCharArray(metadataParams.sensorName, sizeof(metadataParams.sensorName));
    sensorLocation.toCharArray(metadataParams.sensorLocation, sizeof(metadataParams.sensorLocation));

    Serial.println("Asset API - getMetadataParams - sensorName = " + String(metadataParams.sensorName));
    Serial.println("Asset API - getMetadataParams - sensorLocation = " + String(metadataParams.sensorLocation));
    Serial.println("Asset API - getMetadataParams - exit");
    return true;
}