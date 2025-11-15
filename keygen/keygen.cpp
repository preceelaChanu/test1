#include <iostream>
#include <fstream>
#include <chrono>
#include "seal/seal.h"
#include "json.hpp"

using namespace std;
using namespace seal;
using json = nlohmann::json;

int main() {
    cout << "=== Key Generation Center (KGC) ===" << endl;

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
    string secret_key_file = config["secret_key_file"];
    string relin_keys_file = config["relin_keys_file"];

    cout << "Configuration loaded:" << endl;
    cout << "  Polynomial Modulus Degree: " << poly_modulus_degree << endl;
    cout << "  CKKS Scale Bits: " << scale_bits << endl;

    // Setup CKKS parameters
    EncryptionParameters parms(scheme_type::ckks);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, {60, 40, 40, 60}));

    SEALContext context(parms);

    // Validate parameters
    if (!context.parameters_set()) {
        cerr << "Error: SEAL parameters are invalid!" << endl;
        return 1;
    }

    cout << "\nSEAL context created successfully." << endl;
    cout << "Security level: " << context.security_level() << " bits" << endl;

    // Generate keys
    auto start = chrono::high_resolution_clock::now();

    KeyGenerator keygen(context);
    SecretKey secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "\n[METRIC] Key Generation Time: " << duration.count() << " ms" << endl;

    // Save keys to files
    cout << "\nSaving keys..." << endl;

    ofstream pk_stream(public_key_file, ios::binary);
    public_key.save(pk_stream);
    pk_stream.close();
    cout << "  Public key saved to: " << public_key_file << endl;

    ofstream sk_stream(secret_key_file, ios::binary);
    secret_key.save(sk_stream);
    sk_stream.close();
    cout << "  Secret key saved to: " << secret_key_file << endl;

    ofstream rk_stream(relin_keys_file, ios::binary);
    relin_keys.save(rk_stream);
    rk_stream.close();
    cout << "  Relinearization keys saved to: " << relin_keys_file << endl;

    cout << "\n=== Key Generation Complete ===" << endl;

    return 0;
}
