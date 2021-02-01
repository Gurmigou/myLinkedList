// Created by Yehor on 26.01.2021.

#include <time.h>
#include "TextDataBase.h"

/**
 *  Constructor
 */
TextDataBase::TextDataBase(int stringSize,
                           vector<string>& columnTypes,
                           vector<string>& columnNames,
                           unordered_set<int>& deletedLines,
                           int numColumns,
                           string& tableName) :
        stringSize(stringSize), columnTypes(columnTypes),
        columnNames(columnNames), deletedLines(deletedLines),
        numColumns(numColumns), tableName(tableName)
{
    // create a path where data of DB will be stored
    this->pathDB = createPathDB(tableName);
    // create a path where db will be serialized
    this->serialPathDB = createSerialPathDB(tableName);

    this->numRows = 0;
    this->rowSizeBytes = 0;

    for (string type : columnTypes) {
        if (type == "int")
            this->rowSizeBytes += sizeof(int);
        else if (type == "double")
            this->rowSizeBytes += sizeof(double);
        else // type == "string"
            this->rowSizeBytes += stringSize;
    }
}

/**
 *  Destructor
 */
TextDataBase::~TextDataBase() {
    delete &this->columnTypes;
    delete &this->columnNames;
    delete &this->deletedLines;
}

/* Meta block start */

int TextDataBase::getNumOfAvailableFiles(ifstream& reader, int lineLengthInBytes) {
    ifstream::pos_type begin = reader.tellg();
    reader.seekg(0, ios::end);
    ifstream::pos_type end = reader.tellg();

    // return a cursor to the begin of the file
    reader.seekg(0, ios::beg);
    return (end - begin) / lineLengthInBytes;
}

bool TextDataBase::checkAnyTableIsAvailable(ifstream &reader) {
    return getNumOfAvailableFiles(reader, tableNameLengthMeta) != 0;
}

string TextDataBase::createPathDB(string &tableName) {
    return dirDB + tableName + ".bin";
}

string TextDataBase::createSerialPathDB(string &tableName) {
    return dirSerialPathMeta + tableName + ".bin";
}

// todo delete pointer!!!
vector<string>& TextDataBase::printAvailableTables(ifstream &reader) {

    vector<string>* availableTableNames = new vector<string>();

    for (int i = 0, files =
            getNumOfAvailableFiles(reader, tableNameLengthMeta); i < files; ++i)
    {
        string tableNameLocal = strReading(reader, tableNameLengthMeta);
        // print table name
        cout << i << ") Table: " << tableNameLocal << endl;
        // add values to the vector
        (*availableTableNames).push_back(tableNameLocal);
    }
    // return a cursor to the begin of the file
    reader.seekg(0, ios::beg);
    return *availableTableNames;
}


TextDataBase* TextDataBase::deserializeTable(string& serialPath) {
    vector<string>* colNames = new vector<string>();
    vector<string>* colTypes = new vector<string>();
    unordered_set<int>* delLines = new unordered_set<int>();

    // restored object
    TextDataBase* obj = new TextDataBase(*colNames, *colTypes, *delLines);

    ifstream input(serialPath, ios_base::binary);
    obj->numRows = intReading(input);
    obj->numColumns = intReading(input);
    obj->rowSizeBytes = intReading(input);
    obj->stringSize = intReading(input);
    obj->tableName = strReading(input, tableNameLengthMeta);
    obj->pathDB = createPathDB(obj->tableName);
    obj->serialPathDB = createSerialPathDB(obj->tableName);
    deserializeVectorHelper(input, obj->columnTypes, 30);
    deserializeVectorHelper(input, obj->columnNames, 30);
    input.close();
    return obj;
}

void TextDataBase::deserializeVectorHelper(ifstream& input,
                                           vector<string>& vector,
                                           int oneDataBlockSize)
{
    int dataLength = intReading(input);
    for (int i = 0; i < dataLength; ++i) {
        string s = strReading(input, oneDataBlockSize);
        vector.push_back(s);
    }
}

/**
 *  Serialization
 *  Instruction:
 *  1) save [numRows]       4 bytes
 *  2) save [numColumns]    4 bytes
 *  3) save [rowSizeBytes]  4 bytes
 *  4) save [stringSize]    4 bytes
 *  5) save [tableName]     30 bytes
 *
 *  *) save columnTypes.size() integer
 *  6) save [columnTypes]   7 bytes  * elements.size()
 *
 * (*) save columnNames.size() integer
 *  7) save [columnNames]   30 bytes * elements.size()
 *
 * @param serialPath - a path where data will be stored
 */
void TextDataBase::serializeTable(string &serialPath) {
    ofstream output(serialPath, ios_base::binary | ios_base::trunc);
    // write data
    intWriting(output, this->numRows);
    intWriting(output, this->numColumns);
    intWriting(output, this->rowSizeBytes);
    intWriting(output, this->stringSize);
    strWriting(output, tableName, tableNameLengthMeta);
    serializeVectorHelper(output, this->columnTypes, 30);
    serializeVectorHelper(output, this->columnNames, 30);
    output.close();
}


