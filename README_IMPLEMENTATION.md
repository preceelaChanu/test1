# Privacy-Preserving Smart Grid Analytics Framework - Implementation

This implementation provides a complete working system based on the specifications in README.md.

## Files Created

### Core Source Files
- **keygen/keygen.cpp**: Key Generation Center implementation
- **client/client.cpp**: Smart Meter Client simulator
- **server/server.cpp**: Analytics Server implementation

### Configuration & Build Files
- **CMakeLists.txt**: CMake build configuration
- **config.json**: System configuration parameters
- **run_test.sh**: Automated test harness script

### Directory Structure
```
smart_grid_benchmark/
├── CMakeLists.txt
├── config.json
├── run_test.sh
├── include/                 # Place json.hpp here
├── keygen/
│   └── keygen.cpp
├── client/
│   └── client.cpp
├── server/
│   └── server.cpp
├── keys/                    # Generated keys will be stored here
└── data/                    # Encrypted client data will be stored here
```

## Implementation Details

### 1. Key Generation (keygen.cpp)
- Loads configuration from config.json
- Creates CKKS encryption parameters with security level
- Generates public key, secret key, and relinearization keys
- Saves all keys to the keys/ directory
- Reports key generation time metrics

### 2. Smart Meter Client (client.cpp)
- Accepts client ID as command-line argument
- Simulates energy consumption readings (0.5-5.0 kWh)
- Loads public key for encryption
- Encodes real number using CKKS encoder
- Encrypts the value and saves ciphertext
- Reports encryption time, serialization time, and ciphertext size

### 3. Analytics Server (server.cpp)
- Loads all cryptographic keys
- Reads encrypted data from all clients
- Performs homomorphic aggregation (addition)
- Decrypts the final result
- Computes total and average consumption
- Reports deserialization, computation, and decryption times

### 4. Configuration (config.json)
All parameters are configurable without recompilation:
- **poly_modulus_degree**: Security level (8192, 16384, or 32768)
- **ckks_scale_bits**: Precision of real numbers (default: 40)
- **num_clients**: Number of smart meters to simulate

## Setup Instructions

### Prerequisites
Ensure your Ubuntu system has the required tools:
```bash
sudo apt update
sudo apt install build-essential g++ cmake git jq
```

### Step 1: Install Microsoft SEAL v4.0.0
```bash
cd smart_grid_benchmark
git clone https://github.com/microsoft/SEAL.git
cd SEAL
git checkout v4.0.0
cmake -S . -B build -DSEAL_THROW_ON_TRANSPARENT_CIPHERTEXT=OFF
cmake --build build
sudo cmake --install build
cd ..
```

### Step 2: Install JSON Parser
```bash
mkdir -p include
wget -O include/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
```

### Step 3: Build the Project
```bash
mkdir build
cd build
cmake ..
make
cd ..
```

## Usage

### Option A: Automated Test (Recommended)
Run the complete workflow with one command:
```bash
chmod +x run_test.sh
./run_test.sh
```

This script will:
1. Generate cryptographic keys
2. Simulate N clients (from config.json)
3. Run the analytics server
4. Display all performance metrics

### Option B: Manual Execution

1. **Generate Keys**:
```bash
./build/keygen
```

2. **Run Individual Clients**:
```bash
./build/client 1
./build/client 2
./build/client 3
# ... up to num_clients
```

3. **Run Analytics Server**:
```bash
./build/server
```

## Performance Metrics

The system automatically reports:

### Key Generation
- Key generation time (ms)

### Client Metrics (per client)
- Encryption time (μs)
- Serialization time (μs)
- Ciphertext size (bytes)

### Server Metrics
- Deserialization time (ms) - loading all client data
- Computation time (μs) - homomorphic aggregation
- Decryption time (μs)

### Analytics Results
- Total energy consumption (kWh)
- Average energy consumption (kWh)
- Number of clients processed

## Customization

### Changing Security Level
Edit config.json:
```json
{
  "poly_modulus_degree": 16384,  // Higher = more secure, slower
  ...
}
```

### Changing Number of Clients
Edit config.json:
```json
{
  "num_clients": 50,  // Simulate 50 smart meters
  ...
}
```

### Integration with Real Data
To use the London Smart Meter dataset:

1. Download and preprocess the dataset
2. Modify client.cpp to read from CSV files instead of random generation:
```cpp
// Replace the random generation section with:
// double meter_reading = read_from_csv(client_id);
```

## Architecture Highlights

### Security Model
- **Trusted Setup**: Key Generation Center creates keys in secure environment
- **Untrusted Network**: Clients send encrypted data over insecure channels
- **Privacy-Preserving**: Server performs analytics without seeing individual readings
- **Post-Quantum**: CKKS is based on lattice cryptography (quantum-resistant)

### Encryption Scheme
- **CKKS**: Supports approximate arithmetic on encrypted real numbers
- **Leveled**: Multiple levels for complex computations
- **Relinearization**: Reduces ciphertext size after multiplication

### Performance Considerations
- Polynomial degree affects security, computation time, and ciphertext size
- Scale bits affect numerical precision of results
- Serialization overhead simulates network transmission

## Troubleshooting

### Build Errors
- Ensure SEAL is installed correctly: `pkg-config --modversion seal-4.0`
- Check C++17 support: `g++ --version` (should be 7.0+)

### Runtime Errors
- Verify config.json exists and is valid JSON
- Ensure keys/ and data/ directories exist
- Check file permissions for run_test.sh: `chmod +x run_test.sh`

### Missing Client Data
- Server will skip missing client files with a warning
- Ensure clients completed successfully before running server

## Extension Ideas

1. **Advanced Analytics**: Implement variance, weighted averages
2. **Multiplication**: Use relinearization keys for products
3. **Real Data Integration**: Parse London Smart Meter CSV files
4. **Network Simulation**: Add latency and bandwidth constraints
5. **Batch Processing**: Encode multiple values per ciphertext
6. **Comparison Operations**: Implement approximate comparisons

## References

- Microsoft SEAL Documentation: https://github.com/microsoft/SEAL
- CKKS Paper: Cheon et al., "Homomorphic Encryption for Arithmetic of Approximate Numbers"
- Smart Meters in London Dataset: https://www.kaggle.com/datasets/jeanmidev/smart-meters-in-london
