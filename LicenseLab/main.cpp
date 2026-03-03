#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <intrin.h>
#include <functional>
#include <ctime>

using namespace std;

class HardwareInfo {
public:
    static string getDiskSerial() {
        DWORD serialNum = 0;
        if (GetVolumeInformationA("C:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0)) {
            return to_string(serialNum);
        }
        return "UNKNOWN_DISK";
    }

    static string getCpuId() {
        int cpuInfo[4] = { -1 };
        char cpuString[32];
        __cpuid(cpuInfo, 0);
        sprintf_s(cpuString, "%08X%08X%08X", cpuInfo[0], cpuInfo[3], cpuInfo[1]);
        return string(cpuString);
    }

    static string generateMachineID() {
        string raw = getCpuId() + "_" + getDiskSerial();
        size_t hashed = hash<string>{}(raw);
        stringstream ss;
        ss << hex << hashed;
        return ss.str();
    }
};

class LicenseManager {
private:
    const string SALT = "Secret_Lab_Key_Cpp_2024";
    const string LICENSE_FILE = "license.lic";

    string generateSignature(const string& machineID, long long expiryTimestamp) {
        string rawData = machineID + to_string(expiryTimestamp) + SALT;
        size_t hashed = hash<string>{}(rawData);
        stringstream ss;
        ss << hex << hashed;
        return ss.str();
    }

    vector<string> split(const string& s, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(s);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

public:
    string createLicense(string machineID, int daysValid) {
        time_t now = time(0);
        time_t expiry = now + (daysValid * 24 * 60 * 60);
        string signature = generateSignature(machineID, (long long)expiry);
        return to_string((long long)expiry) + "|" + signature;
    }

    void saveLicenseToFile(const string& licenseKey) {
        ofstream outFile(LICENSE_FILE);
        if (outFile.is_open()) {
            outFile << licenseKey;
            outFile.close();
            cout << "[Success] License saved to " << LICENSE_FILE << endl;
        } else {
            cerr << "[Error] Cannot open file." << endl;
        }
    }

    bool verifyLicense(string currentMachineID) {
        ifstream inFile(LICENSE_FILE);
        if (!inFile.is_open()) {
            cout << "[Error] License file not found." << endl;
            return false;
        }

        string line;
        getline(inFile, line);
        inFile.close();

        vector<string> parts = split(line, '|');
        if (parts.size() != 2) return false;

        long long expiryTime = stoll(parts[0]);
        string fileSignature = parts[1];
        string computedSignature = generateSignature(currentMachineID, expiryTime);
        
        if (computedSignature != fileSignature) {
            cout << "[Fail] Invalid License (Wrong MachineID or Tampered File)." << endl;
            return false;
        }

        time_t now = time(0);
        if (now > expiryTime) {
            cout << "[Fail] License Expired." << endl;
            return false;
        }

        cout << "[Success] License is Valid!" << endl;
        return true;
    }
};

int main() {
    string myMachineID = HardwareInfo::generateMachineID();
    cout << "=== LICENSE SYSTEM ===" << endl;
    cout << "Your Machine ID: " << myMachineID << endl << endl;

    LicenseManager mgr;
    int choice;

    cout << "1. Generate License (Admin)" << endl;
    cout << "2. Verify License (User)" << endl;
    cout << "Select: ";
    cin >> choice;

    if (choice == 1) {
        string targetID;
        int days;
        cout << "Enter target MachineID (or 'self'): ";
        cin >> targetID;
        if (targetID == "self") targetID = myMachineID;

        cout << "Days valid: ";
        cin >> days;

        string key = mgr.createLicense(targetID, days);
        cout << "\nGenerated Key:\n" << key << endl;
        mgr.saveLicenseToFile(key);
    }
    else if (choice == 2) {
        if (mgr.verifyLicense(myMachineID)) {
            cout << "\n>>> ACCESS GRANTED <<<" << endl;
        } else {
            cout << "\n>>> ACCESS DENIED <<<" << endl;
        }
    }

    system("pause");
    return 0;
}