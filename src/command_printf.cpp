//
// Created by dylan liang on 2022/1/28.
//

#include <iostream>
#include <algorithm>
#include <regex>
#include <time_utils.h>
#include <algorithm>

#include "command_printf.h"

CommandPrintf::~CommandPrintf()
{
    m_running = false;
    if (m_workThread && m_workThread->joinable())
    {
        m_workThread->join();
    }
}

CommandPrintf::CommandPrintf() : m_contentArea(10)
{
    m_pageSize = 5;
    pageIndex = 0;
    pageLast = 0;
    m_running = false;
    m_workThread = nullptr;
}

// save to file with command  and async
void CommandPrintf::execAsync()
{
    // TODO start a new thread
    m_workThread = new std::thread(std::bind(&CommandPrintf::exec, this));
}

void CommandPrintf::exec()
{
    m_running = true;
    while (m_running)
    {
        clearScreen();
        std::string content;
        content += "CommandPrintfTool\n";
        content += "Author:dylan\n";
        content += "-----------------------\n";
        content += printMsgs();
        content += "-----------------------\n";
        content += "Function List:\n\n";
        content += printFunctions();
        content += "-----------------------\n";
        content += printfPages();
        content += printCommander();
        printf("%s\n", content.c_str());
        CommandInput commandInput = parserInput(getLineInput());
        if (commandInput.mode == EXIT_MODE)
        {
        }
        else if (commandInput.mode == FUNCTION_MODE)
        {
            if (!m_funcCallback)
            {
                static int index = 0;
                m_contentArea.pushData(std::to_string(index++) + "请设置回调函数\n");
            }
            else
            {
                m_funcCallback(commandInput.functionNo, this);
            }
        }
        else if (commandInput.mode == COMMAND_MODE)
        {
            pushBizData("commandMode:%s %s\n", commandInput.commandNo.c_str(), commandInput.commandContent.c_str());
            if (commandInput.commandNo == "j")
            {
                pageIndex = std::max(pageIndex - 1, 0);
            }
            else if (commandInput.commandNo == "k")
            {
                pageIndex = std::min(pageIndex + 1, pageLast);
            }
        }
        else if (commandInput.mode == INIT_MODE)
        {
        }
        printf("\033[2;1H");
    }
}

void CommandPrintf::calPage()
{
    size_t size = m_funcList.size();
    if (size % m_pageSize != 0)
    {
        pageLast = size / m_pageSize;
    }
    else
    {
        pageLast = size / m_pageSize - 1;
    }
}

void CommandPrintf::addCommand(std::string functionName)
{
}

void CommandPrintf::addCommand(int32_t serialNo, std::string functionName)
{
    if (findCommand(serialNo))
    {
        return;
    }
    FunctionRecord functionRecord;
    functionRecord.serialNo = serialNo;
    functionRecord.functionName = functionName;
    m_funcList.push_back(functionRecord);
    std::sort(m_funcList.begin(), m_funcList.end(), [](const FunctionRecord &l, const FunctionRecord &r)
              { return l.serialNo < r.serialNo; });
    calPage();
}

bool CommandPrintf::findCommand(int serialNo)
{
    for (auto &command : m_funcList)
    {
        if (command.serialNo == serialNo)
        {
            return true;
        }
    }
    return false;
}

std::string CommandPrintf::printFunctions()
{
    int32_t functionStartCount = (pageIndex)*m_pageSize;
    int32_t functionEndCount = (pageIndex + 1) * m_pageSize;
    int32_t size = m_funcList.size();
    functionEndCount = functionEndCount < size ? functionEndCount : size;
    std::string printContent;
    char buf[1024];
    for (int i = functionStartCount; i < functionEndCount; i++)
    {
        auto &ele = m_funcList[i];
        sprintf(buf, "%d.%s\n", ele.serialNo, ele.functionName.c_str());
        printContent += std::string(buf);
    }
    return printContent;
}

std::string CommandPrintf::printfPages()
{
    std::string printContent;
    for (int i = 0; i <= pageLast; i++)
    {
        printContent += std::to_string(i) + " ";
    }
    printContent += "\n";
    return printContent;
}

std::string CommandPrintf::printCommander()
{
    return "Prev(j) Next(k)\n";
}

std::string CommandPrintf::getLineInput()
{
    std::string content;
    printf("请输入命令:");
    std::getline(std::cin, content);
    return content;
}

CommandInput CommandPrintf::parserInput(std::string content)
{
    CommandInput commandInput;
    if (content.empty())
    {
        m_contentArea.pushData("请输入指令\n");
    }
    else
    {
        if (matchNumber(content, commandInput))
        {
        }
        else if (matchCommand(content, commandInput))
        {
        }
    }
    return commandInput;
}

// print height 20
std::string CommandPrintf::printMsgs()
{
    std::string content;
    content += m_contentArea.genContent();
    int32_t padSize = m_contentArea.getLimitedSize() - m_contentArea.getAvailableLineSize();
    for (int i = 0; i < padSize; i++)
    {
        content += "\n";
    }
    return content;
}

std::string CommandPrintf::getTimeStamp()
{
    // auto start = std::chrono::steady_clock::now();
    return "";
}

void CommandPrintf::clearScreen()
{
    for (int i = 0; i < 80; i++)
    {
        printf("\n");
    }
    printf("\033[2;1H");
}

void CommandPrintf::setFunctionCallback(std::function<void(int, CommandPrintf *)> f)
{
    m_funcCallback = f;
}

bool CommandPrintf::matchNumber(std::string str, CommandInput &commandInput)
{
    std::regex baseRegex("^\\s*[0-9]+\\s*");
    std::smatch baseMatch;
    bool state = std::regex_match(str, baseMatch, baseRegex);
    if (!state || baseMatch.size() != 1)
    {
        return false;
    }
    commandInput.mode = FUNCTION_MODE;
    commandInput.functionNo = std::stoi(baseMatch[0]);
    return true;
}

bool CommandPrintf::matchCommand(std::string str, CommandInput &commandInput)
{
    std::regex baseRegex("^\\s*([a-zA-Z])\\s*([0-9]*)\\s*");
    std::smatch baseMatch;
    bool state = std::regex_match(str, baseMatch, baseRegex);
    if (!state && baseMatch.size() == 1)
    {
        return false;
    }
    commandInput.mode = COMMAND_MODE;
    commandInput.commandNo = baseMatch[1];
    if (baseMatch.size() == 3)
    {
        commandInput.commandContent = baseMatch[2];
    }
    return true;
}