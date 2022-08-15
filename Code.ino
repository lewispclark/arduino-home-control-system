/*
 * Home Control System
 * 
 * Created 03/2021
 * By Lewis Clark
 */

# include <Wire.h>
# include <Adafruit_RGBLCDShield.h>
# include <utility/Adafruit_MCP23017.h>
# include <avr/pgmspace.h>
# include <EEPROM.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// Free memory check
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else // __ARM__
extern char *__brkval;
#endif // __arm__
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}

//#define testing

#define NUM_OF_FLOORS 3
#define NUM_OF_ROOMS 3
#define NUM_OF_TYPES 4
#define NUM_OF_NAMES 2
#define NUM_OF_ACTIONS 3
#define MAX_LEVEL 100

#define FLOOR_SELECTION 0
#define ROOM_SELECTION 1
#define TYPE_SELECTION 2
#define NAME_SELECTION 3
#define ACTION_SELECTION 4
#define VALUE_SELECTION 5
#define VALUE2_SELECTION 6

#define FLOOR 0
#define ROOM 1
#define TYPE 2
#define NAME 3
#define QUALIFIER 4
#define ACTION 4
#define VALUE 5
#define VALUE2 6

#define LEVEL 2

#define NEXT_FLOOR house.floors[choices[FLOOR] + 1].nameKey
#define NEXT_ROOM house.floors[choices[FLOOR]].rooms[choices[ROOM] + 1].nameKey
#define NEXT_TYPE house.floors[choices[FLOOR]].rooms[choices[ROOM]].deviceTypes[choices[TYPE] + 1].nameKey
#define NEXT_DEVICE house.floors[choices[FLOOR]].rooms[choices[ROOM]].deviceTypes[choices[TYPE]].devices[choices[NAME] + 1].nameKey

#define FLOOR_NAME house.floors[choices[FLOOR]].nameKey
#define ROOM_NAME house.floors[choices[FLOOR]].rooms[choices[ROOM]].nameKey
#define DEVICE_TYPE house.floors[choices[FLOOR]].rooms[choices[ROOM]].deviceTypes[choices[TYPE]].nameKey
#define DEVICE_NAME house.floors[choices[FLOOR]].rooms[choices[ROOM]].deviceTypes[choices[TYPE]].devices[choices[NAME]].nameKey
#define DEVICE_QUALIFIER house.floors[choices[FLOOR]].rooms[choices[ROOM]].deviceTypes[choices[TYPE]].devices[choices[NAME]].qualifierKey

#define FLOOR_WORD keyToWord(FLOOR, FLOOR_NAME)
#define ROOM_WORD keyToWord(ROOM, ROOM_NAME)
#define TYPE_WORD keyToWord(TYPE, DEVICE_TYPE)
#define NAME_WORD keyToWord(NAME, DEVICE_NAME)
#define QUALIFIER_WORD keyToWord(QUALIFIER, DEVICE_QUALIFIER)

// PROGMEM consts to hold refer to when adding floors, rooms, etc to the house.
#define FLOOR_CONSTS 3
const char floorString0[] PROGMEM = "First";
const char floorString1[] PROGMEM = "Ground";
const char floorString2[] PROGMEM = "Outside";

// PROGMEM consts to save RAM when outputting the data
#define ROOM_CONSTS 11
const char roomString0[] PROGMEM = "Kitchen";
const char roomString1[] PROGMEM = "Bathroom";
const char roomString2[] PROGMEM = "Lounge";
const char roomString3[] PROGMEM = "Bedroom 1";
const char roomString4[] PROGMEM = "Bedroom 2";
const char roomString5[] PROGMEM = "Bedroom 3";
const char roomString6[] PROGMEM = "Bedroom 4";
const char roomString7[] PROGMEM = "Garage";
const char roomString8[] PROGMEM = "Garden";
const char roomString9[] PROGMEM = "Play Room";
const char roomString10[] PROGMEM = "Hall";

#define TYPE_CONSTS 4
const char typeString0[] PROGMEM = "Heat";
const char typeString1[] PROGMEM = "Light";
const char typeString2[] PROGMEM = "Lamp";
const char typeString3[] PROGMEM = "Water";

#define NAME_CONSTS 6
const char nameString0[] PROGMEM = "Main";
const char nameString1[] PROGMEM = "Ceiling";
const char nameString2[] PROGMEM = "Desk";
const char nameString3[] PROGMEM = "Bed";
const char nameString4[] PROGMEM = "Cupboard";
const char nameString5[] PROGMEM = "Wall";

#define QUALIFIER_CONSTS 5
const char qualifierString0[] PROGMEM = "One";
const char qualifierString1[] PROGMEM = "Two";
const char qualifierString2[] PROGMEM = "Three";
const char qualifierString3[] PROGMEM = "Left";
const char qualifierString4[] PROGMEM = "Right";

// A table to refer to the strings of each type.
PGM_P const floor_string_table[] PROGMEM = {floorString0, floorString1, floorString2};
PGM_P const room_string_table[] PROGMEM = {roomString0, roomString1, roomString2, roomString3, roomString4, roomString5,
                                           roomString6, roomString7, roomString8, roomString9, roomString10};
