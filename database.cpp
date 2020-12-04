// DO:  more ~username/.my.cnf to see your password
// CHANGE:  MYUSERNAME and MYMYSQLPASSWORD to your username and mysql password
// COMPILE:  g++ -Wall -I/usr/include/cppconn -o database database.cpp -L/usr/lib -lmysqlcppconn
// RUN:      ./odbc_example

// SHOW TABLES;

// DESC POLICIES_SOLD;
// DESC CLIENT;
// DESC AGENTS;
// DESC POLICY;

// SELECT * FROM POLICIES_SOLD;
// SELECT * FROM CLIENT;
// SELECT * FROM AGENTS;
// SELECT * FROM POLICY;

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <iomanip>
#include <iostream>
 
using namespace std;
 
sql::Driver *driver;
sql::Connection *con;
sql::Statement *statement;
sql::ResultSet *resultSet;
sql::ResultSetMetaData *metaData;
sql::Connection* Connect(string, string);
 
void insert(string table, string values );
void initDatabase(const string Username, const string Password, const string SchemaName);
void query (string q);
// added query method to execute updates
bool queryModify (string q);
void print (sql::ResultSet *resultSet);
void disconnect();
void printRecords(sql::ResultSet *resultSet, int numColumns);
void printHeader(sql::ResultSetMetaData *metaData, int numColumns);

// helper method to process user input string
string processInput(string str);

// menu methods
void findAgentsClients();
void purchasePolicy(); 
void agentPolicies(); 
void cancelPolicy(); 
void addAgent(); 
 

// menu options method
int menuOptions()
{
    int Choice;
    do 
    {
        cout << "\nEnter '1' to find all agents and clients in a given city\n"
            << "      '2' to purchase an available policy from a particular agent\n"
            << "      '3' to list all policies sold by a particular agent\n"
            << "      '4' to cancel a policy\n"
            << "      '5' to add a new agent for a city\n"
            << "      '6' to quit\n"
            << "Selection: ";
        cin >> Choice;
    } while (Choice < 1 || Choice > 6);
    return Choice;
}

// main program!
int main() 
{
 
    string Username = "rghosh";             // Change to your own username
    string mysqlPassword = "spring2020";   // Change to your own mysql password

    con = Connect (Username, mysqlPassword);
    initDatabase(Username, mysqlPassword, Username);

    // selecting database options
    int Choice;
    cout << "Welcome to the database! Please select from one of the following options: " << endl;
    Choice = menuOptions();
    while(Choice != 6)
    {
        switch (Choice)
        {
            case 1:   findAgentsClients(); break;
            case 2:   purchasePolicy(); break;
            case 3:   agentPolicies(); break;
            case 4:   cancelPolicy(); break;
            case 5:   addAgent(); break;
        }
        // Ask for next choice
        Choice = menuOptions();
    }
    cout << "Thank you for using the database! Have a nice day :)" << endl;
    disconnect();
    return 0;
}   

 // ************************ 1) Find all agents and clients in a given city ************************

void findAgentsClients() {
    // get city name from user
    string city;
    cout << "Please enter city name (Dallas, Boston, Fayetteville, New York, Rogers, or Pheonix: " << endl;
    cin.ignore();
    getline(cin, city);

    // process input to ensure it is upper case & no spaces
    city = processInput(city);

    // run query
    cout << "Here are all the agents & their associated clients from " << city << endl;
    cout << endl;
    string cityQuery = "SELECT * FROM AGENTS a, CLIENT c WHERE a.A_CITY = \'" + city + "\' AND a.A_CITY = c.C_CITY";
    query(cityQuery);

    // if empty query is returned, prompt user for different/valid city 
    while(resultSet -> rowsCount() == 0) {
        cout << "There are no agents in " << city << ". Please enter the name of another city, or enter 0 to return to the main menu." << endl;
        getline(cin, city);
        if (city == "0") {
            return;
        }
        city = processInput(city);
        string cityQuery = "SELECT * FROM AGENTS a, CLIENT c WHERE a.A_CITY = c.C_CITY AND A_CITY = \'" + city + "\'";
        query(cityQuery);
    }

}

