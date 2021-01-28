// Created by Yehor on 26.01.2021.

#ifndef UNIVPROJECT_TEXTDATABASE_H
#define UNIVPROJECT_TEXTDATABASE_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;

class TextDataBase {
private:
    /* Meta info */
    // File contains info about table name and its internal id
    inline static const string nameIdAvailableFileMeta =
            "C:\\Users\\Yehor\\CLionProjects\\UnivProject\\"
            "MyDataBase\\AvailableTables\\NameIdAllTables.bin";

    // Path to file directory where are files which contain info of each available table
    // To get an access to serial file, should concat this path and table id;
    // for ex. SERIAL_PATH = DIR_PATH + ID_PATH
    inline static const string dirSerialPathMeta =
            "C:\\Users\\Yehor\\CLionProjects\\UnivProject\\MyDataBase\\SerializedTables\\";

    inline static const string dirDB = "C:\\Users\\Yehor\\CLionProjects\\"
                                       "UnivProject\\MyDataBase\\DB_Tables\\";

    // Parameters of stored files
    inline static const int tableNameLengthMeta = 30; // bytes
    inline static const int idLengthMeta = 4; // bytes as standard int

    int numRows;
    int numColumns;
    int rowSizeBytes; // size of row in bytes
    int stringSize;   // number of chars used in each string

    // path to the table
    string pathDB;

    // path to the file of serialization
    string serialPathDB;

    // may contain: string, int, double types
    vector<string>& columnTypes; // each element 7 bytes
    vector<string>& columnNames; // each element 30 bytes
    vector<int>& deletedLines;   // each element 4 bytes

    /*  Serialization */
    /*  Instruction:
     *  1) save [numRows]       4 bytes
     *  2) save [numColumns]    4 bytes
     *  3) save [rowSizeBytes]  4 bytes
     *  4) save [stringSize]    4 bytes
     *  5) save [pathDB]        80 bytes
     *
     *  (*) save columnTypes.size() integer
     *  6) save [columnTypes]   7 bytes  * elements.size()
     *
     *  (*) save columnNames.size() integer
     *  7) save [columnNames]   30 bytes * elements.size()
     */

    /**
     *  Processes incoming commands.
     *  Available commands:
     *      - readD
     *      - write
     */
    void inputHandler(string& command);

    /**
     * Reads and prints rows of database from
     * {@code start} inclusively to {@code end} exclusively.
     * @param start - the line to start reading
     * @param end - the line to end reading
     */
    void read(int startLine, int endLine);

    /**
     * Writes the input words into the database
     * @param words - words to write
     */
    void write(vector<string>& words);

    // Helper methods for reading operation //
    int intReading(ifstream& input);

    double doubleReading(ifstream& input);

    string strReading(ifstream& input);

    // Helper methods for writing operation //
    void intWriting(ofstream& output, int value);

    void doubleWriting(ofstream& output, double value);

    void strWriting(ofstream& output, string& value);

    /**
     * Checks if a string {@code str} is a number
     * @return {@code true} is {@code str} is a number. Otherwise, false
     */
    bool stringIsNumber(string& str);

    /**
     * Parses a string to a vector of words
     * @param s - string which will be parsed
     * @return vector of words
     */
    static vector<string>& parseToWords(string s);

    /**
     * Creates a new database table
     * @return {@code TextDataBase} object
     */
    static TextDataBase& askInitQuestions();

    /* Meta methods */
    /**
     * Prints available tables. In other words, it prints
     * names of tables which have been already created
     * @param reader - an input stream
     * @return {@code map} which represents file names
     *          and ids which correspond to them;
     */
    unordered_map<string, string>& printAvailableTables(ifstream& reader);

    /**
     * @param reader - an input stream
     * @return {@code true} is there is at least one table available. Otherwise {@code false}
     */
    bool checkAnyTableIsAvailable(ifstream& reader);

    /**
     * @param id - id of selected table
     * @return serial path of selected table, where
     *         SERIAL_PATH = DIR_PATH + ID_PATH
     */
    string getSerialPathByName(string& name, unordered_map<string, string>& namesIdsMap);

    /**
     * Serializes current object into the file {@code serialPathDB}
     * @param serialPath
     */
    void serializeTable(string& serialPath);

    template<typename TYPE>
    void serializeVectorHelper(ofstream& input, vector<TYPE>& vector, int oneByteSize);

    /**
     * Creates an object of data base using existing data
     * @param serialPath - a path where serial data of the current table is stored
     * @return {@code TextDataBase} object
     */
    TextDataBase& deserializeTable(string& serialPath);

    template<typename TYPE>
    void deserializeVectorHelper(ifstream& input, vector<TYPE>& vector, int oneByteSize);

    // destructor
    virtual ~TextDataBase();

    // constructor
    TextDataBase(int stringSize, vector<string> &columnTypes,
                 vector<string> &columnNames, vector<int>& deletedLines,
                 int numColumns,  string& dbFileName, int dbSerialId);

    // default constructor
    TextDataBase(vector<string>& columnNames,
                 vector<string>& columnTypes,
                 vector<int>& deletedLines)
                 : columnNames(columnNames), columnTypes(columnTypes), deletedLines(deletedLines) {}

public:
    /**
     *  Runs a database
     */
    static void runDataBase();
};


#endif //UNIVPROJECT_TEXTDATABASE_H
