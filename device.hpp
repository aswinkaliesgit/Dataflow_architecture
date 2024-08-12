#ifndef DEVICE_H
#define DEVICE_H
// The device here is a grid of 512*512
// Each PE has a size of 2048 BYTES and
// Float data that can be stored in a PE is 512 (512 * 4 = 2048)

#define PE_ROWS 512
#define PE_COLUMNS 512
#define SIZE_PER_PE 2048

#endif // DEVICE_H
