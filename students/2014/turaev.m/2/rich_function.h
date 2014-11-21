#ifndef RICH_FUNCTION_H
#define RICH_FUNCTION_H

#include "mathvm.h"
#include "ast.h"

using namespace std;
using namespace mathvm;

class RichFunction : public BytecodeFunction {
    map<string, uint16_t> _local_variables;
    uint16_t _variable_id;
public:
    RichFunction(AstFunction *function): BytecodeFunction(function) {
        for (uint16_t i = 0; i < parametersNumber(); ++i) {
            _local_variables[parameterName(i)] = _variable_id++;
        }
    }

    uint16_t getVariableId(const string &name) {
        return _local_variables[name];
    }

    void addLocalVariable(const string &name) {
        _local_variables[name] =  _variable_id++;
    }

    virtual ~RichFunction() {

    }
};

#endif