// ************************ 2) Add a new client, then purchase an available policy from a particular agent ************************

void purchasePolicy() {
    string clientName;
    string clientCity;
    string clientZip;
    cout << "Please enter client information, separated by commas (Name, City, Zip):" << endl;
    cin.ignore();
    getline(cin, clientName, ',');  
    getline(cin, clientCity, ',');
    getline(cin, clientZip);

    clientName = processInput(clientName);
    clientCity = processInput(clientCity);

    cout << "You entered the following: " << clientName << " " << clientCity << " " << clientZip << endl;

    // incr increments the client id by one and adds the client
    string incr = "(SELECT MAX(C_ID) + 1 FROM (SELECT * FROM CLIENT) AS c), \'" + clientName + "\', \'" + clientCity + "\'," + clientZip;
    insert("CLIENT(C_ID, C_NAME, C_CITY, C_ZIP)", incr);
    query("SELECT * FROM CLIENT");
    cout << endl;
    cout << "Client " << clientName << " has been successfully added!" << endl;

    // see if there is an avaliable agent in the client's city. returns from function if not found
    cout << endl;
    cout << "Here is an avaliable agent in your city: " << endl;
    string agentQuery = "SELECT * FROM AGENTS WHERE A_CITY = \'" + clientCity + "\'";
    query(agentQuery);  
    // if no agents in the city
    if(resultSet -> rowsCount() == 0) {
        cout << "Sorry, there are no agents currently available in the city " << clientCity << endl;
        return;
    }

    // ask for type of policy. 
    string type;
    cout << "What type of policy would you like to purchase? (Dental, Life, Home, Health, Vehicle)" << endl;
    cin >> type;
    type = processInput(type);
    

    cout << endl;
    cout << "Here is the policy you selected: " << endl;

    string policyQuery = "SELECT * FROM POLICY WHERE TYPE = \'" + type + "\'";
    query(policyQuery); 
    
    // if enter invalid policy name, prompt user again
    while(resultSet -> rowsCount() == 0) {
        cout << "Sorry, " << type << " is an invalid policy! Please re-enter a valid policy (Dental, Life, Home, Health, Vehicle). If you would like to return to the main menu, please enter 0." << endl;
        cin >> type;
        if (type == "0") {
            return;
        }
        type = processInput(type);
        cout << endl;
        cout << "Here is the policy you selected: " << endl;
        policyQuery = "SELECT * FROM POLICY WHERE TYPE = \'" + type + "\'";
        query(policyQuery); 
    }

    // ask for agent name to purchase policy from them
    cout << "Now please enter the agent name you wish to use & the amount (as a decimal number, ex 100.00) for the policy, separated by commas:" << endl;
    string agentName;
    string amount;
    getline(cin, agentName, ',');
    getline(cin, amount);
    agentName = processInput(agentName);

    // add increments the purchase id by 1 and adds the purchase to policies sold
    string add = "(SELECT MAX(PURCHASE_ID) + 1 FROM (SELECT * FROM POLICIES_SOLD) AS p), (SELECT A_ID FROM AGENTS WHERE A_NAME = \'" + agentName + "\'), (SELECT MAX(C_ID) FROM CLIENT), (SELECT POLICY_ID FROM POLICY WHERE TYPE = \'" + type + "\'), STR_TO_DATE('2020-03-24', '%Y-%m-%d')," + amount;
    cout << add << endl;
    insert("POLICIES_SOLD", add);
    query("SELECT * FROM POLICIES_SOLD");
    cout << endl;
    cout << "Successfully purchased your policy for $" << amount << " with agent " << agentName << "!" << endl;
}

// ************************ 3) List all policies sold by a particular agent ************************

