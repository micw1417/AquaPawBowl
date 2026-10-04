#include "WebServer.h"
#include "Globals.h"
#include <WiFi.h>

WiFiServer server(80);

void handleClient()
{
  WiFiClient client = server.available();
  if (!client) return;

  String req = client.readStringUntil('\n');
  req.trim();

  Serial.println("Request: " + req);

  if (req.indexOf("GET /reset") >= 0)
  {
    seconds = 0;
    lastSecondTick  = millis();
    Serial.println("RESET triggered");
  }

  else if (req.indexOf("GET /pauseToggle") >= 0)
  {
    sendingEnabled = !sendingEnabled;
     Serial.println("Sending Toggled: " + sendingEnabled);
  }

  // ===== STATUS ENDPOINT =====
  else if (req.indexOf("GET /status") >= 0)
  {
    String json =
      String("{\"Float Triggered\":") + floatTriggered + 
      ",\"seconds\":" + seconds +
      ",\"sending\":" + (sendingEnabled ? "true" : "false") +
      "}";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.println(json);
  }
//  else if (req.indexOf("POST /setEmail") >= 0)
// {
//   String line;
//   int contentLength = 0;

//   // ===== READ HEADERS =====
//   while (client.connected()) {
//     line = client.readStringUntil('\n');

//     if (line.startsWith("Content-Length:")) {
//       contentLength = line.substring(15).toInt();
//     }

//     if (line == "\r") {
//       break; // end of headers
//     }
//   }

//   // ===== READ BODY =====
//   String body = "";
//   while (body.length() < contentLength) {
//     if (client.available()) {
//       body += (char)client.read();
//     }
//   }

//   Serial.println("BODY:");
//   Serial.println(body);

//   // ===== PARSE JSON =====
//   int e1 = body.indexOf("email");
//   if (e1 != -1) {
//     int q1 = body.indexOf("\"", e1 + 6);
//     int q2 = body.indexOf("\"", q1 + 1);

//     email = body.substring(q1 + 1, q2);

//     Serial.println("Email set to: " + email);
//   } else {
//     Serial.println("Email key not found");
//   }

//   // ===== RESPONSE =====
//   client.println("HTTP/1.1 200 OK");
//   client.println("Content-Type: text/plain");
//   client.println();
//   client.println("EMAIL UPDATED");
// }

  client.stop();
}
