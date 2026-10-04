    #include <SPI.h>
    #include <WiFi.h>
    #include "Credentials.h"
    #include "Globals.h"

    // ===================== CONFIG =====================
    constexpr uint8_t FLOAT_PIN = 4;

    constexpr unsigned long DEBOUNCE_DELAY = 50;

    const char* HOST = "192.168.137.1";
    constexpr uint16_t PORT = 5000;

    // ===================== GLOBALS =====================
    bool floatTriggered = false;
    bool lastFloatState = false;

    unsigned long stateStartTime = 0;
    unsigned long lastDebounceTime = 0;

    int seconds = 0;
    unsigned long lastSecondTick = 0;
    bool sendingEnabled = true;
    int displayMode = 0;
    String email = "";

    String macAddress;

    // ==================================================
    void connectWiFi();
    void handleFloatSensor();
    void sendData(bool currentState,
                unsigned long previousStateDuration);
    String getMacString();


    // ====================== SETUP ======================
    void setup()
    {
        Serial.begin(115200);

        Serial.println("\n=== BOOT START ===");

        pinMode(FLOAT_PIN, INPUT_PULLUP);

        connectWiFi();

        // Cache MAC address once
        macAddress = getMacString();

        // Initial sensor state
        floatTriggered = (digitalRead(FLOAT_PIN) == LOW);
        lastFloatState = floatTriggered;

        // Start timing this state
        stateStartTime = millis();

        Serial.print("Initial float state: ");

        if (floatTriggered)
            Serial.println("TRIGGERED");
        else
            Serial.println("NORMAL");

        extern WiFiServer server;
        server.begin();

        Serial.println("Server started");
    }


    // ======================= LOOP ======================
    void loop()
    {
        handleClient();

        handleFloatSensor();

        // Reconnect WiFi if disconnected
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("WiFi disconnected");
            connectWiFi();
        }
    }


    // =================== WIFI CONNECT ==================
    void connectWiFi()
    {
        Serial.print("Connecting to WiFi");

        WiFi.begin(ssid, password);

        unsigned long startAttempt = millis();

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");

            // Restart if connection fails too long
            if (millis() - startAttempt > 10000)
            {
                Serial.println("\nWiFi timeout. Restarting...");
                ESP.restart();
            }
        }

        Serial.println("\nWiFi connected");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }


    // ================= FLOAT SENSOR ====================
    void handleFloatSensor()
    {
        bool currentReading =
            (digitalRead(FLOAT_PIN) == LOW);

        // Detect state change
        if (currentReading != lastFloatState)
        {
            // Debounce check
            if (millis() - lastDebounceTime > DEBOUNCE_DELAY)
            {
                lastDebounceTime = millis();

                unsigned long currentTime = millis();

                // How long previous state lasted
                unsigned long durationSeconds =
                    (currentTime - stateStartTime) / 1000;

                Serial.println("\n=== FLOAT STATE CHANGED ===");

                Serial.print("New State: ");

                if (currentReading)
                    Serial.println("TRIGGERED");
                else
                    Serial.println("NORMAL");

                Serial.print("Previous State Duration: ");
                Serial.print(durationSeconds);
                Serial.println(" seconds");

                // Send event
                sendData(currentReading, durationSeconds);

                // Reset timer for NEW state
                stateStartTime = currentTime;

                // Save new state
                lastFloatState = currentReading;
                floatTriggered = currentReading;
            }
        }
    }


    // ==================== SEND DATA ====================
    void sendData(bool currentState,
                unsigned long previousStateDuration)
    {
        WiFiClient client;
        client.setTimeout(1000);

        if (!client.connect(HOST, PORT))
        {
            Serial.println("Connection failed");
            return;
        }

        String json = "{";

        json += "\"floatTriggered\":";
        json += (currentState ? "true" : "false");

        json += ",\"previousStateDuration\":";
        json += previousStateDuration;

        json += ",\"rssi\":";
        json += WiFi.RSSI();

        json += ",\"mac\":\"";
        json += macAddress;
        json += "\"";

        json += "}";

        // ===== HTTP REQUEST =====
        client.println("POST /data HTTP/1.1");

        client.print("Host: ");
        client.println(HOST);

        client.println("Content-Type: application/json");

        client.print("Content-Length: ");
        client.println(json.length());

        client.println("Connection: close");
        client.println();

        client.println(json);

        client.stop();

        Serial.println("Event sent successfully");
        Serial.println(json);
    }


    // ===================== MAC =========================
    String getMacString()
    {
        return WiFi.macAddress();
    }