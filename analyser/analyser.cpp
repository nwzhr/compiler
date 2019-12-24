#include "analyser.h"

#include <climits>

namespace miniplc0 {
    bool if_return = false;
    int index = 0;

    std::pair<C0, std::optional<CompilationError>> Analyser::Analyse() {
        auto err = analyseProgram();
        if (err.has_value())
            return std::make_pair(C0(), err);
        else
            return std::make_pair(C0(_stack,_functions,_notvarconstlist), std::optional<CompilationError>());
    }

    // <程序> ::= 'begin'<主过程>'end'
    std::optional<CompilationError> Analyser::analyseProgram() {

        auto err = analyseVariableDeclaration();
        if(err.has_value())
            return err;

        err = analyseFunctionDefinition();
        if (err.has_value())
            return err;

        auto next = nextToken();
        if(next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidProgram);
        return {};
    }

    std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
        // 变量声明语句可能有一个或者多个
        while (true) {
            // 预读？
            auto next = nextToken();
            if (!next.has_value()) //if there's nothing, return, cause there may be no more declarations
                return {};
            bool if_const = false;
            // 'const'
            if (next.value().GetType() != TokenType::CONST &&
                next.value().GetType() != TokenType::INT)//may be no variable declaration? return directly
            {
                unreadToken();
                return {};
            }
            //after this if statement, the token must be const or int
            if (next.value().GetType() == TokenType::CONST) {
                next = nextToken(); //it's const, then read in the next token
                if_const = true;
                if (!next.has_value() || next.value().GetType() != TokenType::INT)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteConst);
            }
            //when reaching here, the token must be int
            //because it will be directly int or int after const

