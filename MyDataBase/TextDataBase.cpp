// Created by Yehor on 26.01.2021.

#include <time.h>
#include "TextDataBase.h"

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
                        cout << "The database doesn't contain table \"" << table
                                                                << "\"." << endl;
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
                    "Commands: \n\r*) read [start] [end], "
                    "\n\r*) write {value}, {value}, ..."
                    "\n\r*) delete {line_index \\ all},"
                    "\n\r*) set {line_index} {field_name} {new_value}:" << endl;
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
                    rewriteFile(*textDataBaseObj, false);
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

/**
 *  Constructor
 */
TextDataBase::TextDataBase(int stringSize, vector<string>& columnTypes,
                           vector<string>& columnNames, int numColumns, string& tableName) :
        stringSize(stringSize), columnTypes(columnTypes),
        columnNames(columnNames), numColumns(numColumns), tableName(tableName)
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
    delete this->bitDeletedLines;
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

    // restored object
    TextDataBase* obj = new TextDataBase(*colNames, *colTypes);

    ifstream input(serialPath, ios_base::binary);
    obj->numRows = intReading(input);
    obj->numColumns = intReading(input);
    obj->rowSizeBytes = intReading(input);
    obj->stringSize = intReading(input);
    obj->tableName = strReading(input, tableNameLengthMeta);
    obj->pathDB = createPathDB(obj->tableName);
    obj->serialPathDB = createSerialPathDB(obj->tableName);
    obj->numOfDelLines = intReading(input);
    deserializeVectorHelper(input, obj->columnTypes, 30);
    deserializeVectorHelper(input, obj->columnNames, 30);
    deserializeVectorHelper(input, (*obj->bitDeletedLines));
    input.close();
    return obj;
}

void TextDataBase::
     deserializeVectorHelper(ifstream& input, vector<string>& vector, int oneDataBlockSize)
{
    int dataLength = intReading(input);
    for (int i = 0; i < dataLength; ++i) {
        string s = strReading(input, oneDataBlockSize);
        vector.push_back(s);
    }
}

void TextDataBase::deserializeVectorHelper(ifstream& input, vector<bool>& vector) {
    int dataLength = intReading(input);
    for (int i = 0; i < dataLength; ++i) {
        bool v;
        input.read(reinterpret_cast<char*>(&v), sizeof(bool));
        vector.push_back(v);
    }
}

void TextDataBase::serializeTable(string &serialPath) {
    ofstream output(serialPath, ios_base::binary | ios_base::trunc);
    // write data
    intWriting(output, this->numRows);
    intWriting(output, this->numColumns);
    intWriting(output, this->rowSizeBytes);
    intWriting(output, this->stringSize);
    strWriting(output, tableName, tableNameLengthMeta);
    intWriting(output, this->numOfDelLines);
    serializeVectorHelper(output, this->columnTypes, 30);
    serializeVectorHelper(output, this->columnNames, 30);
    serializeVectorHelper(output, *(this->bitDeletedLines));
    output.close();
}

void TextDataBase::
     serializeVectorHelper(ofstream &output, vector<string>& vector, int oneDataBlockSize)
{
    int dataLength = vector.size();
    intWriting(output, dataLength);
    for (int i = 0; i < dataLength; ++i) {
        strWriting(output, vector[i], oneDataBlockSize);
    }
}

void TextDataBase::
    serializeVectorHelper(ofstream &output, vector<bool>& vector)
{
    int dataLength = vector.size();
    intWriting(output, dataLength);
    for (int i = 0; i < dataLength; ++i) {
        bool val = vector[i];
        output.write((const char*)&val, sizeof(bool));
    }
}

