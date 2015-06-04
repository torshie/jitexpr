#ifndef JIT_EXPR_H_
#define JIT_EXPR_H_

#include <asmjit/asmjit.h>
#include <exception>

struct GarbageFilterPara
{
	int nStartTermDis;
	int nWalkDis;
	int nWalkLimit;
	int nDriveDis;
	int nDriveLimit;
	int nAutoDisLimit;
	int n1stTime;
	int n1stWalkDis;
	int n1stDriveDis;
	double d1stTransferTimes;
	bool bCanWalk;
	int nRankPos;
	bool bWhiteListAggresive;
	bool bPureSub;

	bool bFindOut1stPureSub;
	double d1stPureSubTransferTimes;
	int n1stPureSubWalkDis;
	int n1stPureSubDriveDis;
	double dTransferTimes;
};

class Lexer {
public:
	enum Token {
		kEnd = 0,
		kLeftParenthesis,
		kRightParenthesis,
		kFloatLiteral,
		kVariable,

		kOr = 100,
		kAnd = 200,
		kEqualTo = 300,
		kGreaterThan = 400,
		kLessThan,
		kPlus = 500,
		kSubstract,
		kMultiply = 600,
		kDivide,
	};

	struct BadToken : public std::exception {};

	union Extra {
		struct Variable {
			int offset;
			int size;
			bool float_;
		} var;
		double literal;
	};

	explicit Lexer(const char* str) : offset(0), expression(str) {}

	static bool is_operator(Token t);
	static int precedence(Token t);

	Token get_token(Extra* x);

private:
	int offset;
	const char* expression;

	Token get_variable(Extra* x);
	Token get_float_literal(Extra* x);
};

class JitExpr {
public:
	JitExpr()
			: xmm(asmjit::x86RegData.xmm), reg_index(2), stack_size(0),
			literal_count(0), function(NULL) {}

	bool compile(const char* expr);

	bool operator () (const GarbageFilterPara& param) const {
		return value<bool>(param);
	}

	template<typename T>
	T value(const GarbageFilterPara& param) const {
		typedef T (*FP)(const GarbageFilterPara&);
		return reinterpret_cast<FP>(function)(param);
	}

private:
	enum { kMaxRegIndex = 3};

	class BadExpr : public std::exception {};

	const asmjit::X86XmmReg* xmm;
	int reg_index;
	int stack_size;
	asmjit::JitRuntime runtime;
	double literal_data[100];
	int literal_count;
	void* function;

	void push_value(asmjit::X86Assembler& as, const asmjit::X86XmmReg& src);
	void push_value(asmjit::X86Assembler& as, double value);
	void push_value(asmjit::X86Assembler& as, const Lexer::Extra& extra);
	void pop_value(asmjit::X86Assembler& as, const asmjit::X86XmmReg& tgt);
	void emit_insn(asmjit::X86Assembler& as,
			const asmjit::X86XmmReg& first, Lexer::Token token,
			const asmjit::X86XmmReg& second);
	void calculate(asmjit::X86Assembler& as, Lexer::Token token);

	bool no_value() const {
		return reg_index == 2;
	}
};

#endif // JIT_EXPR_H_
