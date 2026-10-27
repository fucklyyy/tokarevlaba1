#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

struct Pipe
{
    string name;    
    double length;  
    int diameter;  
    bool inRepair;  
};

struct CompressorStation
{
    string name;         
    int workshopCount;    
    int workshopWorking;  
    int stationClass;     
};



string readLine()
{
    string line;
    if (!getline(cin, line))
    {
        cout << "\nВвод закрыт. Выход.\n";
        exit(0);
    }
    return line;
}

bool isBlank(const string& text)
{
    for (char c : text)
        if (!isspace(static_cast<unsigned char>(c)))
            return false;
    return true;
}


template <typename T>
bool parseNumber(const string& text, T& value)
{
    istringstream stream(text);
    string rest;
    return (stream >> value) && !(stream >> rest);
}

template <typename T>
T readNumber(const string& prompt, T minValue, T maxValue)
{
    while (true)
    {
        cout << prompt;
        T value;
        if (!parseNumber(readLine(), value))
            cout << "Ошибка: введите число.\n";
        else if (value < minValue || value > maxValue)
            cout << "Ошибка: значение должно быть от " << minValue
                 << " до " << maxValue << ".\n";
        else
            return value;
    }
}

string readName(const string& prompt)
{
    while (true)
    {
        cout << prompt;
        string text = readLine();
        if (!isBlank(text))
            return text;
        cout << "Ошибка: название не может быть пустым.\n";
    }
}

//труба 

Pipe readPipe()
{
    Pipe pipe;
    pipe.name = readName("Название (километровая отметка): ");
    pipe.length = readNumber<double>("Длина (км): ", 0.001, 100000);
    pipe.diameter = readNumber<int>("Диаметр (мм, 1..5000): ", 1, 5000);
    pipe.inRepair = readNumber<int>("В ремонте? (1 - да, 0 - нет): ", 0, 1) == 1;
    return pipe;
}

void printPipe(const Pipe& pipe)
{
    cout << "--- Труба ---\n";
    cout << "Название: " << pipe.name << "\n";
    cout << "Длина:    " << pipe.length << " км\n";
    cout << "Диаметр:  " << pipe.diameter << " мм\n";
    cout << "Статус:   " << (pipe.inRepair ? "в ремонте" : "в работе") << "\n";
}

void editPipe(Pipe& pipe)
{
    printPipe(pipe);
    pipe.inRepair = readNumber<int>("В ремонте? (1 - да, 0 - нет): ", 0, 1) == 1;
    cout << "Признак изменён.\n";
}



CompressorStation readStation()
{
    CompressorStation station;
    station.name = readName("Название станции: ");
    station.workshopCount = readNumber<int>("Количество цехов (1..1000): ", 1, 1000);
    station.workshopWorking = readNumber<int>("Цехов в работе: ", 0, station.workshopCount);
    station.stationClass = readNumber<int>("Класс станции (1..5): ", 1, 5);
    return station;
}

void printStation(const CompressorStation& station)
{
    cout << "--- КС ---\n";
    cout << "Название:        " << station.name << "\n";
    cout << "Всего цехов:     " << station.workshopCount << "\n";
    cout << "Цехов в работе:  " << station.workshopWorking << "\n";
    cout << "Класс станции:   " << station.stationClass << "\n";
}

void editStation(CompressorStation& station)
{
    printStation(station);
    cout << "1. Запустить цех\n2. Остановить цех\n0. Назад\n";
    int action = readNumber<int>("Выберите действие: ", 0, 2);

    if (action == 1)
    {
        if (station.workshopWorking == station.workshopCount)
            cout << "Все цеха уже работают.\n";
        else
        {
            station.workshopWorking++;
            cout << "Цех запущен.\n";
        }
    }
    else if (action == 2)
    {
        if (station.workshopWorking == 0)
            cout << "Все цеха уже остановлены.\n";
        else
        {
            station.workshopWorking--;
            cout << "Цех остановлен.\n";
        }
    }
}



bool saveToFile(const string& fileName, bool hasPipe, const Pipe& pipe,
                bool hasStation, const CompressorStation& station)
{
    ofstream file(fileName);
    if (!file)
        return false;

    file << setprecision(12);
    file << hasPipe << "\n";
    if (hasPipe)
        file << pipe.name << "\n" << pipe.length << "\n"
             << pipe.diameter << "\n" << pipe.inRepair << "\n";

    file << hasStation << "\n";
    if (hasStation)
        file << station.name << "\n" << station.workshopCount << "\n"
             << station.workshopWorking << "\n" << station.stationClass << "\n";

    return file.good();
}