void agentPolicies() {
    string agentName;
    string city;
    cout << "Please enter an agent name and a city separated by a comma: " << endl;
    cin.ignore();
    getline(cin, agentName, ',');
    getline(cin, city);
    agentName = processInput(agentName);
    city = processInput(city);
    
    cout << endl;
    cout << "Policies sold by: " << agentName;
    string search = "SELECT * FROM POLICIES_SOLD WHERE AGENT_ID = (SELECT A_ID FROM AGENTS WHERE A_NAME = \'" + agentName +"\' AND A_CITY = \'" + city + "\')";
    query(search);

    // if "invalid" agent name and city combo, prompt user again
    while(resultSet -> rowsCount() == 0) {
        cout << endl;
        cout << "Sorry, " << agentName << " in " << city<< " was either not found, or has not yet sold any policies. Please re-enter the agent and city separated by a comma. Enter 0,0 to return to main menu. " << endl;
        getline(cin, agentName, ',');
        getline(cin, city);
        if (agentName == "0") {
            return;
        }
        agentName = processInput(agentName);
        city = processInput(city);
        cout << endl;
        cout << "Policies sold by: " << agentName;
        string search = "SELECT * FROM POLICIES_SOLD WHERE AGENT_ID = (SELECT A_ID FROM AGENTS WHERE A_NAME = \'" + agentName +"\' AND A_CITY = \'" + city + "\')";
        query(search);
    }

    // display additional info
    cout << "Information about policies sold by : " << agentName;
    string search2 = "SELECT p.NAME, p.COMMISION_PERCENTAGE as COMMISION_PERCENT, ps.AMOUNT, (p.COMMISION_PERCENTAGE/100) * ps.AMOUNT AS AMOUNT_EARNED_COMMISIONS FROM POLICY p, POLICIES_SOLD ps WHERE ps.AGENT_ID = (SELECT A_ID FROM AGENTS WHERE A_NAME = \'" + agentName +"\') AND ps.POLICY_ID = p.POLICY_ID";
    query(search2);
}

// ************************ 4) Cancel a policy ************************

void cancelPolicy() {
    //first display all policies
    cout << "Here are all the current sold policies: " << endl;
    query("SELECT * FROM POLICIES_SOLD");

    // prompt user for a selection
    string purchaseID;
    cout << "Please enter the purchase ID you wish to cancel: " << endl;
    cin >> purchaseID;
    string cancel = "DELETE FROM POLICIES_SOLD WHERE PURCHASE_ID = \'" + purchaseID + "\'";
    bool success = queryModify(cancel);
        
    // if "invalid" purchase iD, prompt user again
    while (!success) {
        cout << "That is an invalid purchase ID! Please re-enter a valid purchase ID, or enter 0 to return to main menu." << endl;
        cin >> purchaseID;
        if (purchaseID == "0") {
            return;
        }
        cancel = "DELETE FROM POLICIES_SOLD WHERE PURCHASE_ID = \'" + purchaseID + "\'";
        success = queryModify(cancel);
    }
    query("SELECT * FROM POLICIES_SOLD");
    cout << "Successfully cancelled your policy!" << endl;

}

// ************************ 5) Add a new agent to a city ************************

void addAgent() {
    // prompt user for agent info
    string name, city, zip;
    cout << "Please enter client information, separated by commas (Name, City, Zip): " << endl;
    cin.ignore();
    getline(cin, name, ',');
    getline(cin, city, ',');
    getline(cin, zip);

    name = processInput(name);
    city = processInput(city);

    // add the agent
    string addAgent = "(SELECT MAX(A_ID) + 1 FROM (SELECT * FROM AGENTS) AS a), \'" + name + "\', \'" + city + "\'," + zip;
    insert("AGENTS(A_ID, A_NAME, A_CITY, A_ZIP)", addAgent);

    query("SELECT * FROM AGENTS");
    cout << endl;
    cout << "Successfully added agent " << name << "!" << endl;
}


// helper method to process user input strings to all caps, no spaces (ex, new york -> NEWYORK)
string processInput(string str) {
    for (int i =0; i < str.length(); i++) {
        str[i] = toupper(str[i]);
        if (isspace(str[i])) {
            str.erase(i--, 1);
        }
    }
    return str;
}

// Connect to the database
sql::Connection* Connect(const string Username, const string Password)
 {
	 try{
	
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", Username, Password);
		}
		 
	catch (sql::SQLException &e) {
        cout << "ERROR: SQLException in " << __FILE__;
        cout << " (" << __func__<< ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;
        }
   return con;
}

