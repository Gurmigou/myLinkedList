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
    // File contains names of existing tables
    inline static const string availableTablesFileMeta =
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

    // the name of the table
    string tableName;
    // path to the table
    string pathDB;
    // path to the file of serialization
    string serialPathDB;

    // may contain: string, int, double types
    vector<string>& columnTypes; // each element 10 bytes
    vector<string>& columnNames; // each element 30 bytes
    vector<int>& deletedLines;   // each element 4 bytes

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
    static int intReading(ifstream& input);
    static double doubleReading(ifstream& input);
    static string strReading(ifstream& input, int strSize);

    // Helper methods for writing operation //
    static void intWriting(ofstream& output, int value);
    static void doubleWriting(ofstream& output, double value);
    static void strWriting(ofstream& output, string& value, int strSize);

    /**
     * @param tableName - name of the current table
     * @return a path where data of DB is stored
     */
    static string createPathDB(string& tableName);

    /**
     * @param tableName - name of the current table
     * @return a path where DB object is serialized
     */
    static string createSerialPathDB(string& tableName);

    /**
     * Checks if a string {@code str} is a number
     * @return {@code true} is {@code str} is a number. Otherwise, false
     */
    bool stringIsNumber(string& str);

    void printTableColumnNames();

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
     * @return {@code vector} which contains file names
     */
    static vector<string>& printAvailableTables(ifstream& reader);

    /**
     * @param reader - an input stream
     * @return {@code true} is there is at least one table available. Otherwise {@code false}
     */
    static bool checkAnyTableIsAvailable(ifstream& reader);

    /**
     * Checks if a {@code vector} contains {@code str} value
     * @return {@code true} if contains. Otherwise, {@code false}
     */
    static bool contains(vector<string>& vector, string& str);

    /**
     * @param reader - an input object
     * @return a number of files which are now stored in {@code NameIdAllTables} file
     */
    static int getNumOfAvailableFiles(ifstream& reader);

    /*  Serialization */
    /*  Instruction:
     *  1) save [numRows]       4 bytes
     *  2) save [numColumns]    4 bytes
     *  3) save [rowSizeBytes]  4 bytes
     *  4) save [stringSize]    4 bytes
     *  5) save [tableName]     30 bytes
     *
     *  (*) save columnTypes.size() integer
     *  6) save [columnTypes]   7 bytes  * elements.size()
     *
     *  (*) save columnNames.size() integer
     *  7) save [columnNames]   30 bytes * elements.size()
     */

    /**
     * Serializes current object into the file {@code serialPathDB}
     * @param serialPath
     */
    void serializeTable(string& serialPath);

    // This method helps to serialize vectors
    void serializeVectorHelper(ofstream& output, vector<string>& vector,
                                        int oneDataBlockSize);

    /**
     * Creates an object of data base using existing data
     * @param serialPath - a path where serial data of the current table is stored
     * @return {@code TextDataBase} object
     */
    static TextDataBase* deserializeTable(string& serialPath);

    // This method helps to deserialize vectors
    static void deserializeVectorHelper(ifstream& input, vector<string>& vector,
                                        int oneDataBlockSize);

    // destructor
    ~TextDataBase();

    // constructor
    TextDataBase(int stringSize, vector<string>& columnTypes,
                 vector<string>& columnNames, vector<int>& deletedLines,
                 int numColumns,  string& dbFileName);

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