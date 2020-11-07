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

#define IP(a,b,c,d) (uint32_t)(a | (b << 8) | (c << 16) | (d << 24))

/** Name of file where round and if node was cluster head
 *  will be kept.*/
const char *filename = "/conf.txt";

/** Probability that node will be cluster head for current round.
 *  Determined apriori, depends of number of nodes.*/
const float P = 0.5;

/** This is the address of base station.*/
const IPAddress base_station(192,168,4,1);

/** This address will be used as broadcast*/
const IPAddress broadcast(192,168,5,255);

/** Subnet mask for local network. */
const IPAddress subnet(255,255,255,0);

/** Gateway address*/
const IPAddress gateway(192,168,5,1);

IPAddress ch_address;

/**This node name.*/
char node_name[10];

unsigned short port_to_send;

unsigned short port_to_receive;

volatile unsigned char timeout_udp_flag = 0;

String strongest;

uint32_t apIP;

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

unsigned short read_adc(void)
{
  unsigned short ret;

  ret = analogRead(ADC_PIN);

  return ret;
}
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
    apIP = IP(192, 168, 5, 20);
  }
  else if (mac_address == MAC_NODE_1) {
    node = NODE_1;
    apIP = IP(192, 168, 5, 21);
  }
  else if (mac_address == MAC_NODE_2) {
    node = NODE_2;
    apIP = IP(192, 168, 5, 22);
  }
  else if (mac_address == MAC_NODE_3) {
    node = NODE_3;
    apIP = IP(192, 168, 5, 23);
  }
  else if (mac_address == MAC_NODE_4) {
    node = NODE_4;
    apIP = IP(192, 168, 5, 24);
  }
  else if (mac_address == MAC_NODE_5) {
    node = NODE_5;
    apIP = IP(192, 168, 5, 25);
  }
  else if (mac_address == MAC_NODE_6) {
    node = NODE_6;
    apIP = IP(192, 168, 5, 26);
  }
  else {
    node = "New Node!";
  }

  strcpy(node_name, node);
  return node;
}

void strongest_ch_ssid(void)
{ 
  unsigned char n;
  int power;
  char network[14];

  n = WiFi.scanNetworks();

  if (n == 0) {

#if DEBUG
    Serial.println("Did not find any networks at all!");
#endif

  } 
  else {

#if DEBUG
    Serial.print(n);
    Serial.println(" networks found");
#endif

    power = WiFi.RSSI(0);
    strongest = WiFi.SSID(0);

    for (int i = 0; i < n; ++i) {
     if (check_ch(WiFi.SSID(i).c_str()) > 0) {
        if (WiFi.RSSI(i) > power) {
          power = WiFi.RSSI(i);
          strongest = WiFi.SSID(i);

#if DEBUG
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println();
#endif
  
          delay(10);
        }
      }
    }
  }
}

unsigned char check_ch( const char *txt)
{ 
  unsigned char ret = 0;

  if (
    (strcmp(txt, NODE_0) == 0) || (strcmp(txt, NODE_1) == 0) ||
    (strcmp(txt, NODE_2) == 0) || (strcmp(txt, NODE_3) == 0) ||
    (strcmp(txt, NODE_4) == 0) || (strcmp(txt, NODE_5) == 0) ||
    (strcmp(txt, BASE_SSID) == 0)
    ) {
    ret = 1;
  }
  else {
    ret = 0;
  }

  return ret;

}

void decrypt_node(char *txt)
{
  Udp.beginPacket(broadcast, BROADCAST_PORT);
  
  if (strcmp(txt, MAC_NODE_0) == 0) {
    //Udp.beginPacket(broadcast, NODE0_PORT);
    Udp.write(NODE_0);
    Udp.write(": ");
    Udp.write("9"); 
  }
  else if (strcmp(txt, MAC_NODE_1) == 0) {
    //Udp.beginPacket(broadcast, NODE1_PORT);
    Udp.write(NODE_1);
    Udp.write(": ");
    Udp.write("9");
  }
  else if (strcmp(txt, MAC_NODE_2) == 0) {
    //Udp.beginPacket(broadcast, NODE2_PORT);
    Udp.write(NODE_2);
    Udp.write(": ");
    Udp.write("9");
  }
  else if (strcmp(txt, MAC_NODE_3) == 0) {
    //Udp.beginPacket(broadcast, NODE3_PORT);
    Udp.write(NODE_3);
    Udp.write(": ");
    Udp.write("9");
  }
  else if (strcmp(txt, MAC_NODE_4) == 0) {
    //Udp.beginPacket(broadcast, NODE4_PORT);
    Udp.write(NODE_4);
    Udp.write(": ");
    Udp.write("9");
  }
  else if (strcmp(txt, MAC_NODE_5) == 0) {
    //Udp.beginPacket(broadcast, NODE5_PORT);
    Udp.write(NODE_5);
    Udp.write(": ");
    Udp.write("9");
  }
  else if (strcmp(txt, MAC_NODE_6) == 0) {
    //Udp.beginPacket(broadcast, NODE6_PORT);
    Udp.write(NODE_6);
    Udp.write(": ");
    Udp.write("9");
  }

  Udp.endPacket();
}

