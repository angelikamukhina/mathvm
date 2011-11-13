#include "jit.h"
#include "mathvm.h"
#include "parser.h"
#include "visitors.h"

#include <asmjit/AsmJit.h>
#include <asmjit/MemoryManager.h>
#include <asmjit/Compiler.h>
#include "bytecode_visitor.h"

using namespace AsmJit;
using namespace std;

namespace mathvm {

MachCodeImpl::MachCodeImpl() : _code(0) {
}

MachCodeImpl::~MachCodeImpl() {
  MemoryManager::getGlobal()->free(_code);
}

Status* MachCodeImpl::execute(vector<Var*>& vars) {
  int result = function_cast<int (*)()>(_code)();
  cout << "returned " << result << endl;
  return new Status();
}

MachCodeFunction* MachCodeImpl::functionByName(const string& name) {
    return dynamic_cast<MachCodeFunction*>(Code::functionByName(name));
}

MachCodeFunction* MachCodeImpl::functionById(uint16_t id) {
    return dynamic_cast<MachCodeFunction*>(Code::functionById(id));
}

void MachCodeImpl::error(const char* format, ...) {
}

class MachCodeGenerator : public AstVisitor {
    AstFunction* _top;
    MachCodeImpl* _code;
    Assembler _;
public:
    MachCodeGenerator(AstFunction* top,
                      MachCodeImpl* code) :
    _top(top), _code(code) {
    }

    Status* generate();

#define VISITOR_FUNCTION(type, name)            \
    virtual void visit##type(type* node) {}