PGM_P const type_string_table[] PROGMEM = {typeString0, typeString1, typeString2, typeString3};
PGM_P const name_string_table[] PROGMEM = {nameString0, nameString1, nameString2, nameString3, nameString4, nameString5};
PGM_P const qualifier_string_table[] PROGMEM = {qualifierString0, qualifierString1, qualifierString2, qualifierString3, qualifierString4};

// A table to refer to the tables of strings
const char* const* const all_string_table[] {floor_string_table, room_string_table, type_string_table, name_string_table, qualifier_string_table};

// Stores the maximum values of each string table
const byte max_values[] PROGMEM = {FLOOR_CONSTS, ROOM_CONSTS, TYPE_CONSTS, NAME_CONSTS, QUALIFIER_CONSTS};

// Char arrays to store the keys for the softmenu
const char floor_keys[] PROGMEM = {'F', 'G', 'O'};
const char room_keys[] PROGMEM = {'K', 'B', 'L', '1', '2', '3', '4', 'G', 'R', 'P', 'H'};
const char type_keys[] PROGMEM = {'H', 'L', 'A', 'W'};
const char name_keys[] PROGMEM = {'M', 'C', 'D', 'B', 'P', 'W'};
const char qualifiers_keys[] PROGMEM = {'1', '2', '3', 'L', 'R'};

// A table to store the keys on the different menuPosition of the menu
PGM_P const keys_table[] PROGMEM = {floor_keys, room_keys, type_keys, name_keys, qualifiers_keys};

// Stores the display char arrays for each action on the devices
const char *actionsValues[3] {"On", "Off", "Level"};

// Macros to make code more concise for outputting messages
#define lcdPrint3(a, b, c){ lcd.print(a); lcd.print(b); lcd.print(c); } while(0)
#define serialPrint3(a, b, c){ Serial.print(a); Serial.print(b); Serial.print(c); } while(0)

/* 
 * Check progmem constant that matches the string & menuPosition given exists 
 * Params:
 *  @menuPosition represents the part of the menu referred to (e.g. floor, room, type, etc..)
 *  @nameKey that represents a string that describes that part of the house
 * return true if the nameKey is valid in the specified menuPosition, false if otherwise
 */
bool validDescriptor(byte menuPosition, char nameKey) {
  // Get keys from keys table in progmem
  char * keys = (char *) malloc(pgm_read_byte(&max_values[menuPosition]));
  strncpy_P(keys, (char *)pgm_read_word(&keys_table[menuPosition]), pgm_read_byte(&max_values[menuPosition]));
  // Check if each word is a match
  for (byte i = 0; i < pgm_read_byte(&max_values[menuPosition]); i++) {
    // If key is found
    if (nameKey == keys[i]) {
      free(keys);
      return true;
    }
  }
  // return false if name cannot be found
  serialPrint3(F("Error: \""), nameKey, F("\" is not a valid descriptor.\n"));
  return false;
}

/*
 * Returns the word that the key represents
 * Params:
 *  @menuPosition represents the part of the menu referred to (e.g. floor, room, type, etc..)
 *  @key that represents a string that describes that part of the house
 * return the word that the key represents, or an empty array if it cannot be found
 */
char * keyToWord(byte menuPosition, char key) {
  char * keys = (char *) malloc(pgm_read_byte(&max_values[menuPosition]));
  char * word_ = (char *) malloc(10);

  strncpy_P(keys, (char *)pgm_read_word(&keys_table[menuPosition]), pgm_read_byte(&max_values[menuPosition]));
  // For each key in the specified menuPosition
  for (byte i = 0; i < pgm_read_byte(&max_values[menuPosition]); i++) {
    // If key is a match
    if (key == keys[i]) {
      // Return word from progmem
      const char* const* tablePtr = all_string_table[menuPosition];
      strcpy_P(word_, (char *)pgm_read_word(&tablePtr[i]));
      free(keys);
      return word_;
    }
  }
  // If key is not found, return an empty char array
  return word_;
}

// Structs for house
struct Device {
  int values[NUM_OF_ACTIONS] {0, 0, 0};

  // Keys for device name & qualifier
  char nameKey = ' ';
  char qualifierKey = ' ';
};

struct DeviceType {
  Device devices[NUM_OF_NAMES];

  // Key for device type
  char nameKey = ' ';
};

struct Room {
  DeviceType deviceTypes[NUM_OF_TYPES];

  // Key for room name
  char nameKey = ' ';
};
struct Floor {
  Room rooms[NUM_OF_ROOMS];

  // Key for floor name
  char nameKey = ' ';
};

struct House {
  bool hasFloor = false;

  Floor floors[NUM_OF_FLOORS];

  /*
   * Returns index of the specified floor
   * Params:
   *  @floorKey represents the name of the floor
   * return the index of the given floor in the house, -1 if it cannot be found
   */
  int findKey(char floorKey) {
    for (byte i = 0; i < NUM_OF_FLOORS; i++) {
      if (floors[i].nameKey == ' ') break;
      // if name is match, return index
      if (floorKey == floors[i].nameKey) return i;
    }
    // return -1 if name cannot be found
    return -1;
  }