bool TextDataBase::contains(vector<string>& vector, string& str) {
    for (const string& s : vector)
        if (s == str) return true;
    return false;
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

    // create an object
    TextDataBase* textDataBaseObj = new TextDataBase(
            numOfCharsInString, columnTypesLocal, columnNamesLocal,
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
vector<string>& TextDataBase::parseToWords(string s) {
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
            rewriteFile(*this, false);
            // call write method
            write(command);
        }
    }
    else if (command[0] == "delete") {
        if (command.size() > 1) {
            if (stringIsNumber(command[1])) {
                int index = stoi(command[1]);
                if (index >= this->numRows - this->numOfDelLines)
                    cout << "Error: deleted index is out of bounds" << endl;
                else
                    deleteLine(index); // change number of deleted lines
            } else if (command[1] == "all") {
                rewriteFile(*this, true);
            } else {
                cout << "Error: unexpected word \"" << command[1] << "\"" << endl;
            }
        } else
            cout << "Error: the second param must be a number or \"all\"" << endl;
    }
    else if (command[0] == "set") {
        // set [lineIndex] [column_name] [new_value]
        if (command.size() == 4) {
            // regulates all the aspects of "set" command
           setOperator(command);
        } else
            cout << "Error: inappropriate number of params" << endl;
    } else
        cout << "Error: \"Illegal command.\"" << endl;
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
        if ((*this->bitDeletedLines)[i]) {
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
                else { // type == "string"
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
    if (valuesCorrespondsToTypes(words)) {
        // binary writer
        ofstream output(pathDB, ios_base::binary | ios_base::out | ios_base::app);

        for (int i = 0, size = words.size(); i < size; ++i) {
            // current data type
            string curType = columnTypes[i];

            if (curType == "int")
                intWriting(output, stoi(words[i]));
            else if (curType == "double")
                doubleWriting(output, stod(words[i].c_str()));
            else // curType == "string"
                strWriting(output, words[i], stringSize);
        }
        this->bitDeletedLines->push_back(true);
        this->numRows++;
        output.close();
    } else {
        cout << "Error: invalid parameter types" << endl;
    }
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

bool TextDataBase::valuesCorrespondsToTypes(vector<string> &values) {
    for (int i = 0, size = this->columnTypes.size(); i < size; ++i) {
        string type = this->columnTypes[i];
        if (type == "int") {
            for (char ch : values[i])
                if (ch > '9' || ch < '0') return false;
        } else if (type == values[i]) {
            for (char ch : type)
                if (ch != ',' && (ch > '9' || ch < '0')) return false;
        }
    }
    return true;
}

void TextDataBase::deleteLine(int lineIndex) {
    // compensate for displacement
    int realIndex = getRealLineIndex(*this->bitDeletedLines, lineIndex);
    // set as deleted
    (*this->bitDeletedLines)[realIndex] = false;
    this->numOfDelLines++;
}

void TextDataBase::setOperator(vector<string> &command) {
    int lineIndex;
    // check if the first argument is a number
    if (stringIsNumber(command[1])) {
        lineIndex = stoi(command[1]);
    } else {
        cout << "Error: first argument must be a number" << endl;
        delete &command;
        return;
    }
    // check if line index is inbounds
    if (lineIndex >= numRows) {
        cout << "Error: line index is out of bounds" << endl;
        delete &command;
        return;
    }
    // check if a columnName is correct
    if (!contains(this->columnNames, command[2])) {
        cout << "Error: table \"" << this->tableName <<
             "\" doesn't contain column \"" << command[2] << "\"" << endl;
        delete &command;
        return;
    }
    // get index of column
    int columnIndex = 0;
    for (int i = 0, size = this->columnNames.size(); i < size; ++i) {
        if (this->columnNames[i] == command[2]) {
            columnIndex = i;
            break;
        }
    }
    // number of bytes to skip
    int skipBytes = lineIndex * this->rowSizeBytes;
    for (int i = 0; i < columnIndex; ++i) {
        if (this->columnTypes[i] == "string")
            skipBytes += this->stringSize;
        else if (this->columnTypes[i] == "int")
            skipBytes += sizeof(int);
        else // double
            skipBytes += sizeof(double);
    }
    FILE* file = fopen(this->pathDB.c_str(), "r+b");
    try {
        // set the new value
        if (this->columnTypes[columnIndex] == "int")
            setInt(skipBytes, stoi(command[3]), file);
        else if (this->columnTypes[columnIndex] == "string")
            setStr(skipBytes, command[3], file);
        else
            setDouble(skipBytes, stod(command[3]), file);
    } catch (exception& e) {
        cout << "Error: type of new value is inappropriate" << endl;
    }
    fclose(file);
}

void TextDataBase::setInt(int skipBytes, int value, FILE* file) {
    // binary writer
    fseek(file, skipBytes, SEEK_SET);
    fwrite(&value, sizeof(int), 1, file);
}

void TextDataBase::setDouble(int skipBytes, double value, FILE* file) {
    // binary writer
    fseek(file, skipBytes, SEEK_SET);
    fwrite(&value, sizeof(double ), 1, file);
}

void TextDataBase::setStr(int skipBytes, string &value, FILE* file) {
    // binary writer
    fseek(file, skipBytes, SEEK_SET);
    fwrite(value.c_str(), this->stringSize, 1, file);
}

// A helper method
bool TextDataBase::stringIsNumber(string &str) {
    for (char ch : str)
        if (ch < '0' || ch > '9') return false;
    return true;
}

void TextDataBase::printTableColumnNames() {
    cout << "-------------------------------------------" << endl;
    for (const string& s : this->columnNames)
        cout << "  |  " << s;
    cout << "\n\r-------------------------------------------" << endl;
}

void TextDataBase::rewriteFile(TextDataBase& obj, bool deleteContent) {
    if (deleteContent && obj.numRows > 0) {
        // delete the DB file
        remove(obj.pathDB.c_str());
        // create a new DB file
        ofstream output(obj.pathDB, ios_base::binary); // changed ios::binary todo
        output.close();
        // change field values
        obj.numRows = 0;
        obj.bitDeletedLines->clear();
        obj.numOfDelLines = 0;
    }
    else if (obj.numOfDelLines > 0) {
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
            if ((*obj.bitDeletedLines)[i]) {
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
        obj.numRows = obj.numRows - obj.bitDeletedLines->size();
        obj.bitDeletedLines->clear();
        obj.numOfDelLines = 0;
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
