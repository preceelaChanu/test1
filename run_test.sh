#!/bin/bash

echo "=========================================="
echo "Smart Grid Benchmark Test Harness"
echo "=========================================="
echo ""

# Check if config.json exists
if [ ! -f config.json ]; then
    echo "Error: config.json not found!"
    exit 1
fi

# Read number of clients from config
NUM_CLIENTS=$(jq -r '.num_clients' config.json)
echo "Configuration: Running test with $NUM_CLIENTS clients"
echo ""

# Clean up old data
echo "Cleaning up old data files..."
rm -f data/ct_client_*.seal
echo ""

# Step 1: Generate Keys
echo "=========================================="
echo "Step 1: Generating Cryptographic Keys"
echo "=========================================="
./build/keygen
if [ $? -ne 0 ]; then
    echo "Error: Key generation failed!"
    exit 1
fi
echo ""

# Step 2: Run Clients
echo "=========================================="
echo "Step 2: Simulating Smart Meter Clients"
echo "=========================================="
for (( i=1; i<=$NUM_CLIENTS; i++ ))
do
    echo ""
    echo "--- Running Client $i/$NUM_CLIENTS ---"
    ./build/client $i
    if [ $? -ne 0 ]; then
        echo "Warning: Client $i failed!"
    fi
done
echo ""

# Step 3: Run Server
echo "=========================================="
echo "Step 3: Running Analytics Server"
echo "=========================================="
./build/server
if [ $? -ne 0 ]; then
    echo "Error: Server execution failed!"
    exit 1
fi
echo ""

echo "=========================================="
echo "Test Harness Complete!"
echo "=========================================="