  /*
   * Returns index of the specified room on the floor specified
   * Params:
   *  @floorKey represents the name of the floor
   *  @roomKey represents the name of the room
   * return the index of the given room on the floor, -1 if it cannot be found
   */
  int findKey(char floorKey, char roomKey) {
    // Find floor with floorKey as name
    int floorNum = findKey(floorKey);
    if (floorNum == -1) return -1;

    for (byte i = 0; i < NUM_OF_ROOMS; i++) {
      if (floors[floorNum].rooms[i].nameKey == ' ') break;
      // if name is match, return index
      if (roomKey == floors[floorNum].rooms[i].nameKey) return i;
    }
    // return -1 if name cannot be found
    return -1;
  }

  /*
   * Returns index of the specified device type in the room specified
   * Params:
   *  @floorKey represents the name of the floor
   *  @roomKey represents the name of the room
   *  @typeKey represents the name of the device type
   * return the index of the given device type in the room , -1 if it cannot be found
   */
  int findKey(char floorKey, char roomKey, char typeKey) {
    // Find floor with floorKey as name
    int floorNum = findKey(floorKey);
    if (floorNum == -1) return -1;

    // Find floor with roomKey as name
    int roomNum = findKey(floorKey, roomKey);
    if (roomNum == -1) return -1;

    for (byte i = 0; i < NUM_OF_TYPES; i++) {
      if (floors[floorNum].rooms[roomNum].deviceTypes[i].nameKey == ' ') break;
      // if name is match, return index
      if (typeKey == floors[floorNum].rooms[roomNum].deviceTypes[i].nameKey) return i;
    }
    // return -1 if name cannot be found
    return -1;
  }

  /*
   * Returns index of the specified device in the device type specified
   * Params:
   *  @floorKey represents the name of the floor
   *  @roomKey represents the name of the room
   *  @typeKey represents the name of the device type
   *  @nameKey represents the name of the device
   *  @qualifierKey represents the qualifier of the device
   * return the index of the given device in the device type , -1 if it cannot be found
   */
  int findKey(char floorKey, char roomKey, char typeKey, char nameKey, char qualifierKey) {
    // Find floor with floorKey as name
    int floorNum = findKey(floorKey);
    if (floorNum == -1) return -1;

    // Find floor with roomKey as name
    int roomNum = findKey(floorKey, roomKey);
    if (roomNum == -1) return -1;

    // Find floor with roomKey as name
    int typeNum = findKey(floorKey, roomKey, typeKey);
    if (typeNum == -1) return -1;

    for (byte i = 0; i < NUM_OF_NAMES; i++) {
      if (floors[floorNum].rooms[roomNum].deviceTypes[typeNum].devices[i].nameKey == ' ') break;
      // if name is match, return index
      if (nameKey == floors[floorNum].rooms[roomNum].deviceTypes[typeNum].devices[i].nameKey
          && qualifierKey == floors[floorNum].rooms[roomNum].deviceTypes[typeNum].devices[i].qualifierKey) return i;
    }
    // return -1 if name cannot be found
    return -1;
  }

  /*
   * Add floor to the house structure if there is space
   * Params:
   *  @floorKey to represent the floor that will be added
   * return true if operation was successful, false otherwise.
   */
  bool addFloor(char floorKey) {
    // Check inputs are valid
    if (!validDescriptor(FLOOR, floorKey)) return false;

    // Check if floor already exists
    if (findKey(floorKey) != -1) {
      serialPrint3(F("Error: floor with key \""), floorKey, F("\" already exists.\n"));
      return false;
    }

    // Find empty space for new floor to be added
    for (byte i = 0; i < NUM_OF_FLOORS; i++) {
      // If name is empty, update it
      if (floors[i].nameKey == ' ') {
        floors[i].nameKey = floorKey;
        serialPrint3(F("\""), floorKey, F("\" floor added.\n"));
        // If floor is first to be added, update lcd
        if (!hasFloor) {
          hasFloor = true;
          lcd.clear();
          lcd.print(F("House/"));
          lcd.setCursor(0, 1);
          lcd.print(keyToWord(FLOOR, floors[0].nameKey));
        }
        return true;
      }
    }
    Serial.println(F("Error: There is not enough space in the house for this floor."));
    return false;
  }

  /*
   * Add room to the floor if there is space
   * Params:
   *  @floorKey to represent the floor that will be found or added
   *  @roomKey to represent the room that will be added
   * return true if operation was successful, false otherwise.
   */
  bool addRoom(char floorKey, char roomKey) {
    // Check inputs are valid
    if (!validDescriptor(FLOOR, floorKey) || !validDescriptor(ROOM, roomKey)) return false;

    // Find place in house, add parts if they do not exist
    int floorNum = findKey(floorKey);
    if (floorNum == -1) if(!addFloor(floorKey)) return false;
    floorNum = findKey(floorKey);

    // Check if room already exists
    if (findKey(floorKey, roomKey) != -1) {
      serialPrint3(F("Error: room with key \""), roomKey, F("\" already exists on this floor.\n"));
      return false;
    }

    // Find empty space for new room to be added
    for (byte i = 0; i < NUM_OF_ROOMS; i++) {
      // If name is empty, update it
      if (floors[floorNum].rooms[i].nameKey == ' ') {
        floors[floorNum].rooms[i].nameKey = roomKey;
        serialPrint3(F("\""), roomKey, F("\" room added.\n"));
        return true;
      }
    }
    Serial.println(F("Error: There is not enough space on this floor for this room."));
    return false;
  }

