import torch
from torch import nn
import numpy as np
import sys

INPUT_CHANNELS = 3
OUTPUT_CHANNELS = 64
HEIGHT = 224
WIDTH = 224
KERNEL_SIZE = 3
ONE_BYTE = 8
OFFSET = 2
PADDING = 0
STRIDE = 1

if __name__ == "__main__":
    # fill input with random numbers
    input_npy = [i + 1 for i in range(HEIGHT * WIDTH * INPUT_CHANNELS)]
    input_npy = np.array(input_npy)
    input_npy = input_npy.reshape(1, INPUT_CHANNELS, HEIGHT, WIDTH)
    input_npy = input_npy.astype(np.float32)
    input_tensor = torch.from_numpy(input_npy)

    # add padding to the input
    input_tensor = torch.nn.functional.pad(
        input_tensor, (1, 1, 1, 1), mode="constant", value=0
    )
    print("input shape after padding : ", input_tensor.shape)

    # fill weight with random numbers
    weight_npy = [
        i + 1
        for i in range(OUTPUT_CHANNELS * INPUT_CHANNELS * KERNEL_SIZE * KERNEL_SIZE)
    ]
    weight_npy = np.array(weight_npy)
    weight_npy = weight_npy.reshape(
        OUTPUT_CHANNELS, INPUT_CHANNELS, KERNEL_SIZE, KERNEL_SIZE
    )
    weight_npy = weight_npy.astype(np.float32)
    weight_tensor = torch.from_numpy(weight_npy)
    print("weight shape : ", weight_tensor.shape)
    # define the convolution layer
    conv = nn.Conv2d(
        OUTPUT_CHANNELS, 3, kernel_size=KERNEL_SIZE, padding=0, bias=False, stride=1
    )
    conv.weight.data = weight_tensor

    output = conv(input_tensor)
    print("output shape : ", output.shape)
    np.save("outputs/py_conv_output.npy", output.detach().numpy())
    print("outputs dumped to outputs/py_conv_output.npy")