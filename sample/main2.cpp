#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include "utils.hpp"
#include <string.h>

struct bool_condition : condition
{
    bool flag;
    bool_condition(bool f) : flag(f){};
    bool eval(const record &r) const
    {
        return (r.flag == flag);
    }
};

struct int_condition : condition
{
    int parameter;
    int relation;
    int_condition(int p, int r) : parameter(p), relation(r){};
    bool eval(const record &r) const
    {
        if (relation == -1)
            return r.value > parameter;
        if (relation == 0)
            return r.value == parameter;
        if (relation == 1)
            return r.value < parameter;
        return r.value < parameter;
    }
};

struct string_condition : condition
{
    const std::shared_ptr<std::string> datap;
    string_condition(std::shared_ptr<std::string> dp) : datap(dp){};
    bool eval(const record &r) const
    {
        return *datap == *r.data;
    }
};

void read_partition(int j, std::list<record> &recList)
{
    std::string filename = "input" + std::to_string(j) + ".dat";
    std::ifstream myfile(filename);
    int recordnumber;
    int value;
    bool flag;
    myfile >> recordnumber;
    for (int i = 0; i < recordnumber; i++)
    {
        std::string *data = new std::string;
        std::shared_ptr<std::string> datap(data);
        myfile >> value >> flag >> *data;
        struct record new_record(value, flag, datap);
        recList.push_back(new_record);
    }
}

void partition(int i, Pipe<query> &pin, Pipe<query> &pout)
{
    query Q;
    std::list<record> parts;
    read_partition(i + 1, parts);
    while (true)
    {
        Q = pin.pop();
        if (Q == nullptr)
        {
            pout.push(nullptr);
            break;
        }
        bool ret;
        for (auto it : parts)
        {
            ret = true;
            for (auto qit : Q->conditions)
                ret = ret && qit->eval(it);
            if (ret)
                Q->hits.push_back(it);
        }
        pout.push(Q);
    };
};

void get_final_querys(Pipe<query> &pin)
{
    std::ofstream myfile("output.txt");
    query Q;
    while (true)
    {
        Q = pin.pop();
        if (Q == nullptr)
            break;
        if (Q->hits.size() > 0)
            for (auto pit : Q->hits)
                myfile << pit << "\n";
        else
            myfile << "{could not find records}\n";
    }
    myfile.close();
}

void read_input()
{
    std::ifstream myfile("input.txt");
    int N; 
    int Q; 
    int c; 
    int t;
    bool flag;
    int parameter;
    int relational;

    std::list<query> QList;
    if (myfile.is_open())
    {
        myfile >> N;
        myfile >> Q;
        std::vector<std::thread> threads(N + 1);
        std::vector<Pipe<query>> pipes(N + 1);

        for (int i = 0; i < N; i++)
            threads[i] = std::thread(partition, i, std::ref(pipes[i]), std::ref(pipes[i + 1]));
        threads[N] = std::thread(get_final_querys, std::ref(pipes[N]));

        for (int i = 0; i < Q; i++)
        {
            struct query_condition *qCond = new struct query_condition;
            std::shared_ptr<query_condition> newQ(qCond);
            myfile >> c;
            for (int j = 0; j < c; j++)
            {
                myfile >> t;
                switch (t)
                {
                case 1:
                {
                    myfile >> flag;
                    struct condition *boolC = new struct bool_condition(flag);
                    std::shared_ptr<condition> sBoolC(boolC);
                    newQ->conditions.push_back(sBoolC);
                }
                break; 
                case 2:
                {
                    myfile >> parameter >> relational;
                    struct int_condition *intC = new struct int_condition(parameter, relational);
                    std::shared_ptr<int_condition> sIntC(intC);
                    newQ->conditions.push_back(sIntC);
                }
                break; 
                case 3:
                {
                    std::string *data = new std::string;
                    std::shared_ptr<std::string> datap(data);
                    myfile >> *data;
                    struct string_condition *stringC = new struct string_condition(datap);
                    std::shared_ptr<string_condition> sStringC(stringC);
                    newQ->conditions.push_back(sStringC);
                }
                break; 
                }
            }
            pipes[0].push(newQ);
        }
        pipes[0].push(nullptr);
        myfile.close();
        for (auto &th : threads)
            th.join();
    }
    else
        std::cout << "Unable to open file" << std::endl;
}

int main()
{
    read_input();
    return 0;
}