  /*
   * Add device type to the room if there is space
   * Params:
   *  @floorKey to represent the floor that will be found or added
   *  @roomKey to represent the room that will be found or added
   *  @typeKey to represent the device type that will be added
   * return true if operation was successful, false otherwise.
   */
  bool addType(char floorKey, char roomKey, char typeKey) {
    // Check inputs are valid
    if (!validDescriptor(FLOOR, floorKey) || !validDescriptor(ROOM, roomKey) || !validDescriptor(TYPE, typeKey)) return false;

    // Find place in house, add parts if they do not exist
    int floorNum = findKey(floorKey);
    if (floorNum == -1) if(!addFloor(floorKey)) return false;
    floorNum = findKey(floorKey);

    int roomNum = findKey(floorKey, roomKey);
    if (roomNum == -1) if(!addRoom(floorKey, roomKey)) return false;
    roomNum = findKey(floorKey, roomKey);

    // Check if type already exists
    if (findKey(floorKey, roomKey, typeKey) != -1) {
      serialPrint3(F("Error: type with key \""), typeKey, F("\" already exists in this room.\n"));
      return false;
    }

    // Find empty space for new type to be added
    for (byte i = 0; i < NUM_OF_ROOMS; i++) {
      // If name is empty, update it
      if (floors[floorNum].rooms[roomNum].deviceTypes[i].nameKey == ' ') {
        floors[floorNum].rooms[roomNum].deviceTypes[i].nameKey = typeKey;
        serialPrint3(F("\""), typeKey, F("\" type added.\n"));
        return true;
      }
    }
    Serial.println(F("Error: There is not enough space in this room for this device type."));
    return false;
  }

  /*
   * Add device to the device types if there is space
   * Params:
   *  @floorKey to represent the floor that will be found or added
   *  @roomKey to represent the room that will be found or added
   *  @typeKey to represent the device type that will be found or added
   *  @nameKey to represent the name of the device that will be added
   *  @qualifierKey to represent the qualifier of the device that will be added
   * return true if operation was successful, false otherwise.
   */
  bool addDevice(char floorKey, char roomKey, char typeKey, char nameKey, char qualifierKey) {
    // Check inputs are valid
    if (!validDescriptor(FLOOR, floorKey) || !validDescriptor(ROOM, roomKey) || !validDescriptor(TYPE, typeKey)
        || !validDescriptor(NAME, nameKey)) return false;
    if (qualifierKey != ' ' && !validDescriptor(NAME, nameKey)) return false;

    // Find place in house, add parts if they do not exist
    int floorNum = findKey(floorKey);
    if (floorNum == -1) if(!addFloor(floorKey)) return false;
    floorNum = findKey(floorKey);

    int roomNum = findKey(floorKey, roomKey);
    if (roomNum == -1) if(!addRoom(floorKey, roomKey)) return false;
    roomNum = findKey(floorKey, roomKey);

    int typeNum = findKey(floorKey, roomKey, typeKey);
    if (typeNum == -1) if(!addType(floorKey, roomKey, typeKey)) return false;
    typeNum = findKey(floorKey, roomKey, typeKey);

    // Check if device already exists
    if (findKey(floorKey, roomKey, typeKey, nameKey, qualifierKey) != -1) {
      serialPrint3(F("Error: device with name \""), nameKey, qualifierKey);
      Serial.println(F("\" already exists in this room."));
      return false;
    }

    // check for empty space in devices array
    for (byte i = 0; i < NUM_OF_NAMES; i++) {
      // If name is empty, update it
      if (floors[floorNum].rooms[roomNum].deviceTypes[typeNum].devices[i].nameKey == ' ') {
        floors[floorNum].rooms[roomNum].deviceTypes[typeNum].devices[i].nameKey = nameKey;
        // If qualifier is provided, also update qualifier
        if (qualifierKey != ' ') {
          floors[floorNum].rooms[roomNum].deviceTypes[typeNum].devices[i].qualifierKey = qualifierKey;
        }
        serialPrint3(F("\""), nameKey, F("\" device added.\n"));
        return true;
      }
    }
    Serial.println(F("Error: There is not enough space in this room for this device."));
    return false;
  }

  // addDevice with no qualifier key
  bool addDevice(char floorKey, char roomKey, char typeKey, char nameKey) {
    addDevice(floorKey, roomKey, typeKey, nameKey, ' ');
  }

  // addDevice with no qualifier key or device name
  bool addDevice(char floorKey, char roomKey, char typeKey) {
    addDevice(floorKey, roomKey, typeKey, 'M', ' ');
  }