// Disconnect from the database
void disconnect()
{
        // assumes tables already exists & delete content & remove tables
        statement->executeUpdate("DELETE from POLICIES_SOLD");
        statement->executeUpdate("DELETE from CLIENT");
        statement->executeUpdate("DELETE from AGENTS");
        statement->executeUpdate("DELETE from POLICY");

        statement->executeUpdate("DROP TABLE POLICIES_SOLD");
        statement->executeUpdate("DROP TABLE CLIENT");
        statement->executeUpdate("DROP TABLE AGENTS");
        statement->executeUpdate("DROP TABLE POLICY");
        
		delete resultSet;
		delete statement;
		con -> close();
		delete con;
} 

// Execute an SQL query passed in as a string parameter
// and print the resulting relation
void query (string q)
{
        try {
            resultSet = statement->executeQuery(q);
            if (resultSet -> rowsCount() != 0) {
                cout<<("\n------------------------------------------------------------------\n");
                //cout<<("Query: \n" + q + "\n\nResult: \n");
                    print(resultSet);
                cout<<("------------------------------------------------------------------\n");
            }
        }
        catch (sql::SQLException e) {
            cout << "ERROR: SQLException in " << __FILE__;
            cout << " (" << __func__<< ") on line " << __LINE__ << endl;
            cout << "ERROR: " << e.what();
            cout << " (MySQL error code: " << e.getErrorCode();
            cout << ", SQLState: " << e.getSQLState() << ")" << endl;
        }
}

// additional query method to execute updates
bool queryModify (string q)
{
    // retrieve # of affected rows from query  
    int rows = statement->executeUpdate(q);
    // if no rows affected, then "invalid" user input (when deleting a tuple)
    if (rows == 0) {
        return false;
    }
    return true;
       
}


// Print the results of a query with attribute names on the first line
// Followed by the tuples, one per line
void print (sql::ResultSet *resultSet) 
{
    try{
		if (resultSet -> rowsCount() != 0)
		{
   		   sql::ResultSetMetaData *metaData = resultSet->getMetaData();
           int numColumns = metaData->getColumnCount();
		   printHeader( metaData, numColumns);
           printRecords( resultSet, numColumns);
		}
        else {
			throw runtime_error("Error, nothing found! ):");
        }
    }
	catch (std::runtime_error &e) {}
    
}	

// Print the attribute names
void printHeader(sql::ResultSetMetaData *metaData, int numColumns)
{    
	/*Printing Column names*/  
    for (int i = 1; i <= numColumns; i++) {
            if (i > 1)
               cout<<"  ";
            cout<<  setw(17) << left <<  metaData->getColumnLabel(i); //ColumnName
        }
        cout<<endl;
}		

// Print the attribute values for all tuples in the result
void printRecords(sql::ResultSet *resultSet, int numColumns)   
{ 
        while (resultSet->next()) {
            for (int i = 1; i <= numColumns; i++) {
                if (i > 1)
                    cout<<"  ";
                cout<< setw(17) << left << resultSet->getString(i);
               ;
            }
        cout<<endl;
        }
}
 
// Insert into any table, any values from data passed in as String parameters
void insert(const string table, const string values) 
{
    string query = "INSERT into " + table + " values (" + values + ")";
    statement->executeUpdate(query);
}

 

