#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <random>
#include <ctime>
#include <windows.h>

using namespace std;
HANDLE hConsole;
string padData(string data, int m)
{
    if(data.size() % m==0)
        return data;
    int paddingLength = m - (data.size() % m);
    return data + string(paddingLength, '~');
}

vector<string> createDataBlock(string paddedData, int m) //m in byte so m number of character in a row
{
    vector<string> dataBlock;
    for (int i = 0; i < paddedData.size(); i += m)
    {
        string row = "";
        for (int j = 0; j < m; ++j)
        {
            row += bitset<8>(paddedData[i + j]).to_string();
        }
        dataBlock.push_back(row);
    }
    return dataBlock;
}
string addHammingCode(string data)
{
    int dataBits = data.size();
    int parityBits = 1;

    // Calculate the number of parity bits needed
    while (dataBits + parityBits + 1 > pow(2, parityBits))
    {
        parityBits++;
    }
    int totalBits = dataBits+parityBits;
    string bits = "";
    int j=0;
    int dataIndex = 0;

    for(int i = 0; i < totalBits; i++)
    {
        if (i == pow(2, j) - 1)
        {
            bits += '0'; // Placeholder for parity bits
            j++;
        }
        else
        {
            bits += data[dataIndex++];
        }
    }
    // Calculate and set the parity bits
    for (int i = 0; i < parityBits; i++)
    {
        int parityBitIndex = pow(2, i) - 1; // Calculate the position of the parity bit
        int parityBit = 0; // Initialize the parity bit as '0'

        // Calculate the parity bit value (XOR of relevant data bits)
        for (int j = parityBitIndex; j < totalBits; j += (parityBitIndex + 1) * 2)
        {
            for (int k = 0; k < parityBitIndex + 1 && j + k < totalBits; k++)
            {
                parityBit ^= (bits[j + k] - '0');
            }
        }

        bits[parityBitIndex] = parityBit + '0'; // Set the calculated parity bit
    }

    j = 0;
    for(int i = 0; i < totalBits; i++)
    {
        if (i == pow(2, j) - 1)
        {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN); // Set text color to green
            cout << bits[i];
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
            j++;
        }
        else
        {
            cout << bits[i];
        }

        if(i==totalBits-1)
            cout<<endl;
    }

    return bits;
}

string serializeColumnMajor(vector<string>& dataBlock)
{
    int numRows = dataBlock.size();
    int numCols = dataBlock[0].size();
    string serializedData = "";

    for (int col = 0; col < numCols; ++col)
    {
        for (int row = 0; row < numRows; ++row)
        {
            serializedData += dataBlock[row][col];
        }
    }

    return serializedData;
}

string calculateCRC(string& data, string& generator)
{
    int dataLength = data.size();
    int generatorLength = generator.size();

    // Pad the data with zeros (equal to generator length - 1)
    string paddedData = data + string(generatorLength - 1, '0');

    // Initialize the remainder with the padded data
    string remainder = paddedData;

    for (int i = 0; i < dataLength; i++)
    {
        if (remainder[i] == '1')
        {
            // Perform XOR operation with the generator polynomial
            for (int j = 0; j < generatorLength && i+j < paddedData.size(); j++)
            {
                remainder[i + j] = (remainder[i + j] == generator[j]) ? '0' : '1';
            }
        }
    }

    // Extract the checksum part
    string checksum = remainder.substr(dataLength);

    return checksum;
}

bool verifyCRC(string receivedFrame, string generator)
{
    int generatorLength = generator.size();
    int dataLength = receivedFrame.size() - generatorLength + 1;

    // Pad the received frame with zeros (equal to generator length - 1)
    string paddedReceivedFrame = receivedFrame + string(generatorLength - 1, '0');
    string remainder = paddedReceivedFrame;

    // Apply CRC only to the data part
    for (int i = 0; i < dataLength; i++)
    {
        if (remainder[i] == '1')
        {
            // Perform XOR operation with the generator polynomial
            for (int j = 0; j < generatorLength; j++)
            {
                remainder[i + j] = (remainder[i + j] == generator[j]) ? '0' : '1';
            }
        }
    }

    // After verifying, if the remainder is all zeros, then the data is error-free
    for (int i = dataLength; i < remainder.size(); i++)
    {
        if (remainder[i] == '1')
        {
            return false; // Error detected
        }
    }

    return true; // No error detected
}


pair<string,string> simulateTransmission(string frame, double p)
{
    string transmittedFrame = "";
    string error = "";
    for (int i=0; i<frame.size(); i++)
    {
        if((1.00*rand()/RAND_MAX) < p)
        {
            if(frame[i]=='0')
                transmittedFrame +='1';
            else
                transmittedFrame += '0';
            error += '1';
        }
        else
        {
            transmittedFrame += frame[i];
            error += '0';
        }

    }
    return make_pair(transmittedFrame,error);
}

pair<vector<string>,vector<string>> deserializeData(string serializedData, int numRows, int numCols, string err)
{
    vector<string> dataBlock(numRows, string(numCols, '0'));
    vector<string> errBlock(numRows, string(numCols, '0'));
    int currentIndex = 0;
    for (int col = 0; col < numCols; col++)
    {
        for (int row = 0; row < numRows; row++)
        {
            errBlock[row][col] = err[currentIndex];
            dataBlock[row][col] = serializedData[currentIndex++];
        }
    }

    return make_pair(dataBlock,errBlock);
}