  /*
   * Iterates through each floor, room, device type, device name and it's values
   * converts the keys to the represented strigns and outputs them all to the serial 
   * monitor
   */
  void displayHouse() {
    Serial.println(F("===================================================="));
    // Output each valid floor
    for (byte a = 0; a < NUM_OF_FLOORS; a++) {
      if (floors[a].nameKey != ' ') {
        char * floor_ = keyToWord(FLOOR, floors[a].nameKey);
        // Output each valid room
        for (byte b = 0; b < NUM_OF_ROOMS; b++) {
          if (floors[a].rooms[b].nameKey != ' ') {
            char * room_ = keyToWord(ROOM, floors[a].rooms[b].nameKey);
            // Ouptut each valid device type
            for (byte c = 0; c < NUM_OF_TYPES; c++) {
              if (floors[a].rooms[b].deviceTypes[c].nameKey != ' ') {
                char * type_ = keyToWord(TYPE, floors[a].rooms[b].deviceTypes[c].nameKey);
                // Output each valid device name
                for (byte d = 0; d < NUM_OF_NAMES; d++) {
                  if (floors[a].rooms[b].deviceTypes[c].devices[d].nameKey != ' ') {
                    char * name_ = keyToWord(NAME, floors[a].rooms[b].deviceTypes[c].devices[d].nameKey);
                    serialPrint3(floor_, F("/"), room_);
                    serialPrint3(F("/"), type_, F("/"));
                    Serial.print(name_);
                    
                    // If device has qualifier, print to serial monitor
                    if (floors[a].rooms[b].deviceTypes[c].devices[d].qualifierKey != ' ') {
                      Serial.print(F(" "));
                      char * qualifier_ = keyToWord(QUALIFIER, floors[a].rooms[b].deviceTypes[c].devices[d].qualifierKey);
                      Serial.print(qualifier_);
                      free(qualifier_);
                    }
                    Serial.print(F("/"));
                    // output device values
                    for (byte e = 0; e < NUM_OF_ACTIONS; e++) {
                      char buffer[10];
                      if (e < NUM_OF_ACTIONS - 1) {
                        if (e != 0) Serial.print(F(", "));
                        // Format as time (XX.XX)
                        sprintf(buffer, "%02d.%02d", floors[a].rooms[b].deviceTypes[c].devices[d].values[e] / 100,
                                floors[a].rooms[b].deviceTypes[c].devices[d].values[e] % 100);
                        serialPrint3(actionsValues[e], F(": "), buffer);
                      }
                      else if (floors[a].rooms[b].deviceTypes[c].nameKey != 'W') {
                        if (e != 0) Serial.print(F(", "));
                        // Format as level (XXX)
                        sprintf(buffer, "%03d", floors[a].rooms[b].deviceTypes[c].devices[d].values[e]);
                        serialPrint3(actionsValues[e], F(": "), buffer);
                      }
                    }
                    free(name_);
                    Serial.println();
                  }
                }
                free(type_);
              }
            }
            free(room_);
          }
        }
        free(floor_);
      }
    }
    Serial.println(F("\n===================================================="));
  }

  /*
   * Gets the house structure stored in the EEPROM and uses it as the
   * house structure in the program
   */
  void getFromEEPROM() {
    if(!validEEPROM()){
      Serial.println(F("There is not a valid house structure in the EEPROM".);
      return;
    }
    // Get each floor from the EEPROM
    int address = 0;
    for (byte a = 0; a < NUM_OF_FLOORS; a++) {
      floors[a].nameKey = EEPROM.read(address);
      if(floors[a].nameKey != ' ') hasFloor = true;
      address++;
      // Get each room from the EEPROM
      for (byte b = 0; b < NUM_OF_ROOMS; b++) {
        floors[a].rooms[b].nameKey = EEPROM.read(address);
        address++;
        // Get each type from the EEPROM
        for (byte c = 0; c < NUM_OF_TYPES; c++) {
          floors[a].rooms[b].deviceTypes[c].nameKey = EEPROM.read(address);
          address++;
          // Get each name to the EEPROM
          for (byte d = 0; d < NUM_OF_NAMES; d++) {
            floors[a].rooms[b].deviceTypes[c].devices[d].nameKey = EEPROM.read(address);
            address++;
            floors[a].rooms[b].deviceTypes[c].devices[d].qualifierKey = EEPROM.read(address);
            address++;
          }
        }
      }
    }
  }

  /*
   * Stores the current house structure in the EEPROM
   */
  void updateEEPROM() {
    // Add each floor to the EEPROM
    int address = 0;
    for (byte a = 0; a < NUM_OF_FLOORS; a++) {
      EEPROM.update(address, floors[a].nameKey);
      address++;
      // Add each room to the EEPROM
      for (byte b = 0; b < NUM_OF_ROOMS; b++) {
        EEPROM.update(address, floors[a].rooms[b].nameKey);
        address++;
        // Add each type to the EEPROM
        for (byte c = 0; c < NUM_OF_TYPES; c++) {
          EEPROM.update(address, floors[a].rooms[b].deviceTypes[c].nameKey);
          address++;
          // Add each name to the EEPROM
          for (byte d = 0; d < NUM_OF_NAMES; d++) {
            EEPROM.update(address, floors[a].rooms[b].deviceTypes[c].devices[d].nameKey);
            address++;
            EEPROM.update(address, floors[a].rooms[b].deviceTypes[c].devices[d].qualifierKey);
            address++;
          }
        }
      }
    }
  }
};

// Macro for checking if address contains valid key or is empty (also valid)
#define checkAddress if((char)EEPROM.read(address) != ' ' && !validDescriptor(menuPosition, (char)EEPROM.read(address))) return false;

/*
 * Checks if the current values stored in the EEPROM are valid
 */
bool validEEPROM() {
  // Get each floor from the EEPROM
  int address = 0;
  byte menuPosition = FLOOR;
  for (byte a = 0; a < NUM_OF_FLOORS; a++) {
    menuPosition = FLOOR;
    // Check if address is not empty and invalid (empty doesn't cause errors)
    checkAddress
    address++;
    // Check each room from the EEPROM
    if ((char)EEPROM.read(address) != ' ') {
      for (byte b = 0; b < NUM_OF_ROOMS; b++) {
        menuPosition = ROOM;
        checkAddress
        address++;
        // Check each type from the EEPROM
        if ((char)EEPROM.read(address) != ' ') {
          for (byte c = 0; c < NUM_OF_TYPES; c++) {
            menuPosition = TYPE;
            checkAddress
            address++;
            // Check each name to the EEPROM
            if ((char)EEPROM.read(address) != ' ') {
              for (byte d = 0; d < NUM_OF_NAMES; d++) {
                menuPosition = NAME;
                checkAddress
                address++;
                menuPosition = QUALIFIER;
                checkAddress
                address++;
              }
            }
          }
        }
      }
    }
  }
  // If no issues are found return true
  return true;
}

// Output a useful warning to the lcd if there are no floors in the house
void noFloors() {
  lcd.clear();
  lcd.print(F("Warning: House"));
  lcd.setCursor(0, 1);
  lcd.print(F("has no floors."));
}

// Instantiate house struct
House house;

//#define MANUALCONFIG

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("ENHANCED: LAMP, OUTSIDE, QUERY, MEMORY, SOFT, EEPROM"));

