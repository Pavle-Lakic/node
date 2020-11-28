/** @file includes.h
 *  @brief Functions which node of LEACH protocol will use.
 *
 *  This file contains other necessary includes,
 *	structures, and defines needed for complete
 *	node functionality.
 *
 *  @author Pavle Lakic
 *  @bug No known bugs.
 */
 
#ifndef INCLUDES_H_
#define INCLUDES_H_

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266TrueRandom.h>
#include <stdlib.h>
#include <Ticker.h>
#include <FS.h>

/** Debug flag, set to 1 to see debug messages.*/
#define DEBUG			                1

/** With this flag enabled, round = 0, ch_enable = 1 will
  * be written to filesystem. Useful to set manually
  * when all nodes must be in initial state.*/
#define ROUND_RESET               0

/**Maximum number of nodes in network*/
#define NUMBER_OF_NODES           5

/** String length for round to be writen to filesystem.*/
#define ROUND_NUMBER_LENGTH       2

/** String length for if node was cluster head to be written to filesystem.*/
#define CH_NUMBER_LENGTH          2

/** This will be password for all nodes.*/
#define NODE_PASS                 "teorijazavere"

/** This will be password for base station.*/
#define BASE_PASS                 "teorijazavere"

/** Base station SSID.*/
#define BASE_SSID                 "5G server"

/** This is flag if node is cluster head for current round.*/
#define CLUSTER_HEAD              1

/** This is flag if node is not cluster head for current round.*/
#define NODE                      0

/** Node 0 AP name.*/
#define NODE_0                    "Node 0"

/** Node 1 AP name.*/
#define NODE_1                    "Node 1"

/** Node 2 AP name.*/
#define NODE_2                    "Node 2"

/** Node 3 AP name.*/
#define NODE_3                    "Node 3"

/** Node 4 AP name.*/
#define NODE_4                    "Node 4"

/** Node 5 AP name.*/
#define NODE_5                    "Node 5"

/** Node 6 AP name.*/
#define NODE_6                    "Node 6"

/** MAC address of node which name will be Node 0.*/
#define MAC_NODE_0                "A4:CF:12:C3:04:FD"

/** MAC address of node which name will be Node 1*/
#define MAC_NODE_1                "FC:F5:C4:AC:36:86"

/** MAC address of node which name will be Node 2.*/
#define MAC_NODE_2                "C8:2B:96:29:F1:03"

/** MAC address of node which name will be Node 3.*/
#define MAC_NODE_3                "A4:CF:12:C3:0A:9B"

/** MAC address of node which name will be Node 4.*/
#define MAC_NODE_4                "C8:2B:96:29:FB:9E"

/** MAC address of node which name will be Node 5.*/
#define MAC_NODE_5                "A4:CF:12:C3:0F:77"

/** MAC address of node which name will be Node 6.*/
#define MAC_NODE_6                "A4:CF:12:C3:08:1C"

/**WiFi channel.*/
#define WIFI_CHANNEL              1

/** Maximum possible number of connected devices to node.*/
#define MAX_CONNECTED             7

/** Analog input pin.*/
#define ADC_PIN                   A0

/** Time for how long will CH/node wait response for node/CH in miliseconds.*/
#define WAIT_FOR_NODES_TIMEOUT    16000

/** Time for how long will node wait for cluster heads to set up their AP`s.
 *  Can be reduced even more with condition of turning on all nodes in same time.
 */
#define WAIT_FOR_CHS_TIMEOUT      4000

/** Port where broadcast data will be sent*/
#define BROADCAST_PORT            2000

/** Time from restart to restart of microcontrolers.*/
#define PERIOD                    60000000

/** Node numbers.*/
typedef enum
{
  NODE0,
  NODE1,
  NODE2,
  NODE3,
  NODE4,
  NODE5,
  NODE6
} nodes;

/**  
 * @brief Check if received message from broadcast port contains
 * value for this node 
 * @param String of packet received.
 * @param Length of packet.
 * @return True if it contains message for node, else false.
 * 
 */
bool check_if_for_me(char *txt, unsigned short l);

/**  
 * @brief Calculates number of nodes present in message. 
 * @param String of packet received.
 * @return number of nodes in message.
 * 
 */
unsigned char message_length(char *txt);

/**  
 * @brief Stores value when program started executing.
 * @param Value of that time.
 * @return none.
 * 
 */
void start_count(unsigned long a);

/**  
 * @brief Calculates for how long node will be in deep sleep.
 * @param none.
 * @return none.
 * 
 */
void sleeping_time(void);

/**  
 * @brief Time for how long node/CH will sleep before
 * beggining new cycle. This function should be called
 * after each successful transim of UDP message to base
 * station.
 * @param Current time in miliseconds.
 * @return Time for how long it will sleep in microseconds.
 * 
 */