void TextDataBase::serializeVectorHelper(ofstream &output,
                                         vector<string>& vector,
                                         int oneDataBlockSize)
{
    int dataLength = vector.size();
    intWriting(output, dataLength);
    for (int i = 0, size = vector.size(); i < size; ++i) {
        strWriting(output, vector[i], oneDataBlockSize);
    }
}

bool TextDataBase::contains(vector<string>& vector, string& str) {
    for (const string& s : vector)
        if (s == str) return true;
    return false;
}

bool TextDataBase::contains(unordered_set<int>& set, int value) {
    return set.find(value) != set.cend();
}

/**
 *  Runs a database
 */
void TextDataBase::runDataBase() {
    ifstream reader(availableTablesFileMeta, ios_base::binary);
    // an object to work with
    TextDataBase* textDataBaseObj = nullptr;
    while (true) {
        if (textDataBaseObj != nullptr) {
            delete textDataBaseObj;
            textDataBaseObj = nullptr;
        }
        cout << "Enter \"stop\" to terminate.\n\r"
                "Select a table or create a new one: (new / select)." << endl;
        string tableChoice;
        cin >> tableChoice;
        cin.ignore(32767, '\n');

        if (tableChoice._Starts_with("new")) {
            // start creating a new table
            textDataBaseObj = &askInitQuestions();
        }
        else if (tableChoice._Starts_with("select")) {
            // checks if register contains at least one stored table
            if (!checkAnyTableIsAvailable(reader)) {
                cout << "There is no available tables. Try to create a new one." << endl;
            } else {
                vector<string>& availableTables = printAvailableTables(reader);
                while (true) {
                    cout << "Enter \"leave\" to leave from the selection of table.\n\r"
                            "Select a table:" << endl;
                    string table;
                    getline(cin, table);
                    if (table._Starts_with("leave")) {
                        break;
                    }
                    if (!contains(availableTables, table)) {
                        cout << "The database doesn't contain table \"" << table << "\"." << endl;
                    } else {
                        string serialPathLocal = createSerialPathDB(table);
                        textDataBaseObj = deserializeTable(serialPathLocal);
                        break;
                    }
                }
                delete &availableTables;
            }
        } else if (tableChoice._Starts_with("stop")) {
            break;
        }
        else {
            cout << "Invalid command." << endl;
        }
        // terminate program if this flag is true
        bool fullStop = false;
        // work with the table
        if (textDataBaseObj != nullptr) {
            cout << "Enter \"stop\" to terminate.\n\r"
                    "Enter \"leave\" to select a another table or create a new one. "
                    "Commands: \n\r*) read [start] [end], \n\r*) write {value}, {value},"
                    " ... \n\r*) delete {line_index \\ all}:" << endl;
            // runs available commands
            while (true) {
                string command;
                getline(cin, command);

                if (!command._Starts_with("stop") &&
                    !command._Starts_with("leave"))
                {
                    (*textDataBaseObj).inputHandler(command);
                    cout << "A command was done." << endl;
                } else {
                    // remove deleted lines
                    rewriteFile(*textDataBaseObj, false,
                                &textDataBaseObj->deletedLines);
                    // serialize current object
                    (*textDataBaseObj).serializeTable((*textDataBaseObj).serialPathDB);
                    if (command._Starts_with("stop"))
                        fullStop = true;
                    break;
                }
            }
        }
        if (fullStop) break;
    }
    reader.close();
    // delete an object
    if (textDataBaseObj != nullptr)
        delete textDataBaseObj;
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
    if (columnNamesLocal.size() != columnTypesLocal.size()) {
        cout << "Error: \"Different number of types and names\"" << endl;
        throw exception();
    }

    // lines that were deleted
    unordered_set<int>* deletedLines = new unordered_set<int>();

    // create an object
    TextDataBase* textDataBaseObj = new TextDataBase(
            numOfCharsInString, columnTypesLocal,
           columnNamesLocal, *deletedLines,
            columnNamesLocal.size(), tableName);

    // create a file
    ofstream writer(textDataBaseObj->pathDB, ios_base::binary);
    writer.close();

    // add info about a new table to the registry
    ofstream outputRegister(TextDataBase::availableTablesFileMeta,
                            ios_base::binary | ios_base::out | ios_base::app);
    strWriting(outputRegister, tableName, tableNameLengthMeta);
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
        } else i++;
    }
    return *words;
}

/**
 *  Input Handler
 */
