import struct
import random

# Network dimensions
L1_SIZE = 64

# Set seed for reproducibility (optional - remove for truly random)
random.seed(42)

def random_int16():
    """Generate a random int16_t value between -32768 and 32767"""
    return random.randint(-32768, 32767)

with open('wilted-net-test.bin', 'wb') as f:
    # Input weights: 2 * 6 * 64 * L1_SIZE int16_t values
    for i in range(2):
        for j in range(6):
            for k in range(64):
                for l in range(L1_SIZE):
                    f.write(struct.pack('<h', random_int16()))

    # Input biases: L1_SIZE int16_t values
    for i in range(L1_SIZE):
        f.write(struct.pack('<h', random_int16()))

    # Output weights (stm): L1_SIZE int16_t values
    for i in range(L1_SIZE):
        f.write(struct.pack('<h', random_int16()))

    # Output weights (nstm): L1_SIZE int16_t values
    for i in range(L1_SIZE):
        f.write(struct.pack('<h', random_int16()))

    # Output bias: 1 int16_t value
    f.write(struct.pack('<h', random_int16()))

print("Generated wilted-net-test.bin with random weights")
