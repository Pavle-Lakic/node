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

/** Probability that node will be cluster head for current round.*/
float Prob = 1/NUMBER_OF_NODES;

/** This is the address of base station.*/
const IPAddress base_station(192,168,4,1);

/** This address will be used as broadcast*/
const IPAddress broadcast(192,168,5,255);

/** Subnet mask for local network. */
const IPAddress subnet(255,255,255,0);

/** Gateway address*/
const IPAddress gateway(192,168,5,1);

/**Cluster head address which node is connected to*/
IPAddress ch_address;

/**This node name.*/
char node_name[10];

/** One of ID`s which define node.*/
unsigned char number_of_node;

/** Strongest valid network.*/
String strongest;

/** Access point IP.*/
uint32_t apIP;

/** This variable holds time when cycle started in miliseconds.*/
unsigned long cycle_start;

/** This variable will track fictive number of nodes due to algorithm change.*/
unsigned char modified_node_number = NUMBER_OF_NODES;

/** Holds the value for how long node will wait before sending ADC value.*/
char SLEEP_STRING[10];

/** Needed for UDP functionalities.*/
WiFiUDP Udp;

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

void start_count(unsigned long a)
{
  cycle_start = a;
}

void sleeping_time(void)
{
    unsigned long sleep_time;
  
    sleep_time = PERIOD - (micros() - cycle_start);

#if DEBUG
      Serial.print("Going to sleep for: ");
      Serial.print(sleep_time);
      Serial.println(" microseconds!");
#endif

    ESP.deepSleep(sleep_time);//, WAKE_RF_DEFAULT);
}

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

  node_name[9] = '\0';

  mac_address = WiFi.macAddress();

#if DEBUG
  Serial.println(mac_address);
#endif

  if (mac_address == MAC_NODE_0) {
    node = NODE_0;
    apIP = IP(192, 168, 5, 20);
    strncpy(node_name, NODE_0, sizeof(NODE_0));
    number_of_node = NODE0;
  }
  else if (mac_address == MAC_NODE_1) {
    node = NODE_1;
    apIP = IP(192, 168, 5, 21);
    strncpy(node_name, NODE_1, sizeof(NODE_1));
    number_of_node = NODE1;
  }
  else if (mac_address == MAC_NODE_2) {
    node = NODE_2;
    apIP = IP(192, 168, 5, 22);
    strncpy(node_name, NODE_2, sizeof(NODE_2));
    number_of_node = NODE2;
  }
  else if (mac_address == MAC_NODE_3) {
    node = NODE_3;
    apIP = IP(192, 168, 5, 23);
    strncpy(node_name, NODE_3, sizeof(NODE_3));
    number_of_node = NODE3;
  }
  else if (mac_address == MAC_NODE_4) {
    node = NODE_4;
    apIP = IP(192, 168, 5, 24);
    strncpy(node_name, NODE_4, sizeof(NODE_4));
    number_of_node = NODE4;
  }
  else if (mac_address == MAC_NODE_5) {
    node = NODE_5;
    apIP = IP(192, 168, 5, 25);
    strncpy(node_name, NODE_5, sizeof(NODE_5));
    number_of_node = NODE5;
  }
  else if (mac_address == MAC_NODE_6) {
    node = NODE_6;
    apIP = IP(192, 168, 5, 26);
    strncpy(node_name, NODE_6, sizeof(NODE_6));
    number_of_node = NODE6;
  }
  else {
    node = "New Node!";
  }

  return node;
}

float update_probability(float n)
{
  float p = 10;
  
  p = 1/n;

  return p;
}

bool modify_N(unsigned char p)
{
  bool ret = false;

  modified_node_number = round((NUMBER_OF_NODES * p)/100);

#if DEBUG
  Serial.print("Modified number of nodes = ");
  Serial.println(modified_node_number);
#endif

  if ((modified_node_number < 1) && (modified_node_number >= 0)){
    modified_node_number = 1;
  }

  if (modified_node_number >= 0) {

    Prob = update_probability(modified_node_number);

#if DEBUG
    Serial.print("Updated probability = ");
    Serial.println(Prob, 3);
#endif
    
    ret = true;
  }

#if DEBUG
  Serial.print("Modified number of nodes after change = ");
  Serial.println(modified_node_number);
#endif

  return ret;
}

