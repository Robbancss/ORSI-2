#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <string.h>
#include "pipe.hpp"

struct pipeQuery
{
    std::vector<int> points;
    std::vector<std::vector<std::vector<int>>> matrixes;
};

using query = std::shared_ptr<pipeQuery>;


void calculate(int i, Pipe<query> &pin, Pipe<query> &pout)
{
    query inPipe;

    inPipe = pin.pop();

    // std::cout << i << " - Pipe in: ";
    // for (size_t i = 0; i < inPipe->points.size(); i++)
    // {
    //     std::cout << inPipe->points[i];
    // }
    // std::cout << std::endl;

    std::vector<int> newVector(4);

    for (size_t j = 0; j < 4; j++)
    {
        newVector[j] = 0;
        for (size_t k = 0; k < 4; k++)
        {
            newVector[j] += inPipe->matrixes[i][j][k] * inPipe->points[k];
        }
    }
    
    pipeQuery *pQ = new struct pipeQuery;
    query pipeOut(pQ);
    pipeOut->points.resize(4);
    pipeOut->points = newVector;
    pipeOut->matrixes = inPipe->matrixes;
    pout.push(pipeOut);
}

void getLastQuery(Pipe<query> &pin)
{
    query inPipe = pin.pop();
    if (inPipe == nullptr)
    {
        return;
    }
    std::cout << "End of pipe: ";

    for (size_t i = 0; i < inPipe->points.size(); i++)
    {
        std::cout << inPipe->points[i] << "";
    }
    std::cout << std::endl;
    
    // todo write pin data to file
}

int main(int argc, char const *argv[])
{
    std::ifstream matrixesFile("input_matrices.txt");
    std::vector<std::vector<std::vector<int>>> matrixes;
    int M;
    std::ifstream vectorsFile("input_points.txt");
    std::vector<std::vector<int>> vectors;
    int N;

    if (!matrixesFile.is_open())
    {
        return 0;
    }

    {
        std::string tempM;
        std::getline(matrixesFile, tempM);
        std::istringstream iss(tempM);
        iss >> M;
        std::cout << "Number of threads (M): " << M << std::endl;
    }

    matrixes.resize(M);
    for (size_t i = 0; i < M; i++)
    {
        matrixes[i].resize(4);
        for (size_t j = 0; j < 4; j++)
        {
            matrixes[i][j].resize(4);
        }
    }

    { // Read matrixes from file to var
        for (size_t i = 0; i < M; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                for (size_t k = 0; k < 4; k++)
                {
                    matrixesFile >> matrixes[i][j][k];
                }
            }
        }
    }

    std::string tempN;
    std::getline(vectorsFile, tempN);
    std::istringstream iss(tempN);
    iss >> N;
    std::cout << "Number of vectors (N): " << N << std::endl;

    vectors.resize(N);
    for (size_t i = 0; i < N; i++)
    {
        vectors[i].resize(4);
        vectors[i][3] = 1;
    }
    
    { // Read vectors from file to var
        for (size_t i = 0; i < N; i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                vectorsFile >> vectors[i][j];
            }
        }
    }    

    matrixesFile.close();
    vectorsFile.close();

    std::vector<Pipe<query>> pipes(M + 1);
    std::vector<std::thread> threads;

    for (size_t i = 0; i < M; i++)
    {
        threads.push_back(std::thread(calculate, i, std::ref(pipes[i]), std::ref(pipes[i + 1])));
    }
    threads.push_back(std::thread(getLastQuery, std::ref(pipes[M])));


    for (size_t i = 0; i < N; i++)
    {
        pipeQuery *pQ = new struct pipeQuery;
        query pipeInput(pQ);
        pipeInput->matrixes = matrixes;
        pipeInput->points = vectors[i];
        // std::cout << "PIPE: " << pipeInput->points[0] << pipeInput->points[1] << pipeInput->points[2] << pipeInput->points[3] << std::endl;
        pipes[0].push(pipeInput);
    }

    for (auto &th : threads)
    {
        th.join();   
    }

    std::cout << "Program shuts down" << std::endl;
    return 0;
}
