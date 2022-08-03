#include <iostream>

#include <string>

#include <fstream>

#include <sstream>

#include <vector>

#include <map>

#include <queue>

#include <iomanip>

#include <math.h>>


//#include "reservationStation.hpp"

using namespace std;

string tobinary(int temp);
bool branch_issued = false;
struct inst

{

    string op = "";

    int Rd = 0, RS = 0, RT = 0, imm = 0;

    string label = "";

    string Label_op = "";

};
struct Rs

{

    string name = "";

    bool busy = false;

    // string op = "";

    string op;

    int Vj = 0, Vk = 0, Qj = 0, Qk = 0, A = 0;

    int index = -1;


};
void issuing();
void execute();
void Writeback();
void writingbackReally(int i, string result);
int todecimal(string bin);
class RsrvStations

{

public:

    vector <Rs> reservations = vector<Rs>(12);

    RsrvStations()

    {
        reservations[0].name = "LOAD";

        reservations[1].name = "LOAD";

        reservations[2].name = "STORE";

        reservations[3].name = "STORE";

        reservations[4].name = "BEQ";

        reservations[5].name = "JAL/RET";

        reservations[6].name = "ADD/ADDI";

        reservations[7].name = "ADD/ADDI";

        reservations[8].name = "ADD/ADDI";

        reservations[9].name = "NEG";

        reservations[10].name = "NAND";

        reservations[11].name = "MUL";

    }

};


struct tracking

{

    int index;

    int issued, started_execution, finished_execution, write_back;
    inst my_inst;
    string my_inst_string;

};

vector<tracking> Track = vector<tracking>(0);
tracking temp;

int RegisterStat[8] = { -1,-1,-1,-1,-1,-1,-1,-1 };
vector<string> Regs(8, "0000000000000000");
vector<int> will_flush;

int clk = 0;

int current_inst = 0;

int branches_encountered = 0;

int branches_mispredicted = 0;

int branch_index = current_inst;

RsrvStations RS;

queue <int> load_store_queue;

map<int, string> dataMem;

bool RS_empty()
{
    for (int i = 0; i < RS.reservations.size(); i++)
    {
        if (RS.reservations[i].busy == true)
        {
            return false;
        }
    }
    return true;
}

vector<inst> instructions;
vector<string> instructions_strings;

int get_label_location(string x)
{
    int y;
    for (int i = 0; i < instructions.size(); i++)
    {
        if (x == instructions[i].Label_op)
            y=i;
    }
    
    return y;
}

void flush()
{



    for (int i = 0; i < will_flush.size(); i++)
    {
        int r = will_flush[i];

        RS.reservations[r].A = 0;
        RS.reservations[r].busy = false;
        RS.reservations[r].Vj = 0;
        RS.reservations[r].Vk = 0;
        RS.reservations[r].Qj = 0;
        RS.reservations[r].Qk = 0;
        RS.reservations[r].op = "";
        RS.reservations[r].index = -1;
        Track.pop_back();
    }


    will_flush.clear();
}