void base_signal_strength(void)
{
  unsigned char n;
  int power;
  bool changed;
  bool base_found = false;

  n = WiFi.scanNetworks();

  if (n == 0){

#if DEBUG
    Serial.println("Did not find any networks at all!");
    Serial.println("Going to sleep ...");
#endif

    sleeping_time();
  }
  else {
    for (int i = 0; i < n; i++) {
      if (WiFi.SSID(i) == BASE_SSID) {

        base_found = true;
        power = -WiFi.RSSI(i);
        changed = modify_N(power);

#if DEBUG
        Serial.print("Base SSID = ");
        Serial.println(WiFi.SSID(i));
        Serial.print("Signal strength = ");
        Serial.println(WiFi.RSSI(i));
#endif

#if DEBUG
        if (changed == true) {
          Serial.println("Successfully changed modified number of nodes!");  
        }
        else {
          Serial.println("Could not change modified number of nodes!");
        }
#endif

      }
      delay(20);
    }

    if (base_found == false) {

#if DEBUG
      Serial.println("Could not find base!");
#endif
      
      sleeping_time();
    }
    
  }
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
        Serial.print(i);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println();
#endif
  
          delay(20);
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
    (strcmp(txt, NODE_6) == 0) || (strcmp(txt, BASE_SSID) == 0)
    ) {
    ret = 1;
  }
  else {
    ret = 0;
  }

  return ret;

}

unsigned char message_length(char *txt)
{
    unsigned char ret = 0;
    unsigned short i = 0;
    
    while (txt[i] != '\0') {
        if (txt[i] == 'N') {
            ret += 1;
        }
        i += 1;
    }
    
    return ret;
}

bool check_if_for_me(char *txt, unsigned short l)
{
    char sleep_value[20];
    //unsigned short node;
    unsigned short k;
    unsigned short number;
    bool ret = false;

    
  for (int i = 0; i < l; i++) {

    k = 0;
    while (!isdigit(txt[0])) {
        txt += 1;
    }

     number = txt[0] - '0';
     txt += 2;
     
     while (isdigit(txt[0])) {
        sleep_value[k] = txt[0];
        k += 1;
        txt += 1;
     }
     sleep_value[k] = '\0';
     Serial.println(sleep_value);

     if (number == number_of_node) {

      strncpy(SLEEP_STRING, sleep_value, sizeof(sleep_value));
      ret = true;

#if DEBUG
     Serial.println("I`m called");
#endif
     }
     
  }

  return ret;
}

void decrypt_node(char *txt)
{
  //Udp.beginPacket(broadcast, BROADCAST_PORT);

  if (strcmp(txt, MAC_NODE_0) == 0) {
    Udp.write(NODE_0);
    Udp.write(":");
    Udp.write("3 "); 
  }
  else if (strcmp(txt, MAC_NODE_1) == 0) {
    Udp.write(NODE_1);
    Udp.write(":");
    Udp.write("4 ");
  }
  else if (strcmp(txt, MAC_NODE_2) == 0) {
    Udp.write(NODE_2);
    Udp.write(":");
    Udp.write("5 ");
  }
  else if (strcmp(txt, MAC_NODE_3) == 0) {
    Udp.write(NODE_3);
    Udp.write(":");
    Udp.write("6 ");
  }
  else if (strcmp(txt, MAC_NODE_4) == 0) {
    Udp.write(NODE_4);
    Udp.write(":");
    Udp.write("7 ");
  }
  else if (strcmp(txt, MAC_NODE_5) == 0) {
    Udp.write(NODE_5);
    Udp.write(":");
    Udp.write("8 ");
  }
  else if (strcmp(txt, MAC_NODE_6) == 0) {
    Udp.write(NODE_6);
    Udp.write(":");
    Udp.write("9 ");
  }

 // Udp.endPacket();
}

