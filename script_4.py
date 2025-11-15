
# Create client.cpp
client_code = """#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include "seal/seal.h"
#include "json.hpp"

using namespace std;
using namespace seal;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <client_id>" << endl;
        return 1;
    }
    
    int client_id = atoi(argv[1]);
    cout << "=== Smart Meter Client " << client_id << " ===" << endl;
    
    // Load configuration
    ifstream config_file("config.json");
    if (!config_file.is_open()) {
        cerr << "Error: Could not open config.json" << endl;
        return 1;
    }
    
    json config;
    config_file >> config;
    config_file.close();
    
    size_t poly_modulus_degree = config["poly_modulus_degree"];
    int scale_bits = config["ckks_scale_bits"];
    string public_key_file = config["public_key_file"];
    string data_path_prefix = config["data_path_prefix"];
    
    // Setup CKKS parameters (must match keygen)
    EncryptionParameters parms(scheme_type::ckks);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, {60, 40, 40, 60}));
    
    SEALContext context(parms);
    
    // Load public key
    cout << "Loading public key..." << endl;
    ifstream pk_stream(public_key_file, ios::binary);
    if (!pk_stream.is_open()) {
        cerr << "Error: Could not open public key file" << endl;
        return 1;
    }
    
    PublicKey public_key;
    public_key.load(context, pk_stream);
    pk_stream.close();
    
    // Create encryptor and encoder
    Encryptor encryptor(context, public_key);
    CKKSEncoder encoder(context);
    
    // Simulate smart meter reading (kWh value)
    // In a real scenario, this would come from the London Smart Meter dataset
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.5, 5.0);  // Typical hourly consumption: 0.5 - 5.0 kWh
    
    double meter_reading = dis(gen);
    cout << "Meter Reading: " << meter_reading << " kWh" << endl;
    
    // Encode and encrypt
    double scale = pow(2.0, scale_bits);
    Plaintext plain;
    encoder.encode(meter_reading, scale, plain);
    
    auto enc_start = chrono::high_resolution_clock::now();
    
    Ciphertext encrypted;
    encryptor.encrypt(plain, encrypted);
    
    auto enc_end = chrono::high_resolution_clock::now();
    auto enc_duration = chrono::duration_cast<chrono::microseconds>(enc_end - enc_start);
    
    cout << "[METRIC] Encryption Time: " << enc_duration.count() << " μs" << endl;
    
    // Serialize and save
    string output_file = data_path_prefix + to_string(client_id) + ".seal";
    
    auto save_start = chrono::high_resolution_clock::now();
    
    ofstream ct_stream(output_file, ios::binary);
    encrypted.save(ct_stream);
    ct_stream.close();
    
    auto save_end = chrono::high_resolution_clock::now();
    auto save_duration = chrono::duration_cast<chrono::microseconds>(save_end - save_start);
    
    // Get ciphertext size
    stringstream ss;
    encrypted.save(ss);
    size_t ct_size = ss.str().size();
    
    cout << "[METRIC] Serialization Time: " << save_duration.count() << " μs" << endl;
    cout << "[METRIC] Ciphertext Size: " << ct_size << " bytes" << endl;
    cout << "Encrypted data saved to: " << output_file << endl;
    
    cout << "=== Client " << client_id << " Complete ===" << endl;
    
    return 0;
}
"""

with open('smart_grid_benchmark/client/client.cpp', 'w') as f:
    f.write(client_code)

print("Created client/client.cpp")