int main() {

    string temp, line;

    ifstream file;

    file.open("/Users/macbookair/CA_project/instructions.txt", ios::in);
    //file.open("instructions.txt");

    if (!file) {

        cout << "Error: file does not be opened" << endl;

        exit(1);

    }

    while (getline(file, temp))
    {

        inst tempInst;

        line = temp;
        instructions_strings.push_back(line);

        bool flag = true;

        tempInst.op = line.substr(0, line.find(" "));

        do

        {

            if (tempInst.op == "LOAD")

            {

                tempInst.Rd = stoi(line.substr(line.find("r") + 1, 1));

                tempInst.imm = stoi(line.substr(line.find(",") + 1, line.find("(") - line.find(",")));

                tempInst.RS = stoi(line.substr(line.find("(") + 2, 1));

                flag = false;

            }

            else

                if (tempInst.op == "STORE")

                {

                    tempInst.RS = stoi(line.substr(line.find("r") + 1, 1));

                    tempInst.imm = stoi(line.substr(line.find(",") + 1, line.find("(") - line.find(",")));

                    tempInst.Rd = stoi(line.substr(line.find("(") + 2, 1));

                    flag = false;

                }

                else

                    if (tempInst.op == "ADDI" || tempInst.op == "ADD" || tempInst.op == "NAND" || tempInst.op == "MUL")

                    {
                        tempInst.Rd = stoi(line.substr(line.find("r") + 1, 1));

                        tempInst.RS = stoi(line.substr(line.find(",") + 2, 1));

                        if (tempInst.op == "ADDI")

                            tempInst.imm = stoi(line.substr(line.find(",") + 4, line.size() - line.find(",") + 4));

                        else

                            tempInst.RT = stoi(line.substr(line.find(",") + 5, 1));

                        flag = false;

                    }

                    else

                        if (tempInst.op == "BEQ")

                        {
                            tempInst.RS = stoi(line.substr(line.find("r") + 1, 1));

                            tempInst.RT = stoi(line.substr(line.find(",") + 2, 1));

                            tempInst.label = line.substr(line.find(",") + 4, line.size() - line.find(",") + 4);

                            flag = false;

                        }
                        else
                            if (tempInst.op == "JAL")

                            {
                                tempInst.RS = 1;

                                tempInst.label = line.substr(line.find(" ") + 1, line.size() - line.find(" ") + 1);

                                flag = false;

                            }

                            else

                                if (tempInst.op == "RET")

                                {

                                    tempInst.Rd = 1;

                                    flag = false;

                                }

                                else

                                    if (tempInst.op == "NEG")

                                    {
                                        tempInst.Rd = stoi(line.substr(line.find("r") + 1, 1));

                                        tempInst.RS = stoi(line.substr(line.find(",") + 2, 1));

                                        flag = false;

                                    }

                                    else
                                    {
                                        tempInst.Label_op = line.substr(0, line.find(":"));

                                        tempInst.op = line.substr(line.find(":") + 1, line.find(" ") - line.find(":") - 1);

                                        flag = true;
                                    }

        } while (flag);



        instructions.push_back(tempInst);

    }
    /*
    cout << instructions[10].op << endl;
    cout << instructions[10].Rd << endl;
    cout << instructions[10].RS << endl;
    cout << instructions[10].RT << endl;
    cout << instructions[10].label << endl;
    cout << instructions[10].imm << endl;
    cout << instructions[10].Label_op << endl;*/

    
    string temp2, line2;

    ifstream memoryInit;

    memoryInit.open("/Users/macbookair/CA_project/memory.txt", ios::in);
    //file.open("instructions.txt");

    if (!memoryInit) {

        cout << "Error: memory file does not be opened" << endl;

        exit(1);

    }

    while (getline(memoryInit, temp2))
    {
        int location;
        string value;
        line2 = temp2;
        location= stoi(line2.substr(line2.find("[")+1,line2.find("]")-4));
        value=line2.substr(line2.find("=")+1,line2.size());
        dataMem[location]=value;
    }
    
    while (current_inst < instructions.size() || RS_empty() == false)
    {
        issuing();
        execute();
        Writeback();
        clk++;
    }
    cout.setf(ios::left);
    cout << setw(20) << "name" << setw(20) << "issuing" << setw(25) << "starting execution" << setw(25) << "finished execution" << setw(25) << "write back" << endl;
    for (int i = 0; i < Track.size(); i++)
        cout << setw(20) << Track[i].my_inst_string << setw(20) << Track[i].issued << setw(25) << Track[i].started_execution << setw(25) << Track[i].finished_execution << setw(25) << Track[i].write_back << endl;
    
    
    double percentage = 0.0;
    
    if (branches_encountered!=0)
        percentage =  (static_cast<double>(branches_mispredicted)/branches_encountered)*100;
    
    double IPC = static_cast<double> (Track.size())/clk;
    
    cout<<endl<<"The total execution time: "<< clk<< endl;
    cout<<"The IPC: "<<setprecision(3)<<IPC<<endl;
    cout<<"The branch mispredicted: "<< percentage<<"%"<<endl;

    return 0;

}

