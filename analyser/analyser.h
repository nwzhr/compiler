#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {



    class Function{
    public:
        Function(int index,int param,int level) : index(index), param(param), level(level) {}
    public:
        int index;
        int param;
        int level;


    };

    class C0{
    public:
        C0(std::vector<std::vector<Instruction>> stack,
           std::vector<Function> functions,
           std::list<std::pair<std::string,std::string >> notvarconstlist):
           stack(stack),functions(functions),notvarconstlist(notvarconstlist){}

        C0(){}
        std::list<std::pair<std::string,std::string >> getConsts() {
            return notvarconstlist;
        }

        std::vector<Function> getFunction(){
            return functions;
        }

        std::vector<Instruction> getBegin(){
            return stack[0];
        }

        std::vector<std::vector<Instruction>> getProgram(){
            return stack;
        }

    private:
        std::vector<std::vector<Instruction>> stack;
        std::vector<Function> functions;
        std::list<std::pair<std::string,std::string >> notvarconstlist;

    };

	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_constlist({}), cptr(_constlist.begin()),
			_varlist({}), vptr(_varlist.begin()),
			_uninitvarlist({}), uvptr(_uninitvarlist.begin()),
			_nextTokenIndex(0),_nextFuncIndex(0),_nextConstIndex(0){}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		std::list<std::pair<std::string,std::string >> getNotVarConsts(){
		    return _notvarconstlist;
		}

		std::vector<Function> getFunctions(){
		    return _functions;
		}
		std::vector<std::vector<Instruction>> getProgram(){
		    return _stack;
		}
		std::vector<Instruction> getBegin(){
		    return _stack[0];
		}



		// 唯一接口
		std::pair<C0, std::optional<CompilationError>> Analyse();
	private:
		// 所有的递归子程序

		// <程序>
		std::optional<CompilationError> analyseProgram();
		// <主过程>
		std::optional<CompilationError> analyseMain();
		// <常量声明>
		std::optional<CompilationError> analyseConstantDeclaration();
		// <变量声明>
		std::optional<CompilationError> analyseVariableDeclaration();

        std::optional<CompilationError> analyseFunctionDefinition();

        int analyseParameterDeclaration();

        std::optional<CompilationError> analyseCompoundStatement();
		// <语句序列>
		std::optional<CompilationError> analyseStatement();

		std::optional<CompilationError> analyseStatementSequence();
		// <常表达式>
		// 这里的 out 是常表达式的值
		std::optional<CompilationError> analyseConstantExpression(int32_t& out);
		// <表达式>
		std::optional<CompilationError> analyseExpression();

        std::optional<CompilationError> analyseCondition();

        std::optional<CompilationError> analyseConditionStatement();
		// <赋值语句>
		std::optional<CompilationError> analyseAssignmentStatement();
		// <输出语句>
		std::optional<CompilationError> analyseOutputStatement();
		// <项>
		std::optional<CompilationError> analyseAdditiveExpression();
		// <因子>
		std::optional<CompilationError> analyseMultiplicativeExpression();

        std::optional<CompilationError> analyseUnaryExpression();

        std::optional<CompilationError> analysePrimaryExpression();

        std::optional<CompilationError> analyseFunctionCall();

        std::optional<CompilationError> analyseLoopStatement();

        std::optional<CompilationError> analyseInputStatement();

        std::optional<CompilationError> analyseJumpStatement();
		// Token 缓冲区相关操作

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token
		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&, std::list<std::pair<std::string, int32_t>>&);
		// 添加变量、常量、未初始化的变量
        void addFunction(std::string fname, int param, int level);
		void addVariable(const Token&);
		void addConstant(const Token&);
		void addNotVarConstant(const Token&);
		void addUninitializedVariable(const Token&);

        bool isDeclaredFunction(const std::string&);
		// 是否被声明过
		bool isDeclared(const std::string&);
		// 是否是未初始化的变量
		bool isUninitializedVariable(const std::string&);
		// 是否是已初始化的变量
		bool isInitializedVariable(const std::string&);
		// 是否是常量
		bool isConstant(const std::string&);
		// 获得 {变量，常量} 在栈上的偏移
        int32_t getNotVarConstantIndex(const std::string&);
        int32_t getFunctionIndex(const std::string&);
		std::pair<bool,int32_t >getIndex(const std::string&);
	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		std::vector<Instruction> _instructions;
		std::vector<std::vector<Instruction>> _stack;
		std::vector<Function> _functions;
		std::pair<uint64_t, uint64_t> _current_pos;

		std::map<std::string, int32_t> _functable;
        std::map<std::string, int32_t> _notvarconsttable;

        std::list<std::pair<std::string,std::string >> _notvarconstlist;
        std::list<Function> _funclist;
        //this list maintains those function name and numbers
        std::list<std::pair<std::string,int32_t >> _constlist;
        std::list<std::pair<std::string,int32_t >>::iterator cptr;
        std::list<std::pair<std::string,int32_t >> _varlist;
        std::list<std::pair<std::string,int32_t >>::iterator vptr;
        std::list<std::pair<std::string,int32_t >> _uninitvarlist;
        std::list<std::pair<std::string,int32_t >>::iterator uvptr;

		// 下一个 token 在栈的偏移
		int32_t _nextTokenIndex;
		//next function
        int32_t _nextFuncIndex;
        //next Constant
        int32_t _nextConstIndex;
    };


}
