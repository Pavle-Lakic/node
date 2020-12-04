/** @file node.ino
 *  @brief
 *  
 *  This file represents baremetal implementation 
 *  of node for LEACH protocol
 *   
 *  @author Pavle Lakic
 *  @bug No known bugs
 */
#include "includes.h"

unsigned char round_cnt;
unsigned char ch_enable;
unsigned char CH;
unsigned char mac_address[6];

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);

#if DEBUG
  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.println(WiFi.macAddress());
#endif

  start_count(micros());

  if (mount_fs()) {

#if ROUND_RESET    
    write_fs(0, 1);
#endif  

    read_fs(&round_cnt, &ch_enable);
  }

  base_signal_strength();

  CH = cluster_head(&round_cnt, &ch_enable);

  if (CH == CLUSTER_HEAD) {
    digitalWrite(LED_BUILTIN, LOW); // this will make LED go ON on ESP-12E.
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
    }

  wifi_connect(CH);
  full_circle(&round_cnt, &ch_enable);
  delay(1000); // need some time so udp packets can be sent.

#if ROUND_RESET    
    write_fs(0, 1);
#endif  

#if DEBUG
  unsigned char rnd_cnt;
  unsigned char ch_en;
  read_fs(&rnd_cnt, &ch_en);
#endif
  
  sleeping_time();

}
void loop() 
{

}