void issuing()
{


    if (current_inst < instructions.size())
    {
        int r = -1;

        bool flag = false;

        for (int i = 0; i < 12; i++)

        {

            if (instructions[current_inst].op == RS.reservations[i].name && RS.reservations[i].busy == false)

            {

                flag = true;
                r = i;

            }

            if ((instructions[current_inst].op == "ADD" || instructions[current_inst].op == "ADDI") && RS.reservations[i].name == "ADD/ADDI" && RS.reservations[i].busy == false)
            {
                flag = true;
                r = i;
            }

            if ((instructions[current_inst].op == "JAL" || instructions[current_inst].op == "RET") && RS.reservations[5].busy == false)
            {
                flag = true;
                r = 5;
            }
        }

        if (flag)
        {
            if (instructions[current_inst].op == "LOAD")
            {
                if (RegisterStat[instructions[current_inst].RS] != -1)
                    RS.reservations[r].Qj = RegisterStat[instructions[current_inst].RS];
                else
                {
                    RS.reservations[r].Vj = todecimal(Regs[instructions[current_inst].RS]);
                    RS.reservations[r].Qj = 0;
                }
                RS.reservations[r].A = instructions[current_inst].imm;
                RS.reservations[r].busy = true;
                RS.reservations[r].op = instructions[current_inst].op;
                RegisterStat[instructions[current_inst].Rd] = r;
                temp.index = current_inst;
                temp.my_inst = instructions[current_inst];
                temp.my_inst_string = instructions_strings[current_inst];
                temp.issued = clk;
                load_store_queue.push(current_inst);
                if (branch_issued == true)
                    will_flush.push_back(r);

            }
            else
                if (instructions[current_inst].op == "STORE")
                {
                    if (RegisterStat[instructions[current_inst].RS] != -1)
                        RS.reservations[r].Qj = RegisterStat[instructions[current_inst].RS];
                    else
                    {
                        RS.reservations[r].Vj = todecimal(Regs[instructions[current_inst].RS]);
                        RS.reservations[r].Qj = 0;
                    }
                    RS.reservations[r].A = instructions[current_inst].imm;
                    RS.reservations[r].busy = true;

                    if (RegisterStat[instructions[current_inst].RT] != -1)
                        RS.reservations[r].Qk = RegisterStat[instructions[current_inst].RT];
                    else
                    {
                        RS.reservations[r].Vk = todecimal(Regs[instructions[current_inst].RT]);
                        RS.reservations[r].Qk = 0;
                    }
                    temp.index = current_inst;
                    temp.issued = clk;
                    temp.my_inst = instructions[current_inst];
                    RS.reservations[r].op = instructions[current_inst].op;
                    temp.my_inst_string = instructions_strings[current_inst];
                    load_store_queue.push(current_inst);
                    if (branch_issued == true)
                        will_flush.push_back(r);

                }
                else
                    if (instructions[current_inst].op == "ADD" || instructions[current_inst].op == "MUL" || instructions[current_inst].op == "NEG" || instructions[current_inst].op == "NAND")
                    {
                        if (RegisterStat[instructions[current_inst].RS] != -1)
                            RS.reservations[r].Qj = RegisterStat[instructions[current_inst].RS];
                        else
                        {
                            RS.reservations[r].Vj = todecimal(Regs[instructions[current_inst].RS]);
                            RS.reservations[r].Qj = 0;

                        }
                        if (RegisterStat[instructions[current_inst].RT] != -1)
                            RS.reservations[r].Qk = RegisterStat[instructions[current_inst].RT];
                        else
                        {
                            RS.reservations[r].Vk = todecimal(Regs[instructions[current_inst].RT]);
                            RS.reservations[r].Qk = 0;
                        }
                        RS.reservations[r].busy = true;

                        RegisterStat[instructions[current_inst].Rd] = r;
                        RS.reservations[r].op = instructions[current_inst].op;
                        temp.index = current_inst;
                        temp.issued = clk;
                        temp.my_inst = instructions[current_inst];
                        temp.my_inst_string = instructions_strings[current_inst];
                        if (branch_issued == true)
                            will_flush.push_back(r);
                    }
                    else
                        if (instructions[current_inst].op == "ADDI")
                        {
                            if (RegisterStat[instructions[current_inst].RS] != -1)
                                RS.reservations[r].Qj = RegisterStat[instructions[current_inst].RS];
                            else
                            {
                                RS.reservations[r].Vj = todecimal(Regs[instructions[current_inst].RS]);
                                RS.reservations[r].Qj = 0;

                            }
                            RS.reservations[r].Vk = instructions[current_inst].imm;
                            RS.reservations[r].Qk = 0;

                            RS.reservations[r].busy = true;
                            RegisterStat[instructions[current_inst].Rd] = r;
                            RS.reservations[r].op = instructions[current_inst].op;
                            temp.index = current_inst;
                            temp.issued = clk;
                            temp.my_inst = instructions[current_inst];
                            temp.my_inst_string = instructions_strings[current_inst];
                            if (branch_issued == true)
                                will_flush.push_back(r);
                        }
                        else if (instructions[current_inst].op == "BEQ")
                        {
                            if (RegisterStat[instructions[current_inst].RS] != -1)
                                RS.reservations[r].Qj = RegisterStat[instructions[current_inst].RS];
                            else
                            {
                                RS.reservations[r].Vj = todecimal(Regs[instructions[current_inst].RS]);
                                RS.reservations[r].Qj = 0;

                            }
                            if (RegisterStat[instructions[current_inst].RT] != -1)
                                RS.reservations[r].Qk = RegisterStat[instructions[current_inst].RT];
                            else
                            {
                                RS.reservations[r].Vk = todecimal(Regs[instructions[current_inst].RT]);
                                RS.reservations[r].Qk = 0;
                            }
                            RS.reservations[r].busy = true;
                            string beq_label = instructions[current_inst].label;
                            int imm = get_label_location(beq_label);
                            RS.reservations[r].op = instructions[current_inst].op;
                            RS.reservations[r].A = imm;
                            temp.index = current_inst;
                            temp.issued = clk;
                            temp.my_inst = instructions[current_inst];
                            if (branch_issued == true)
                                will_flush.push_back(r);
                            branch_issued = true;
                            branches_encountered++;
                            branch_index = current_inst;
                            temp.my_inst_string = instructions_strings[current_inst];

                        }
                        else if (instructions[current_inst].op == "JAL")
                        {
                            RegisterStat[1] = r;
                            RS.reservations[r].busy = true;
                            RS.reservations[r].op = instructions[current_inst].op;
                            RS.reservations[r].Vj = current_inst;
                            string JAL_label = instructions[current_inst].label;
                            int JAL_imm = get_label_location(JAL_label);
                            RS.reservations[r].A = JAL_imm;
                            temp.index = current_inst;
                            temp.issued = clk;
                            temp.my_inst = instructions[current_inst];
                            if (branch_issued == true)
                                will_flush.push_back(r);
                            branch_issued = true;
                            branch_index = current_inst;
                            temp.my_inst_string = instructions_strings[current_inst];
                        }
                        else if (instructions[current_inst].op == "RET")
                        {
                            if (RegisterStat[1] != -1)
                                RS.reservations[r].Qj = RegisterStat[1];
                            else
                            {
                                RS.reservations[r].Vj = todecimal(Regs[1]);
                                RS.reservations[r].Qj = 0;

                            }
                            RS.reservations[r].busy = true;
                            RegisterStat[1] = r;
                            temp.index = current_inst;
                            temp.issued = clk;
                            temp.my_inst = instructions[current_inst];
                            RS.reservations[r].op = instructions[current_inst].op;
                            if (branch_issued == true)
                                will_flush.push_back(r);
                            branch_issued = true;
                            branch_index = current_inst;
                            temp.my_inst_string = instructions_strings[current_inst];

                        }
            current_inst++;
            RS.reservations[r].index = Track.size();
            temp.finished_execution = -1;
            temp.started_execution = -1;
            temp.write_back = -1;
            Track.push_back(temp);
        }
    }


}

