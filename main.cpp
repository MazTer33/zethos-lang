#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<unordered_map>
#include<variant>
#include<chrono>
#include<cmath>
#include<SDL2/SDL.h>

std::vector<std::string> breakDown(const std::string& statement, int idx = 0) { //chatGPT "assisted" me on this
    bool inStr = false;
    std::vector<std::string> out;
    std::string word;
    int bracketDepth = 0;

    // find opening '['
    while (idx < (int)statement.size() && statement[idx] != '[') idx++;
    idx++; // skip the first '['

    while (idx < (int)statement.size()) {
        char c = statement[idx];

        if (c == '"') {
            inStr = !inStr;
            word.push_back(c); // keep quotes
        } else if (!inStr) {
            if (c == '[') {
                bracketDepth++;
                word.push_back(c); // keep inner brackets
            } else if (c == ']') {
                if (bracketDepth == 0) {
                    break; // outermost closing bracket
                } else {
                    bracketDepth--;
                    word.push_back(c);
                }
            } else if (c == ':') {
                if (bracketDepth == 0) {
                    out.push_back(word);
                    word.clear();
                } else {
                    word.push_back(c); // inside inner brackets
                }
            } else {
                word.push_back(c);
            }
        } else { // inside string
            if (c == '\\') {
                idx++;
                if (idx < (int)statement.size()) {
                    char next = statement[idx];
                    if (next == 'n') word.push_back('\n');
                    else if (next == 't') word.push_back('\t');
                    else if (next == '\\') word.push_back('\\');
                    else if (next == '"') word.push_back('"');
                    else word.push_back(next);
                }
            } else {
                word.push_back(c);
            }
        }

        idx++;
    }

    if (!word.empty()) out.push_back(word);
    return out;
}


std::vector<std::string> breakDownProgram(const std::string& program) {
	std::vector<std::string> out;
	std::string statement;
	int idx = 0;

	while(idx < (int)program.size()) {
		//find a '['
		while(idx < (int)program.size() && program[idx] != '[') {
			idx++;
		}

		//get the statement itself
		while(idx < (int)program.size() && program[idx] != ']') {
			statement.push_back(program[idx]);
			idx++;
		}
		statement.push_back(program[idx]);
	
		//push to vector
		out.push_back(statement);
		if (statement == "[return]") {
			statement.clear();
			break;
		} else {
			statement.clear();
		}
	}

	return out;
}

enum class VarType {
	BOOL,
	INT,
	DOUBLE,
	STRING
};

struct Variable {
	VarType type;
	std::variant<bool, int, double, std::string> value;
};

class Environment {
public:
	void setVar(const std::string& name, Variable var) {
		variables[name] = var;
	}
	Variable getVar(const std::string& name) {
		if (variables.find(name) != variables.end()) {
			return variables[name];
		}
		throw std::runtime_error("runtime error: variable not found: " + name);
	}
private:
	std::unordered_map<std::string, Variable> variables;
};

int main(int argc, char* argv[]) {
	std::string version = "1.0";
	std::string vername = "Volt";
	bool inDebugMode = false;
	Environment env;
	
	if (argc < 2) {
		std::cerr << "zthos: error: no input files\n";
		return 1;
	}

	if (std::string(argv[1]) == "-v" || std::string(argv[1]) == "--version") {
		std::cout << "Zethos EarlyDev "<< version << " " << vername << "\n";
		return 0;
	}

	if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
		std::cout << "usage: zthos <options> <input file>\noptions:\n\t-v --version\tShows the version and the name of the version.\n\t-h --help\tShows this help message and stops\n\t-d --debug\tPuts program in debug mode (shows how long the program lasted)\n";
		return 0;
	}

	std::ifstream inputF;

	if (std::string(argv[1]) == "-d" || std::string(argv[1]) == "--debug") {
		inDebugMode = true;
		inputF.open(argv[2]);
	} else {
		inputF.open(argv[1]);
	}

	if (!inputF) {
		std::cerr << "zthos: error: input file does not exist or file unsuccessfully opened\n";
		return 2;
	}

	std::string line;
	std::string programRaw;

	while(std::getline(inputF, line)) {
		programRaw += line;
	}

	inputF.close();

	std::vector<std::string> programRawInSTS = breakDownProgram(programRaw);
	std::vector<std::vector<std::string>> program;

	// turn program into vectors of vectors of type std::string
	for(int i = 0; i < (int)programRawInSTS.size(); i++) {
		program.push_back(breakDown(programRawInSTS.at(i)));
	}

	if (program.at(0).at(0) != "zthos") {
		std::cout << "zthos: runtime error: expected statement \"zthos\" to be the first statement\nFIX: just put the [zthos:" << version << "] statement at the absolute top of the file.\n";
		return 3;
	}

	auto start = std::chrono::high_resolution_clock::now();

	std::string programVer = program.at(0).at(1);

	if (std::stof(version) < std::stof(programVer)) {
		std::cout << "zthos: runtime error: program version higher than current version\nFIX: use [zthos:" << version << "] instead of [zthos:" << programVer << "].\n";
		return 4;
	}

	for(int stsi = 1; stsi < (int)program.size(); stsi++) {
		if (program.at(stsi).at(0) == "#") {
			continue;
		} else if (program.at(stsi).at(0) == "print") {
			std::cout << program.at(stsi).at(1);
		} else if (program.at(stsi).at(0) == "return") {
			break;
		} else if (program.at(stsi).at(0) == "new") {
			VarType vtype;
			if (program.at(stsi).at(1) == "bool") {
				vtype = VarType::BOOL;
			} else if (program.at(stsi).at(1) == "int") {
				vtype = VarType::INT;
			} else if (program.at(stsi).at(1) == "double") {
				vtype = VarType::DOUBLE;
			} else if (program.at(stsi).at(1) == "string") {
				vtype = VarType::STRING;
			} else {
				std::cout << "zthos: specific runtime error: no such variable type \"" << program.at(stsi).at(1) << "\" exists\n";
				return 0;
			}
			if (program.at(stsi).size() < 4) {
				env.setVar(program.at(stsi).at(2), {vtype, 0});
			} else {
				std::string initValue = (program.at(stsi).size() >= 4) ? program.at(stsi).at(3) : "";

				if (vtype == VarType::BOOL) {
					env.setVar(program.at(stsi).at(2), {vtype, initValue == "true"});
				} else if (vtype == VarType::INT) {
					env.setVar(program.at(stsi).at(2), {vtype, std::stoi(initValue)});
				} else if (vtype == VarType::DOUBLE) {
					env.setVar(program.at(stsi).at(2), {vtype, std::stod(initValue)});
				} else if (vtype == VarType::STRING) {
					env.setVar(program.at(stsi).at(2), {vtype, initValue});
				}
			}
		} else if (program.at(stsi).at(0) == "if") {
			std::cout << program.at(stsi).at(1) << "\n";
			return 696969;
		} else {
			std::cout << "runtime error: function " << program.at(stsi).at(0) << " not found\n";
			break;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> elapsed = end - start;

	if (inDebugMode) {
		std::cout << "Program lasted for " << std::floor(elapsed.count()) << " seconds.\n";
	}

	return 0;
}