unsigned long time_to_sleep(unsigned long a);

/**  
 * @brief Checkes if SSID is valid or not. Valid SSID`s
 * are only ones defining network of nodes (NODE_0, NODE_1 ...)
 * and with BASE_SSID. Rest are declared as not valid, and will
 * not participate in LEACH protocol
 * @param SSID to be checked.
 * @return 0 - invalid network, 1 - valid network.
 * 
 */
unsigned char check_ch( const char * txt);


/**  
 * @brief Node blockes here, waiting for CH`s UDP message
 * containing nodes name, and value for how long node will
 * wait before sending back its ADC value to the CH.
 * All nodes listen to BROADCAST_PORT, but will only take
 * the messages declared to them.
 * @param none.
 * @return none.
 * 
 */
void wait_for_CH (void);

/**  
 * @brief Scan`s for WiFi SSID`s, findes the strongest
 * among them, and if valid copies its name to global
 * variable.
 * @param none.
 * @return none.
 * 
 */
void strongest_ch_ssid(void);

/**  
 * @brief Scanes for nodes connected to CH.
 * @param none.
 * @return Number of nodes connected to CH.
 * 
 */
unsigned char scan_nodes(void);

/**  
 * @brief Compares names of nodes and their MAC addresses.
 * Also sends UDP message to node, which contains node name
 * and time for how long it will sleep.
 * @param MAC address of node connected to CH.
 * @return none.
 * 
 */
void decrypt_node(char *txt);

/**  
 * @brief This function call will get IP address of CH
 * which node is connected to.
 * @param Name of cluster head which node is connected to.
 * @return none.
 * 
 */
void get_ch_address(const char *txt);

/**  
 *  @brief Will print connected networks with IP`s and MAC`s
 *  to this node, if node is in AP mode. Also needed to uncomment
 *  it
 * @param none.
 * @return none.
 * 
 */
void print_connected(void);
/**
 * @brief In case of node it will send udp package to connected,
 * cluster head to infrom it that it will be part of its cluster. 
 * In case of cluster head, it will wait for some time for some
 * nodes to connect to it.
 * @param CH Flag if node is cluster head or not.
 * @return none.
 */
void advertise(unsigned char CH);

/**
 * @brief Creates string which will be used for ssid. Name of network
 * will be "Node x" where x is parameter passed to this function.
 * @param txt MY_NODE should be passed here for each node.
 * @return none.
 */
const char* create_node_id (void);

/**
 * @brief Connects to closest node if not cluster head, else becomes access point.
 * for other nodes.
 * @param CH Indicates if this node is cluster head for current round.
 * @return none.
 */
void wifi_connect(unsigned char CH);

/**
 * @brief Resets values of round and ch_enable if
 * all nodes were cluster heads, so full new circle can begin.
 * @param round_cnt Current round number.
 * @param ch_enable Flag if node was CH in previous rounds.
 * @return none.
 */
void full_circle(unsigned char *round_cnt, unsigned char *ch_enable);

/**
 * @brief Decides if node will be cluster head or not.
 * @param round_cnt Current round number.
 * @param ch_enable Flag if node was CH in previous rounds.
 * Also writes 0 for ch_enable if node was declared as CH
 * in this round.
 * @return 1 if node is CH for this round, 0 if not.
 */
unsigned char cluster_head(unsigned char *round_cnt, unsigned char *ch_enable);

/**
 * @brief Mounts filesystem.
 * Mounts filesystem, so values of round and cluster 
 * head enable can be written to filename file.
 * @param none.
 * @return true if mounted successfully.
 */
bool mount_fs(void);

/**
 * @brief Generates threshold.
 *  This function is called only if current node was
 *  not CH for the current round.
 * @param P Probability that node will become CH.
 * @param r Current round.
 * @return Returns threshold for current round.
 */
float calculate_threshold(float P, unsigned char r);

/**
 * @brief Generates random number between 0 and 1.
 * @param none.
 * @return random number between 0 and 1.
 */
float random_number(void);

/**
 *	@brief Reads round and if node was cluster head from
 *	filesystem.
 *	@param round_cnt Address of round is passed as pointer.
 *	@param ch Address of CH indicator is passed as pointer.
 * 	@return none.
 */
void read_fs(unsigned char *round_cnt, unsigned char *ch);

/**
 *	@brief Writes round, and indicator that node was cluster head to filesystem.
 *	@param round_cnt Value of round to be written.
 *	@param ch Value of CH indicator.
 * 	@return none.
 */
void write_fs(int round_cnt, int ch);

/**
 *  @brief Reads value of ADC.
 *  @param none.
 *  @return Value of ADC (10 bit).
 */
unsigned short read_adc(void);

#endif // INCLUDES_H_
