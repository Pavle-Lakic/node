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
 *  will be kept.*/
const char *filename = "/conf.txt";

/** Probability that node will be cluster head for current round.
 *  Determined apriori, depends of number of nodes.*/
const float P = 0.5;

/** This is the address of base station.*/
const IPAddress base_station(192,168,4,1);

/** This address will be used as broadcast*/
const IPAddress broadcast(192,168,4,255);

const IPAddress send_broadcast(192,168,4,254);

/** Subnet mask for local network. */
const IPAddress subnet(255,255,255,0);

/** Gateway address*/
const IPAddress gateway(192,168,4,1);

/** Port where broadcast data will be sent*/
unsigned int BROADCAST_PORT = 2000;

/**This node name.*/
const char *node_name;

Ticker timeout;
WiFiUDP Udp;
WiFiUDP CH_udp;

#if DEBUG
void print_connected(void)
{
  unsigned char number_client;
  struct station_info *stat_info;
  unsigned int prvi;
  
  struct ip4_addr *IPaddress;
  IPAddress address;
  int i=1;
  char macStr[18] = {0};
  
  number_client= wifi_softap_get_station_num();
  stat_info = wifi_softap_get_station_info();
  
  Serial.print(" Total Connected Clients are = ");
  Serial.println(number_client);
  
    while (stat_info != NULL) {
    
      IPaddress = &stat_info->ip;
      address = IPaddress->addr;

      Serial.print("client= ");
      Serial.print(i);
      Serial.print(" IP adress is = ");
      Serial.print((address));
      Serial.print(" with MAC adress is = ");

      //TODO napravi da nodovi koji su povezani salju svoju  MAC
      // adresu na AP preko UDP, a da AP prosledi to do bazne stanice.
      // bazna stanica moze da razvrsta na osnovu MAC adrese da li je to nod 0
      // nod 1 itd.

      Serial.print(stat_info->bssid[0],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[1],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[2],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[3],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[4],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[5],HEX);Serial.print(" ");
      sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", stat_info->bssid[0],  stat_info->bssid[1], stat_info->bssid[2], stat_info->bssid[3], stat_info->bssid[4], stat_info->bssid[5]);
      Serial.println(macStr);
      stat_info = STAILQ_NEXT(stat_info, next);
      i++;
      Serial.println();
    }
}
#endif

const char* create_node_id (void)
{
  const char *node;
  String mac_address;

  mac_address = WiFi.macAddress();

#if DEBUG
  Serial.println(mac_address);
#endif

  if (mac_address == MAC_NODE_0) {
    node = NODE_0;
  }
  else if (mac_address == MAC_NODE_1) {
    node = NODE_1;
  }
  else if (mac_address == MAC_NODE_2) {
    node = NODE_2;
  }
  else if (mac_address == MAC_NODE_3) {
    node = NODE_3;
  }
  else if (mac_address == MAC_NODE_4) {
    node = NODE_4;
  }
  else if (mac_address == MAC_NODE_5) {
    node = NODE_5;
  }
  else if (mac_address == MAC_NODE_6) {
    node = NODE_6;
  }
  else {
    node = "New Node!";
  }

  node_name = node;
  return node;
}

