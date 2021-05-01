#include "Interpreter.h"

Interpreter::Interpreter(std::ostream& out_stream) 
    : out_stream(out_stream), setting(Config::Dec), position(0)
{
    configs.emplace("dec", Config::Dec);
    configs.emplace("hex", Config::Hex);
    configs.emplace("bin", Config::Bin);
}

void Interpreter::clear()
{
    variables.clear();
    setting = Config::Dec;
}

void Interpreter::evaluate(const std::vector<std::string>& tokens)
{
    this->tokens = tokens; // Current tokens to evaluate

    position = 0;
    var_name = "";

    try
    {
        parse_Stmt();
    }
    catch (std::exception exception)
    {
        std::cout << exception.what();
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

    if (position - steps < 0)
        throw std::out_of_range(steps + " is out of range\n");

    return tokens[position + steps];
}

void Interpreter::consume(const std::string& token)
{
    std::string next_token = peek();
    
    if (next_token.empty())
        throw std::runtime_error("Consumed past last token\n");
    if (next_token != token)
        throw std::runtime_error("Could not consume token '" + token + "'\n");

    ++position;
}

void Interpreter::parse_Stmt()
{
    std::string next_token = peek();
    if (is_variable(next_token))
    {
        consume(next_token);
        if (peek() == "=")
        {
            var_name = next_token;

            consume("=");
            parse_AssgStmt();
        }
        else if (next_token == "config")
            parse_ConfigStmt();
        else if (next_token == "print")
            parse_PrintStmt();
        else
            throw std::runtime_error("'" + next_token + " is not a valid statement; expected: config, = or print\n");
    }
    else
        throw std::runtime_error("syntax error\n");
}
void Interpreter::parse_ConfigStmt()
{
    std::string next_token = peek();
    if (configs.find(next_token) != configs.end())
    {
        consume(next_token);
        setting = configs[next_token];
    }
    else
        throw std::runtime_error("'" + next_token + "' is not a valid configuration; expected: dec, hex or bin\n");
}
void Interpreter::parse_AssgStmt()
{
    if (var_name.empty())
        throw std::runtime_error("variable name is undefined\n");

    variables[var_name] = parse_MathExp();
}
void Interpreter::parse_PrintStmt()
{
    int val = parse_MathExp();
    switch (setting)
    {
    case Config::Dec: out_stream << std::dec << val << '\n';                              
        return;
    case Config::Hex: out_stream << "0x" << std::hex << val << '\n';                  
        return;
    case Config::Bin: out_stream << std::bitset<32>(val).to_string() << '\n'; 
        return;
    default:
        throw std::runtime_error("configuration is undefined\n");
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
        else 
            break;

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
        else 
            break;

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
        consume(next_token);
        val = std::stoi(next_token);
    }
    else if (is_variable(next_token))
    {
        consume(next_token);
        val = get_variable(next_token);
    }
    else if (next_token == "(")
    {
        consume("(");

        val = parse_MathExp();

        if (peek() == ")")
            consume(")");
        else
            throw std::runtime_error("no enclosing paranthesis found\n");
    }
    else
        throw std::runtime_error("the given expression: '" + next_token  + "' is not valid\n");

    return val;
}

bool Interpreter::is_integer(const std::string& token)
{
    return std::regex_match(token, std::regex("-?[0-9]+"));
}
bool Interpreter::is_variable(const std::string& token)
{
    return std::regex_match(token, std::regex("[a-zA-z][a-zA-z0-9]*"));
}

int Interpreter::get_variable(const std::string& name)
{
    if (variables.find(name) != variables.end())
        return variables[name];
    else
        throw std::runtime_error("variable '" + name + "' is not defined\n");
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
            if (c == ' ' && !stmt.empty())
            {
                tokens.push_back(stmt);
                stmt = std::string();
            }
            else
            {
                if (c != ' ')
                    stmt += c;
            }
        }

        if (!stmt.empty())
            tokens.push_back(stmt);

        if (!tokens.empty())
            evaluate(tokens);

        codeLines.pop();
    }
}

void Interpreter::read_stream(std::istream& in_stream)
{
    std::queue<std::string> codeLines;

    std::string line;
    while (true)
    {
        if (getline(in_stream, line))
        {
            if (!line.empty())
                codeLines.push(line);
        }

        if (in_stream.eof())
            break;

        if (in_stream.fail())
        {
            in_stream.clear();
            in_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            out_stream << "bad input, try again" << '\n';
        }
    }

    if (!tokens.empty())
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
        throw std::runtime_error("unable to open file\n");

    tokenize(codeLines);
}