    FOR_NODES(VISITOR_FUNCTION)
#undef VISITOR_FUNCTION
};

union type {
  double d;
  int64_t i;
  uint16_t s;
};

int printi(long long int i) {
  return printf("%lld\n", i);
}

int printd(long long int i) {
  double d = *reinterpret_cast<double*>(&i);
  return printf("%lf\n", d);
}

int prints(char* i) {
  return printf("%s", i);
}

Status* MachCodeGenerator::generate() {
  BytecodeFunction* function = new BytecodeFunction(_top);
  _code->addFunction(function);
  _code->makeStringConstant("");
  BytecodeVisitor* visitor = new BytecodeVisitor(_code);
  visitor->visitBlockNode(_top->node()->body());
  function->bytecode()->add(BC_STOP);
  // function->bytecode()->dump(std::cout);
  Bytecode *bytecode_ = function->bytecode();

  uint8_t command = 0;
  uint32_t pos = 0;
  uint16_t id = 0;
  id = id;
  int16_t offset = 0;
  offset = offset; 
  type arg; 
  arg = arg;
  std::vector<double*> dvar;
  std::vector<int64_t*> ivar;
  std::vector<uint8_t*> svar;
  std::vector<void*> var;

  // Prologue.
  _.push(nbp);
  _.push(rax);
  _.push(rbx);
  _.mov(nbp, nsp);

  while (command != BC_STOP) {
    command = bytecode_->get(pos);
    // std::cerr << "Command " << command << " at " << pos << std::endl;
    ++pos;
    switch (command) {
      case (BC_DLOAD): {
        arg.d = bytecode_->getDouble(pos);
        _.push(imm(*reinterpret_cast<sysint_t*>(&arg.d)));
        pos += 8;
        break;
      }
      case (BC_ILOAD): {
        arg.i = bytecode_->getInt64(pos);
        _.push(imm(arg.i));
        pos += 8;
        break;
      }
      case (BC_SLOAD): {
        arg.s = bytecode_->getInt16(pos);
        _.push(imm(arg.s));
        pos += 2;
        break;
      }
      case (BC_DLOAD0): {
        arg.d = 0.0;
        _.push(imm(*reinterpret_cast<sysint_t*>(&arg.d)));
        break;
      }
      case (BC_ILOAD0): {
        arg.i = 0;
        _.push(imm(arg.i));
        break;
      }
      case (BC_SLOAD0): {
        arg.s = 0;
        _.push(imm(arg.s));
        break;
      }
      case (BC_DLOAD1): {
        arg.d = 0.0;
        _.push(imm(*reinterpret_cast<sysint_t*>(&arg.d)));
        break;
      }
      case (BC_ILOAD1): {
        arg.i = 0;
        _.push(imm(arg.i));
        break;
      }

      // case (BC_DADD): {
      //   arg = stack.top();
      //   stack.pop();
      //   arg.d += stack.top().d;
      //   stack.pop();
      //   stack.push(arg);
      //   break;
      // }
      case (BC_IADD): {
        _.pop(rax);
        _.pop(rbx);
        _.add(rax, rbx);
        _.push(rax);
        break;
      }
      // case (BC_DSUB): {
      //   arg = stack.top();
      //   stack.pop();
      //   arg.d -= stack.top().d;
      //   stack.pop();
      //   stack.push(arg);
      //   break;
      // }
      case (BC_ISUB): {
        _.pop(rax);
        _.pop(rbx);
        _.sub(rax, rbx);
        _.push(rax);
        break;
      }
      // case (BC_DMUL): {
      //   arg = stack.top();
      //   stack.pop();
      //   arg.d *= stack.top().d;
      //   stack.pop();
      //   stack.push(arg);
      //   break;
      // }
      case (BC_IMUL): {
        _.pop(rax);
        _.pop(rbx);
        _.imul(rax, rbx);
        _.push(rax);
        break;
      }
      // case (BC_DDIV): {
      //   arg = stack.top();
      //   stack.pop();
      //   arg.d /= stack.top().d;
      //   stack.pop();
      //   stack.push(arg);
      //   break;
      // }
      // case (BC_IDIV): {
      //   _.pop(rax);
      //   _.pop(rbx);
      //   _.idiv(rbx);
      //   _.push(rax);
      //   break;
      // }
      // case (BC_DNEG): {
      //   arg = stack.top();
      //   arg.d = -arg.d;
      //   stack.pop();
      //   stack.push(arg);
      //   break;
      // }
      // case (BC_INEG): {
      //   arg = stack.top();
      //   arg.i = -arg.i;
      //   stack.pop();
      //   stack.push(arg);
      //   break;
      // }
      case (BC_IPRINT): {
        _.mov(ndi, ptr(nsp));
        _.call((void*)printi); 
        break;
      }
      // case (BC_DPRINT): {
      //   _.mov(ndi, ptr(nsp));
      //   _.call((void*)printd); 
      //   break;
      // }
      // case (BC_SPRINT): {
      //   arg = stack.top();
      //   std::string s = constantById(arg.s);
      //   printf("%s", s.c_str());
      //   break;
      // }
      // case (BC_I2D): {
      //   arg = stack.top();
      //   stack.pop();
      //   arg.d = (double)arg.i;
      //   stack.push(arg);
      //   break;
      // }
      // case (BC_D2I): {
      //   arg = stack.top();
      //   stack.pop();
      //   arg.i = (uint64_t)arg.d;
      //   stack.push(arg);
      //   break;
      // }
      case (BC_SWAP): {
        _.pop(rax);
        _.pop(rbx);
        _.push(rax);
        _.push(rbx);
        break;
      }
      case (BC_POP): {
        _.pop(rax);
        break;
      }
      case (BC_LOADDVAR): {
        id = bytecode_->getInt16(pos);
        _.push(sysint_ptr_abs(var[id]));
        pos += 2;
        break;
      }
      case (BC_LOADIVAR): {
        id = bytecode_->getInt16(pos);
        _.push(sysint_ptr_abs(var[id]));
        pos += 2;
        break;
      }
      case (BC_LOADSVAR): {
        id = bytecode_->getInt16(pos);
        _.push(sysint_ptr_abs(var[id]));
        pos += 2;
        break;
      }
      case (BC_STOREDVAR): {
        id = bytecode_->getInt16(pos);
        if (id >= var.size()) {
          var.push_back(new GPVar());
        }
        _.pop(sysint_ptr_abs(var[id]));
        pos += 2;
        break;
      }
      case (BC_STOREIVAR): {
        id = bytecode_->getInt16(pos);
        if (id >= var.size()) {
          var.push_back(new GPVar());
        }
        _.pop(sysint_ptr_abs(var[id]));
        pos += 2;
        break;
      }
      case (BC_STORESVAR): {
        id = bytecode_->getInt16(pos);
        if (id >= var.size()) {
          var.push_back(new GPVar());
        }
        _.pop(sysint_ptr_abs(var[id]));
        pos += 2;
        break;
      }
      // case (BC_JA): {
      //   offset = bytecode_->getInt16(pos);
      //   pos += offset;
      //   break;
      // }
      // case (BC_IFICMPNE): {
      //   offset = bytecode_->getInt16(pos);
      //   arg = stack.top();
      //   stack.pop();
      //   type arg2 = stack.top();
      //   stack.pop();
      //   if (arg.i != arg2.i) pos += offset; else pos += 2;
      //   break;
      // }
      // case (BC_IFICMPE): {
      //   offset = bytecode_->getInt16(pos);
      //   arg = stack.top();
      //   stack.pop();
      //   type arg2 = stack.top();
      //   stack.pop();
      //   if (arg.i == arg2.i) pos += offset; else pos += 2;
      //   break;
      // }
      // case (BC_IFICMPG): {
      //   offset = bytecode_->getInt16(pos);
      //   arg = stack.top();
      //   stack.pop();
      //   type arg2 = stack.top();
      //   stack.pop();
      //   if (arg.i > arg2.i) pos += offset; else pos += 2;
      //   break;
      // }
      // case (BC_IFICMPGE): {
      //   offset = bytecode_->getInt16(pos);
      //   arg = stack.top();
      //   stack.pop();
      //   type arg2 = stack.top();
      //   stack.pop();
      //   if (arg.i >= arg2.i) pos += offset; else pos += 2;
      //   break;
      // }
      // case (BC_IFICMPL): {
      //   offset = bytecode_->getInt16(pos);
      //   arg = stack.top();
      //   stack.pop();
      //   type arg2 = stack.top();
      //   stack.pop();
      //   if (arg.i < arg2.i) pos += offset; else pos += 2;
      //   break;
      // }
      // case (BC_IFICMPLE): {
      //   offset = bytecode_->getInt16(pos);
      //   arg = stack.top();
      //   stack.pop();
      //   type arg2 = stack.top();
      //   stack.pop();
      //   if (arg.i <= arg2.i) pos += offset; else pos += 2;
      //   break;
      // }
      case (BC_STOP): {
        break;
      }
      // case (BC_CALL): {
      //   uint16_t id = bytecode_->getInt16(pos);
      //   pos += 2; 
      //   call_stack.push(std::make_pair(bytecode_, pos));
      //   bytecode_ = ((BytecodeFunction *)functionById(id))->bytecode();
      //   pos = 0;
      //   break;
      // }
      // case (BC_RETURN): {
      //   bytecode_ = call_stack.top().first;
      //   pos = call_stack.top().second;
      //   call_stack.pop();
      //   break;
      // }
      default: {
        printf("Unrecognized command! %d\n", command);
      }
    }
  }

  // Return value.
  _.mov(nax, 42);   

  // Epilogue.
  _.mov(nsp, nbp);
  _.pop(rbx);
  _.pop(rax);
  _.pop(nbp);
  _.ret();
   
  _code->setCode(_.make());

  return 0;
}

Status* MachCodeTranslatorImpl::translateMachCode(const string& program,
                                                  MachCodeImpl* *result) {
  MachCodeImpl* code = 0;
  Status* s = 0;
  Parser parser;

  // Build an AST.
  s = parser.parseProgram(program);
  if (s == 0) {
    code = new MachCodeImpl();
    MachCodeGenerator codegen(parser.top(), code);
    s = codegen.generate();
  }

  if (s != 0) {
    delete code;
  } else {
    *result = code;
  }
  return s;
}

MachCodeTranslatorImpl::MachCodeTranslatorImpl() {
}

MachCodeTranslatorImpl::~MachCodeTranslatorImpl() {
}

Status* MachCodeTranslatorImpl::translate(const string& program, Code* *result) {
    MachCodeImpl* code = 0;
    Status* status = 0;

    status = translateMachCode(program, &code);
    if (status != 0) {
        assert(code == 0);
        *result = 0;
        return status;
    }

    //code->disassemble();
    assert(code);
    *result = code;

    return new Status();
}

MachCodeFunction::MachCodeFunction(AstFunction* function) :
  TranslatedFunction(function) {
}

MachCodeFunction::~MachCodeFunction() {
}

void MachCodeFunction::disassemble(ostream& out) const {
}

}