void TextDataBase::inputHandler(string& commandStr) {
    // separated words (commands)
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
        if (readEnd < readFrom) {
            cout << "Error: \"Read from\" line have to be less "
                    "than \"read to\" line.\"" << endl;
            delete &command;
            return;
        }
        // check correctness of bounds
        if (readFrom < 0 || readEnd > numRows) {
            cout << "Error: \"Illegal bounds of reading.\"" << endl;
            delete &command;
            return;
        }
        // call read method
        read(readFrom, readEnd);
    }
    else if (command[0] == "write") {
        if (command.size() > 1) {
            // remove first word == "write"
            command.erase(command.cbegin());
            // check if number of words in the vector == number of columns
            if (command.size() != numColumns) {
                cout << "Error: \"Inappropriate number of words\"" << endl;
                delete &command;
                return;
            }
            // perform deletion of deleted lines
            rewriteFile(*this, false, &this->deletedLines);
            // call write method
            write(command);
        }
    }
    else if (command[0] == "delete") {
        if (command.size() > 1) {
            if (stringIsNumber(command[1])) {
                int index = stoi(command[1]);
                if (index >= this->numRows - this->deletedLines.size())
                    cout << "Error: deleted index is out of bounds" << endl;
                else
                    deleteLine(index); // change number of deleted lines
            } else if (command[1] == "all") {
                rewriteFile(*this, true, nullptr);
            } else {
                cout << "Error: unexpected word \"" << command[1] << "\"" << endl;
            }
        } else {
            cout << "Error: the second param must be a number or \"all\"" << endl;
        }
    }
    else {
        cout << "Error: \"Illegal command.\"" << endl;
    }
    delete &command;
}

/**
 *  Reader
 */
void TextDataBase::read(int startLine, int endLine) {
    // binary reader
    // binary reader
    ifstream input(pathDB, ios_base::binary);

    int startSkip = rowSizeBytes * startLine;
    int availLines = getNumOfAvailableFiles(input, this->rowSizeBytes);
    input.seekg(startSkip);

    // print column names
    printTableColumnNames();

    for (int i = startLine, printCounter = 0; i < endLine && i < availLines; ++i) {
        if (!contains(this->deletedLines, i)) {
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
                    string strRead = strReading(input, stringSize);
                    resultLine.append(strRead).append("  ");
                }
                columnIndex++;
            }
            cout << (printCounter++) << ") " << resultLine << endl;
        } else {
            // skip one row
            input.seekg(this->rowSizeBytes, SEEK_CUR);
            endLine++;
        }
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

string TextDataBase::strReading(ifstream &input, int strSize) {
    char* readStr = new char[strSize + 1];
    input.read(readStr, strSize);
    readStr[strSize] = '\0';
    return string(readStr);
}

/**
 *  Writer
 */
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
            strWriting(output, words[i], stringSize);
        }
    }
    this->bitLines->push_back(true);
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

void TextDataBase::strWriting(ofstream &output, string &value, int strSize) {
    output.write(value.c_str(), strSize);
}

void TextDataBase::deleteLine(int lineIndex) {
    // compensate for displacement
    int realIndex = getRealLineIndex(*this->bitLines, lineIndex);
    // set as deleted
    (*this->bitLines)[realIndex] = false;
    this->deletedLines.insert(realIndex);
}

// A helper method
bool TextDataBase::stringIsNumber(string &str) {
    for (char ch : str)
        if (ch < '0' || ch > '9') return false;
    return true;
}

void TextDataBase::printTableColumnNames() {
    cout << "|------------------------------------------|" << endl;
    for (const string& s : this->columnNames)
        cout << "  | " << s;
    cout << "  |\n|------------------------------------------|" << endl;
}

void TextDataBase::rewriteFile(TextDataBase& obj, bool deleteContent,
                               unordered_set<int> *deletedLinesSet) {
    if (deleteContent) {
        // delete the DB file
        remove(obj.pathDB.c_str());
        // create a new DB file
        ofstream output(obj.pathDB, ios::binary);
        output.close();
        // change field values
        obj.deletedLines.clear();
        obj.numRows = 0;
        obj.bitLines->clear();
    }
    else if (!deletedLinesSet->empty()) {
        srand(time(nullptr));
        // a random number for temporary file
        int tempNum = rand();
        // current file
        ifstream input(obj.pathDB, ios_base::binary);
        // new file writer
        string tempFileName = dirDB + obj.tableName + to_string(tempNum) + ".bin";
        ofstream output(tempFileName, ios::binary);
        // a number of lines in the file
        int numOfLines = getNumOfAvailableFiles(input, obj.rowSizeBytes);
        for (int i = 0; i < numOfLines; ++i)
        {
            if (!contains(*deletedLinesSet, i)) {
                unsigned char* line = new unsigned char[obj.rowSizeBytes];
                input.read(reinterpret_cast<char*>(line), obj.rowSizeBytes);
                output.write(reinterpret_cast<const char*>(line), obj.rowSizeBytes);
                delete[] line;
            } else {
                // skip bytes
                input.seekg(obj.rowSizeBytes, SEEK_CUR);
            }
        }
        // close streams
        input.close();
        output.close();

        // delete current file
        remove(obj.pathDB.c_str());
        // rename the temporary file to the original name
        rename(tempFileName.c_str(), obj.pathDB.c_str());

        // change filed values
        obj.numRows = obj.numRows - deletedLinesSet->size();
        obj.deletedLines.clear();
        obj.bitLines->clear();
    }
}

int TextDataBase::getRealLineIndex(vector<bool>& bits, int notRealIndex) {
    // a real index
    int index = 0;
    // a special case
    if (notRealIndex == 0 && !bits[0]) {
        while (!bits[index]) index++;
        return index;
    }
    while (index <= notRealIndex) {
        if (!bits[index]) notRealIndex++;
        index++;
    }
    return index - 1;
}



