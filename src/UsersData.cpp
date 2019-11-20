#include "UsersData.h"

using namespace std;

UsersData::UsersData()
{
}

UsersData::~UsersData()
{
}

void UsersData::readFromFile()
{
    ifstream in(USERDATAFILEPATH);
    if (in)
    {
        string username;
        string fileId;
        while (getline(in, username) && getline(in, fileId))
            data[username] = fileId;
    }
}

void UsersData::saveToFile()
{
    ofstream out(USERDATAFILEPATH);
    for (auto &user : data)
    {
        out << user.first << endl;
        out << user.second << endl;
    }
}

