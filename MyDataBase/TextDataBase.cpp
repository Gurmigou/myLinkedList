// Created by Yehor on 26.01.2021.

#include <time.h>
#include "TextDataBase.h"

/**
 *  Constructor
 */
TextDataBase::TextDataBase(int stringSize,
                           vector<string>& columnTypes,
                           vector<string>& columnNames,
                           vector<int>& deletedLines,
                           int numColumns,
                           string& dbFileName,
                           int dbSerialId) :
        stringSize(stringSize), columnTypes(columnTypes),
        columnNames(columnNames), deletedLines(deletedLines), numColumns(numColumns)
{
    // create a path where data of DB will be stored
    this->pathDB = dirDB + dbFileName + ".bin";
    // create a path where db will be serialized
    this->serialPathDB = dirSerialPathMeta + to_string(dbSerialId) + ".bin";
    this->numRows = 0;
    this->rowSizeBytes = 0;
    for (string type : columnTypes) {
        if (type == "int") {
            this->rowSizeBytes += sizeof(int);
        } else if (type == "double") {
            this->rowSizeBytes += sizeof(double);
        }
        // type == "string"
        else {
            this->rowSizeBytes += stringSize;
        }
    }
}

/**
 *  Destructor
 */
TextDataBase::~TextDataBase() {
    delete &this->columnTypes;
    delete &this->columnNames;
}

/* Meta block start */

bool TextDataBase::checkAnyTableIsAvailable(ifstream &reader) {
    ifstream::pos_type begin = reader.tellg();
    reader.seekg(0, ios::end);
    ifstream::pos_type end = reader.tellg();

    // return a cursor to the begin of the file
    reader.seekg(0, ios::beg);
    return (end - begin) != 0;
}

// todo delete pointer!!!
unordered_map<string, string>& TextDataBase::printAvailableTables(ifstream &reader) {
    // key: tableName <-> value: id
    unordered_map<string, string>* map = new unordered_map<string, string>();

    int counter = 0;
    while (!reader.eof()) {
        char* tableName = new char[TextDataBase::tableNameLengthMeta + 1];
        char* tableID = new char[TextDataBase::idLengthMeta + 1];

        reader.read(tableName, TextDataBase::tableNameLengthMeta);
        reader.read(tableID, TextDataBase::idLengthMeta);

        // print table name
        cout << counter << ") Table: " << tableName << endl;

        // add values to the map
        (*map)[string(tableName)] = string(tableID);
    }
    return *map;
}

string TextDataBase::getSerialPathByName(string& name, unordered_map<string, string>& namesIdsMap) {
    return TextDataBase::dirSerialPathMeta + namesIdsMap[name] + ".bin";
}

TextDataBase& TextDataBase::deserializeTable(string& serialPath) {
    // there is no sense in this objects. They are used for initialization todo
    auto colNames = new vector<string>();
    auto colTypes = new vector<string>();
    auto delLines = new vector<int>();
    // restored object
    TextDataBase* obj = new TextDataBase(*colNames, *colTypes, *delLines);

    ifstream input(serialPath, ios_base::binary);
    obj->numRows = intReading(input);
    obj->numColumns = intReading(input);
    obj->rowSizeBytes = intReading(input);
    obj->stringSize = intReading(input);
    // pathDB
    char* pathDB_STR = new char[80];
    input.read(pathDB_STR, 79);
    delete[] pathDB_STR; // todo ???

    // todo
    // columnTypes
    deserializeVectorHelper(input, *colTypes, 7);
//    int sizeCT;
//    input.read(reinterpret_cast<char *>(sizeCT), sizeof(int));
//    input.read(reinterpret_cast<char *>(colTypes->data()), 7 * sizeCT);

    // columnNames
//    int sizeCN;
//    input.read(reinterpret_cast<char *>(sizeCN), sizeof(int));
//    input.read(reinterpret_cast<char *>(colNames->data()), 30 * sizeCN);
    deserializeVectorHelper(input, *colNames, 30);

    // deletedLines
//    int sizeDel;
//    input.read(reinterpret_cast<char *>(sizeDel), sizeof(int));
//    input.read(reinterpret_cast<char *>(delLines->data()), 4 * sizeDel);
//    deserializeVectorHelper(input, *delLines, 4);

    return *obj;
}

template<typename TYPE>
void TextDataBase::deserializeVectorHelper(ifstream& input,
                                           vector<TYPE> &vector, int oneByteSize)
{
    int sizeDat = 0;
    input.read(reinterpret_cast<char*>(sizeDat), sizeof(int));
    input.read(reinterpret_cast<char *>(vector.data()), oneByteSize * sizeDat);
}

/**
 *  Runs a database
 */
void TextDataBase::runDataBase() {
    // todo Сделать возможным запуск уже существуещей БД
    // todo Сохранять состояние полей бд в файл

    TextDataBase& textDataBaseObj = TextDataBase::askInitQuestions();


    cout << "Commands: \n\r{read [start] [end]}, \n\r{write}. "
            "\n\rEnter \"stop\" to terminate." << endl;

    // runs available commands
    while (true) {
        string command;
        getline(cin, command);
        if (command == "stop")
            break;
        textDataBaseObj.inputHandler(command);
        cout << "A Command was successfully done!" << endl;
    }
}