            //read in the 1st identifier
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVD);
            // <标识符>
            while (1) {
                if (!next.has_value()) //if no value? there must be some faults,cause it should end up with ';'
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVD);
                else if (next.value().GetType() == TokenType::SEMICOLON) {
                    unreadToken();
                    break;
                } else if (next.value().GetType() == TokenType::IDENTIFIER) {
                    if(isDeclared(next.value().GetValueString()))
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
                    Token tmp = next.value();//if not declared, see if need initialization?

                    next = nextToken();
                    //std::cout << next.value().GetValueString() << std::endl;
                    if (!next.has_value())
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteVD);
                    else if (next.value().GetType() ==
                             TokenType::LEFT_BRACKET) //if there's a '(', it proves that it could be a function definition instead of a declaration
                    {
                        if (if_const == true)
                            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFD);
                        unreadToken(); //left bracket out
                        unreadToken(); //identifier out
                        unreadToken(); //INT out
                        return {};
                    }
                    else if (next.value().GetType() != TokenType::COMMA_SIGN &&
                               next.value().GetType() != TokenType::ASSIGN_SIGN &&
                               next.value().GetType() != TokenType::SEMICOLON)
                        return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
                    //else if (next.value().GetType() == TokenType::ASSIGN_SIGN && if_const == true) //if after the identifier is not '=' and it's a const declaration, it's wrong
                        //return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);

                    if (next.value().GetType() == TokenType::SEMICOLON) {
                        addUninitializedVariable(next.value());
                        index = index+5;
                        _instructions.emplace_back(ipush,0);
                        unreadToken();
                        break;
                    }
                    if (next.value().GetType() == TokenType::ASSIGN_SIGN) {
                        //printf("here\n");
                        auto err = analyseExpression();
                        if (err.has_value())
                            return err;
                        if (if_const == true)
                            addConstant(tmp);
                        else
                            addVariable(tmp);

                    } else if (next.value().GetType() == TokenType::COMMA_SIGN) {
                        addUninitializedVariable(next.value());
                        index = index+5;
                        _instructions.emplace_back(ipush,0); //uninitialized means 0
                        continue;
                    }

                } else {
                    std::cout << next.value().GetValueString();
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidVD);//here
                }
                next = nextToken();
            }
            // ';'
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
        }
        return {};
    }

    std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
        bool if_main = false;
        while (true) {
            auto next = nextToken();

            if (!next.has_value() && if_main == false)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrLackOfMain);
            else if (!next.has_value()) {
                _stack.emplace_back(_instructions);
                _instructions.empty();
                break;
            }
            _stack.emplace_back(_instructions);
            _instructions.empty();

            index = 0;
            _nextTokenIndex = 0;

            if (next.value().GetType() != TokenType::INT && next.value().GetType() != TokenType::VOID)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFD);
            if(next.value().GetType() == TokenType::VOID)
                if_return = false;
            else
                if_return = true;

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFD);
            if(isDeclaredFunction(next.value().GetValueString()))
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
            addNotVarConstant(next.value());
            std::string fname = next.value().GetValueString();
            //entering a new block
            if (next.value().GetValueString() == "main")
                if_main = true;

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPD);
            int err = analyseParameterDeclaration();
            if (err == -1)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPD);;
            addFunction(fname,err,1);

            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPD);

            auto err2 = analyseCompoundStatement();
            if(err2.has_value())
                return err2;

            //exit the current block
            _constlist.erase(cptr,_constlist.end());
            _varlist.erase(vptr,_varlist.end());
            _uninitvarlist.erase(uvptr,_uninitvarlist.end());
        }
        return {};
    }

    int Analyser::analyseParameterDeclaration() {
        bool if_const = false;
        int param_cnt = 0;
        while (1) {
            if_const = false;
            auto next = nextToken();
            if (!next.has_value())
                return -1;
            else if (next.value().GetType() != TokenType::CONST && next.value().GetType() != TokenType::INT)
            {
                unreadToken();
                return param_cnt;
            }
            param_cnt++;
            if (next.value().GetType() == TokenType::CONST) {
                if_const = true;
                next = nextToken();
                if (!next.has_value() || next.value().GetType() != TokenType::INT)
                    return -1;
            }
            next = nextToken();
            if (!next.has_value() || next.value().GetType() != TokenType ::IDENTIFIER)
                return -1;

            if(if_const == true)
                addConstant(next.value());
            else
                addVariable(next.value());
            next = nextToken();
            if (!next.has_value())
                return -1;
            else if (next.value().GetType() != TokenType::COMMA_SIGN &&
                     next.value().GetType() != TokenType::RIGHT_BRACKET)
                return -1;
            if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
                unreadToken();
                break;
            }
        }

        // ')'
        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
            return -1;
        unreadToken();
        return param_cnt;
    }

    std::optional<CompilationError> Analyser::analyseCompoundStatement() {
        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrCompoundState);
        auto err = analyseVariableDeclaration();
        if (err.has_value())
            return err;

        err = analyseStatementSequence();
        if(err.has_value())
            return err;

        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrCompoundState);

        return {};
    }

    // <语句序列> ::= {<语句>}
    // <语句> :: = <赋值语句> | <输出语句> | <空语句>
    // <赋值语句> :: = <标识符>'='<表达式>';'
    // <输出语句> :: = 'print' '(' <表达式> ')' ';'
    // <空语句> :: = ';'
    // 需要补全
    std::optional<CompilationError> Analyser::analyseStatement()
    {
        auto next = nextToken();
        if (!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidState);

        unreadToken();
        if (next.value().GetType() != TokenType::IDENTIFIER &&
            next.value().GetType() != TokenType::LEFT_BRACE &&
            next.value().GetType() != TokenType::SEMICOLON &&
            next.value().GetType() != TokenType::IF &&
            next.value().GetType() != TokenType::WHILE &&
            next.value().GetType() != TokenType::RETURN &&
            next.value().GetType() != TokenType::PRINT &&
            next.value().GetType() != TokenType::SCAN) {
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidState);
        }
        std::optional<CompilationError> err;
        switch (next.value().GetType()) {
            // 这里需要你针对不同的预读结果来调用不同的子程序
            // 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回
            case TokenType::IF:
                err = analyseConditionStatement();
                if(err.has_value())
                    return err;
                //printf("here\n");
                break;
            case TokenType::WHILE:
                err = analyseLoopStatement();
                if(err.has_value())
                    return err;
                break;
            case TokenType::IDENTIFIER:
                next = nextToken();
                next = nextToken();
                if(!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidState);
                else if((next.value().GetType() != TokenType::LEFT_BRACKET)&&(next.value().GetType() != TokenType::EQUAL_SIGN))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidState);

                unreadToken();
                unreadToken();
                if(next.value().GetType() == TokenType::LEFT_BRACKET)
                {
                    err = analyseFunctionCall();
                    if(err.has_value())
                        return err;
                }
                else if(next.value().GetType() == TokenType::EQUAL_SIGN)
                {
                    err = analyseAssignmentStatement();
                    if (err.has_value())
                        return err;
                }

                next = nextToken();
                if(!next.has_value()||next.value().GetType() != TokenType::SEMICOLON)
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidState);

                break;
            case TokenType ::LEFT_BRACE:
                err = analyseCompoundStatement();
                if(err.has_value())
                    return err;
                break;
            case TokenType ::RETURN:
                err = analyseJumpStatement();
                if(err.has_value())
                    return err;
                break;
            case TokenType::SCAN:
                err = analyseInputStatement();
                if(err.has_value())
                    return err;
                break;
            case TokenType::PRINT:
                err = analyseOutputStatement();
                if (err.has_value())
                    return err;
                break;
            case TokenType::SEMICOLON:
                next = nextToken();
                break;
            default:
                break;
        }
        return {};
    }

    std::optional<CompilationError> Analyser::analyseStatementSequence() {
        while (true) {
            // 预读
            auto next = nextToken();
            if(next.value().GetType() == TokenType::RIGHT_BRACE){
                unreadToken();
                break;
            }
            unreadToken();
            auto err = analyseStatement();
            if(err.has_value())
                return err;
        }
        return {};
    }
    std::optional<CompilationError> Analyser::analyseCondition(){
        //then condition::=expression [relational-op expression]
        auto err = analyseExpression();
        if(err.has_value())
            return err;
        //printf("here\n");
        auto next = nextToken();

        if(next.has_value()){
            if(next.value().GetType() != TokenType::LESS_SIGN&&
               next.value().GetType() != TokenType::LESSEQUAL_SIGN&&
               next.value().GetType() != TokenType::MORE_SIGN&&
               next.value().GetType() != TokenType::MOREEQUAL_SIGN&&
               next.value().GetType() != TokenType::NEQUAL_SIGN&&
               next.value().GetType() != TokenType::EQUAL_SIGN)
            {
                unreadToken();//after expression ,there's no relational operator
            }
            else // there's a relational operator
            {
                err = analyseExpression();
                if(err.has_value())
                    return err;
                if(next.value().GetType() == EQUAL_SIGN)
                {
                    _instructions.emplace_back(icmp,0);
                }
                else if(next.value().GetType() == NEQUAL_SIGN)
                {
                    _instructions.emplace_back(isub,0);
                }
                else if(next.value().GetType() == MORE_SIGN)
                    _instructions.emplace_back(icmp,0);
                else if(next.value().GetType() == MOREEQUAL_SIGN)
                {
                    _instructions.emplace_back(icmp,0);
                    _instructions.emplace_back(ipush,1);
                    _instructions.emplace_back(iadd,0);
                }
                else if(next.value().GetType() == LESS_SIGN)
                {
                    _instructions.emplace_back(icmp,0);
                    _instructions.emplace_back(ineg,0);
                }
                else if(next.value().GetType() == LESSEQUAL_SIGN)
                {
                    _instructions.emplace_back(icmp,0);
                    _instructions.emplace_back(ipush,1);
                    _instructions.emplace_back(isub,0);
                }
            }
        }


        return {};
    }
    //if ( condition) statement [else statement]
    std::optional<CompilationError> Analyser::analyseConditionStatement() {
        //the first use of nextToken will get 'if'
        auto next = nextToken();

        //read in '('
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrCondState);

        auto err = analyseCondition();
        if(err.has_value())
            return err;
        index = index+3;
        _instructions.emplace_back(je,0);


        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrCondState);

        //printf("here\n");
        err = analyseStatement();
        if(err.has_value())
            return err;

        //printf("here\n");
        next = nextToken();
        if(next.has_value()&&next.value().GetType() == TokenType::ELSE)
        {
            _instructions[_instructions.size()-1].SetX(index);
            err = analyseStatement();
            if(err.has_value())
                return err;
        }
        else
            unreadToken();
        //printf("here\n");
        return {};
    }

    //while ( condition ) statement
    std::optional<CompilationError> Analyser::analyseLoopStatement()
    {
        //the first token is definitely 'while'
        auto next = nextToken();

        //read in '('
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteLoop);

        auto err = analyseCondition();
        if(err.has_value())
            return err;

        _instructions.emplace_back(jne,0);
        index++;
        int jne_index = _instructions.size()-1;

        //read in ')'
        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteLoop);

        err = analyseStatement();
        if(err.has_value())
            return err;

        _instructions.emplace_back(Operation::jmp,jne_index);
        index++;
        _instructions[jne_index].SetX(index);
        return {};
    }

    //jump::=return-statement::=return [expression] ;
    std::optional<CompilationError> Analyser::analyseJumpStatement() {
        //read in 'return'
        bool if_has_return = false;
        auto next = nextToken();

        if(!next.has_value()||next.value().GetType() != TokenType::RETURN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidJump);

        //is there an expression?
        next = nextToken();
        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidJump);
        else if(next.value().GetType() == TokenType::SEMICOLON)
        {
            unreadToken();
        }
        else
        {
            unreadToken();
            auto err = analyseExpression();
            if(err.has_value())
                return err;
            if_has_return = true;
        }
        //read in ';'
        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidJump);

        //printf("here\n");
        if(if_return == false&&if_has_return==false)
        {
            _instructions.emplace_back(popn,_nextTokenIndex);
            index = index + 3;
            _instructions.emplace_back(ret,0);
            index = index + 3;
        }
        else if(if_return == true&&if_has_return == true) {
            index = index + 1;
            _instructions.emplace_back(iret, 0);
        }
        else
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidJump);
        return {};
    }

    std::optional<CompilationError> Analyser::analyseOutputStatement() {
        auto next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::PRINT)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        //[printable-list] maybe there are just print()
        next = nextToken();
        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);
        else if(next.value().GetType() != TokenType::RIGHT_BRACKET)
        {
            unreadToken();
            auto err = analyseExpression();
            if(err.has_value())
                return err;
            index = index + 1;
            _instructions.emplace_back(iprint,0);
            //{, expression} ?  )
            while(1)
            {
                next = nextToken();
                if(!next.has_value())
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);
                else if(next.value().GetType() != TokenType::COMMA_SIGN) //the following token after an expression is not ',' ,it means printable-list ends
                {
                    unreadToken();
                    break;
                }

                //if it's a ','
                err = analyseExpression();
                if(err.has_value())
                    return err;
                index = index + 1;
                _instructions.emplace_back(iprint,0);
            }
        }
        else //no printable-list
            unreadToken();

        //then read in ')'
        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        //last, raed in ';'
        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);

        return {};
    }

    std::optional<CompilationError> Analyser::analyseInputStatement() {
        auto next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::SCAN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidScan);

        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidScan);

        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidScan);

        std::pair<bool,int32_t >p = getIndex(next.value().GetValueString());
        _instructions.emplace_back(loada,(p.first == false)?0:1,p.second);
        index = index + 3;
        _instructions.emplace_back(iscan,0);
        index = index + 3;
        _instructions.emplace_back(istore,0);
        index = index + 3;

        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidScan);

        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidScan);

        return {};
    }


    // <表达式> ::= <项>{<加法型运算符><项>}
    std::optional<CompilationError> Analyser::analyseExpression() {
        // <项>
        auto err = analyseAdditiveExpression();
        if (err.has_value())
            return err;

        return {};
    }

    // <赋值语句> ::= <标识符>'='<表达式>';'
    std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
        auto next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
        if (!isDeclared(next.value().GetValueString()))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
        if (isConstant(next.value().GetValueString()))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);

        Token name = next.value();
        next = nextToken();
        if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);

        auto err = analyseExpression();
        if (err.has_value())
            return err;


        //store the initialized value
        std::pair<bool, int32_t > p = getIndex(name.GetValueString());
        _instructions.emplace_back(loada,(p.first == false)?0:1,p.second);
        index = index + 3;

        _instructions.emplace_back(iscan,0);
        index = index + 3;

        _instructions.emplace_back(istore,0);
        index = index + 3;

        next = nextToken();
        if(!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
        return {};
    }

    std::optional<CompilationError> Analyser::analyseAdditiveExpression() {
        // 可以参考 <表达式> 实现
        auto err = analyseMultiplicativeExpression();
        if (err.has_value())
            return err;

        while (true) {
            auto next = nextToken();
            if (!next.has_value())
                return {};
            auto type = next.value().GetType();
            if (type != TokenType::PLUS_SIGN && type != TokenType::MINUS_SIGN) {
                unreadToken();
                return {};
            }

            err = analyseMultiplicativeExpression();
            if (err.has_value())
                return err;

            // 根据结果生成指令
            if (type == TokenType::PLUS_SIGN) {
                index = index + 1;
                _instructions.emplace_back(Operation::iadd, 0);
            }
            else if (type == TokenType::MINUS_SIGN) {
                index = index + 1;
                _instructions.emplace_back(Operation::isub, 0);
            }
        }
        return {};
    }

    std::optional<CompilationError> Analyser::analyseMultiplicativeExpression()
    {
        auto err = analyseUnaryExpression();
        if (err.has_value())
            return err;

        while (true) {
            auto next = nextToken();
            if (!next.has_value())
                return {};
            auto type = next.value().GetType();
            if (type != TokenType::MULTIPLICATION_SIGN && type != TokenType::DIVISION_SIGN) {
                unreadToken();
                return {};
            }

            err = analyseUnaryExpression();
            if (err.has_value())
                return err;

            // 根据结果生成指令
            if (type == TokenType::MULTIPLICATION_SIGN) {
                index = index + 1;
                _instructions.emplace_back(Operation::imul, 0);
            }
            else if (type == TokenType::DIVISION_SIGN) {
                index = index + 1;
                _instructions.emplace_back(Operation::idiv, 0);
            }
        }
        return {};
    }

    std::optional<CompilationError> Analyser::analyseUnaryExpression()
    {
        bool if_neg = false;
        auto next = nextToken();
        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidUE);
        if(next.value().GetType() == TokenType::MINUS_SIGN)//ineg?
            if_neg = true;
        else if(next.value().GetType() == TokenType::PLUS_SIGN);
        else
            unreadToken();

        auto err = analysePrimaryExpression();
        if(err.has_value())
            return err;
        if(if_neg) {
            index = index + 1;
            _instructions.emplace_back(ineg, 0);
        }
        return {};
    }

    std::optional<CompilationError> Analyser::analysePrimaryExpression() {
        auto next = nextToken();

        if(!next.has_value())
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPExpression);
        if(next.value().GetType() == TokenType::LEFT_BRACKET)
        {
            auto err = analyseExpression();
            if(err.has_value())
                return err;
            next = nextToken();
            if(!next.has_value()||next.value().GetType() != TokenType::RIGHT_BRACKET)
            {
                return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPExpression);
            }
        }
        else if(next.value().GetType() == TokenType::IDENTIFIER)
        {
            std::string name = next.value().GetValueString();
            next = nextToken();
            if(isDeclared(name))
            {
                if(isUninitializedVariable(name))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPExpression);

                std::pair<bool,int32_t > p = getIndex(name);
                _instructions.emplace_back(loada,(p.first == false)?0:1,p.second);
                index = index + 4;

                _instructions.emplace_back(iload,0);
                index = index + 4;
            }
            if(next.has_value()&&next.value().GetType() == TokenType::LEFT_BRACKET)
            {
                if(!isDeclaredFunction(name))
                    return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPExpression);
                unreadToken();
                unreadToken();
                auto err = analyseFunctionCall();
                if(err.has_value())
                    return err;
            }
            else
                unreadToken();
        }
        else if(next.value().GetType() == TokenType::DECIMAL)
        {
            addNotVarConstant(next.value());
            index = index + 2;
            _instructions.emplace_back(loadc,_notvarconstlist.size()-1);
            return {};
        }
        else
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidUE);

        return {};
    }

    std::optional<CompilationError> Analyser::analyseFunctionCall()
    {
        auto next = nextToken();

        if(!next.has_value()||next.value().GetType() != TokenType::IDENTIFIER)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFC);

        if(!isDeclaredFunction(next.value().GetValueString()))
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFC);

        int32_t  func_index = getFunctionIndex(next.value().GetValueString());

        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::LEFT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFC);

        auto err = analyseExpression();
        if(err.has_value())
            return err;


        while(1)
        {
            next = nextToken();
            if(!next.has_value()||next.value().GetType() != TokenType::COMMA_SIGN)
            {
                unreadToken();
                break;
            }
            err = analyseExpression();
            if(err.has_value())
                return err;
        }

        next = nextToken();
        if(!next.has_value()||next.value().GetType() != TokenType::RIGHT_BRACKET)
            return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFC);

        index = index + 3;
        _instructions.emplace_back(call,func_index);
        return {};
    }

    std::optional<Token> Analyser::nextToken() {
        if (_offset == _tokens.size())
            return {};
        // 考虑到 _tokens[0..._offset-1] 已经被分析过了
        // 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
        _current_pos = _tokens[_offset].GetEndPos();
        return _tokens[_offset++];
    }

    void Analyser::unreadToken() {
        if (_offset == 0)
            DieAndPrint("analyser unreads token from the begining.");
        _current_pos = _tokens[_offset - 1].GetEndPos();
        _offset--;
    }

    void Analyser::_add(const Token& tk, std::list<std::pair<std::string, int32_t>>& mp) {
        if (tk.GetType() != TokenType::IDENTIFIER)
            DieAndPrint("only identifier can be added to the table.");
        mp.push_back(std::pair<std::string,int32_t>(tk.GetValueString(),_nextTokenIndex));
        _nextTokenIndex++;
    }

    void Analyser::addNotVarConstant(const Token &tk)
    {
        _notvarconstlist.push_back(std::make_pair(tk.GetValueString(),
                tk.GetType() == TokenType::IDENTIFIER?std::string("IDEN"):std::string("INT")));
        _notvarconsttable[tk.GetValueString()] = _notvarconstlist.size()-1;
    }

    int Analyser::getNotVarConstantIndex(const std::string& s)
    {
        return _notvarconsttable[s];
    }

    void Analyser::addConstant(const Token& tk) {
        _add(tk, _constlist);
    }

    std::pair<bool,int32_t > Analyser::getIndex(const std::string & s)
    {
        for(auto ptr = cptr;ptr != _constlist.end();ptr++)
        {
            if((*ptr).first == s)
                return std::pair(false,(*ptr).second);
        }
        for(auto ptr = uvptr;ptr != _uninitvarlist.end();ptr++)
        {
            if((*ptr).first == s)
                return std::pair(false,(*ptr).second);
        }
        for(auto ptr = vptr;ptr != _varlist.end();ptr++)
        {
            if((*ptr).first == s)
                return std::pair(false,(*ptr).second);
        }
        for(auto ptr = _constlist.begin();ptr != cptr;ptr++)
        {
            if((*ptr).first == s)
                return std::pair(true,(*ptr).second);
        }
        for(auto ptr = _uninitvarlist.begin();ptr != uvptr;ptr++)
        {
            if((*ptr).first == s)
                return std::pair(true,(*ptr).second);
        }
        for(auto ptr = _varlist.begin();ptr != vptr;ptr++)
        {
            if((*ptr).first == s)
                return std::pair(true,(*ptr).second);
        }
        return {};
    }

    void Analyser::addUninitializedVariable(const Token& tk) {
        _add(tk, _uninitvarlist);
    }

    void Analyser::addVariable(const Token& tk) {
        _add(tk, _varlist);
    }

    void Analyser::addFunction(std::string fname, int param, int level)
    {
        int index = getNotVarConstantIndex(fname);
        Function new_f(index,param,level);
        _functions.emplace_back(new_f);
        _functable[fname] = _functions.size()-1;
    }

    int32_t Analyser::getFunctionIndex(const std::string & s) {
        return _functable[s];
    }

    bool Analyser::isDeclaredFunction(const std::string& s) {
        return _functable.find(s) != _functable.end();
    }

    bool Analyser::isDeclared(const std::string& s) {
        return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
    }

    bool Analyser::isUninitializedVariable(const std::string& s) {
        for(std::pair<std::string, int32_t> & it : _uninitvarlist)
        {
            if(it.first == s)
                return true;
        }
        return false;
    }
    bool Analyser::isInitializedVariable(const std::string&s) {
        for(std::pair<std::string, int32_t> & it : _varlist)
        {
            if(it.first == s)
                return true;
        }
        return false;
    }

    bool Analyser::isConstant(const std::string&s) {
        for(std::pair<std::string, int32_t> & it : _constlist)
        {
            if(it.first == s)
                return true;
        }
        return false;
    }
}