//  Allows the house to be configured manually instead of using soft menu or eeprom
#ifdef MANUALCONFIG
  house.addDevice('F', '1', 'L');
  house.addDevice('F', '1', 'H');
  house.addDevice('F', '1', 'A');
  house.addDevice('F', '2', 'L');
  house.addDevice('F', '2', 'H');
  house.addDevice('F', '2', 'A');
  house.addDevice('F', 'B', 'L');
  house.addDevice('F', 'B', 'H');
  house.addDevice('F', 'B', 'A');
  house.addDevice('G', 'K', 'L');
  house.addDevice('G', 'K', 'H');
  house.addDevice('G', 'K', 'A');
  house.addDevice('G', 'H', 'L');
  house.addDevice('G', 'H', 'H');
  house.addDevice('G', 'H', 'A');
  house.addDevice('G', 'L', 'L');
  house.addDevice('G', 'L', 'H');
  house.addDevice('G', 'L', 'A');
  house.addDevice('O', 'G', 'L');
  house.addDevice('O', 'R', 'W');
#endif

  //  house.updateEEPROM();
  house.getFromEEPROM();

  // Check if EEPROM contains a valid house structure
  if (validEEPROM()) Serial.println("VALID");

  lcd.begin(16, 2);
  // If house has floor, output to serial monitor
  if (house.hasFloor) {
    lcd.print(F("House/"));
    lcd.setCursor(0, 1);
    lcd.print(keyToWord(FLOOR, house.floors[0].nameKey));
  // Else output warning message
  } else {
    noFloors();
  }
}

/*
 * Add parts to the house from given descriptors
 * Params:
 *  @sentenceLength the number of terms given
 *  @terms that describe the floors, rooms, etc to be added
 */
void addFromInput(byte sentenceLength, char * terms) {
  // Add parts to house based on how many items in descriptor
  switch (sentenceLength - 1) {
    case FLOOR:
      house.addFloor(terms[FLOOR]);
      break;
    case ROOM:
      house.addRoom(terms[FLOOR], terms[ROOM]);
      break;
    case TYPE:
      house.addDevice(terms[FLOOR], terms[ROOM], terms[TYPE]);
      break;
    case NAME:
      house.addDevice(terms[FLOOR], terms[ROOM], terms[TYPE], terms[NAME]);
      break;
    case QUALIFIER:
      house.addDevice(terms[FLOOR], terms[ROOM], terms[TYPE], terms[NAME], terms[QUALIFIER]);
      break;
  }
}

/*
 * Update the LCD with the current menu the user is going through
 * Params:
 *  @choices the user has picked while going through the menu
 *  @menuPosition the user is at (floor, room, etc..)
 *  @currentValue of the on,off,level the user is looking at
 * return used as a break
 */