void execute()
{

    for (int i = 0; i < 12; i++)
    {
        if ((RS.reservations[i].name != "BEQ" && RS.reservations[i].name != "JAL/RET") && (branch_issued == true && RS.reservations[i].index != -1 && RS.reservations[i].index > branch_index))
        {

        }
        else
        {
            if (RS.reservations[i].busy == true && Track[RS.reservations[i].index].issued != clk)
            {
                if (RS.reservations[i].name == "ADD/ADDI" || RS.reservations[i].name == "MUL" || RS.reservations[i].name == "NAND" || RS.reservations[i].name == "NEG")
                {
                    if (RS.reservations[i].Qj == 0 && RS.reservations[i].Qk == 0 && Track[RS.reservations[i].index].started_execution == -1)
                        Track[RS.reservations[i].index].started_execution = clk;
                }
                else if (RS.reservations[i].name == "LOAD")
                {
                    if (RS.reservations[i].Qj == 0 && Track[RS.reservations[i].index].started_execution == -1 && load_store_queue.front() == RS.reservations[i].index)
                        Track[RS.reservations[i].index].started_execution = clk;
                }
                else if (RS.reservations[i].name == "STORE")
                {
                    if (RS.reservations[i].Qj == 0 && Track[RS.reservations[i].index].started_execution == -1 && load_store_queue.front() == RS.reservations[i].index)
                        Track[RS.reservations[i].index].started_execution = clk;
                }
                else if (RS.reservations[i].name == "BEQ")
                {
                    if (RS.reservations[i].Qj == 0 && RS.reservations[i].Qk == 0 && Track[RS.reservations[i].index].started_execution == -1)
                        Track[RS.reservations[i].index].started_execution = clk;
                }
                else if (RS.reservations[i].name == "JAL/RET")
                {
                    if (RS.reservations[i].Qj == 0 && RS.reservations[i].Qk == 0 && Track[RS.reservations[i].index].started_execution == -1)
                        Track[RS.reservations[i].index].started_execution = clk;
                }
            }
        }

    }


    for (int i = 0; i < 12; i++)
    {
        if (RS.reservations[i].busy == true && Track[RS.reservations[i].index].started_execution != -1)
        {
            if (RS.reservations[i].name == "ADD/ADDI")
                if (clk - Track[RS.reservations[i].index].started_execution + 1 == 2)
                    Track[RS.reservations[i].index].finished_execution = clk;


            if (RS.reservations[i].name == "NAND" || RS.reservations[i].name == "NEG" || RS.reservations[i].name == "JAL/RET" || RS.reservations[i].name == "BEQ")
                if (clk - Track[RS.reservations[i].index].started_execution + 1 == 1)
                    Track[RS.reservations[i].index].finished_execution = clk;

            if (RS.reservations[i].name == "LOAD")
                if (clk - Track[RS.reservations[i].index].started_execution + 1 == 6)
                    Track[RS.reservations[i].index].finished_execution = clk;

            if (RS.reservations[i].name == "STORE")
                if (clk - Track[RS.reservations[i].index].started_execution + 1 == 3)
                    Track[RS.reservations[i].index].finished_execution = clk;

            if (RS.reservations[i].name == "MUL")
                if (clk - Track[RS.reservations[i].index].started_execution + 1 == 8)
                    Track[RS.reservations[i].index].finished_execution = clk;


        }
    }
}

