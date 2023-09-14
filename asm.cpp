#include <bits/stdc++.h>
#include <time.h>
using namespace std;

map<string, pair<int, int>> lables;  // name, address, value
map<string, bool> lables_used;       // name, address, value
map<string, string> opcode;          // name, opcode
vector<string> inter_code;           // intermidiate code: pc code
unordered_map<string, int> op_off;   // opcode that takes offset as operand
unordered_map<string, int> op_value; // opcode that takes value as operand

string WHITESPACE = " \n\r\t\f\v";
// string to lower
string str_low(string s)
{
    string ans;
    for (auto c : s)
        ans.push_back(tolower(c));
    return ans;
}

// check whether a decimal
bool is_deci(string s)
{
    string decimal = "0123456789";
    if (!(s.length() > 0 && (s[0] == '+' || s[0] == '-' || isdigit(s[0]))))
        return false;
    for (int i = 1; i < s.length(); i++)
        if (find(decimal.begin(), decimal.end(), s[i]) == decimal.end())
            return false;
    return true;
}

// check whether a hexadecimal
bool is_hex(string s)
{
    string hexa = "0123456789abcdefABCDEF";
    if (s.length() < 2 || s[0] != '0' || s[1] != 'x')
        return false;
    for (int i = 2; i < s.length(); i++)
        if (find(hexa.begin(), hexa.end(), s[i]) == hexa.end())
            return false;
    return true;
}

// check whether a octal
bool is_oct(string s)
{
    string oct = "01234567";
    if (s.length() < 2 || s[0] != '0' || s[1] != 'o')
        return false;
    for (int i = 2; i < s.length(); i++)
        if (find(oct.begin(), oct.end(), s[i]) == oct.end())
            return false;
    return true;
}

// check whether a number(hex or deci or octal)
bool is_num(string s)
{
    return is_deci(s) || is_hex(s) || is_oct(s);
}

// find a char in a string, returns index
int find_c(string s, char c)
{
    return find(s.begin(), s.end(), c) - s.begin();
}

// converts string to a decimal
int str_int(string s)
{
    if (is_deci(s))
        return stoi(s, nullptr, 10);
    if (is_oct(s))
        return stoi(s, nullptr, 8);
    if (is_hex(s))
        return stoi(s, nullptr, 16);
    return 0;
}

// converts decimal to hexadecimal string
string int_to_hexstring(int n)
{
    string ans = "";
    stringstream ss;
    ss << hex << n;
    ans = ss.str(); // return 8 char if negative
    while (ans.length() < 8)
        ans = '0' + ans;
    return ans;
}

void opcode_table()
{
    opcode["ldc"] = "0";
    opcode["adc"] = "1";
    opcode["ldl"] = "2";
    opcode["stl"] = "3";
    opcode["ldnl"] = "4";
    opcode["stnl"] = "5";
    opcode["add"] = "6";
    opcode["sub"] = "7";
    opcode["shl"] = "8";
    opcode["shr"] = "9";
    opcode["adj"] = "10";
    opcode["a2sp"] = "11";
    opcode["call"] = "13";
    opcode["sp2a"] = "12";
    opcode["return"] = "14";
    opcode["brz"] = "15";
    opcode["brlz"] = "16";
    opcode["br"] = "17";
    opcode["halt"] = "18";
    opcode["data"] = "19";
    opcode["set"] = "20";
    op_value["data"] = 1, op_value["adc"] = 1, op_value["ldc"] = 1, op_value["adj"] = 1, op_value["set"] = 1;
    op_off["ldl"] = 1, op_off["stl"] = 1, op_off["ldnl"] = 1, op_off["stnl"] = 1, op_off["call"] = 1, op_off["brz"] = 1, op_off["brlz"] = 1, op_off["br"] = 1;
    for (auto &op : opcode) // storing the hexadecimal in the map
        op.second = int_to_hexstring(str_int(op.second));
}

// checks whether a valid lable name
bool valid_lable(string s)
{
    if (lables.find(s) != lables.end())
        return false;
    if (!isalpha(s[0]))
        return false;
    for (auto c : s)
        if (!isalnum(c))
            return false;
    return true;
}

