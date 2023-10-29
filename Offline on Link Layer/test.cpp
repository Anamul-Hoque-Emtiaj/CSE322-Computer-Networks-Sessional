#include <iostream>
#include <windows.h>

int main() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED); // Set text color to red
    std::cout << "This is red text." << std::endl;
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN); // Set text color to green
    std::cout << "This is green text." << std::endl;
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE); // Set text color to cyan
    std::cout << "This is cyan text." << std::endl;


    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
    std::cout << "This is default text color." << std::endl; // Reset text color to default

    return 0;
}