// Remove all records and fill them with values for testing
// Assumes that the tables are already created
void initDatabase(const string Username, const string Password, const string SchemaName) 
{
        // Create a connection 
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", Username, Password);

        // Connect to the MySQL test database 
        con->setSchema(SchemaName);
 
        statement = con->createStatement();
        

        // create tables
        statement->executeUpdate("CREATE TABLE CLIENT(C_ID int, C_NAME char(50) NOT NULL, C_CITY char(50) NOT NULL, C_ZIP int NOT NULL, PRIMARY KEY (C_ID))");
        statement->executeUpdate("CREATE TABLE AGENTS(A_ID int, A_NAME char(50) NOT NULL, A_CITY char(50) NOT NULL, A_ZIP int NOT NULL, PRIMARY KEY (A_ID))");
        statement->executeUpdate("CREATE TABLE POLICY(POLICY_ID int, NAME char(50) NOT NULL, TYPE char(50) NOT NULL, COMMISION_PERCENTAGE int NOT NULL, PRIMARY KEY (POLICY_ID))");
        statement->executeUpdate("CREATE TABLE POLICIES_SOLD (PURCHASE_ID int, AGENT_ID int NOT NULL, CLIENT_ID int NOT NULL, POLICY_ID int NOT NULL, DATE_PURCHASED date NOT NULL, AMOUNT dec(6,2) NOT NULL, PRIMARY KEY (PURCHASE_ID), FOREIGN KEY (AGENT_ID) REFERENCES AGENTS(A_ID) ON DELETE RESTRICT, FOREIGN KEY (CLIENT_ID) REFERENCES CLIENT(C_ID) ON DELETE RESTRICT, FOREIGN KEY (POLICY_ID) REFERENCES POLICY(POLICY_ID) ON DELETE RESTRICT)");

        // populate CLIENT table
        insert("CLIENT", "101, 'CHRIS', 'DALLAS', 43214");
        insert("CLIENT", "102, 'OLIVIA', 'BOSTON', 83125");
        insert("CLIENT", "103, 'ETHAN', 'FAYETTEVILLE', 72701");
        insert("CLIENT", "104, 'DANIEL', 'NEWYORK', 53421");
        insert("CLIENT", "105, 'TAYLOR', 'ROGERS', 78291");
        insert("CLIENT", "106, 'CLAIRE', 'PHEONIX', 85011");

        insert("AGENTS", "201, 'ANDREW', 'DALLAS', 43214");
        insert("AGENTS", "202, 'PHILIP', 'PHEONIX', 85011");
        insert("AGENTS", "203, 'JERRY', 'BOSTON', 83125");
        insert("AGENTS", "204, 'BRYAN', 'ROGERS', 78291");
        insert("AGENTS", "205, 'TAMMY', 'DALLAS', 43214");
        insert("AGENTS", "206, 'BRANT', 'FAYETTEVILLE', 72701");
        insert("AGENTS", "207, 'SMITH', 'ROGERS', 78291");

        insert("POLICY", "301, 'CIGNAHEALTH', 'DENTAL', 5");
        insert("POLICY", "302, 'GOLD', 'LIFE', 8");
        insert("POLICY", "303, 'WELLCARE', 'HOME', 10");
        insert("POLICY", "304, 'UNITEDHEALTH', 'HEALTH', 7");
        insert("POLICY", "305, 'UNITEDCAR', 'VEHICLE', 9");

        insert("POLICIES_SOLD", "401, 204, 106, 303, STR_TO_DATE('2020-01-02', '%Y-%m-%d'), 2000.00");
        insert("POLICIES_SOLD", "402, 201, 105, 305, STR_TO_DATE('2019-08-11', '%Y-%m-%d'), 1500.00");
        insert("POLICIES_SOLD", "403, 203, 106, 301, STR_TO_DATE('2019-09-08', '%Y-%m-%d'), 3000.00");
        insert("POLICIES_SOLD", "404, 207, 101, 305, STR_TO_DATE('2019-06-21', '%Y-%m-%d'), 1500.00");
        insert("POLICIES_SOLD", "405, 203, 104, 302, STR_TO_DATE('2019-11-14', '%Y-%m-%d'), 4500.00");
        insert("POLICIES_SOLD", "406, 207, 105, 305, STR_TO_DATE('2019-12-25', '%Y-%m-%d'), 1500.00");
        insert("POLICIES_SOLD", "407, 205, 103, 304, STR_TO_DATE('2020-10-15', '%Y-%m-%d'), 5000.00");
        insert("POLICIES_SOLD", "408, 204, 103, 304, STR_TO_DATE('2020-02-15', '%Y-%m-%d'), 5000.00");
        insert("POLICIES_SOLD", "409, 203, 103, 304, STR_TO_DATE('2020-01-10', '%Y-%m-%d'), 5000.00");
        insert("POLICIES_SOLD", "410, 202, 103, 303, STR_TO_DATE('2020-01-30', '%Y-%m-%d'), 2000.00");

}