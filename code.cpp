#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <vector>
#include <algorithm>
#include <stack>

using namespace std;

string trim(const string &str)
{
    string result = str;
    result.erase(result.begin(), find_if(result.begin(), result.end(), [](int ch)
                                         { return !isspace(ch); }));
    result.erase(find_if(result.rbegin(), result.rend(), [](int ch)
                         { return !isspace(ch); })
                     .base(),
                 result.end());
    return result;
    //cout << result;
}
string evaluateExpression(unordered_map<string, string> &circuitMap,
                          unordered_map<string, string> &curr_map,
                          const string key)
{
    auto it = circuitMap.find(key);
    // if (it == circuitMap.end()) {
    //     return key;  // Return the variable if not found in the circuit map
    // }

    if (curr_map.find(key) != curr_map.end())
    {
        return curr_map[key];
    }
    const string &expression = it->second;
    istringstream iss(expression);
    string token;
    string result = "";
    while (iss >> token)
    {
        if (circuitMap.count(token))
        {
            result += "(" + evaluateExpression(circuitMap, curr_map, token) + ")";
        }
        else
        {
            result += token;
        }
    }
    curr_map[key] = result;
    return result;
}

bool getResult(string expression, unordered_map<string, bool> variableValues)
{
    std::stack<bool> operandStack;
    std::stack<char> operatorStack;

    for (char ch : expression)
    {
        string str(1, ch);
        if (ch == ' ')
            continue;

        if (ch == '(' || ch == '&' || ch == '|' || ch == '^' || ch == '~')
        {
            operatorStack.push(ch);
        }
        else if (ch == ')')
        {
            while (!operatorStack.empty() && operatorStack.top() != '(')
            {
                char op = operatorStack.top();
                operatorStack.pop();

                bool operand2 = operandStack.top();
                operandStack.pop();
                bool operand1;
                if (op == '&' || op == '|' || op == '^')
                {
                    operand1 = operandStack.top();
                    operandStack.pop();
                }
                bool result = true;
                switch (op)
                {
                case '&':
                    result = operand1 && operand2;
                    break;
                case '|':
                    result = operand1 || operand2;
                    break;
                case '^':
                    result = operand1 != operand2;
                    break;
                case '~':
                    result = !operand2;
                    break;
                }

                operandStack.push(result);
            }

            operatorStack.pop(); // Discard the matching '('
        }
        else if (variableValues.count(str))
        {
            operandStack.push(variableValues.at(str));
        }
        else
        {
            std::cerr << "Invalid character in expression: " << ch << std::endl;
            return false;
        }
    }

    while (!operatorStack.empty())
    {
        char op = operatorStack.top();
        operatorStack.pop();

        bool operand2 = operandStack.top();
        operandStack.pop();
        bool operand1;
        if (op == '&' || op == '|' || op == '^')
        {
            operand1 = operandStack.top();
            operandStack.pop();
        }

        bool result;
        switch (op)
        {
        case '&':
            result = operand1 && operand2;
            break;
        case '|':
            result = operand1 || operand2;
            break;
        case '^':
            result = operand1 != operand2;
            break;
        case '~':
            result = !operand2;
            break;
        }

        operandStack.push(result);
    }

    return operandStack.top();
}

int main()
{
    ifstream file("circuit.txt");
    string FAULT_AT = "net_f";
    string FAULT_TYPE = "SA0";
    unordered_map<string, string> expressionMap;

    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            istringstream iss(line);
            string key, expression;
            getline(iss, key, '=');
            getline(iss, expression);

            key = trim(key);
            expression = trim(expression);
            key = trim(key);

            expressionMap[key] = expression;
        }

        file.close();
    }
    // for(auto &it: expressionMap) {
    //     cout<<it.first<<" -- "<<it.second<<endl;
    // }
    bool fault_value = (FAULT_TYPE == "SA0") ? false : true;
    unordered_map<string, string> resultMap;
    string original = evaluateExpression(expressionMap, resultMap, "Z");
    // cout<<original<<endl;
    unordered_map<string, bool> value;

    resultMap.clear();
    expressionMap[FAULT_AT] = (FAULT_TYPE == "SA0") ? "0" : "1";
    value["1"] = true;
    value["0"] = false;
    string fault = evaluateExpression(expressionMap, resultMap, "Z");
    // cout<<fault<<endl;

    for (int val = 0; val < (1 << 4); val++)
    {
        value["A"] = (1 << 0) & val;
        value["B"] = (1 << 1) & val;
        value["C"] = (1 << 2) & val;
        value["D"] = (1 << 3) & val;
        bool originalCal = getResult(original, value);
        bool faultyCal = getResult(fault, value);
        // cout<<value["A"]<<value["B"]<<value["C"]<<value["D"]<<endl;
        // cout<<"Result calculated: "<< getResult(original, value)<<endl;
        // cout<<"Result calculated faulty: "<< faultyCal <<endl;

        if (originalCal != faultyCal)
        {
            cout << "A=" << value["A"] << ", B=" << value["B"] << ", C=" << value["C"] << ", D=" << value["D"] << endl;
            cout << "Expected faulty value: " << faultyCal << endl;
            break;
        }
    }

    return 0;
}