TextDataBase& TextDataBase::askInitQuestions() {
    // creates a table
    string tableName, names, types;

    cout << "Enter table name: " << endl;
    getline(cin, tableName);

    cout << "Enter names of columns:" << endl;
    getline(cin, names);
    auto& columnNamesLocal = parseToWords(names);

    cout << "Enter types of columns:" << endl;
    getline(cin, types);
    auto& columnTypesLocal = parseToWords(types);

    int numOfCharsInString;
    cout << "Enter a number of characters in the string type:" << endl;
    cin >> numOfCharsInString;
    cin.ignore(32767, '\n');

    // check if sizes are equal
    if (columnNamesLocal.size() != columnTypesLocal.size())
        throw exception("Different number of types and names");

    srand(time(nullptr));
    int serialTableId = rand() * rand();

    vector<int>* deletedLines = new vector<int>();

    // create an object
    TextDataBase* textDataBaseObj = new TextDataBase(
            numOfCharsInString, columnTypesLocal,
         columnNamesLocal, *deletedLines,
            columnNamesLocal.size(), tableName, serialTableId);


    // create a file
    ofstream writer(textDataBaseObj->pathDB, ios_base::binary);
    writer.close();

    // add info about a new table to the registry
    ofstream outputRegister(TextDataBase::nameIdAvailableFileMeta,
                            ios_base::binary | ios_base::out | ios_base::app);
    textDataBaseObj->strWriting(outputRegister, tableName);
    textDataBaseObj->intWriting(outputRegister, serialTableId);
    outputRegister.close();

    return *textDataBaseObj;
}

/* Meta block end */


// A helper method: parser
vector<string>& TextDataBase::parseToWords(string s)
{
    vector<string>* words = new vector<string>();
    for (int i = 0, length = s.length(); i < length ; ) {
        if (s[i] != ' ') {
            char* buffer = new char[30];
            char* pointer_buf = buffer;
            do {
                *(pointer_buf++) = s[i++];
            } while (i < length && s[i] != ' ');
            *(pointer_buf) = '\0';
            words->push_back(string(buffer));
        } else
            i++;
    }
    return *words;
}

/**
 *  Input Handler
 */
void TextDataBase::inputHandler(string& commandStr) {

    vector<string>& command = parseToWords(commandStr);

    if (command[0] == "read") {
        int readFrom = 0;
        int readEnd = numRows;

        if (command.size() > 1) {
            // check the string correctness
            if (command.size() >= 2 && stringIsNumber(command[1]))
                readFrom = stoi(command[1]);
            if (command.size() >= 3 && stringIsNumber(command[2]))
                readEnd = stoi(command[2]);
        }
        // check correctness of order
        if (readEnd < readFrom)
            throw exception("\"Read from\" line have to be less than \"read to\" line.");

        // check correctness of bounds
        if (readFrom < 0 || readEnd > numRows)
            throw exception("Illegal bounds of reading.");

        // call read method
        read(readFrom, readEnd);
    }
    else if (command[0] == "write") {
        if (command.size() > 1) {
            // remove first word == "write"
            command.erase(command.cbegin());

            // check if number of words in the vector == number of columns
            if (command.size() != numColumns)
                throw exception("Not enough words to write");

            // call write method
            write(command);
        }
    }
    else {
        throw exception("Illegal command.");
    }
}

/**
 *  Reader
 */
void TextDataBase::read(int startLine, int endLine) {
    // binary reader
    ifstream input(pathDB, ios_base::binary);

    int startSkip = rowSizeBytes * startLine;
    input.seekg(startSkip);

    // todo сделать печать названия столбцев таблицы

    for (int i = startLine; i < endLine; ++i) {

        // index of column
        int columnIndex = 0;
        string resultLine;

        while (columnIndex < numColumns) {
            // type of current column
            string type = columnTypes[columnIndex];

            // choose a correct type
            if (type == "int") {
                int intRead = intReading(input);
                resultLine.append(to_string(intRead)).append("  ");
            }
            else if (type == "double") {
                double doubleRead = doubleReading(input);
                resultLine.append(to_string(doubleRead)).append("  ");
            }
            // type == "string"
            else {
                string strRead = strReading(input);
                resultLine.append(strRead).append("  ");
            }
            columnIndex++;
        }
        cout << i << ") " << resultLine << endl;
    }

    input.close();
}

// Reading helper methods //
int TextDataBase::intReading(ifstream &input) {
    int readI = 0;
    input.read((char*)&readI, sizeof(readI));
    return readI;
}

double TextDataBase::doubleReading(ifstream &input) {
    double readD = 0;
    input.read((char*) &readD, sizeof(readD));
    return readD;
}

string TextDataBase::strReading(ifstream &input) {
    char* readStr = new char[stringSize + 1];
    input.read(readStr, stringSize);
    return string(readStr);
}

/**
 *  Writer
 */
 // todo Ошибка при повторной записи! Недоделано append файла
void TextDataBase::write(vector<string> &words) {
    // binary writer
    ofstream output(pathDB, ios_base::binary | ios_base::out | ios_base::app);

    for (int i = 0, size = words.size(); i < size; ++i) {
        // current data type
        string curType = columnTypes[i];

        // todo Добавить детальную проверку типов данных
        if (curType == "int") {
            intWriting(output, stoi(words[i]));
        }
        else if (curType == "double") {
            doubleWriting(output, stod(words[i].c_str()));
        }
        // curType == "string"
        else {
            strWriting(output, words[i]);
        }
    }

    this->numRows++;
    output.close();
}

// Writing helper methods //
void TextDataBase::intWriting(ofstream &output, int value) {
    output.write((const char*)&value, sizeof(int));
}

void TextDataBase::doubleWriting(ofstream &output, double value) {
    output.write((const char*)&value, sizeof(double));
}

void TextDataBase::strWriting(ofstream &output, string &value) {
    output.write(value.c_str(), stringSize);
}

// A helper method
bool TextDataBase::stringIsNumber(string &str) {
    for (char ch : str) {
        if (ch < '0' || ch > '9') return false;
    }
    return true;
}