void Writeback()
{
    for (int i = 0; i < 12; i++)
    {

        if (RS.reservations[i].index != -1 && Track[RS.reservations[i].index].finished_execution != -1 && Track[RS.reservations[i].index].finished_execution != clk)
        {
            if (RS.reservations[i].op == "ADD")
            {
                int temp = RS.reservations[i].Vj + RS.reservations[i].Vk;
                string reg_result;
                reg_result = tobinary(temp);
                if (Track[RS.reservations[i].index].my_inst.Rd != 0)
                    Regs[Track[RS.reservations[i].index].my_inst.Rd] = reg_result;
                Track[RS.reservations[i].index].write_back = clk;
                RS.reservations[i].busy = false;
                writingbackReally(i, reg_result);
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                RegisterStat[Track[RS.reservations[i].index].my_inst.Rd] = -1;
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";
                break;
            }
            if (RS.reservations[i].op == "ADDI")
            {
                int temp = RS.reservations[i].Vj + RS.reservations[i].Vk;
                string reg_result;
                reg_result = tobinary(temp);
                if (Track[RS.reservations[i].index].my_inst.Rd != 0)
                    Regs[Track[RS.reservations[i].index].my_inst.Rd] = reg_result;
                Track[RS.reservations[i].index].write_back = clk;
                RS.reservations[i].busy = false;
                writingbackReally(i, reg_result);
                RegisterStat[Track[RS.reservations[i].index].my_inst.Rd] = -1;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";


                break;
            }
            if (RS.reservations[i].op == "NAND")
            {
                string num, num2;
                num = tobinary(RS.reservations[i].Vj);
                num2 = tobinary(RS.reservations[i].Vk);
                string number = "";
                for (int j = 0; j < num.size(); j++)
                {
                    if (num[j] == '0' || num2[j] == '0')
                        number = number + "1";
                    else
                        number = number + "0";
                }
                if (Track[RS.reservations[i].index].my_inst.Rd != 0)
                    Regs[Track[RS.reservations[i].index].my_inst.Rd] = number;
                Track[RS.reservations[i].index].write_back = clk;
                RS.reservations[i].busy = false;
                RegisterStat[Track[RS.reservations[i].index].my_inst.Rd] = -1;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                writingbackReally(i, number);
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";

                break;
            }
            if (RS.reservations[i].op == "NEG")
            {
                string num;
                num = tobinary(RS.reservations[i].Vj);
                string number = "";
                for (int j = 0; j < num.size(); j++)
                {
                    if (num[j] == '1')
                        number = number + "0";
                    else
                        number = number + "1";
                }
                int num_dec = todecimal(number) + 1;
                string num_bin = tobinary(num_dec);
                if (Track[RS.reservations[i].index].my_inst.Rd != 0)
                    Regs[Track[RS.reservations[i].index].my_inst.Rd] = num_bin;
                Track[RS.reservations[i].index].write_back = clk;
                RegisterStat[Track[RS.reservations[i].index].my_inst.Rd] = -1;
                RS.reservations[i].busy = false;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                writingbackReally(i, num_bin);

                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";

                break;
            }
            if (RS.reservations[i].op == "JAL")
            {
                Regs[1] = tobinary(RS.reservations[i].Vj + 1);
                current_inst = RS.reservations[i].A;
                Track[RS.reservations[i].index].write_back = clk;
                RegisterStat[1] = -1;
                RS.reservations[i].busy = false;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                writingbackReally(i, tobinary(RS.reservations[i].Vj + 1));
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";
                branch_issued = false;
                flush();

                break;
            }
            if (RS.reservations[i].op == "RET")
            {
                current_inst = todecimal(Regs[1]);
                Track[RS.reservations[i].index].write_back = clk;
                RS.reservations[i].busy = false;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";
                branch_issued = false;
                flush();
                break;
            }
            if (RS.reservations[i].op == "BEQ")
            {
                if (RS.reservations[i].Vj == RS.reservations[i].Vk)
                {
                    current_inst = RS.reservations[i].A;
                    branches_mispredicted++;
                    flush();
                }
                branch_issued = false;
                Track[RS.reservations[i].index].write_back = clk;
                RS.reservations[i].busy = false;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";

                break;
            }
            if (RS.reservations[i].op == "LOAD")
            {
                RS.reservations[i].A = RS.reservations[i].A + RS.reservations[i].Vj;
                if (Track[RS.reservations[i].index].my_inst.Rd != 0)
                    Regs[Track[RS.reservations[i].index].my_inst.Rd] = dataMem[RS.reservations[i].A];
                Track[RS.reservations[i].index].write_back = clk;
                RegisterStat[Track[RS.reservations[i].index].my_inst.Rd] = -1;
                RS.reservations[i].busy = false;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                writingbackReally(i, dataMem[RS.reservations[i].A]);
                RS.reservations[i].A = 0;
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";

                load_store_queue.pop();
                break;
            }
            if (RS.reservations[i].op == "STORE")
            {

                RS.reservations[i].A = RS.reservations[i].A + RS.reservations[i].Vk;
                dataMem[RS.reservations[i].A] = tobinary(RS.reservations[i].Vj);
                Track[RS.reservations[i].index].write_back = clk;
                RS.reservations[i].busy = false;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";
                load_store_queue.pop();
                break;
            }
            if (RS.reservations[i].op == "MUL")
            {
                int temp = RS.reservations[i].Vj * RS.reservations[i].Vk;
                string reg_result;
                reg_result = tobinary(temp);
                if (Track[RS.reservations[i].index].my_inst.Rd != 0)
                    Regs[Track[RS.reservations[i].index].my_inst.Rd] = reg_result;
                Track[RS.reservations[i].index].write_back = clk;
                RegisterStat[Track[RS.reservations[i].index].my_inst.Rd] = -1;
                RS.reservations[i].busy = false;
                RS.reservations[i].Vj = 0;
                RS.reservations[i].Vk = 0;
                RS.reservations[i].Qj = 0;
                writingbackReally(i, reg_result);
                RS.reservations[i].Qk = 0;
                RS.reservations[i].A = 0;
                RS.reservations[i].index = -1;
                RS.reservations[i].op = "";

                break;
            }
        }

    }
}
string tobinary(int temp)
{
    long long bin = 0;
    int rem;
    long long i = 1;

    while (temp != 0) {
        rem = temp % 2;
        temp /= 2;
        bin += rem * i;
        i *= 10;
    }
    string num = to_string(bin);
    int size = num.size();
    for (int i = 0; i < 16 - size; i++)
    {
        num = "0" + num;
    }
    return num;

}

int todecimal(string bin)
{
    int dec = 0;
    for (int i = 0, j = bin.size() - 1; i < bin.size(); i++, j--)
    {
        if (bin[j] == '1')
        {
            dec = dec + pow(2, i);
        }

    }

    return dec;
}

void writingbackReally(int i, string result)
{
    for (int j = 0; j < RS.reservations.size(); j++)
    {
        if (RS.reservations[j].Qj == i)
        {
            RS.reservations[j].Vj = todecimal(result);
            RS.reservations[j].Qj = 0;
        }
        if (RS.reservations[j].Qk == i)
        {
            RS.reservations[j].Vk = todecimal(result);
            RS.reservations[j].Qk = 0;
        }
    }
}


