#ifndef HPC_CLUSTER_CONSTANTS_H
#define HPC_CLUSTER_CONSTANTS_H


long WIDTH = 235;
long HEIGHT = 215;
long CHANNELS = 1;

enum Padding{
    ZERO,
    SYMMETRIC
};

enum Padding pad = SYMMETRIC;

#endif  // HPC_CLUSTER_CONSTANTS_H