#include "Interpreter.h"

Interpreter::Interpreter(std::ostream& out_stream) 
    : out_stream(out_stream), config(ConfigType::Dec), position(0)
{
    configs["dec"] = ConfigType::Dec;
    configs["hex"] = ConfigType::Hex;
    configs["bin"] = ConfigType::Bin;
    
    stmts["config"] = std::bind(&Interpreter::parse_ConfigStmt, this);
    stmts["="]      = std::bind(&Interpreter::parse_AssgStmt, this);
    stmts["print"]  = std::bind(&Interpreter::parse_PrintStmt, this);
}

void Interpreter::evaluate(const std::vector<std::string>& tokens)
{
    this->tokens = tokens; // Current tokens to evaluate
    position = 0;

    while (position < tokens.size())
    {
        std::string token = peek();

        if (stmts.find(token) != stmts.end()) // If this token is a valid statement
        {
            stmts[token]();
            continue;
        }
        
        consume(token); // If not a statement, consume current token
    }
}

std::string Interpreter::peek()
{
    return peek(0);
}
std::string Interpreter::peek(int steps)
{
    if (position + steps >= tokens.size())
        return "\u0003"; // End of text

    return tokens[position + steps];
}

void Interpreter::consume(const std::string& token)
{
    std::string next_token = peek();
    
    if (next_token.empty())
        throw std::runtime_error("Consumed past last token\n");
    if (next_token != token)
        throw std::runtime_error("Could not consume token " + token + "\n");

    ++position;
}

void Interpreter::parse_ConfigStmt()
{
    consume("config");

    std::string next_token = peek();
    if (next_token == "dec" || next_token == "hex" || next_token == "bin")
    {
        config = configs[next_token];
        consume(next_token);
    }
}
void Interpreter::parse_AssgStmt()
{
    consume("=");

    std::string name = peek(-2);
    if (is_variable_name(name))
        variables[name] = parse_MathExp();
}
void Interpreter::parse_PrintStmt()
{
    consume("print");
    
    int val = parse_MathExp();
    switch (config)
    {
    case ConfigType::Dec: 
        out_stream << std::dec << val << '\n';                              
        return;
    case ConfigType::Hex: 
        out_stream << "0x" << std::hex << val << '\n';                  
        return;
    case ConfigType::Bin: 
        out_stream << std::bitset<32>(val).to_string() << '\n'; 
        return;
    default:
        out_stream << "configuration is undefined" << '\n';
        return;
    }
}

int Interpreter::parse_MathExp()
{
    return parse_SumExp();
}
int Interpreter::parse_SumExp()
{
    int val = parse_ProductExp();

    std::string next_token = peek();
    while (true)
    {
        if (next_token == "+")
        {
            consume("+");
            val += parse_ProductExp();
        }
        else if (next_token == "-")
        {
            consume("-");
            val -= parse_ProductExp();
        }
        else break;

        next_token = peek();
    }

    return val;
}
int Interpreter::parse_ProductExp()
{
    int val = parse_PrimaryExp();

    std::string next_token = peek();
    while (true)
    {
        if (next_token == "*")
        {
            consume("*");
            val *= parse_PrimaryExp();
        }
        else if (next_token == "/")
        {
            consume("/");
            val /= parse_PrimaryExp();
        }
        else break;

        next_token = peek();
    }

    return val;
}
int Interpreter::parse_PrimaryExp()
{
    int val = 0;
    std::string next_token = peek();

    if (is_integer(next_token))
    {
        val = std::stoi(next_token);
        consume(next_token);
    }
    else if (variables.find(next_token) != variables.end())
    {
        val = variables[next_token];
        consume(next_token);
    }
    else if (next_token == "(")
    {
        consume("(");
        val = parse_MathExp();

        if (peek() == ")") 
            consume(")");
        else
            throw std::runtime_error("no enclosing paranthesis found");
    }

    return val;
}

void Interpreter::tokenize(std::queue<std::string>& codeLines)
{
    while (!codeLines.empty())
    {
        std::vector<std::string> tokens;

        std::string line = codeLines.front();
        std::string stmt = std::string();
        
        for (const char& c : line)
        {
            if (c == ' ')
            {
                tokens.push_back(stmt);
                stmt = std::string();
            }
            else
            {
                stmt += c;
            }
        }
        tokens.push_back(stmt);
        
        evaluate(tokens);

        codeLines.pop();
    }
}

bool Interpreter::is_integer(const std::string& token)
{
    return std::regex_match(token, std::regex("-?[0-9]+"));
}

bool Interpreter::is_variable_name(const std::string& token)
{
    return std::regex_match(token, std::regex("[a-zA-z][a-zA-z0-9]*"));
}

void Interpreter::read_stream(std::istream& in_stream)
{
    std::queue<std::string> codeLines;

    std::string line;
    while (true)
    {
        if (getline(in_stream, line))
            codeLines.push(line);

        if (in_stream.eof())
            break;

        if (in_stream.fail())
        {
            in_stream.clear();
            in_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            out_stream << "bad input, try again" << '\n';
        }
    }

    tokenize(codeLines);
}
void Interpreter::read_file(const std::string& filename)
{
    std::queue<std::string> codeLines;

    std::string line;
    std::ifstream file(filename);
    if (file.is_open())
    {
        while (file.good())
        {
            getline(file, line);
            codeLines.push(line);
        }
        file.close();
    }
    else
    {
        std::cout << "Unable to open file";
    }

    tokenize(codeLines);
}
