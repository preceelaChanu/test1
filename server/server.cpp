#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <filesystem>
#include "seal/seal.h"
#include "json.hpp"

using namespace std;
using namespace seal;
using json = nlohmann::json;
namespace fs = std::filesystem;

int main() {
    cout << "=== Analytics Server ===" << endl;

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
    int num_clients = config["num_clients"];
    string public_key_file = config["public_key_file"];
    string secret_key_file = config["secret_key_file"];
    string relin_keys_file = config["relin_keys_file"];
    string data_path_prefix = config["data_path_prefix"];

    cout << "Configuration: " << num_clients << " clients expected" << endl;

    // Setup CKKS parameters
    EncryptionParameters parms(scheme_type::ckks);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, {60, 40, 40, 60}));

    SEALContext context(parms);

    // Load keys
    cout << "\nLoading keys..." << endl;

    ifstream pk_stream(public_key_file, ios::binary);
    PublicKey public_key;
    public_key.load(context, pk_stream);
    pk_stream.close();

    ifstream sk_stream(secret_key_file, ios::binary);
    SecretKey secret_key;
    secret_key.load(context, sk_stream);
    sk_stream.close();

    ifstream rk_stream(relin_keys_file, ios::binary);
    RelinKeys relin_keys;
    relin_keys.load(context, rk_stream);
    rk_stream.close();

    cout << "Keys loaded successfully." << endl;

    // Create evaluator, encoder, and decryptor
    Evaluator evaluator(context);
    CKKSEncoder encoder(context);
    Decryptor decryptor(context, secret_key);

    // Load encrypted data from all clients
    cout << "\nLoading encrypted data from clients..." << endl;
    vector<Ciphertext> client_data;

    auto load_start = chrono::high_resolution_clock::now();

    for (int i = 1; i <= num_clients; i++) {
        string filename = data_path_prefix + to_string(i) + ".seal";

        if (!fs::exists(filename)) {
            cerr << "Warning: " << filename << " not found. Skipping client " << i << endl;
            continue;
        }

        ifstream ct_stream(filename, ios::binary);
        Ciphertext ct;
        ct.load(context, ct_stream);
        ct_stream.close();

        client_data.push_back(ct);
        cout << "  Loaded data from client " << i << endl;
    }

    auto load_end = chrono::high_resolution_clock::now();
    auto load_duration = chrono::duration_cast<chrono::milliseconds>(load_end - load_start);

    cout << "[METRIC] Deserialization Time (all clients): " << load_duration.count() << " ms" << endl;

    if (client_data.empty()) {
        cerr << "Error: No client data loaded!" << endl;
        return 1;
    }

    // Perform homomorphic aggregation (sum)
    cout << "\nPerforming homomorphic aggregation..." << endl;

    auto compute_start = chrono::high_resolution_clock::now();

    Ciphertext result = client_data[0];
    for (size_t i = 1; i < client_data.size(); i++) {
        evaluator.add_inplace(result, client_data[i]);
    }

    auto compute_end = chrono::high_resolution_clock::now();
    auto compute_duration = chrono::duration_cast<chrono::microseconds>(compute_end - compute_start);

    cout << "[METRIC] Computation Time (aggregation): " << compute_duration.count() << " μs" << endl;

    // Decrypt result
    cout << "\nDecrypting result..." << endl;

    auto decrypt_start = chrono::high_resolution_clock::now();

    Plaintext plain_result;
    decryptor.decrypt(result, plain_result);

    vector<double> decoded_result;
    encoder.decode(plain_result, decoded_result);

    auto decrypt_end = chrono::high_resolution_clock::now();
    auto decrypt_duration = chrono::duration_cast<chrono::microseconds>(decrypt_end - decrypt_start);

    cout << "[METRIC] Decryption Time: " << decrypt_duration.count() << " μs" << endl;

    // Display result
    double total_consumption = decoded_result[0];
    double average_consumption = total_consumption / client_data.size();

    cout << "\n=== Analytics Results ===" << endl;
    cout << "Total Energy Consumption: " << total_consumption << " kWh" << endl;
    cout << "Average Energy Consumption: " << average_consumption << " kWh" << endl;
    cout << "Number of Clients: " << client_data.size() << endl;

    cout << "\n=== Server Analytics Complete ===" << endl;

    return 0;
}
