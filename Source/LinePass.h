#pragma once

#include <memory>

class Program;
class RenderList;

class LinePass
{
    std::unique_ptr<Program> program;
public:

    LinePass();
    ~LinePass();

    void execute(const RenderList& objects);
    
private:

    void UseProgram();
    void GenerateProgram();

};