string decodeHammingCode(string receivedData)
{
    int receivedBits = receivedData.size();

    // Calculate the number of parity bits
    int parityBits = 0;
    while (receivedBits > pow(2, parityBits))
    {
        parityBits++;
    }
    int errorBit = 0;

    // Calculate error bit based on parity bits
    for (int i = 0; i < parityBits; i++)
    {
        int parityBitIndex = pow(2, i) - 1;
        int parityBit = (receivedData[parityBitIndex] - '0'); // Initialize parity bit

        // Calculate the parity bit value (XOR of relevant bits)
        for (int j = parityBitIndex; j < receivedBits; j += (parityBitIndex + 1) * 2)
        {
            for (int k = 0; k < parityBitIndex + 1 && j + k < receivedBits; k++)
            {
                parityBit ^= (receivedData[j + k] - '0');
            }
        }

        // If a parity bit is incorrect, set the corresponding errorBit
        if (parityBit != (receivedData[parityBitIndex] - '0'))
        {
            errorBit += pow(2, i);
        }
    }

    // Correct the error bit if an error was detected
    if (errorBit > 0 && errorBit <= receivedBits)
    {
        receivedData[errorBit - 1] = (receivedData[errorBit - 1] == '0') ? '1' : '0';
    }

    // Remove parity bits from the decoded data
    string finalData = "";
    int j = 0;
    for (int i = 0; i < receivedBits; i++)
    {
        if (i != pow(2, j) - 1)
        {
            finalData += receivedData[i];
        }
        else
        {
            j++;
        }
    }

    return finalData;
}


// Convert binary string to ASCII characters
string binaryToAscii(vector<string> blocks)
{
    string asciiString;
    for (string binaryString: blocks)
    {
        for (size_t i = 0; i < binaryString.size(); i += 8)
        {
            string byte = binaryString.substr(i, 8);
            char asciiChar = static_cast<char>(bitset<8>(byte).to_ulong());
            asciiString += asciiChar;
        }
    }
    return asciiString;
}


int main()
{
    srand(time(0));
    //srand(1);
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    cout<<"enter data string: ";
    string data_string;
    getline(cin, data_string);

    cout<<"enter number of data bytes in a row (m): ";
    int m;
    cin>>m;

    cout<<"enter probability (p): ";
    double p;
    cin>>p;

    cout<<"enter generator polynomial: ";
    string gp;
    cin>>gp;

    string pad_data_string = padData(data_string,m);
    cout<<"\n\ndata string after padding: "<<pad_data_string<<endl;

    vector<string> dataBlock = createDataBlock(pad_data_string, m);
    cout << "\ndata block (ascii code of m characters per row):" <<endl;
    for (string row : dataBlock)
    {
        cout << row << endl;
    }

    vector<string> dataBlockWithHamming;
    cout << "\ndata block after adding check bits:" << endl;
    for (string row : dataBlock)
    {
        dataBlockWithHamming.push_back(addHammingCode(row));
    }

    cout<<"\ndata bits after column-wise serialization:\n";
    string serialized_data_bit = serializeColumnMajor(dataBlockWithHamming);
    cout<<serialized_data_bit<<endl;

    cout<<"\ndata bits after appending CRC checksum <sent frame>\n";
    string checksum = calculateCRC(serialized_data_bit,gp);
    string sentFrame = serialized_data_bit+checksum;
    cout<<serialized_data_bit;
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout<<checksum<<endl;
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);

    cout<<"\nreceived frame:\n";
    pair<string,string> r = simulateTransmission(sentFrame,p);
    string receivedFrame = r.first;
    string err = r.second;
    for(int i=0; i<receivedFrame.size(); i++)
    {
        if(err[i]=='1')
        {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            cout<<receivedFrame[i];
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
        }
        else
            cout<<receivedFrame[i];
    }
    cout<<endl;

    if(!verifyCRC(receivedFrame,gp))
        cout<<"\nresult of CRC checking matching: error detected\n";
    else
        cout<<"\nresult of CRC checking matching: no error detected\n";

    string checksumRemoved = receivedFrame.substr(0,receivedFrame.size()-gp.size()+1);
    cout<<"\ndata block after removing CRC checksum bits:\n";
    int row = dataBlock.size();
    int col = checksumRemoved.size()/dataBlock.size();
    pair<vector<string>,vector<string>> dr = deserializeData(checksumRemoved,row,col,err);
    vector<string> receivedDatablock = dr.first;
    vector<string> receivedErrblock = dr.second;

    for(int i=0; i<row; i++)
    {
        for(int j=0; j<col; j++)
        {
            if(receivedErrblock[i][j]=='1')
            {
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                cout<<receivedDatablock[i][j];
                SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
            }
            else
                cout<<receivedDatablock[i][j];
        }
        cout<<endl;
    }

    cout<<"\ndata block after removing check bits:\n";
    vector<string> decodedBlock;
    for (string row : receivedDatablock)
    {
        string r = decodeHammingCode(row);
        decodedBlock.push_back(r);
        cout << r << endl;
    }

    string outputFrame = binaryToAscii(decodedBlock);
    cout<<"\noutput frame: "<<outputFrame<<endl;
}