unsigned char scan_nodes(void)
{
  unsigned char number_of_nodes;
  struct station_info *stat_info;
  struct ip4_addr *IPaddress;
  char macStr[18] = {0};
  IPAddress address;
  unsigned short adc;
  char ADC_string[5];

#if DEBUG
  Serial.println("Waiting for nodes to find strongest and connect ...");
  Serial.println("Try modem sleep here.");
#endif

  delay(15000);

  number_of_nodes = wifi_softap_get_station_num();
  stat_info = wifi_softap_get_station_info();

  if (stat_info == NULL) {
    
#if DEBUG
    Serial.println("No nodes are connected to me!");
    Serial.println("Should connect to base and send my adc val");
#endif

   }
   else {

    while(stat_info != NULL) {
        
      IPaddress = &stat_info->ip;
      address = IPaddress->addr;

#if DEBUG
      Serial.print(" IP adress is = ");
      Serial.print(address);
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

      decrypt_node(macStr);
      }
   }

   return number_of_nodes;
}

void get_ch_address(const char *txt)
{
  if (strcmp(txt, NODE_0) == 0) {
    ch_address = IP(192, 168, 5, 20);
    port_to_send = NODE0_PORT;
  }
  else if (strcmp(txt, NODE_1) == 0) {
    ch_address = IP(192, 168, 5, 21);
    port_to_send = NODE1_PORT;
  }
  else if (strcmp(txt, NODE_2) == 0) {
    ch_address = IP(192, 168, 5, 22);
    port_to_send = NODE2_PORT;
  }
  else if (strcmp(txt, NODE_3) == 0) {
    ch_address = IP(192, 168, 5, 23);
    port_to_send = NODE3_PORT;
  }
  else if (strcmp(txt, NODE_4) == 0) {
    ch_address = IP(192, 168, 5, 24);
    port_to_send = NODE4_PORT;
  }
  else if (strcmp(txt, NODE_5) == 0) {
    ch_address = IP(192, 168, 5, 25);
    port_to_send = NODE5_PORT;
  }
  else if (strcmp(txt, NODE_6) == 0) {
    ch_address = IP(192, 168, 5, 26);
    port_to_send = NODE6_PORT;
  }
  ch_address.toString();
  Serial.println(ch_address);
}

void wait_for_nodes(unsigned char nodes)
{
  bool received_from_all = false;
  unsigned char i = 0;
  char packetBuffer[255];
  char accumulateBuffer[255];
  unsigned short adc;
  char ADC_string[5];

  accumulateBuffer[0] = '\0';
  
  Udp.begin(BROADCAST_PORT);

#if DEBUG
  Serial.print("I should receive data from ");
  Serial.print(nodes);
  Serial.println(" nodes.");
  Serial.println("Waiting for nodes to send me their data ...");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
#endif

  while(!received_from_all) {

    yield(); // needed or WDT will triger reset.

    int packetSize = Udp.parsePacket();

    if(packetSize) {

      i += 1;
  
#if DEBUG
      Serial.println("Got something from some node!.");
#endif

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
      Serial.print("Received: ");
      Serial.println(packetBuffer);
#endif

      strcat(accumulateBuffer, packetBuffer);
      //strcat(accumulateBuffer, "\n");

      if (i == nodes) {
        received_from_all = true;
      }
      else {
        received_from_all = false;
      }
    
    }
  }

#if DEBUG
  Serial.println("Received from all nodes !");
#endif

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(BASE_SSID, BASE_PASS);

#if DEBUG
  Serial.println("Connecting to base!");
#endif
     
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }

#if DEBUG
  Serial.println("Acumulate from nodes and send ...");
#endif

  adc = read_adc();
  sprintf(ADC_string, "%hu", adc);
  strcat(accumulateBuffer, node_name);
  strcat(accumulateBuffer, ":");
  strcat(accumulateBuffer, ADC_string);

  Udp.beginPacket(base_station, BROADCAST_PORT);
  Udp.write(accumulateBuffer);
  Udp.endPacket();
}

