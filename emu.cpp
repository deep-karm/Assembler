#include <bits/stdc++.h>
using namespace std;

#define MEM_SIZE 10000
vector<int32_t> mem(MEM_SIZE + 1); // declaring the size of memory
int A = 0, B = 0, SP = MEM_SIZE;   // sp starts from top and decrements

int totallines = 0;
bool trace = false;

// converts the whole string to lower case
string str_low(string s)
{
    string ans;
    for (auto c : s)
        ans.push_back(tolower(c));
    return ans;
}

// converts the decimal to a hexadecimal string
string int_hexstring(int n)
{
    string ans = "";
    stringstream ss;
    ss << hex << n;
    ans = ss.str(); // return 8 char if negative
    while (ans.length() < 8)
        ans = '0' + ans;
    return ans;
}

// outputs the operand(decimal) from the instructions
int get_operand(int32_t x)
{
    string s = int_hexstring(x).substr(0, 6) + "00";
    int n;
    sscanf(s.c_str(), "%x", &n);
    return n >> 8;
}

// print memory dump
void dump()
{
    cout << "memory dump:\n";
    for (int i = 0; i < totallines; i++)
    {
        string line;
        if (!(i % 4))
            cout << int_hexstring(i) << " ";
        cout << int_hexstring(mem[i]);
        if (i % 4 == 3 || i == totallines - 1)
            cout << endl;
        else
            cout << " ";
    }
}

// print trace is required
void print_trace(int pc)
{
    if (trace)
    {
        cout << "PC = " << int_hexstring(pc) << ", SP = " << int_hexstring(SP) << ", A = " << int_hexstring(A) << ",B = " << int_hexstring(B);
        map<int, string> opcode;
        opcode[0] = "ldc";
        opcode[1] = "adc";
        opcode[2] = "ldl";
        opcode[3] = "stl";
        opcode[4] = "ldnl";
        opcode[5] = "stnl";
        opcode[6] = "add";
        opcode[7] = "sub";
        opcode[8] = "shl";
        opcode[9] = "shr";
        opcode[10] = "adj";
        opcode[11] = "a2sp";
        opcode[13] = "call";
        opcode[12] = "sp2a";
        opcode[14] = "return";
        opcode[15] = "brz";
        opcode[16] = "brlz";
        opcode[17] = "br";
        opcode[18] = "halt";
        opcode[19] = "data";
        opcode[20] = "set";
        int opc = mem[pc] & 0xff;
        int opr = get_operand(mem[pc]);
        cout << " " << opcode[opc] << " " << opr << endl;
    }
}

void print_isa()
{
    cout << "Mnemonic\tOpcode\n"
         << "ldc\t\t0\n"
         << "adc\t\t1\n"
         << "ldl\t\t2\n"
         << "stl\t\t3\n"
         << "ldnl\t\t4\n"
         << "stnl\t\t5\n"
         << "add\t\t6\n"
         << "sub\t\t7\n"
         << "shl\t\t8\n"
         << "shr\t\t9\n"
         << "adj\t\t10\n"
         << "a2sp\t\t11\n"
         << "sp2a\t\t12\n"
         << "call\t\t13\n"
         << "return\t\t14\n"
         << "brz\t\t15\n"
         << "brlz\t\t16\n"
         << "br\t\t17\n"
         << "HALT\t\t18\n"
         << "SET\t\t19\n"
         << "data\t\t20\n";
}

// execution of the file
int run()
{
    int PC = 0;
    while (true)
    {
        int curr_pc = PC;
        int opcode = mem[PC] & 0xff;
        int operand = get_operand(mem[PC]);
        print_trace(PC);
        // carring out the operations by their opcode
        if (opcode == 0)
            B = A, A = operand;
        else if (opcode == 1)
            A += operand;
        else if (opcode == 2)
            B = A, A = mem[SP + operand];
        else if (opcode == 3)
            mem[SP + operand] = A, A = B;
        else if (opcode == 4)
            A = mem[A + operand];
        else if (opcode == 5)
            mem[A + operand] = B;
        else if (opcode == 6)
            A += B;
        else if (opcode == 7)
            A = B - A;
        else if (opcode == 8)
            A = B << A;
        else if (opcode == 9)
            A = B >> A;
        else if (opcode == 10)
            SP = SP + operand;
        else if (opcode == 11)
            SP = A, A = B;
        else if (opcode == 12)
            B = A, A = SP;
        else if (opcode == 13)
            B = A, A = PC, PC += operand;
        else if (opcode == 14)
            PC = A, A = B;
        else if (opcode == 15)
        {
            if (A == 0)
                PC = PC + operand;
        }
        else if (opcode == 16)
        {
            if (A < 0)
            {
                PC = PC + operand;
            }
        }
        else if (opcode == 17)
            PC = PC + operand;
        else if (opcode == 18)
            return 0;
        PC++;
        if (curr_pc == PC)
        {
            cout << "the program jumping to same line \nno operation available to come out of this loop";
            return 0;
        }
        if (PC > MEM_SIZE)
        {
            cout << "Memory limit exceed";
            return 0;
        }
        if (PC > totallines) // if all the lines of code are executed
            break;
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cout << "enter in the following format:\n ./emu.exe -option filename.o\noptions:\n";
        cout << "\tbefore: memory dump  before execution\n";
        cout << "\tafter: memory dump  after execution\n";
        cout << "\tisa: ISA\n";
        cout << "\ttrace: trace the execution\n";
        return 0;
    }

    string filename = argv[2];
    ifstream objfile(filename, ios::binary | ios::in); // reading the binary object file.

    string func = argv[1]; // option
    if (str_low(func) == "-isa")
        print_isa();
    else
    {
        if (str_low(func) == "-trace")
            trace = true;
        int i = 0;
        // reading the object file
        int32_t temp;
        while (objfile.read((char *)&temp, sizeof(temp)))
        {
            mem[i] = temp;
            string line = int_hexstring(temp);
            // cout << (temp & 0xff) << endl;
            if ((temp & 0xff) == 19 || (temp & 0xff) == 20)
                mem[i] >>= 8;
            i++;
        }
        totallines = i; // total lines in the code loaded
        if (str_low(func) == "-before")
            dump();
        else
            run();
        if (str_low(func) == "-after")
            dump();
    }

    objfile.close();
    return 0;
}