unsigned char scan_nodes(void)
{
  unsigned char number_of_nodes = 0;
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

  delay(WAIT_FOR_NODES_TIMEOUT);

  number_of_nodes = WiFi.softAPgetStationNum();
#if DEBUG
  Serial.print("Number of nodes returned from wifi_softap_get_station_num() = ");
  Serial.println(number_of_nodes);
#endif
  
  stat_info = wifi_softap_get_station_info();

  if (stat_info == NULL) {
    
#if DEBUG
    Serial.println("No nodes are connected to me!");
    Serial.println("Should connect to base and send my adc val");
#endif

   }
   else {

    Udp.beginPacket(broadcast, BROADCAST_PORT);

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

  Udp.endPacket();

  return number_of_nodes;
}

void get_ch_address(const char *txt)
{
  if (strcmp(txt, NODE_0) == 0) {
    ch_address = IP(192, 168, 5, 20);
  }
  else if (strcmp(txt, NODE_1) == 0) {
    ch_address = IP(192, 168, 5, 21);
  }
  else if (strcmp(txt, NODE_2) == 0) {
    ch_address = IP(192, 168, 5, 22);
  }
  else if (strcmp(txt, NODE_3) == 0) {
    ch_address = IP(192, 168, 5, 23);
  }
  else if (strcmp(txt, NODE_4) == 0) {
    ch_address = IP(192, 168, 5, 24);
  }
  else if (strcmp(txt, NODE_5) == 0) {
    ch_address = IP(192, 168, 5, 25);
  }
  else if (strcmp(txt, NODE_6) == 0) {
    ch_address = IP(192, 168, 5, 26);
  }
  ch_address.toString();
  Serial.println(ch_address);
}

void wait_for_nodes(unsigned char nodes)
{
  bool received_from_all = false;
  bool timed_out = false;
  bool timed_out_base = false;
  unsigned char i = 0;
  unsigned short k = 0;
  char packetBuffer[255];
  char accumulateBuffer[255];
  unsigned short adc;
  char ADC_string[5];
  unsigned long timeout;
  unsigned long sleep_time;

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

  timeout = millis();  

  while(!received_from_all && !timed_out) {

    yield(); // needed or WDT will triger reset.

    if((millis() - timeout) > WAIT_FOR_NODES_TIMEOUT) {
      timed_out = true;
    }
    else {
      timed_out = false;
    }

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
      strcat(accumulateBuffer, "\n\r");

      if (i == nodes) {
        received_from_all = true;
      }
      else {
        received_from_all = false;
      }
    
    }
  }

#if DEBUG
  if(timed_out == true) {
    Serial.println("Timed ou while waiting for nodes to send their data, should send just my value");
  }
  else if (received_from_all == true) {
    Serial.println("Received from all nodes! Should accumulate with my adc value and send.");
  }
  else {
    Serial.println("No nodes are connected to me, should just send my value.");    
  }
#endif  

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(BASE_SSID, BASE_PASS);

#if DEBUG
    Serial.println("Connecting to base!");
#endif
     
    while (WiFi.status() != WL_CONNECTED && !timed_out_base) {
      
      //yield();

      if(k == 750) {

/* Careful when uncommenting, time consuming
#if DEBUG
        Serial.println("Timed out while waiting for to connect to base!");
        Serial.println("Going to deep sleep ...");
#endif
*/
        timed_out_base = true;
      }
        delay(20);
        k += 1;
    }

    if (timed_out_base == true) {
      sleeping_time();
    }

    Udp.beginPacket(base_station, BROADCAST_PORT);

    adc = read_adc();
    sprintf(ADC_string, "%hu", adc);
  
    if (received_from_all == true) {
      strcat(accumulateBuffer, node_name);
      strcat(accumulateBuffer, ":");
      strcat(accumulateBuffer, ADC_string);
      Udp.write(accumulateBuffer);
    }
    else {
      Udp.write(node_name);
      Udp.write(":");
      Udp.write(ADC_string);
    }

    Udp.endPacket();
}