void advertise(unsigned char CH)
{
  bool received = false;
  char packetBuffer[255];
  unsigned short adc;
  char ADC_string[5];
  unsigned char send_time;
  unsigned char number_of_nodes;

  adc = read_adc();
  sprintf(ADC_string, "%hu", adc);
  
  switch(CH) {

    case NODE:
    
      strongest_ch_ssid();

#if DEBUG
      Serial.println("Connecting to strongest valid network ...");
#endif
     
      WiFi.begin(strongest, NODE_PASS);

      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }

#if DEBUG
       Serial.print("Connected to: ");
       Serial.println(WiFi.SSID());
       Serial.print("My IP address is : ");
       Serial.println(WiFi.localIP());
#endif

       get_ch_address(WiFi.SSID().c_str());

       if (strcmp(strongest.c_str(), BASE_SSID) == 0) {

        adc = read_adc();
        sprintf(ADC_string, "%hu", adc);
        
        Udp.beginPacket(base_station, BROADCAST_PORT);
        Udp.write(node_name);
        Udp.write(":");
        Udp.write(ADC_string);
        Udp.endPacket();
       }
       else {
        wait_for_CH();
       }

    // wait for response of CH for when node should
    // send ADC value and sleep.
    break;

    case CLUSTER_HEAD:

      number_of_nodes = scan_nodes();
      if(number_of_nodes > 0) {
        wait_for_nodes(number_of_nodes);
      }
      else {
        WiFi.mode(WIFI_STA);
        WiFi.begin(BASE_SSID, BASE_PASS);

#if DEBUG
        Serial.print("No nodes found! ");
        Serial.println("Connecting to base!");
#endif
     
        while (WiFi.status() != WL_CONNECTED) {
          delay(1000);
        

        adc = read_adc();
        sprintf(ADC_string, "%hu", adc);
      
        Udp.beginPacket(base_station, BROADCAST_PORT);
        Udp.write(node_name);
        Udp.write(":");
        Udp.write(ADC_string);
        Udp.endPacket();
      }
      break;
    }
  }
}

unsigned char wait_for_CH (void)
{
    char packetBuffer[255];
    //unsigned long tick = millis();
    boolean time_out = false;
    boolean received = false;
    unsigned char ret = 0;
    char *ptr;
    char CH_NAME[7];
    char SLEEP_STRING[2];
    unsigned char sleep_time;
    unsigned short adc;
    char ADC_string[5];

    CH_NAME[6] = '\0';

    Udp.begin(BROADCAST_PORT);

#if DEBUG
    Serial.println("Waiting for CH ...");
#endif

    //while (!time_out && !received) 
    while (!received) {

      yield(); // needed or WDT will triger reset.

      int packetSize = Udp.parsePacket();

//      if((millis() - tick) > 10000) {
//        time_out = true;
//      }
  
      if (packetSize) {

#if DEBUG
        Serial.println("Got something from some1.");
#endif

        if(strcmp(Udp.remoteIP().toString().c_str(), ch_address.toString().c_str()) == 0) {

#if DEBUG
        Serial.println("Received from CH I`m connected to!");
        Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                packetSize,
                Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                Udp.destinationIP().toString().c_str(), Udp.localPort(),
                ESP.getFreeHeap());
#endif

        int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        packetBuffer[n] = '\0';
        strncpy(CH_NAME, packetBuffer, 6);
        strncpy(SLEEP_STRING, &packetBuffer[8], 2);

#if DEBUG
        Serial.print("CH_NAME = ");
        Serial.println(CH_NAME);
        Serial.print("SLEEP_STRING = ");
        Serial.println(SLEEP_STRING);
        Serial.println(node_name);
#endif

        if (strcmp(CH_NAME, node_name) == 0) {

#if DEBUG
          Serial.println("CH_NAME = node_name");
#endif

          sleep_time = (unsigned char)strtol(SLEEP_STRING, &ptr, 10);

#if DEBUG
          Serial.print("Got message from CH and i should go to sleep for ");
          Serial.print(sleep_time);
          Serial.println(" seconds.");
#endif    

          received = true;
          ret = sleep_time;
        }
        else {
          received = false;
          ret = 0;
        }
        
      }
     }
    }

#if DEBUG
          Serial.println("In future try modem sleep.");
#endif

          delay(sleep_time * 1000);

#if DEBUG
          Serial.print("Sending packet to CH =  ");
          Serial.println(WiFi.SSID());
          Serial.print(ch_address.toString());
          Serial.print(" port ");
          Serial.println(BROADCAST_PORT);
#endif

          adc = read_adc();
          sprintf(ADC_string, "%hu", adc);

          Udp.beginPacket(ch_address, BROADCAST_PORT);
          Udp.write(node_name);
          Udp.write(":");
          Udp.write(ADC_string);
          Udp.endPacket();

#if DEBUG
          Serial.println("Packet sent, should go deep sleep now ...");
#endif
    
      return ret;
}

void wifi_connect(unsigned char CH)
{ 
  int n;
  String strongest;
  int power;
  const char *node_id;
  char  ReplyBuffer[] = "acknowledged\r\n";
  unsigned short adc;
  char ADC_string[5];

  switch (CH) {
    
    case  NODE:

      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      // wait for CH to create AP.
      delay(3000);
      advertise(CH);

    break;

    case  CLUSTER_HEAD:

      WiFi.mode(WIFI_AP_STA);
      WiFi.disconnect();
      node_id = create_node_id();
      WiFi.softAPConfig(IPAddress(apIP), IPAddress(apIP), subnet);
      WiFi.softAP(node_id, NODE_PASS);
      delay(1000);

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

      advertise(CH);

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
    Serial.println("This is end of round.");
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