void advertise(unsigned char CH)
{
  bool received = false;
  char packetBuffer[255];
  unsigned char number_of_nodes;
  struct station_info *stat_info;
  struct ip4_addr *IPaddress;
  char macStr[18] = {0};
  IPAddress address; 

  switch(CH) {

    case NODE:

      Udp.begin(BROADCAST_PORT);

      while(!received) {

      int packetSize = Udp.parsePacket();

        if (packetSize) {

#if DEBUG
          Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                  packetSize,
                  Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                  Udp.destinationIP().toString().c_str(), Udp.localPort(),
                  ESP.getFreeHeap());
#endif

          int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
          packetBuffer[n] = '\0';

#if DEBUG         
          Serial.println("Contents:");
          Serial.println(packetBuffer);
          Serial.println(node_name);
#endif

          if (strcmp(packetBuffer,node_name) == 0) {
            received = true;

#if DEBUG
            Serial.println("Sending MAC address and ADC value ...");
#endif

                Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                Udp.write(WiFi.macAddress().c_str());
                // after sleep, send adc value.
                Udp.endPacket();
                yield();
          }
          else {
            Serial.println("not same");
            received = false;
          }

        }
      }

#if DEBUG
        Serial.print("I will be part of :");
        Serial.println(WiFi.SSID());
        Serial.print("IP address:\t");
        Serial.println(WiFi.localIP());
#endif

    break;

    case CLUSTER_HEAD:

#if DEBUG
        Serial.println("Waiting for other nodes ...");
#endif

        delay(15000);

#if DEBUG
        Serial.println("Done waiting for other nodes to connect!");
        //print_connected();
#endif

        number_of_nodes = wifi_softap_get_station_num();
        stat_info = wifi_softap_get_station_info();

    while (stat_info != NULL) {
    
      IPaddress = &stat_info->ip;
      address = IPaddress->addr;

#if DEBUG
      Serial.print(" IP adress is = ");
      Serial.print((address));
      Serial.print(" with MAC adress is = ");
      Serial.print(stat_info->bssid[0],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[1],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[2],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[3],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[4],HEX);Serial.print(" ");
      Serial.print(stat_info->bssid[5],HEX);Serial.print(" ");
#endif

      sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", stat_info->bssid[0],  stat_info->bssid[1], stat_info->bssid[2], stat_info->bssid[3], stat_info->bssid[4], stat_info->bssid[5]);

#if DEBUG
      Serial.println(macStr);
#endif

      stat_info = STAILQ_NEXT(stat_info, next);
    }
    
    Udp.beginPacket(broadcast, BROADCAST_PORT);

#if DEBUG
    Serial.print("macStr = ");
    Serial.println(macStr);
    Serial.print("local MAC :");
    Serial.println(WiFi.macAddress());
#endif

    if (strcmp(macStr, MAC_NODE_0) == 0) {

#if DEBUG
      Serial.println("Ovde je MAC_NODE_0");
#endif

      Udp.write(NODE_0);
    }
    else if (strcmp(macStr, MAC_NODE_1) == 0) {

#if DEBUG
      Serial.println("Ovde je MAC_NODE_1");
#endif

      Udp.write(NODE_1);
    }
    else if (strcmp(macStr, MAC_NODE_2) == 0) {

#if DEBUG
      Serial.println("Ovde je MAC_NODE_2");
#endif

      Udp.write(NODE_2);
    }
    else if (strcmp(macStr, MAC_NODE_3) == 0) {

#if DEBUG
      Serial.println("Ovde je MAC_NODE_3");
#endif

      Udp.write(NODE_3);
    }
    else if (strcmp(macStr, MAC_NODE_4) == 0) {

#if DEBUG
      Serial.println("Ovde je MAC_NODE_4");
#endif

      Udp.write(NODE_4);
    }
    else if (strcmp(macStr, MAC_NODE_5) == 0) {

#if DEBUG
      Serial.println("Ovde je MAC_NODE_5");
#endif

      Udp.write(NODE_5);
    }
    else if (strcmp(macStr, MAC_NODE_6) == 0) {

#if DEBUG
      Serial.println("Ovde je MAC_NODE_6");
#endif

      Udp.write(NODE_6);
    }
    else {

#if DEBUG
    Serial.println("Dont know who is connected to me!");
#endif

    }
    Udp.endPacket();

    Udp.begin(BROADCAST_PORT);

        while (number_of_nodes > 0) {

          int packetSize = Udp.parsePacket();
   
          if (packetSize) {

              number_of_nodes -= 1;

#if DEBUG
              Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                      packetSize,
                      Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                      Udp.destinationIP().toString().c_str(), Udp.localPort(),
                      ESP.getFreeHeap());
#endif

              int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
              packetBuffer[n] = '\0';

#if DEBUG         
              Serial.println("Contents:");
              Serial.println(packetBuffer);
#endif

              if (strcmp(packetBuffer, MAC_NODE_0) == 0) {

#if DEBUG
                Serial.println("This is from Node 0");
#endif

              }
              else if (strcmp(packetBuffer, MAC_NODE_1) == 0) {

#if DEBUG
                Serial.println("This is from Node 1");
#endif

              }
              else if (strcmp(packetBuffer, MAC_NODE_2) == 0) {

#if DEBUG
                Serial.println("This is from Node 2");
#endif

              }
              else if (strcmp(packetBuffer, MAC_NODE_3) == 0) {

#if DEBUG
                Serial.println("This is from Node 3");
#endif
              }
              else if (strcmp(packetBuffer, MAC_NODE_4) == 0) {

#if DEBUG
                Serial.println("This is from Node 4");
#endif

              }
              else if (strcmp(packetBuffer, MAC_NODE_5) == 0) {

#if DEBUG
                Serial.println("This is from Node 5");
#endif

              }
              else if (strcmp(packetBuffer, MAC_NODE_6) == 0) {

#if DEBUG
                Serial.println("This is from Node 6");
#endif

              }
              else {

#if DEBUG
                Serial.println("I dont know who this is.");
#endif

              }
          }
        }     
    break;
  }
}

void wifi_connect(unsigned char CH)
{ 
  int n;
  String strongest;
  int power;
  const char *node_id;
  char  ReplyBuffer[] = "acknowledged\r\n";

  node_id = create_node_id();  

  switch (CH) {
    
    case  NODE:
    
      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      delay(2000);

      n = WiFi.scanNetworks();
     if (n == 0) {
     } 
     else {

#if DEBUG
        Serial.print(n);
        Serial.println(" networks found");
#endif

        strongest = WiFi.SSID(0);
        power = WiFi.RSSI(0);

        for (int i = 0; i < n; ++i) {

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

    case  CLUSTER_HEAD:

      Udp.begin(BROADCAST_PORT);
      WiFi.mode(WIFI_AP_STA);
      WiFi.disconnect();
      WiFi.begin(BASE_SSID, BASE_PASS);
     
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
      }
        
      WiFi.softAPConfig(WiFi.localIP(), WiFi.localIP(), subnet);
      WiFi.softAP(node_id, NODE_PASS);

#if DEBUG
      Serial.print("node_id = ");
      Serial.println(node_id);
      Serial.print("AP IP address: ");
      Serial.print(WiFi.softAPIP());
      Serial.println();
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
