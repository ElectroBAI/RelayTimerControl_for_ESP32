// ESP32 RELAY TIMER CONTROL
// Supports user-defined durations (any value, any unit)
// Supports multiple commands with && separator
// Devices: bulb, light (pin 26), fan, ceilingfan (pin 27)

const int RELAY_BULB_PIN = 26;  // bulb/light (active-low)
const int RELAY_FAN_PIN  = 27;  // fan/ceilingfan (active-low)

// Structure to hold timer information
struct TimerInfo {
  bool active;
  unsigned long startTime;
  unsigned long durationMs;
  int pin;
};

TimerInfo bulbTimer = {false, 0, 0, RELAY_BULB_PIN};
TimerInfo fanTimer = {false, 0, 0, RELAY_FAN_PIN};

void setup() {
  Serial.begin(115200);
  
  // Initialize relay pins (active-low, so HIGH = OFF)
  pinMode(RELAY_BULB_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);
  digitalWrite(RELAY_BULB_PIN, HIGH);  // OFF
  digitalWrite(RELAY_FAN_PIN, HIGH);   // OFF
  
  Serial.println("==============================================");
  Serial.println("Relay Timer Control (User-defined durations)");
  Serial.println("==============================================");
  Serial.println();
  Serial.println("Command Format: /device-state-duration");
  Serial.println("  Devices: bulb, light, fan, ceilingfan");
  Serial.println("  States: ON, OFF");
  Serial.println();
  Serial.println("Multiple commands: Use && separator");
  Serial.println("  Example: /fan-ON-500ms && /bulb-ON-1s");
  Serial.println();
  Serial.println("Supported duration units:");
  Serial.println("  ms, s/sec/secs, m/min/mins, h/hr/hrs, d/day/days");
  Serial.println();
  Serial.println("Single command examples:");
  Serial.println("  /bulb-ON-500ms       - Turn bulb on for 500ms");
  Serial.println("  /bulb-ON-30s         - Turn bulb on for 30 seconds");
  Serial.println("  /fan-ON-15min        - Turn fan on for 15 minutes");
  Serial.println("  /bulb-ON-2hrs        - Turn bulb on for 2 hours");
  Serial.println("  /fan-ON-1day         - Turn fan on for 1 day");
  Serial.println("  /bulb-ON             - Turn bulb on manually");
  Serial.println("  /bulb-OFF            - Turn bulb off");
  Serial.println();
  Serial.println("Multiple command examples:");
  Serial.println("  /fan-ON-500ms && /bulb-ON-1s");
  Serial.println("  /bulb-ON-10s && /fan-ON-5s");
  Serial.println("  /bulb-OFF && /fan-OFF");
  Serial.println("  /bulb-ON && /fan-ON");
  Serial.println();
  Serial.println("Ready for commands...");
  Serial.println();
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      processMultiCommand(cmd);
    }
  }
  
  // Check timers
  checkTimer(&bulbTimer, "Bulb");
  checkTimer(&fanTimer, "Fan");
}

void processMultiCommand(String input) {
  // Split by && separator
  int cmdCount = 0;
  int startPos = 0;
  
  while (startPos < input.length()) {
    int separatorPos = input.indexOf("&&", startPos);
    
    String singleCmd;
    if (separatorPos == -1) {
      // Last command (or only command)
      singleCmd = input.substring(startPos);
    } else {
      // Extract command before &&
      singleCmd = input.substring(startPos, separatorPos);
      startPos = separatorPos + 2;  // Move past "&&"
    }
    
    singleCmd.trim();
    if (singleCmd.length() > 0) {
      processCommand(singleCmd);
      cmdCount++;
    }
    
    if (separatorPos == -1) {
      break;  // No more commands
    }
  }
  
  if (cmdCount > 1) {
    Serial.print("✓ Executed ");
    Serial.print(cmdCount);
    Serial.println(" commands");
  }
}