// removes whitespaces on the left and right of a string
void trim(string &s)
{
    int start = s.find_first_not_of(WHITESPACE);
    s = (start == std::string::npos) ? "" : s.substr(start);
    size_t end = s.find_last_not_of(WHITESPACE);
    s = (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

// creates the object file using the listing file
void create_objfile(string filename)
{
    ofstream objfile(filename + ".o", ios::binary | ios::out); // object file with same name and .o extension
    ifstream Lfile(filename + ".L");
    string line;
    while (getline(Lfile, line))
    {
        int space = find_c(line, ' ');
        if (space >= line.length())
            continue;
        // takes the word after first space of every line
        line = line.substr(space + 1, 8);

        // line is ignored is its only spaces
        if (line == "        ")
            continue;

        // line is ignored is its not a hexadecimal number
        if (!is_hex("0x" + line))
            continue;

        // cout << line << " ";

        // hexadecimal string converted to a unsigned 32 bit to be converted to binary
        uint32_t obj = stoul(line, nullptr, 16);
        // cout << obj << endl;
        objfile.write((char *)&obj, sizeof(obj));
    }
    objfile.close();
    Lfile.close();
}

void pass_1(string filename)
{
    string line;
    string in_file_name = filename + ".asm";
    string err_file_name = filename + ".log";
    ifstream in_file(in_file_name);
    ofstream err_file(err_file_name);

    int pc = 0; // program counter
    while (getline(in_file, line))
    {
        string inter_line = "";

        // find location of first semi-colon and ingnore string after that.
        int semi_colon = find(line.begin(), line.end(), ';') - line.begin();
        line = line.substr(0, semi_colon);

        if (line.empty())
            continue;
        trim(line);
        int colon = find(line.begin(), line.end(), ':') - line.begin();

        // if there exists atleast one colon
        if (colon != line.length())
        {
            string pre_colon = line.substr(0, colon);                        // lable
            string post_colon = line.substr(colon + 1, line.size() - colon); // code after lable
            trim(pre_colon);
            trim(post_colon);

            if (!valid_lable(pre_colon))
            {
                cout << "Error in lable: " << pre_colon << ", either duplicate or wrong format for a lable\n";
                err_file << "Error in lable: " << pre_colon << ", either duplicate or wrong format for a lable\n";
            }

            lables[pre_colon] = {pc, INT_MAX}; // saving the lable with its address and default data
            if (lables_used.find(pre_colon) == lables_used.end())
                lables_used[pre_colon] = false; // to track unused lables
            inter_line = to_string(pc) + " " + pre_colon + " : ";

            // if the lable is set then the address is changed
            if (post_colon.substr(0, 3) == "SET")
            {
                istringstream ss(post_colon.substr(3));
                string num;
                ss >> num;
                if (!is_num(num))
                {
                    cout << "Error: " << num << " not a valid number\n";
                    err_file << "Error: " << num << " not a valid number\n";
                }

                int n = str_int(num);                              // convert number in any format to decimal
                lables[pre_colon] = {n, lables[pre_colon].second}; // updating the lable
                inter_line += post_colon;
                post_colon = "";
                pc++;
            }

            else if (post_colon.substr(0, 4) == "data")
            {
                istringstream ss(post_colon.substr(4));
                string num;
                ss >> num;
                // error not number
                int n = stoi(num); // could be hex
                lables[pre_colon] = {lables[pre_colon].first, n};
                inter_line += post_colon;
                post_colon = "";
                pc++;
            }
            inter_code.push_back(inter_line); // code push_back into the intermediate code with the pc
            inter_line = "";                  // reset the temp line but has the same pc
            line = post_colon;                // if any commnands left after the colon
        }

        if (line.empty())
            continue;

        // if there are no lable in the line
        int space = find_c(line, ' ');
        if (space == line.length())
        {
            line = str_low(line);
            if (!(op_off.find(line) == op_off.end() && op_value.find(line) == op_value.end() && opcode.find(line) != opcode.end()))
            {
                cout << "Error: the opcode doesn't accept any opreand\n";
                err_file << "Error: the opcode doesn't accept any opreand\n";
            }
            if (op_off.find(line) == op_off.end() && op_value.find(line) == op_value.end() && opcode.find(line) == opcode.end())
            {
                cout << "Error: no such mnemonics:" << line << "\n";
                err_file << "Error: no such mnemonics:" << line << "\n";
            }
        }
        else
        {
            string mnemonic = line.substr(0, space), opreand = line.substr(space + 1);
            mnemonic = str_low(mnemonic);
            if (op_off.find(mnemonic) == op_off.end() && op_value.find(mnemonic) == op_value.end() && opcode.find(mnemonic) != opcode.end())
            {
                cout << "Error: wrong type of opcode\n";
                err_file << "Error: wrong type of opcode\n";
            }
            if (op_off.find(mnemonic) == op_off.end() && op_value.find(mnemonic) == op_value.end() && opcode.find(mnemonic) == opcode.end())
            {
                cout << "Error: no such mnemonics:" << mnemonic << "\n";
                err_file << "Error: no such mnemonics:" << mnemonic << "\n";
            }
            // the opreand is not number, it is considered as lable, that is has been used once.
            if (!is_num(opreand))
                lables_used[opreand] = true;
        }
        inter_line = to_string(pc) + " " + line;
        inter_code.push_back(inter_line);

        // check whether other statements are correct
        pc++;
    }

    // to detect undeclared lables
    for (auto l : lables_used)
    {
        if (lables.find(l.first) == lables.end())
        {
            cout << "Error: " << line << " has been used as an opreand but is not declared as a lable\n";
            err_file << "Error: " << line << " has been used as an opreand but is not declared as a lable\n";
        }
    }

    // to detect unused opreand
    for (auto l : lables)
    {
        if (!lables_used[l.first])
        {
            cout << "Warning: " << l.first << " hasn't been used as a opreand\n";
            err_file << "Warning: " << l.first << " hasn't been used as a opreand\n";
        }
    }

    in_file.close();
    err_file.close();
}

void pass_2(string filename)
{
    string err_file_name = filename + ".log";
    string L_file_name = filename + ".L";
    fstream err_file(err_file_name);
    ofstream Lfile(L_file_name);
    Lfile.clear();

    for (auto line : inter_code)
    {
        trim(line);
        string pc = "", maccode = "", code = ""; // PC, machine code and high level code

        int space = find_c(line, ' '), pc_int;
        pc = line.substr(0, space); // extracting the PC fro the intermediate code

        pc_int = str_int(pc);
        pc = int_to_hexstring(str_int(pc));   // program counter as a string in hexadecimal
        code = line = line.substr(space + 1); // rest of the high level code

        // if line starts with a lable
        if (find_c(line, ':') < line.length())
        {
            // if the line only has a lable
            if (find_c(line, ':') == line.length() - 1)
                maccode = "        ";

            // if the line has code after lable, line is updated for further execution
            else
                line = line.substr(find_c(line, ':') + 1), trim(line);
        }

        // code after removing the lable or if doesn't have a lable
        if (find_c(line, ':') == line.length())
        {
            int space = find_c(line, ' ');

            // if the mnemonic doesn't take any input
            if (space == line.length())
            {
                string mnemonic = line;
                maccode = opcode[str_low(mnemonic)]; // machince code will just have the opcode
            }

            // if there are more characters after mnemonic
            else
            {
                string mnemonic = line.substr(0, space);
                mnemonic = str_low(mnemonic);

                string operand = line.substr(space);
                trim(mnemonic), trim(operand);

                // if the operand is a lable
                if (lables.find(operand) != lables.end())
                {
                    // if the mnemonic takes the opreand as a value
                    if (op_value.find(mnemonic) != op_value.end())
                    {
                        maccode = int_to_hexstring(lables[operand].first).substr(2, 6); // first 6 bits are the opreand
                        maccode = maccode + opcode[mnemonic].substr(6, 2);              // last 2 bits are the opcode
                    }

                    // if the mnemonic takes the opreand as an offset
                    else if (op_off.find(mnemonic) != op_off.end())
                    {
                        int offset = lables[operand].first - (pc_int + 1);
                        maccode = int_to_hexstring(offset).substr(2, 6); // address store in the lable
                        maccode = maccode + opcode[mnemonic].substr(6, 2);
                    }
                }

                // for numerical operands
                else if (is_num(operand))
                {
                    maccode = int_to_hexstring(str_int(operand)).substr(2, 6);
                    maccode = maccode + opcode[mnemonic].substr(6, 2);
                }
            }
        }
        // cout << pc << " " << maccode << " " << code << endl;
        // the data stored  in the list file
        Lfile << pc << " " << maccode << " " << code << endl;
    }
    // object file is created using the data saved in the list file
    create_objfile(filename);

    err_file.close();
    Lfile.close();
}

int main(int argc, char **argv)
{
    clock_t tStart = clock();

    if (argc < 2)
    {
        cout << "Error: No input file\n";
        cout << "/asm.exe filename.asm\n";
        return 0;
    }
    string in_file_name = argv[1];

    if (in_file_name[0] = '.' && in_file_name[1] == '\\')
        in_file_name = in_file_name.substr(2);

    ifstream file(in_file_name);
    if (!file)
    {
        cout << "Error: couldn't open the input file\n";
        cout << ": /asm.exe filename.asm\n";
        return 0;
    }

    // name of all the files to be created
    string filename = in_file_name.substr(0, in_file_name.find("."));
    opcode_table();

    pass_1(filename);
    pass_2(filename);

    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    return 0;
}