template <typename T>
bool readFromFile(ifstream& file, T& value, T minValue, T maxValue)
{
    string line;
    return getline(file, line) && parseNumber(line, value)
           && value >= minValue && value <= maxValue;
}

bool loadPipe(ifstream& file, Pipe& pipe)
{
    Pipe temp;
    int repairFlag;
    if (!getline(file, temp.name) || isBlank(temp.name)
        || !readFromFile<double>(file, temp.length, 0.001, 100000)
        || !readFromFile<int>(file, temp.diameter, 1, 5000)
        || !readFromFile<int>(file, repairFlag, 0, 1))
        return false;

    temp.inRepair = (repairFlag == 1);
    pipe = temp;
    return true;
}

bool loadStation(ifstream& file, CompressorStation& station)
{
    CompressorStation temp;
    if (!getline(file, temp.name) || isBlank(temp.name)
        || !readFromFile<int>(file, temp.workshopCount, 1, 1000)
        || !readFromFile<int>(file, temp.workshopWorking, 0, temp.workshopCount)
        || !readFromFile<int>(file, temp.stationClass, 1, 5))
        return false;

    station = temp;
    return true;
}


bool loadFromFile(const string& fileName, bool& hasPipe, Pipe& pipe,
                  bool& hasStation, CompressorStation& station)
{
    ifstream file(fileName);
    if (!file)
        return false;

    int pipeFlag, stationFlag;
    Pipe newPipe = pipe;
    CompressorStation newStation = station;

    if (!readFromFile<int>(file, pipeFlag, 0, 1))
        return false;
    if (pipeFlag == 1 && !loadPipe(file, newPipe))
        return false;
    if (!readFromFile<int>(file, stationFlag, 0, 1))
        return false;
    if (stationFlag == 1 && !loadStation(file, newStation))
        return false;

    hasPipe = (pipeFlag == 1);
    hasStation = (stationFlag == 1);
    pipe = newPipe;
    station = newStation;
    return true;
}

// менюшка 

void printMenu()
{
    cout << "\n===== МЕНЮ =====\n"
         << "1. Добавить трубу\n"
         << "2. Добавить КС\n"
         << "3. Просмотр всех объектов\n"
         << "4. Редактировать трубу\n"
         << "5. Редактировать КС\n"
         << "6. Сохранить\n"
         << "7. Загрузить\n"
         << "0. Выход\n";
}

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    const string fileName = "data.txt";
    Pipe pipe = { "", 0, 0, false };
    CompressorStation station = { "", 0, 0, 1 };
    bool hasPipe = false;
    bool hasStation = false;

    while (true)
    {
        printMenu();
        switch (readNumber<int>("Выберите действие: ", 0, 7))
        {
        case 1:
            pipe = readPipe();
            hasPipe = true;
            cout << "Труба добавлена.\n";
            break;
        case 2:
            station = readStation();
            hasStation = true;
            cout << "КС добавлена.\n";
            break;
        case 3:
            if (!hasPipe && !hasStation)
                cout << "Объектов пока нет.\n";
            if (hasPipe)
                printPipe(pipe);
            if (hasStation)
                printStation(station);
            break;
        case 4:
            if (hasPipe)
                editPipe(pipe);
            else
                cout << "Трубы нет. Сначала добавьте её.\n";
            break;
        case 5:
            if (hasStation)
                editStation(station);
            else
                cout << "КС нет. Сначала добавьте её.\n";
            break;
        case 6:
            if (saveToFile(fileName, hasPipe, pipe, hasStation, station))
                cout << "Данные сохранены в " << fileName << ".\n";
            else
                cout << "Ошибка: не удалось сохранить файл.\n";
            break;
        case 7:
            if (loadFromFile(fileName, hasPipe, pipe, hasStation, station))
                cout << "Данные загружены из " << fileName << ".\n";
            else
                cout << "Ошибка: файл не найден или повреждён. Данные не изменены.\n";
            break;
        case 0:
            cout << "До свидания!\n";
            return 0;
        }
    }
}