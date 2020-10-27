/** @file functions.cpp
 *  @brief
 *  
 *  This file contains relevant defines, structures and
 *  functions that node.ino will call.
 *  
 *  @author Pavle Lakic
 *  @bug No known bugs
 */
#include "includes.h"

/** Name of file where round and if node was cluster head
 *  will be kept.
 */
const char *filename = "/conf.txt";

/** Probability that node will be cluster head for current round.
 *  Determined apriori, depends of number of nodes.*/
const float P = 0.5;

/** This is the address of base station.*/
const IPAddress base_station(192,168,4,1);

/** This address will be used as broadcast*/
const IPAddress broadcast(192,168,4,255);

/** Subnet mask for local network. */
const IPAddress subnet(255,255,255,0);

/** Gateway address*/
const IPAddress gateway(192,168,4,0);

const char* create_node_id (char *txt)
{
  char *node = "Node ";
  strcat(node, txt);

  return node;
}

void wifi_connect(unsigned char CH)
{ 
  int n;
  String strongest;
  int power;
  const char *node_id;

  switch (CH) {
    
    case  0:
    
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      n = WiFi.scanNetworks();
     if (n == 0) {
        Serial.println("no networks found");
        // what to do if no networks are found?
        // declare as CH?
     } 
     else {
#if DEBUG
        Serial.print(n);
        Serial.println(" networks found");
#endif
        strongest = WiFi.SSID(0);
        power = WiFi.RSSI(0);        
        for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
          if (WiFi.RSSI(i) > power) {
            power = WiFi.RSSI(i);
            strongest = WiFi.SSID(i);
          }
#if DEBUG
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
#endif          
        delay(10);
        }
        if (strongest == BASE_SSID) {
          WiFi.begin(BASE_SSID, BASE_PASS);
        }
        else {
          WiFi.begin(strongest, NODE_PASS); 
        }

          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
          }
      }

#if DEBUG
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  Serial.println(strongest);
  Serial.println(power);
  Serial.println("I`m not CH for this round");
#endif
    break;

    case  1:

      node_id = create_node_id(MY_NODE);
      WiFiUDP Udp;
      WiFi.mode(WIFI_AP_STA);
      //WiFi.disconnect();
      WiFi.begin(BASE_SSID, BASE_PASS);
     
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }

      WiFi.softAPConfig(WiFi.localIP(), gateway, subnet);
      WiFi.softAP(node_id, NODE_PASS);
#if DEBUG
      Serial.print("node_id = ");
      Serial.println(node_id);
      Serial.print("AP IP address: ");
      Serial.print(WiFi.softAPIP());
      Serial.println();
#endif
/*
     Udp.begin(BROADCAST_PORT);
     Udp.beginPacket(broadcast, BROADCAST_PORT);
     Udp.write(Reply);
     Udp.endPacket();
*/
#if DEBUG
     Serial.print("Connected to ");
     Serial.println(WiFi.SSID());
     Serial.print("IP address:\t");
     Serial.println(WiFi.localIP());
     Serial.println("I`m CH for this round");
#endif

    break;  
  }
}

void full_circle(unsigned char *round_cnt, unsigned char *ch_enable)
{
  *round_cnt += 1;
  if (*round_cnt > NUMBER_OF_NODES - 1)
  {
    *round_cnt = 0;
    *ch_enable = 1;
#if DEBUG    
    Serial.println("Full circle from beggining.");
#endif    
  }
  write_fs(*round_cnt, *ch_enable);
}

unsigned char cluster_head(unsigned char *round_cnt, unsigned char *ch_enable)
{
  float rnd_nmb;
  float threshold;
  unsigned char ret;

  rnd_nmb = random_number();
  threshold = calculate_threshold(P, *round_cnt);

  if ((rnd_nmb < threshold) && (*ch_enable == 1)) {
    *ch_enable = 0;
    ret = 1;
  }
  else {
    *ch_enable = 1;
    ret = 0;
  }

  write_fs(*round_cnt, *ch_enable);

#if DEBUG
  if (*ch_enable == 0) {
    Serial.println("Node will be cluster head for current round.");
  }
  else {
    Serial.println("Node will not be cluster head for current round.");
  }
#endif

  return ret;
}

bool mount_fs(void)
{ 
  bool success = SPIFFS.begin();
  if (!success){
    Serial.println("Could not mount SPIFFS!");
  }
  return success; 
}

void write_fs(int round_cnt, int ch)
{
  File fp;
  char round_cnt_str[ROUND_NUMBER_LENGTH];
  char ch_str[CH_NUMBER_LENGTH];

  sprintf(round_cnt_str, "%d", round_cnt);
  sprintf(ch_str, "%d", ch);

  fp = SPIFFS.open(filename, "w");

  if (!fp) {
    Serial.println("file open failed");
  }
  else {
      fp.write(round_cnt_str, ROUND_NUMBER_LENGTH);
      fp.write(ch_str, CH_NUMBER_LENGTH);
      fp.close();
  }
}

void read_fs(unsigned char *round_cnt, unsigned char *ch)
{
  File fp;
  char round_cnt_str[ROUND_NUMBER_LENGTH];
  char ch_str[CH_NUMBER_LENGTH];

  fp = SPIFFS.open(filename, "r");

  fp.read((uint8_t*)round_cnt_str, ROUND_NUMBER_LENGTH);
  fp.read((uint8_t*)ch_str, CH_NUMBER_LENGTH);
  fp.close();

  *round_cnt = atol(round_cnt_str);
  *ch = atol(ch_str);

#if DEBUG
  Serial.print("round_cnt = ");
  Serial.println(*round_cnt);
  Serial.print("ch_enable = ");
  Serial.println(*ch);
#endif
}

float calculate_threshold(float P, unsigned char r)
{
  float T;

  T = P/(1 - P*(r % ((unsigned char)round(1/P))));

#if DEBUG
  Serial.print("r = ");
  Serial.println(r);
  Serial.print("T = ");
  Serial.println(T, 3);
#endif;
  
  return T;
}

float random_number(void)
{
  float a;

  a = ESP8266TrueRandom.random(10000);
  a = a/10000;

#if DEBUG
  Serial.print("random_number = ");
  Serial.println(a, 3);
#endif

  return a;
}
