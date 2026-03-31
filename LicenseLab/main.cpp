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

// Hardware ID ստանալու համար
class HardwareInfo {
public:
    // Ստանում ենք C սկավառակի սերիական համարը
    static string getDiskSerial() {
        DWORD serialNum = 0;
        // GetVolumeInformationA-ն վերադարձնում է տեղեկություն սկավառակի մասին
        if (GetVolumeInformationA("C:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0)) {
            return to_string(serialNum);
        }
        return "UNKNOWN_DISK";
    }

    // Ստանում ենք պրոցեսորի (CPU) եզակի կոդը
    static string getCpuId() {
        int cpuInfo[4] = { -1 };
        char cpuString[32];
        // __cpuid-ն ցածր մակարդակի հրաման է, որը հարցում է անում պրոցեսորին
        __cpuid(cpuInfo, 0);
        // Ստացված տվյալները ձևափոխում ենք տեքստային (Hex) ֆորմատի
        sprintf_s(cpuString, "%08X%08X%08X", cpuInfo[0], cpuInfo[3], cpuInfo[1]);
        return string(cpuString);
    }

    // Միացնում ենք CPU-ի և Սկավառակի տվյալները և սարքում մեկ եզակի ID
    static string generateMachineID() {
        string raw = getCpuId() + "_" + getDiskSerial(); // Միացնում ենք CPU-ի և սկավառակի կոդերը
        size_t hashed = hash<string>{}(raw);                // Սարքում ենք անհասկանալի երկար թիվ (Hash)
        stringstream ss;
        ss << hex << hashed; // Թիվը դարձնում ենք տասնվեցական տեքստ hex
        return ss.str();
    }
};

// Դաս՝ լիցենզիայի ստեղծման և ստուգման համար
class LicenseManager {
private:
    const string SALT = "Secret_Lab_Key_Cpp_2024"; // Գաղտնի բառ՝ անվտանգության համար
    const string LICENSE_FILE = "license.lic";   // Ֆայլի անունը

    // Ստեղծում է թվային ստորագրություն (MachineID + Ժամկետ + Գաղտնի բառ)
    string generateSignature(const string& machineID, long long expiryTimestamp) {
        string rawData = machineID + to_string(expiryTimestamp) + SALT;
        size_t hashed = hash<string>{}(rawData);
        stringstream ss;
        ss << hex << hashed;
        return ss.str();
    }

    // Օժանդակ ֆունկցիա՝ տեքստը մասերի բաժանելու համար (ըստ '|' նշանի)
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
    // Լիցենզիայի ստեղծում (վերադարձնում է "ժամկետ|ստորագրություն")
    string createLicense(string machineID, int daysValid) {
        time_t now = time(0); // Այսօրվա ամսաթիվը
        time_t expiry = now + (daysValid * 24 * 60 * 60); // Ավելացնում ենք օրերը վայրկյաններով
        string signature = generateSignature(machineID, (long long)expiry);
        return to_string((long long)expiry) + "|" + signature; // օրինակ՝ 1725000000|abcd1234 
        //Առաջին մասը ժամկետն է, երկրորդը՝ դրա վավերականությունը հաստատող կոդը։
    }

    // Լիցենզիան պահպանում ենք ֆայլի մեջ
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

    // Լիցենզիայի վավերականության ստուգում
    bool verifyLicense(string currentMachineID) {
        ifstream inFile(LICENSE_FILE);
        if (!inFile.is_open()) {
            cout << "[Error] License file not found." << endl;
            return false;
        }

        string line;
        getline(inFile, line); // Կարդում ենք լիցենզիայի տողը
        inFile.close();

        vector<string> parts = split(line, '|'); // Բաժանում ենք մասերի
        if (parts.size() != 2) return false;

        long long expiryTime = stoll(parts[0]); // Ժամկետը
        string fileSignature = parts[1];        // Ֆայլի ստորագրությունը

        // Հաշվում ենք նոր ստորագրություն ընթացիկ համակարգչի համար
        string computedSignature = generateSignature(currentMachineID, expiryTime);
        
        // Եթե ստորագրությունները չեն համընկնում, ուրեմն ֆայլը փոխված է կամ ID-ն սխալ է
        if (computedSignature != fileSignature) {
            cout << "[Fail] Invalid License (Wrong MachineID or Tampered File)." << endl;
            return false;
        }

        // Ստուգում ենք ժամկետը
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
    // 1. Ստանում ենք այս համակարգչի ID-ն
    string myMachineID = HardwareInfo::generateMachineID();
    cout << "=== LICENSE SYSTEM ===" << endl;
    cout << "Your Machine ID: " << myMachineID << endl << endl;

    LicenseManager mgr;
    int choice;

    cout << "1. Generate License (Admin)" << endl;
    cout << "2. Verify License (User)" << endl;
    cout << "Select: ";
    cin >> choice;

    if (choice == 1) { // Ադմինի մաս (Լիցենզիայի գեներացիա)
        string targetID; //Հարցնում է ID-ն և օրերը։ Ստեղծում է լիցենզիայի տողը։Գրում է license.lic ֆայլի մեջ։
        int days;
        cout << "Enter target MachineID (or 'self'): ";
        cin >> targetID;
        if (targetID == "self") targetID = myMachineID;

        cout << "Days valid: ";
        cin >> days;

        // Ստեղծում և պահպանում ենք լիցենզիան
        string key = mgr.createLicense(targetID, days);
        cout << "\nGenerated Key:\n" << key << endl;
        mgr.saveLicenseToFile(key);
    }
    else if (choice == 2) { // Օգտատիրոջ մաս (Ստուգում)
        if (mgr.verifyLicense(myMachineID)) {
            cout << "\n>>> ACCESS GRANTED <<<" << endl;
        } else {
            cout << "\n>>> ACCESS DENIED <<<" << endl;
        }
    }

    system("pause"); // Էկրանը բաց պահելու համար
    return 0;
}