void updateLCD(byte choices[], byte menuPosition, int * currentValue){
  if(!house.hasFloor){
      noFloors(); 
      return;
  }
  
  // Clear lcd screen
  lcd.clear();
  char * topLine, * bottomLine, * extra;

  // Update menu on lcd screen
  switch (menuPosition) {
    case FLOOR:
      // If first menu position, output "House/" on top line
      lcd.print(F("House/"));
      lcd.setCursor(0, 1);
      bottomLine = FLOOR_WORD;
      lcd.print(bottomLine);
      free(bottomLine);
      break;
    case ROOM:
      topLine = FLOOR_WORD;
      bottomLine = ROOM_WORD;
      lcdPrint3(F("../"), topLine, F("/"));
      lcd.setCursor(0, 1);
      lcd.print(bottomLine);
      free(topLine);
      free(bottomLine);
      break;
    case TYPE:
      topLine = ROOM_WORD;
      bottomLine = TYPE_WORD;
      lcdPrint3(F("../"), topLine, F("/"));
      lcd.setCursor(0, 1);
      lcd.print(bottomLine);
      free(topLine);
      free(bottomLine);
      break;
    case NAME:
      topLine = TYPE_WORD;
      bottomLine = NAME_WORD;
      lcdPrint3(F("../"), topLine, F("/"));
      lcd.setCursor(0, 1);
      lcd.print(bottomLine);
      free(topLine);
      free(bottomLine);
      if (DEVICE_QUALIFIER != ' ') {
        extra = QUALIFIER_WORD;
        lcd.print(F(" "));
        lcd.print(extra);
        free(extra);
      }
      break;
    case ACTION:
      topLine = NAME_WORD;
      lcd.print(F("../"));
      lcd.print(topLine);
      if (DEVICE_QUALIFIER != ' ') {
        extra = QUALIFIER_WORD;
        lcd.print(F(" "));
        lcd.print(extra);
        free(extra);
      }
      free(topLine);
      lcd.print(F("/"));
      lcd.setCursor(0, 1);
      lcd.print(actionsValues[choices[ACTION]]);
      break;
    default:
      lcdPrint3(F("../"), actionsValues[choices[ACTION]], F(":"));
      lcd.setCursor(0, 1);
      char buffer[10];
      // Output level in the format XXX
      if (choices[ACTION] == LEVEL) sprintf(buffer, "%03d", *currentValue);
      // Output time in the format XX.XX
      else sprintf(buffer, "%02d.%02d", *currentValue / 100, *currentValue % 100);
      lcd.print(buffer);
      break;
  }
}

#define IN_RANGE choices[state] < maxes[state] - 1