void processCommand(String cmd) {
  cmd.toLowerCase();  // Normalize to lowercase
  
  // Commands must start with '/'
  if (!cmd.startsWith("/")) {
    Serial.println("ERROR: Commands must start with '/'");
    return;
  }
  
  // Remove leading '/'
  cmd = cmd.substring(1);
  
  // Parse device-state-duration
  int firstDash = cmd.indexOf('-');
  if (firstDash == -1) {
    Serial.println("ERROR: Invalid format. Use /device-state-duration");
    return;
  }
  
  String device = cmd.substring(0, firstDash);
  device.trim();
  
  String rest = cmd.substring(firstDash + 1);
  int secondDash = rest.indexOf('-');
  
  String state;
  String durationStr = "";
  
  if (secondDash == -1) {
    // No duration specified
    state = rest;
  } else {
    // Duration specified
    state = rest.substring(0, secondDash);
    durationStr = rest.substring(secondDash + 1);
  }
  
  state.trim();
  durationStr.trim();
  
  // Validate device
  TimerInfo* timer = nullptr;
  String deviceName;
  
  if (device == "bulb" || device == "light") {
    timer = &bulbTimer;
    deviceName = "Bulb";
  } else if (device == "fan" || device == "ceilingfan") {
    timer = &fanTimer;
    deviceName = "Fan";
  } else {
    Serial.print("ERROR: Unknown device '");
    Serial.print(device);
    Serial.println("'. Use: bulb, light, fan, or ceilingfan");
    return;
  }
  
  // Validate state
  if (state == "on") {
    if (durationStr.length() > 0) {
      // Parse duration
      unsigned long durationMs = parseDurationMs(durationStr);
      if (durationMs == 0) {
        Serial.print("ERROR: Invalid duration '");
        Serial.print(durationStr);
        Serial.println("'");
        Serial.println("Check format: <number><unit> (e.g., 500ms, 30s, 15min)");
        return;
      }
      
      // Turn ON with timer
      digitalWrite(timer->pin, LOW);  // Active-low relay
      timer->active = true;
      timer->startTime = millis();
      timer->durationMs = durationMs;
      
      Serial.print("✓ ");
      Serial.print(deviceName);
      Serial.print(" ON for ");
      Serial.print(printReadableDuration(durationMs));
      Serial.print(" (");
      Serial.print(durationMs);
      Serial.println(" ms)");
      
    } else {
      // Manual ON (no auto-off)
      digitalWrite(timer->pin, LOW);  // Active-low relay
      timer->active = false;  // No timer
      
      Serial.print("✓ ");
      Serial.print(deviceName);
      Serial.println(" ON (manual)");
    }
    
  } else if (state == "off") {
    // Turn OFF
    digitalWrite(timer->pin, HIGH);  // Active-low relay
    timer->active = false;  // Cancel any timer
    
    Serial.print("✓ ");
    Serial.print(deviceName);
    Serial.println(" OFF");
    
  } else {
    Serial.print("ERROR: Invalid state '");
    Serial.print(state);
    Serial.println("'. Use: ON or OFF");
  }
}

unsigned long parseDurationMs(String durationStr) {
  durationStr.trim();
  durationStr.toLowerCase();
  
  // Extract numeric part
  int i = 0;
  while (i < durationStr.length() && isDigit(durationStr[i])) {
    i++;
  }
  
  if (i == 0) {
    return 0;  // No number found
  }
  
  long value = durationStr.substring(0, i).toInt();
  if (value <= 0) {
    return 0;
  }
  
  // Extract unit part
  String unit = durationStr.substring(i);
  unit.trim();
  
  unsigned long multiplier = 0;
  
  // Milliseconds
  if (unit == "ms" || unit == "millisecond" || unit == "milliseconds") {
    multiplier = 1;
  }
  // Seconds
  else if (unit == "s" || unit == "sec" || unit == "secs" || 
           unit == "second" || unit == "seconds") {
    multiplier = 1000UL;
  }
  // Minutes
  else if (unit == "m" || unit == "min" || unit == "mins" || 
           unit == "minute" || unit == "minutes") {
    multiplier = 60UL * 1000UL;
  }
  // Hours
  else if (unit == "h" || unit == "hr" || unit == "hrs" || 
           unit == "hour" || unit == "hours") {
    multiplier = 60UL * 60UL * 1000UL;
  }
  // Days
  else if (unit == "d" || unit == "day" || unit == "days") {
    multiplier = 24UL * 60UL * 60UL * 1000UL;
  }
  else {
    return 0;  // Unknown unit
  }
  
  // Calculate duration in milliseconds
  unsigned long durationMs = (unsigned long)value * multiplier;
  
  // Check for overflow (if result is 0 or wrapped around)
  if (durationMs == 0) {
    return 0;
  }
  
  return durationMs;
}

String printReadableDuration(unsigned long ms) {
  String result = "";
  
  if (ms < 1000) {
    result = String(ms) + " milliseconds";
  } else if (ms < 60000) {
    result = String(ms / 1000.0, 1) + " seconds";
  } else if (ms < 3600000) {
    result = String(ms / 60000.0, 1) + " minutes";
  } else if (ms < 86400000) {
    result = String(ms / 3600000.0, 1) + " hours";
  } else {
    result = String(ms / 86400000.0, 1) + " days";
  }
  
  return result;
}

void checkTimer(TimerInfo* timer, String deviceName) {
  if (timer->active) {
    unsigned long currentTime = millis();
    
    // Handle millis() overflow (wraps around every ~49 days)
    unsigned long elapsed;
    if (currentTime >= timer->startTime) {
      elapsed = currentTime - timer->startTime;
    } else {
      // Overflow occurred
      elapsed = (0xFFFFFFFF - timer->startTime) + currentTime + 1;
    }
    
    // Check if timer expired
    if (elapsed >= timer->durationMs) {
      digitalWrite(timer->pin, HIGH);  // Turn OFF (active-low)
      timer->active = false;
      
      Serial.print("✓ ");
      Serial.print(deviceName);
      Serial.println(" OFF (timer expired)");
    }
  }
}
