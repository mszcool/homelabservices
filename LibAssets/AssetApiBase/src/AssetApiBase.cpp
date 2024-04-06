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
    this->registerPutEndpoint(MszAssetApiBase::API_ENDPOINT_UPDATEINFO, std::bind(&MszAssetApiBase::handleUpdateInfo, this));
    this->registerPutEndpoint(MszAssetApiBase::API_ENDPOINT_SETTIME, std::bind(&MszAssetApiBase::handleSetSensorTime, this));

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

String MszAssetApiBase::getErrorJsonDocument(int errorCode, String errorTitle, String errorMessage)
{
    JsonDocument errDoc;

    errDoc["errorCode"] = errorCode;
    errDoc["errorTitle"] = errorTitle;
    errDoc["details"] = errorMessage;

    String errJsonStr;
    serializeJsonPretty(errDoc, errJsonStr);
    return errJsonStr;
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
        response.returnContent = this->getMetadataJson("running", metadata);

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
            response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
            response.returnContent = this->getErrorJsonDocument(HTTP_BAD_REQUEST_CODE, "BadRequest", "You provided invalid parameters for the Switch Information!");
            
            Serial.println("Asset API - handleUpdateInfo - exit");
            return response;
        }

        // Validation succeeded, let's write the data to the repository.
        assetRepository.saveMetadata(metadataParams);

        CoreHandlerResponse response;
        response.statusCode = HTTP_OK_CODE;
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
        response.returnContent = this->getMetadataJson("updated", metadataParams);
        return response;
    });
    Serial.println("Asset API - handleUpdateInfo - exit");
}

void MszAssetApiBase::handleSetSensorTime()
{
    Serial.println("Asset API - handleSetSensorTime - enter");
    performAuthorizedAction([this]() -> CoreHandlerResponse {

        Serial.println("Asset API - handleSetSensorTime - validating prameters...");
        bool invalidParams = false;
        String paramHourString = this->getQueryStringParam(MszAssetApiBase::PARAM_HOUR);
        String paramMinuteString = this->getQueryStringParam(MszAssetApiBase::PARAM_MINUTE);
        String paramSecondString = this->getQueryStringParam(MszAssetApiBase::PARAM_SECOND);
        String paramDayString = this->getQueryStringParam(MszAssetApiBase::PARAM_DAY);
        String paramMonthString = this->getQueryStringParam(MszAssetApiBase::PARAM_MONTH);
        String paramYearString = this->getQueryStringParam(MszAssetApiBase::PARAM_YEAR);

        // Check if any parameters are missing.
        if ( (paramHourString == nullptr) || (paramMinuteString == nullptr) || (paramSecondString == nullptr) || (paramDayString == nullptr) || (paramMonthString == nullptr) || (paramYearString == nullptr) || 
             (paramHourString == "") || (paramMinuteString == "") || (paramSecondString == "") || (paramDayString == "") || (paramMonthString == "") || (paramYearString == "") )
        {
            Serial.println("Asset API - handleSetSensorTime - missing parameters - exit");
            invalidParams = true;
        }

        // Now convert all parameters into integers.
        int hour, min, sec, day, month, year;
        std::istringstream intStreamConverter(paramHourString.c_str());
        intStreamConverter >> std::noskipws >> hour;
        invalidParams = invalidParams || intStreamConverter.fail();
        intStreamConverter.clear();
        intStreamConverter.str(paramMinuteString.c_str());
        intStreamConverter >> std::noskipws >> min;
        invalidParams = invalidParams || intStreamConverter.fail();
        intStreamConverter.clear();
        intStreamConverter.str(paramSecondString.c_str());
        intStreamConverter >> std::noskipws >> sec;
        invalidParams = invalidParams || intStreamConverter.fail();
        intStreamConverter.clear();
        intStreamConverter.str(paramDayString.c_str());
        intStreamConverter >> std::noskipws >> day;
        invalidParams = invalidParams || intStreamConverter.fail();
        intStreamConverter.clear();
        intStreamConverter.str(paramMonthString.c_str());
        intStreamConverter >> std::noskipws >> month;
        invalidParams = invalidParams || intStreamConverter.fail();
        intStreamConverter.clear();
        intStreamConverter.str(paramYearString.c_str());
        intStreamConverter >> std::noskipws >> year;
        invalidParams = invalidParams || intStreamConverter.fail();

        // If any of the conversion steps failed above, return a bad request response.
        if(invalidParams)
        {
            Serial.println("Asset API - handleSetSensorTime - invalid metadata parameters - exit");

            CoreHandlerResponse response;
            response.statusCode = HTTP_BAD_REQUEST_CODE;
            response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
            response.returnContent = this->getErrorJsonDocument(HTTP_BAD_REQUEST_CODE, "BadRequest", "You provided invalid parameters for the Switch Information!");
            
            Serial.println("Asset API - handleSetSensorTime - exit");
            return response;
        }

        // Validation succeeded, now let's set the time.
        Serial.println("Asset API - handleSetSensorTime - setting time...");
        setTime(hour, min, sec, day, month, year);

        // Now get the time in ticks and return that value to the client for confirmation.
        time_t currentTime = now();

        // Provide responses back to the client.
        Serial.println("Asset API - handleSetSensorTime - returning response...");
        CoreHandlerResponse response;
        response.statusCode = HTTP_OK_CODE;
        response.contentType = HTTP_RESPONSE_CONTENT_TYPE_APPLICATION_JSON;
        response.returnContent = String(currentTime);
        return response;
    });
    Serial.println("Asset API - handleSetSensorTime - exit");
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

String MszAssetApiBase::getMetadataJson(String status, AssetMetadataParams &params)
{
    String jsonResp;
    JsonDocument responseDoc;
    responseDoc["status"] = status;
    responseDoc["sensorName"] = params.sensorName;
    responseDoc["sensorLocation"] = params.sensorLocation;
    serializeJsonPretty(responseDoc, jsonResp);
    return jsonResp;
}