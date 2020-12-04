# Node implementation for modified LEACH protocol
 This repo will contain needed files for nodes in modified LEACH protocol.
 
 Nodes will decide on their own if they are cluster head for current round or not.
 Decision will be made based if node was cluster head in any round before, distance to base station
 and threshold. If random number is less that threshold, then node will become cluster head for current round.
 Fictive number of nodes is calculated where signal strength is calculated, as per formula below:
 
 Nf = round( -N * RSSI/100)
 
 where N is true number of nodes in network, RSSI is signal strength towards base station.
 
 Threshold is calculated per formula below:

  T = P/(1 - P * (r % 1/P));
  
  r - current round, P = 1/Nf.
  
  Where P is probability that there will be 1 node as cluster head in Nf rounds, 
  r is current round. Nodes will calculate threshold only if they were not 
  cluster heads in previous rounds. Rounds reset after Nf-1 number of rounds, 
  where node can become cluster head again.