void advertise(unsigned char CH)
{
  bool received = false;
  bool timed_out = false;
  char packetBuffer[255];
  unsigned short adc;
  char ADC_string[5];
  unsigned char send_time;
  unsigned char number_of_nodes;
  unsigned long sleep_time;
  unsigned short i = 0;

  adc = read_adc();
  sprintf(ADC_string, "%hu", adc);
  
  switch(CH) {

    case NODE:
    
      strongest_ch_ssid();

#if DEBUG
      Serial.println("Connecting to strongest valid network ...");
#endif
     
      WiFi.begin(strongest, NODE_PASS);

      while (WiFi.status() != WL_CONNECTED && !timed_out) {

        //yield();

        if ( i == 750) {

/* Careful when uncomenting, time consuming
#if DEBUG
          Serial.println("Could not connect to strongest network for 10 seconds!");
         Serial.println("Going to deep sleep ...");
#endif
*/

          timed_out = true;
        }

        delay(20);
        i += 1;
      }

      if (timed_out == true) {
        sleeping_time();
      }

#if DEBUG
       Serial.print("Connected to: ");
       Serial.println(WiFi.SSID());
       Serial.print("My IP address is : ");
       Serial.println(WiFi.localIP());
       Serial.println(strongest.c_str());
#endif

       if (strcmp(strongest.c_str(), BASE_SSID) == 0) {

#if DEBUG
        Serial.println("Base is strongest");
#endif
        adc = read_adc();
        sprintf(ADC_string, "%hu", adc);
        
        Udp.beginPacket(base_station, BROADCAST_PORT);
        Udp.write(node_name);
        Udp.write(":");
        Udp.write(ADC_string);
        Udp.endPacket();
       }
       else {
        get_ch_address(WiFi.SSID().c_str());
        wait_for_CH();
       }
    break;

    case CLUSTER_HEAD:

      number_of_nodes = scan_nodes();
      

#if DEBUG
      Serial.print("Number of nodes returned from scan_nodes = ");
      Serial.println(number_of_nodes);
#endif

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
     
        while (WiFi.status() != WL_CONNECTED && !timed_out) {

          //yield();
          
          if ( i == 750 ) {
/* Careful when uncommenting, time consuming.
#if DEBUG
            Serial.println("Could not connect for 10 seconds to base station!");
            Serial.println("Going to deep sleep ...");
#endif
*/
            timed_out = true;
          }
          delay(20);
          i += 1;
        }

        if (timed_out == true) {
          sleeping_time();
        }

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

void wait_for_CH (void)
{
    char packetBuffer[255];
    unsigned long tick;
    boolean time_out = false;
    boolean received = false;
    char *ptr;
    char CH_NAME[7];
    unsigned char delay_time;
    unsigned short adc;
    char ADC_string[5];
    unsigned short packet_length;
    boolean for_me = false;

    CH_NAME[6] = '\0';
    SLEEP_STRING[9] = '\0';

    Udp.begin(BROADCAST_PORT);

#if DEBUG
    Serial.println("Waiting for CH ...");
#endif

    tick = millis();
 
    while (!time_out && !received) {

      yield(); // needed or WDT will triger reset.

      int packetSize = Udp.parsePacket();

      if((millis() - tick) > WAIT_FOR_NODES_TIMEOUT) {
        
#if DEBUG
      Serial.println("Timed out while waiting for CH!");
#endif

        time_out = true;
      }
  
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

#if DEBUG
        Serial.println("Contents of packet buffer:");
        Serial.println(packetBuffer);
#endif

        //strncpy(CH_NAME, packetBuffer, 6);

        packet_length = message_length(packetBuffer);
        for_me = check_if_for_me(packetBuffer, packet_length);

        if (for_me == true) {

#if DEBUG 
          Serial.println("Message contains for how long i should sleep !");
#endif

          delay_time = (unsigned char)strtol(SLEEP_STRING, &ptr, 10);

#if DEBUG
          Serial.print("Got message from CH and i should send my adc in");
          Serial.print(delay_time);
          Serial.println(" seconds.");
#endif    

          received = true;
        }
        else {
          received = false;
        }
      }
     }
    }

    if (received == true) {

#if DEBUG
          Serial.println("In future try modem sleep.");
#endif

          delay(delay_time * 1000);

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
    }

}

void wifi_connect(unsigned char CH)
{ 
  int n;
  String strongest;
  int power;
  const char *node_id;
  unsigned short adc;
  char ADC_string[5];

  node_id = create_node_id();

  switch (CH) {
    
    case  NODE:

      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      // wait for CH to create AP.
      delay(WAIT_FOR_CHS_TIMEOUT);
      advertise(CH);

    break;

    case  CLUSTER_HEAD:

      WiFi.mode(WIFI_AP_STA);
      WiFi.disconnect();

      WiFi.softAPConfig(IPAddress(apIP), IPAddress(apIP), subnet);
      WiFi.softAP(node_id, NODE_PASS, WIFI_CHANNEL, false, MAX_CONNECTED);

#if DEBUG
      Serial.print("node_id = ");
      Serial.println(node_id);
      Serial.print("AP IP address: ");
      Serial.print(WiFi.softAPIP());
      Serial.println();
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
  if (*round_cnt > modified_node_number - 1)
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
  threshold = calculate_threshold(*round_cnt);

  if ((rnd_nmb < threshold) && (*ch_enable == 1)) {
    *ch_enable = 0;
    ret = 1;
  }
  else {
    ret = 0;
  }

  write_fs(*round_cnt, *ch_enable);

#if DEBUG
  if (ret == 1) {
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

float calculate_threshold(unsigned char r)
{
  float T;

  T = Prob/(1 - Prob*(r % ((unsigned char)round(1/Prob))));

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
