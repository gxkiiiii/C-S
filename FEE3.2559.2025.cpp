#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <string>
using namespace std;

struct Student {
    uint32_t id;
    string name;
    double grade;
};

const uint32_t MAGIC = 0x42494E31; // This magic number represents BIN1-Binary File
const uint8_t VERSION = 1;

//Calculate Checksum Function
uint32_t calculateChecksum(const vector<uint8_t>& data) {
    uint32_t sum = 0;
    for (uint8_t b : data) sum += b;
    return sum;
}

// Code to Save Student Data to File
bool writeGradeFile(const string& filename, const vector<Student>& students) {
    ofstream file(filename, ios::binary);
    if (!file) return false;

    // Write header
    uint32_t magic = MAGIC;
    uint8_t version = VERSION;
    uint32_t count = students.size();

    file.write((char*)&magic, 4);
    file.write((char*)&version, 1);
    file.write((char*)&count, 4);

    // Write students
    for (const auto& s : students) {
        file.write((char*)&s.id, 4);

        uint32_t nameLen = s.name.length();
        file.write((char*)&nameLen, 4);
        file.write(s.name.c_str(), nameLen);

        file.write((char*)&s.grade, 8);
    }


    file.close();

    ifstream inFile(filename, ios::binary);
    inFile.seekg(0, ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, ios::beg);

    vector<uint8_t> buffer(fileSize);
    inFile.read((char*)buffer.data(), fileSize);
    inFile.close();

    uint32_t checksum = calculateChecksum(buffer);

    // Append checksum
    ofstream outFile(filename, ios::binary | ios::app);
    outFile.write((char*)&checksum, 4);
    outFile.close();

    cout << "Saved " << count << " students.\n";
    return true;
}

// Reading and Verifying the file
bool readAndVerifyGradeFile(const string& filename, vector<Student>& students, bool& integrityOk) {
    ifstream file(filename, ios::binary);
    if (!file) return false;

    // Get file size
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);

    // Read entire file
    vector<uint8_t> fileData(fileSize);
    file.read((char*)fileData.data(), fileSize);

    // Separate data and checksum
    size_t dataSize = fileSize - 4;
    vector<uint8_t> data(fileData.begin(), fileData.begin() + dataSize);

    // Obtain the stored checksum
    uint32_t storedChecksum;
    memcpy(&storedChecksum, fileData.data() + dataSize, 4);

    // Verify checksum
    uint32_t calcChecksum = calculateChecksum(data);
    integrityOk = (storedChecksum == calcChecksum);

    // Parse header from data
    size_t offset = 0;

    uint32_t magic;
    memcpy(&magic, &data[offset], 4); offset += 4;

    uint8_t version = data[offset]; offset += 1;

    uint32_t count;
    memcpy(&count, &data[offset], 4); offset += 4;


    if (magic != MAGIC || version != VERSION) return false;
    if (count > 1000) return false; // Sanity check


    students.clear();
    for (uint32_t i = 0; i < count; i++) {
        if (offset >= data.size()) return false;

        Student s;


        memcpy(&s.id, &data[offset], 4); offset += 4;


        uint32_t nameLen;
        memcpy(&nameLen, &data[offset], 4); offset += 4;

        // Read name
        if (offset + nameLen > data.size()) return false;
        s.name.assign(data.begin() + offset, data.begin() + offset + nameLen);
        offset += nameLen;

        // Read grade
        if (offset + 8 > data.size()) return false;
        memcpy(&s.grade, &data[offset], 8); offset += 8;

        students.push_back(s);
    }

    return true;
}

// This is the interactive system
int main() {
    vector<Student> students;
    string filename = "studentfiles.bin";
    int nextId = 1;

    while (true) {
        cout << "\n1. Add Student\n2. Save to file\n3. Verify File Integrity\n4. Exit\nOption?: ";
        int option;
        cin >> option;

        if (option == 1) {
            Student s;
            s.id = nextId++;
            cout << "Name: "; cin >> s.name;
            cout << "Grade: "; cin >> s.grade;
            students.push_back(s);
            cout << "Added ID: " << s.id << "\n";
        }
        else if (option == 2) {
            if (writeGradeFile(filename, students)) {
                cout << "Save successful!\n";
            } else {
                cout << "Save failed!\n";
            }
        }
        else if (option == 3) {
            vector<Student> temp;
            bool integrityOk = false;

            if (readAndVerifyGradeFile(filename, temp, integrityOk)) {
                cout << (integrityOk ? "VALID: File is intact.\n" : "CORRUPTED: Checksum mismatch!\n");
            } else {
                cout << "CORRUPTED FILE.CHECK VALIDITY OF FILE.\n";
            }
        }
        else if (option == 4) {
            cout << "Thank you for using our system!\n";
			return 0;
		}
		else{
			cout<<"Invalid data entered. Please choose one of the available options\n";
            return 0;
			}
        }
    }