void loop() {
  static bool softMenuActivated = false;
  static char inputChar = ' ';

  // If chars have been entered following an S
  if (Serial.available() && softMenuActivated) {
    // Get char from input
    char ch = Serial.read();
    static char temp[5];
    static byte len = 0;
    // Check if char matches a breakpoint character
    if (ch == ',' || ch == '\n' || ch == '.') {
      // Create descriptor of correct length and copy from temp to terms
      char * terms = (char *) malloc(len);
      strncpy(terms, temp, len);
      addFromInput(len, terms);
      // Reset variables
      free(terms);
      len = 0;
      if (ch == '.') softMenuActivated = false;
    }
    // Add character to temp array
    else if (validDescriptor(len, ch)) {
      temp[len] = ch;
      len++;
    }
  }
  else if (softMenuActivated) softMenuActivated = false;
  else if (Serial.available()) {
    // read the input char
    inputChar = Serial.read();
    // If user inputs a Q then output the house to the serial monitor
    if (inputChar == 'Q') house.displayHouse();
    // If user inputs an M then output the free memory to the serial monitor
    else if (inputChar == 'M') {
      Serial.print(F("Data: "));
      Serial.println(freeMemory());
    }
    // If user inputs an S then treat the following characters as descriptors
    else if (inputChar == 'S') softMenuActivated = true;
  }


  // Variables for controlling button inputs
  static byte button_state, prev = 0;
  // Variables for controlling menu
  static byte maxes[5] {NUM_OF_FLOORS, NUM_OF_ROOMS, NUM_OF_TYPES, NUM_OF_NAMES, NUM_OF_ACTIONS},
         choices[5] {0, 0, 0, 0, 0}, state = 0;

  static bool outputHouse = false;

  // Get button input from user
  button_state = lcd.readButtons();

  // If button state has changed
  if (button_state != prev) {

    // Pointers to store values of current selection to make code more concise and readable
    int * valuesArray = house.floors[choices[FLOOR]].rooms[choices[ROOM]].deviceTypes[choices[TYPE]].devices[choices[NAME]].values;
    int * currentValue = &valuesArray[choices[ACTION]];
    
    prev = button_state;

    // Make sure that the "Level" attribute doesn't show up for water devices
    DEVICE_TYPE == 'W' ? maxes[ACTION] = 2 : maxes[ACTION] = 3;

    // ========== MAIN FSM ==========
    switch(state){
      case FLOOR_SELECTION:
        // When UP button is pressed
        if (button_state & BUTTON_UP) {
          // Decrement menu item
          if (choices[state] > FLOOR && !outputHouse && choices[state] > 0) choices[state]--;
          else if (outputHouse) outputHouse = false;
        }
        // When DOWN button is pressed
        else if (button_state & BUTTON_DOWN) {
          // Increment menu item if next item exists
          if (IN_RANGE && NEXT_FLOOR != ' ') choices[state]++;
          // Go to "send values" menu item
          else if (choices[state] < maxes[state] || NEXT_FLOOR == ' ') {
            lcd.clear();
            lcd.print("Send values");
            outputHouse = true;
          }
        }
        // When RIGHT button is pressed
        else if (button_state & BUTTON_RIGHT && ROOM_NAME != ' ') {
          // Go to next state
          state = ROOM_SELECTION;
        }
        // When SELECT button is pressed
        else if (button_state & BUTTON_SELECT && outputHouse) {
          // If on output house menu item, display house
          outputHouse = false;
          house.displayHouse();
        }
        break;
        
      case ROOM_SELECTION:
        // When UP button is pressed
        if (button_state & BUTTON_UP && choices[state] > 0) {
          // Decrement menu item
          choices[state]--;
        }
        // When DOWN button is pressed
        else if (button_state & BUTTON_DOWN) {
          // Increment menu item if next item exists
          if (IN_RANGE && NEXT_ROOM != ' ') choices[state]++;
        }
        // When LEFT button is pressed
        else if (button_state & BUTTON_LEFT) {
          // Go back to previous state
          choices[state] = 0;
          state = FLOOR_SELECTION;
        }
        // When RIGHT button is pressed
        else if (button_state & BUTTON_RIGHT && DEVICE_TYPE != ' ') {
          // Go to next state
          state = TYPE_SELECTION;
        }
        break;
        
      case TYPE_SELECTION:
        // When UP button is pressed
        if (button_state & BUTTON_UP && choices[state] > 0) {
          // Decrement menu item
          choices[state]--;
        }
        // When DOWN button is pressed
        else if (button_state & BUTTON_DOWN) {
          // Increment menu item if next item exists
          if (IN_RANGE && NEXT_TYPE != ' ') choices[state]++;
        }
        // When LEFT button is pressed
        else if (button_state & BUTTON_LEFT) {
          // Go back to previous state
          choices[state] = 0;
          state = ROOM_SELECTION;
        }
        // When RIGHT button is pressed
        else if (button_state & BUTTON_RIGHT && DEVICE_NAME != ' ') {
          // Go to next state
          state = NAME_SELECTION;
        }
        break;
        
      case NAME_SELECTION:
        // When UP button is pressed
        if (button_state & BUTTON_UP && choices[state] > 0) {
          // Decrement menu item
          choices[state]--;
        }
        // When DOWN button is pressed
        else if (button_state & BUTTON_DOWN) {
          // Increment menu item if next item exists
          if (IN_RANGE && NEXT_DEVICE != ' ') choices[state]++;
        }
        // When LEFT button is pressed
        else if (button_state & BUTTON_LEFT) {
          // Go back to previous state
          choices[state] = 0;
          state = TYPE_SELECTION;
        }
        // When RIGHT button is pressed
        else if (button_state & BUTTON_RIGHT) {
          // Go to next state
          state = ACTION_SELECTION;
        }
        break;
        
      case ACTION_SELECTION:
        // When UP button is pressed
        if (button_state & BUTTON_UP && choices[state] > 0) {
          // Decrement menu item
          choices[state]--;
        }
        // When DOWN button is pressed
        else if (button_state & BUTTON_DOWN) {
          // Increment menu item if next item exists
          if (IN_RANGE) choices[state]++;
        }
        // When LEFT button is pressed
        else if (button_state & BUTTON_LEFT) {
          // Go back to previous state
          choices[state] = 0;
          state = NAME_SELECTION;
        }
        // When RIGHT button is pressed
        else if (button_state & BUTTON_RIGHT) {
          // Go to next state
          state = VALUE_SELECTION;
        }
        break;
        
      case VALUE_SELECTION:
        // When UP button is pressed
        if (button_state & BUTTON_UP) {
          // Increase current value
          if (actionsValues[choices[4]] == "Level" && *currentValue < MAX_LEVEL) *currentValue += 1;
          else if (*currentValue < 2300) *currentValue += 100;
        }
        // When DOWN button is pressed
        else if (button_state & BUTTON_DOWN) {
          // Decrease current value
          if (actionsValues[choices[4]] == "Level" && *currentValue > 0) *currentValue -= 1;
          else if (*currentValue > 99) *currentValue -= 100;
        }
        // When LEFT button is pressed
        else if (button_state & BUTTON_LEFT) {
          // Go back to previous state
          choices[state] = 0;
          state = ACTION_SELECTION;
        }
        // When RIGHT button is pressed
        else if (button_state & BUTTON_RIGHT && choices[ACTION] != LEVEL) {
          // Go to next state if value is in a time format
          state = VALUE2_SELECTION;
        }
        break;
        
      case VALUE2_SELECTION:
        // When UP button is pressed
        if (button_state & BUTTON_UP) {
          if (*currentValue % 100 < 59) *currentValue += 1;
        }
        // When DOWN button is pressed
        else if (button_state & BUTTON_DOWN) {
          if (*currentValue % 100 > 0) *currentValue -= 1;
        }
        // When LEFT button is pressed
        else if (button_state & BUTTON_LEFT) {
          if (state < VALUE) choices[state] = 0;
          state = VALUE_SELECTION;
        }
        break;
        
    }
    if(button_state != 0 && !outputHouse){
      // Whenever a button is pressed, update the lcd on the screen
      updateLCD(choices, state, currentValue);

      // Prints free memory to serial monitor if in testing mode
      #ifdef testing
        Serial.print(F("Free Memory: "));
        Serial.println(freeMemory);
      #endif
    }
  }
}
