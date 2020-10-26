# node
 This repo will contain needed files for nodes in LEACH protocol.
 
 Nodes will decide on their own if they are cluster head for current round or not.
 Decision will be made based if node was cluster head before and random number between 0 and 1.
 If random number is less that threshold, then node will become cluster head for current round.
 Threshold is calculated per formula below:

  T = P/(1 - P*(r % 1/P));
  
  Where P is determined apriori, r is current round. Nodes will calculate
  threshold only if they were not cluster heads in previous rounds.