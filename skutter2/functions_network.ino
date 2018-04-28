// Setup WiFi network, reporting local IP address to serial
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}


// Maintain MQTT broker connection, subscribe to topics on (re)connect
void reconnect() {
  // Loop until we're reconnected
  //digitalWrite(02, HIGH);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    //digitalWrite(02, LOW);
    if (client.connect(skutterNameArray)) {
      Serial.println("connected");
      client.publish("pirograph/announce", subsTargetArray);
      client.subscribe(subsTargetArray); // Subscribe to the specific channel
      // Subscribe to the global channel - disabled under discrimination in place
      // client.subscribe("pirograph/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //digitalWrite(00, HIGH);
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println(">>> MQTT message received");
    // Serial.print("topic: ");
    // Serial.println(topic);
    // Serial.print("payload: ");
    // Serial.println(*payload);

    String payloadString;
    for (int i = 0; i < length ; i++) {
        payloadString += String((char)payload[i]);
    }
    Serial.print("payload (string): ");
    Serial.println(payloadString);

    // Declare the JSON buffer here, so we never reuse a buffer
    // (it's destroyed when the callback is re-entered)
    // There's a chance a command won't be parsed, if the decode and action
    // isn't completed before the next message arrives. But the D1 is pretty quick,
    // and queing commands just a little from the controller should prevent that from
    // happening. So we'll leave it up to the controller.
    StaticJsonBuffer<256> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) {
        Serial.println("Parsing failed!");
        return;
    }

    // TODO: Error check the parsed json for a key before we use it.

    Serial.print("Command: ");
    Serial.println(root["command"].as<String>());
    // Serial.println(root["command"].as<char*>());
    // Serial.println(root["value"].as<char*>());
    
    // String handling: see https://arduinojson.org/example/string/
    // Can cast to String on parse:
    // Serial.println(root["name"].as<String>());

    if (root["command"] == "setLEDhue") {
        String position = root["position"];
        String state = root["state"];
        int targetHue = root["value"];
        // Output diagnostics to serial for debugging.
        Serial.print("Setting ");
        Serial.print(position);
        Serial.print(" LED in state: ");
        Serial.print(state);
        Serial.print(" to hue: ");
        Serial.println(targetHue);
        // Work out which end of the pixel string we're addressing
        int pixel_number;
        if (position == "start") {
            pixel_number = 0;
        } else {
            pixel_number = (PIXEL_COUNT - 1); // I think that's right. Probably.
        }
        // Are we writing to state A or B?
        int stateIndex;
        if (state == "A") {
            stateIndex = 1;
        } else {
            stateIndex = 2;
        }
        // Update the appropriate array entry:
        // Serial.print("Setting ");
        // Serial.print(pixel_number);
        // Serial.print(" LED in state: ");
        // Serial.print(stateIndex);
        // Serial.print(" to hue: ");
        // Serial.println(targetHue);
        ledsHSV[pixel_number][stateIndex].hue = targetHue;
        // Serial.println(ledsHSV[pixel_number][stateIndex].hue);
        // Now update the pixel hue interpolation for this state:
        updateLEDgradient(stateIndex);
        // Set the transition timer running
        // time_current = millis();
        // time_end = time_current + transitionTime;
    }

    if (root["command"] == "setLEDhsv") {
        String position = root["position"];
        String state = root["state"];
        int targetH = root["H"];
        int targetS = root["S"];
        int targetV = root["V"];
        // Output diagnostics to serial for debugging.
        Serial.print("Setting ");
        Serial.print(position);
        Serial.print(" LED in state: ");
        Serial.print(state);
        Serial.print(" to HSV colour: ");
        Serial.print(targetH);
        Serial.print(", ");
        Serial.print(targetS);
        Serial.print(", ");
        Serial.println(targetV);
        // Work out which end of the pixel string we're addressing
        int pixel_number;
        if (position == "start") {
            pixel_number = 0;
        } else {
            pixel_number = (PIXEL_COUNT - 1); // I think that's right. Probably.
        }
        // Are we writing to state A or B?
        int stateIndex;
        if (state == "A") {
            stateIndex = 1;
        } else {
            stateIndex = 2;
        }
        // Update the appropriate array entry:
        // Serial.print("Setting ");
        // Serial.print(pixel_number);
        // Serial.print(" LED in state: ");
        // Serial.print(stateIndex);
        // Serial.print(" to hue: ");
        // Serial.println(targetHue);
        ledsHSV[pixel_number][stateIndex].hue = targetH;
        ledsHSV[pixel_number][stateIndex].sat = targetS;
        ledsHSV[pixel_number][stateIndex].val = targetV;
        // ledsHSV[pixel_number][stateIndex].hue = targetHue;
        // Serial.println(ledsHSV[pixel_number][stateIndex].hue);
        // Now update the pixel hue interpolation for this state:
        updateLEDgradient(stateIndex);
        // Set the transition timer running
        // time_current = millis();
        // time_end = time_current + transitionTime;
    }

    if (root["command"] == "setBrightness") {
        Serial.println(">>> setBrightness command received!");
        targetBrightness = root["value"];
        // Loops through all states, update all brightnesses
        for (int state = 0; state < 3; state++) {
            for (int i = 0; i < PIXEL_COUNT; i++) {
                ledsHSV[i][state].val = targetBrightness;
            }
        }
    }

    if (root["command"] == "setServoPosition") {
        int servoNum = root["servoNum"];
        String state = root["state"];
        float targetPosition = root["angle"];
        // Output parsed structure to serial, for debugging
        Serial.print("Set Servo: ");
        Serial.print(servoNum);
        Serial.print(" in state: ");
        Serial.print(state);
        Serial.print(" to position: ");
        Serial.println(targetPosition);
        // Now act on it.
        int stateIndex;
        // TODO: find out why `if (state == "A")` doesn't work here
        // Something weird to do with String handling
        if (state == "A") {
            stateIndex = 1;
        } else {
            stateIndex = 2;
        }
        Serial.print("Servo: ");
        Serial.print(servoNum);
        Serial.print(" State: ");
        Serial.print(stateIndex);
        Serial.print(" to position: ");
        Serial.println(targetPosition);
        servoPosition[servoNum-1][stateIndex] = targetPosition;
        Serial.print("Target servo set to : ");
        Serial.println(servoPosition[servoNum-1][stateIndex]);
        diagnostics();
        Serial.println("<<< END PROCESSING setServoPosition");
        // Set the transition timer running again
        // time_current = millis();
        // time_end = time_current + transitionTime;
    }

    if (root["command"] == "setTransitionType") {
        String tempType = root["value"];
        // Output parsed structure to serial, for debugging
        Serial.print("Transition Type: ");
        Serial.println(tempType);
        // Now act on it.
        transitionType = tempType;
    }

    if (root["command"] == "setTransitionTime") { //Comparisons here must use " " and not ''
        Serial.print("PING");
        transitionTime = root["value"];
        Serial.print("Transition time: ");
        Serial.println(transitionTime);
        // Set the transition timer running again - added this to see what goes on
        time_current = millis();
        time_end = time_current + transitionTime;
    }

    if (root["command"] == "setStepperAngle") {
        int angle = root["angle"];
        String state = root["state"];
        // Output parsed structure to serial, for debugging
        // Now act on it.
        if (state == "A") {
            stepperAngle[0] = angle;
        } else {
            stepperAngle[1] = angle;
        }
    }

    if (root["command"] == "setStepperSpeed") {
        int speed = root["speed"];
        String state = root["state"];
        // Now act on it.
        if (state == "A") {
            stepperSpeed[0] = speed;
                } else {
            stepperSpeed[1] = speed;
        }
    }

}