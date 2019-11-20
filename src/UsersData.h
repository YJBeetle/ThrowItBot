#include <fstream>
#include <string>
#include <unordered_map>

#define USERDATAFILEPATH "UsersData.txt"

class UsersData
{
private:
 
public:
std::unordered_map<std::string, std::string> data;

    UsersData();
    ~UsersData();

    void readFromFile();
    void saveToFile();


    inline void set(const std::string &user,const std::string &fileId)
    {
    data[user] = fileId;
    saveToFile();
    }

};
