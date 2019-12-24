#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include <iostream>
#include <fstream>

std::vector<miniplc0::Token> _tokenize(std::istream& input) {
	miniplc0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Tokenize(std::istream& input, std::ostream& output) {
	auto v = _tokenize(input);
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void Analyse(std::istream& input, std::ostream& output){
	auto tks = _tokenize(input);
	miniplc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}
	auto v = p.first;
	output << fmt::format(".constants:\n");
    int cnt = 0;
	for(auto i = v.getConsts().begin();i != v.getConsts().end();i++,cnt++)
    {
	    if((*i).second == "INT")
	        output << fmt::format("{} {} {}\n",cnt,"I",(*i).first);
	    else if((*i).second == "IDEN")
	        output << fmt::format("{} {} {}\n",cnt,"S",(*i).first);
    }

	output << fmt::format("\n.start:\n");
	for(int i = 0;i < v.getBegin().size();i++)
	    output << fmt::format("{} {}\n",i,v.getBegin()[i]);

	output << fmt::format("\n.functions:\n");
	cnt = 0;
    for(int i = 0;i < v.getFunction().size();i++)
        output << fmt::format("{} {} {} {}\n",i,v.getFunction()[i].index-1,v.getFunction()[i].param,v.getFunction()[i].level);

    output << fmt::format("\n");
    for(int i = 1;i < v.getProgram().size();i++)
    {
        auto p = v.getProgram()[i];
        output << fmt::format(".F{}:\n",i-1);
        for(int j = 0;j < p.size();j++)
            output << fmt::format("{} {}\n",j,p[j]);
    }
	return;
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("miniplc0");
	program.add_argument("input")
		.help("speicify the file to be compiled.");
	program.add_argument("-t")
		.default_value(false)
		.implicit_value(true)
		.help("perform tokenization for the input file.");
	program.add_argument("-l")
		.default_value(false)
		.implicit_value(true)
		.help("perform syntactic analysis for the input file.");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		fmt::print(stderr, "{}\n\n", err.what());
		program.print_help();
		exit(2);
	}

	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");
	std::istream* input;
	std::ostream* output;
	std::ifstream inf;
	std::ofstream outf;
	if (input_file != "-") {
		inf.open(input_file, std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		input = &inf;
	}
	else
		input = &std::cin;
	if (output_file != "-") {
		outf.open(output_file, std::ios::out | std::ios::trunc);
		if (!outf) {
			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
			exit(2);
		}
		output = &outf;
	}
	else
		output = &std::cout;
	if (program["-t"] == true && program["-l"] == true) {
		fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
		exit(2);
	}
	if (program["-t"] == true) {
		Tokenize(*input, *output);
	}
	else if (program["-l"] == true) {
		Analyse(*input, *output);
	}
	else {
		fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
		exit(2);
	}
	return 0;
}