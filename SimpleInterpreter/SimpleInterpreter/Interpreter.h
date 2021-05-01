#pragma once

#include <iostream>
#include <fstream>
#include <bitset>
#include <functional>
#include <limits>
#include <queue>
#include <vector>
#include <regex>
#include <unordered_map>
#include <map>

enum class Config
{
	Dec,
	Hex,
	Bin
};

class Interpreter
{
public:
	Interpreter(std::ostream& out_stream);
	~Interpreter() = default;

	void read_stream(std::istream& in_stream);
	void read_file(const std::string& filename);

	void clear();

private:
	void tokenize(std::queue<std::string>& codeLines);

private:	
	void evaluate(const std::vector<std::string>& tokens);

	std::string peek();
	std::string peek(int steps);

	void consume(const std::string& token);

	void parse_Stmt();
	void parse_ConfigStmt();
	void parse_AssgStmt();
	void parse_PrintStmt();

	int parse_MathExp();
	int parse_SumExp();
	int parse_ProductExp();
	int parse_PrimaryExp();

	bool is_integer(const std::string& token);
	bool is_variable(const std::string& token);

	int get_variable(const std::string& name);

private:
	std::ostream& out_stream;

	size_t position;
	std::vector<std::string> tokens;

	Config setting;
	std::unordered_map<std::string, Config> configs;

	std::string var_name; // Current evaluated variable that is to be assigned a value
	std::map<std::string, int> variables;

private:
	Interpreter(const Interpreter& rhs) = delete;
	Interpreter& operator=(const Interpreter& rhs) = delete;
};

