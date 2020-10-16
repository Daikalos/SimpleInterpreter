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

enum class ConfigType
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

private:
	void tokenize(std::queue<std::string>& codeLines);

private:	
	void evaluate(const std::vector<std::string>& tokens);

	std::string peek();
	std::string peek(int steps);

	void consume(const std::string& token);

	void parse_ConfigStmt();
	void parse_AssgStmt();
	void parse_PrintStmt();

	int parse_MathExp();
	int parse_SumExp();
	int parse_ProductExp();
	int parse_PrimaryExp();

	bool is_integer(const std::string& token);
	bool is_variable_name(const std::string& token);

private:
	std::ostream& out_stream;

	ConfigType config;
	std::unordered_map<std::string, ConfigType> configs;
	std::unordered_map<std::string, std::function<void()>> stmts;
	std::map<std::string, int> variables;

	std::vector<std::string> tokens;
	int position;

private: // Not Interesting, placed bottom
	Interpreter(const Interpreter& rhs) = delete;
	Interpreter& operator=(const Interpreter& rhs) = delete;
};

