// **********************************************************************************
// ULPNode RF protocol paylaods used for transmission
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// For any explanation of ULPNode see 
// https://hallard.me/category/ulpnode/
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2014-07-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************
#include <arduino.h>
#include "ULPNode_RF_Protocol.h"

#ifdef SPARK
const char * rf_frame[] = {
#else
const char * const rf_frame[] PROGMEM = {
#endif
    // Payload command code (14 total) "x" are free
    "UNKNOWN",    "ALIVE",      "PING",       "PINGBACK", 
    "OTA_CONFIG", "OTA_UPDATE", "OTA_AUTOID", "SYS_END",
    "8",          "9",          "A",          "B",
    "C",          "D",          "E",          "F" ,
    // Payload DATA code (14 total) "x" are free
    "DAT_START",  "DATA",       "DAT_END",    "3",
    "4",          "5",          "6",          "7",
    "8",          "9",          "A",          "B",
    "C",          "D",          "E",          "F" ,
  };

/*
#ifdef SPARK
const char * rf_data[] = {
#else
const char * const rf_data[] PROGMEM = {
#endif
    "SYS00","SYS01","TEMP", "HUM", 
    "LUX",  "CO2",  "RSSI", "VOLT", 
    "BAT",  "COUNT", "ERR", "ERR"
  };
*/
// Buffer used to decode specific data value
char pbuf[24]; 

// Buffer containing JSON data string to return
char json_str[128]; 

/* ======================================================================
Function: decode_bat
Purpose : print the battery voltage value
Input   : battery (mV)
          index of sensor number (0 to 3)
====================================================================== */
char * decode_bat(uint16_t bat, char * index)
{
  sprintf_P(pbuf, PSTR("\"bat%s\":"), index);
  dtostrf(bat/1000.0f, 5, 3, pbuf+strlen(pbuf));
  return pbuf;
}

/* ======================================================================
Function: decode_lowbat
Purpose : print low bat state
Input   : true if low bat
====================================================================== */
char * decode_lowbat(uint8_t low, char * index)
{
  sprintf_P(pbuf, PSTR("\"lowbat%s\":%c"), index, '0'+low);
  return pbuf;
}

/* ======================================================================
Function: decode_volt
Purpose : print the voltage value
Input   : voltage (mV)
          index of sensor number (0 to 3)
====================================================================== */
char * decode_volt(uint16_t volt, char * index)
{
  // convert voltage to V format xx.yyy
  sprintf_P(pbuf, PSTR("\"volt%s\":%s"), index);
  dtostrf(volt/1000.0f, 5, 3, pbuf+strlen(pbuf));
  return pbuf;
}

/* ======================================================================
Function: decode_temp
Purpose : print the temperature value
Input   : temp (*100)
          index of sensor number (0 to 3)
====================================================================== */
char * decode_temp(int16_t temp, char * index)
{
  sprintf_P(pbuf, PSTR("\"temp%s\":"), index);
  dtostrf(temp/100.0f, 4, 2,  pbuf+strlen(pbuf));
  return pbuf;
}

/* ======================================================================
Function: decode_hum
Purpose : print the humidity
Input   : humidity (*10)
          index of sensor number (0 to 3)
====================================================================== */
char * decode_hum(uint16_t hum, char * index)
{
  sprintf_P(pbuf, PSTR("\"hum%s\":%d"), index, hum);
  dtostrf(hum/10.0f, 4, 1,  pbuf+strlen(pbuf));
  return pbuf;
}

/* ======================================================================
Function: decode_lux
Purpose : print the lux value
Input   : lux (*10)
          index of sensor number (0 to 3)
====================================================================== */
char * decode_lux(uint16_t lux, char * index)
{
  sprintf_P(pbuf, PSTR("\"lux%s\":"), index);
  dtostrf(lux/10.0f, 3, 1,  pbuf+strlen(pbuf));
  return pbuf;
}

/* ======================================================================
Function: decode_co2
Purpose : print the co2 value
Input   : co2
          index of sensor number (0 to 3)
====================================================================== */
char * decode_co2(uint16_t co2, char * index)
{
  sprintf_P(pbuf, PSTR("\"co2%s\":%d"), index, co2);
  return pbuf;
}

/* ======================================================================
Function: decode_rssi
Purpose : print the rssi value
Input   : rssi
          index of sensor number (0 to 3)
====================================================================== */
char * decode_rssi(int8_t rssi, char * index)
{
  sprintf_P(pbuf, PSTR("\"rssi%s\":%d"), index, rssi);
  return pbuf;
}

/* ======================================================================
Function: decode_counter
Purpose : print a counter value
Input   : counter
          index of sensor number (0 to 3)
====================================================================== */
char * decode_counter(uint32_t counter, char * index)
{
  sprintf_P(pbuf, PSTR("\"count%s\":%ld"), index, counter);
  return pbuf;
}

/* ======================================================================
Function: decode_digital_io
Purpose : print a digital IO value
Input   : Digital IO value
          digital pin (0 to 16)
====================================================================== */
char * decode_digital_io(uint8_t value, char * pin)
{
  sprintf_P(pbuf, PSTR("\"d%d\":%d"), pin, value);
  return pbuf;
}

/* ======================================================================
Function: decode_analog_io
Purpose : print a analog IO value
Input   : analog value 
          analog pin (0 to 7)
====================================================================== */
char * decode_analog_io(uint16_t value, char * pin)
{
  sprintf_P(pbuf, PSTR("\"a%d\":%ld"), pin, value);
  return pbuf;
}

/* ======================================================================
Function: add_json_data
Purpose : Add json data to json string
Input   : json global buffer
          json data to add
Comments: -
====================================================================== */
char * add_json_data(char * str, char * json)
{
  uint8_t l = strlen(str);

  // Some checking on size, just in case
  if ( l + strlen(json) < sizeof(json_str))
    sprintf_P(&str[l], PSTR(", %s"), json);

  return str;
}

/* ======================================================================
Function: decode_frame_type
Purpose : print the frame type
Input   : type
Output  : -
Comments: -
====================================================================== */
char * decode_frame_type(uint8_t type)
{
  // check command type is known
  if (!isPayloadValid(type))
    type =0;

  #ifdef SPARK
    strcpy(pbuf, rf_frame[type]);
  #else
    strcpy(pbuf, (char*)pgm_read_word(&(rf_frame[type])));
  #endif

  return pbuf;
}

/* ======================================================================
Function: decode_received_data
Purpose : send to serial received data in human format
Input   : node id
          rssi of data received
          size of data
          command of frame received
          pointer to the data
Output  : command code validated by payload size type reveived
Comments: if we had a command and payload does not match
          code as been set to 0 to avoid check in next
====================================================================== */
uint8_t decode_received_data(uint8_t nodeid, int8_t rssi, uint8_t len, uint8_t c, uint8_t * ppayload)
{
  char *    pjson = json_str;
  uint8_t * pdat = ppayload;
  char *    ptype ;

  // Show packet type name
  ptype = decode_frame_type(c);

  // Start our buffer string
  //sprintf_P(json_str, PSTR("{\"id\":%d, \"type\":\"%s\", \"rssi\":%d"), nodeid, ptype, rssi);
  sprintf_P(json_str, PSTR("{\"id\":%d, \"rssi\":%d"), nodeid, rssi);

  // this is for known packet command
  // Alive packet ?
  if ( c==RF_PL_ALIVE && len==sizeof(RFAlivePayload))
  {
    sprintf_P(pbuf, PSTR("\"state\":%04X"), ((RFAlivePayload*)pdat)->status);
    add_json_data(json_str, pbuf);
    add_json_data(json_str, decode_bat(((RFAlivePayload*)pdat)->vbat,""));
  }
  // ping/ping back packet ?
  else if ( (c==RF_PL_PING || c==RF_PL_PINGBACK) && len==sizeof(RFPingPayload))
  {
    sprintf_P(pbuf, PSTR("\"state\":%04X"), ((RFAlivePayload*)pdat)->status);
    add_json_data(json_str, pbuf);
    // Vbat is sent only on emiting ping packet, not ping back
    if (c==RF_PL_PING )
      add_json_data(json_str, decode_bat(((RFPingPayload*)pdat)->vbat,""));

    // RSSI from other side is sent only in pingback response
    // this is the 2nd rssi value, we call it myrssi 
    if (c==RF_PL_PINGBACK)
    {
      sprintf_P(pbuf, PSTR("\"myrssi\":%d"), rssi);
      add_json_data(json_str, pbuf);
    }
  }
  // payload Packet with datas
  // we need at least size of payload > 2 
  // 1 payload command + 1 sensor type + 1 sensor data) 
  // and is one of our known data code. This is for received data
  else if ( isPayloadData(c) && len>2)
  {
    uint8_t data_size ;
    uint8_t data_type ;
    uint8_t l ;
    char *  pval;
    boolean error ;

    // Ok we set up on 1st data field
    data_type = *(++pdat);
    l = len -1;

    // Loop through all data contained into the payload
    // discard 1st byte on each, which is header data code
    do
    {      
      // each sensor type can have 4 values sent, 
      char str_idx[] = " ";
      data_size = 0;
      pval= NULL;
      error = false;

      // If index of sensor value is > 0 change string
      // label adding the index if at least 2 values
      // i.e if 2 sensor temp are sent/received the
      // result will look in JSON like
      // ie : {temp:20.1, temp1:22.11, ...}
      *str_idx = '0' + (data_type & ~RF_DAT_SENSOR_MASK);

      // the first we don't add index number this save 1 char
      if (*str_idx=='0')
        *str_idx='\0'; 

      // Temperature, and have enought data ?
      if (isDataTemp(data_type) && l>=sizeof(s_temp))
      {
        pval = decode_temp(((s_temp*)pdat)->temp, str_idx);
        data_size = sizeof(s_temp);
      }
      // Humidity, and have enought data ?
      else if (isDataHum(data_type) && l>=sizeof(s_hum))
      {
        pval = decode_hum(((s_hum*)pdat)->hum, str_idx);
        data_size = sizeof(s_hum);
      }
      // Luminosity and have enought data ?
      else if (isDataLux(data_type) && l>=sizeof(s_lux))
      {
        pval =  decode_lux(((s_lux*)pdat)->lux, str_idx);
        data_size = sizeof(s_lux);
      }
      // CO2 and have enought data ?
      else if (isDataCO2(data_type) && l>=sizeof(s_co2))
      {
        pval = decode_co2(((s_co2*)pdat)->co2, str_idx);
        data_size = sizeof(s_co2);
      }
      // voltage and have enought data ?
      else if (isDataVolt(data_type) && l>=sizeof(s_volt))
      {
        pval = decode_volt(((s_volt*)pdat)->volt, str_idx);
        data_size = sizeof(s_volt);
      }
      // battery (same payload as volt) and have enought data ?
      else if (isDataBat(data_type) && l>=sizeof(s_volt))
      {
        pval = decode_bat(((s_volt*)pdat)->volt, str_idx);
        data_size = sizeof(s_volt);
      }
      // RSSI and have enought data ?
      else if (isDataRSSI(data_type) && l>=sizeof(s_rssi))
      {
        pval =  decode_rssi(((s_rssi*)pdat)->rssi, str_idx);
        data_size = sizeof(s_rssi);
      }
      // counter and have enought data ?
      else if (isDataCounter(data_type) && l>=sizeof(s_counter))
      {
        pval =  decode_counter(((s_counter*)pdat)->counter, str_idx);
        data_size = sizeof(s_counter);
      }
      // lowbat and have enought data ?
      else if (isDataLowBat(data_type) && l>=sizeof(s_lowbat))
      {
        pval =  decode_lowbat(((s_lowbat*)pdat)->lowbat, str_idx);
        data_size = sizeof(s_lowbat);
      }
      // Unknown data code, so we can't check data value
      // nor size, so we decide to discard the 
      // end of this frame
      else
      {
        ULPNP_DebugF("Parsing error");
        error = true;
      }

/*
      ULPNP_DebugF("[0x");
      ULPNP_Debug(data_type,HEX);
      ULPNP_DebugF("] -> l=");
      ULPNP_Debug(l);
      ULPNP_DebugF(" added=");
      ULPNP_Debug(data_size);
      ULPNP_DebugF(" pval='");
      ULPNP_Debug(pval);
      ULPNP_DebugF("' next=");
      */

      // Something to add ?
      if (!error && data_size && pval)
      {
        // Add to JSon string
        add_json_data(json_str, pval);

        // remove data size we just worked on
        l-= data_size;    

        // Rest some data after the code
        if (l>=2)
        {
          pdat+=data_size; // Pointer to next data on buffer
          data_type=*pdat;         // get next data field

          //ULPNP_Debug(data_type,HEX);
        }
        else
        {
          //ULPNP_DebugF("none");
        }
      }

      //ULPNP_Debugln();

    } // while data
    while(l>1 && !error);

  } // If sensor data
  // not known data code, raw display packet
  else
  {
    uint8_t * p = (uint8_t *) ppayload;
  
    // send raw values
    strcat(json_str, ", \"raw\":\"");

    // Add each received value
    while (len--)
      sprintf_P(&json_str[strlen(json_str)], PSTR("%02X "), *p++);

    strcat(json_str, "\"");

    // here we did not validated known packet, so clear command
    // code for the rest of the operation
    c=0;
  }

  // End our buffer string
  strcat(json_str, "}");
  ULPNP_Debug(json_str);

  return (c);
}
