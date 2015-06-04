#include "jit_expr.h"
#include <cctype>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <typeinfo>
#include <stack>

using namespace asmjit;

bool Lexer::is_operator(Token t) {
	return t == kPlus || t == kSubstract || t == kMultiply || t == kDivide
			|| t == kGreaterThan || t == kLessThan || t == kEqualTo
			|| t == kAnd || t == kOr;
}

int Lexer::precedence(Token t) {
	return t / 10;
}

Lexer::Token Lexer::get_token(Extra* x) {
	while (std::isspace(expression[offset])) {
		++offset;
	}
	switch (expression[offset]) {
	case '(':
		++offset;
		return kLeftParenthesis;
	case ')':
		++offset;
		return kRightParenthesis;
	case '+':
		++offset;
		return kPlus;
	case '-':
		++offset;
		return kSubstract;
	case '*':
		++offset;
		return kMultiply;
	case '/':
		++offset;
		return kDivide;
	case '<':
		++offset;
		return kLessThan;
	case '>':
		++offset;
		return kGreaterThan;
	case '=':
		if (expression[offset + 1] == '=') {
			offset += 2;
			return kEqualTo;
		}
		throw BadToken();
	case '&':
		if (expression[offset + 1] == '&') {
			offset += 2;
			return kAnd;
		}
		throw BadToken();
	case '|':
		if (expression[offset + 1] == '|') {
			offset += 2;
			return kOr;
		}
		throw BadToken();
	case '$':
		return get_variable(x);
	case '0': case '1': case '2': case '3': case '4': case '5': case '6':
	case '7': case '8': case '9':
		return get_float_literal(x);
	case '#': case '\0':
		return kEnd;
	default:
		throw BadToken();
	}
}

Lexer::Token Lexer::get_variable(Extra* x) {
#define DEFINE_ARRAY_MEMBER(name) \
	{#name, \
		{ \
			offsetof(GarbageFilterPara, name), \
			sizeof(((GarbageFilterPara*)0)->name), \
			typeid(((GarbageFilterPara*)0)->name) == typeid(double), \
		} \
	}

	static const struct {
		const char* name;
		Extra::Variable var;
	} kTable[] = {
			DEFINE_ARRAY_MEMBER(nStartTermDis),
			DEFINE_ARRAY_MEMBER(nWalkLimit),
			DEFINE_ARRAY_MEMBER(nDriveLimit),
			DEFINE_ARRAY_MEMBER(nAutoDisLimit),
			DEFINE_ARRAY_MEMBER(bWhiteListAggresive),
			DEFINE_ARRAY_MEMBER(nWalkDis),
			DEFINE_ARRAY_MEMBER(dTransferTimes),
			DEFINE_ARRAY_MEMBER(bPureSub),
			DEFINE_ARRAY_MEMBER(nDriveDis),
			DEFINE_ARRAY_MEMBER(bCanWalk),
			DEFINE_ARRAY_MEMBER(nRankPos),
			DEFINE_ARRAY_MEMBER(n1stTime),
			DEFINE_ARRAY_MEMBER(n1stWalkDis),
			DEFINE_ARRAY_MEMBER(n1stDriveDis),
			DEFINE_ARRAY_MEMBER(d1stTransferTimes),
			DEFINE_ARRAY_MEMBER(bFindOut1stPureSub),
			DEFINE_ARRAY_MEMBER(d1stPureSubTransferTimes),
			DEFINE_ARRAY_MEMBER(n1stPureSubWalkDis),
			DEFINE_ARRAY_MEMBER(n1stPureSubDriveDis)
	};
#undef DEFINE_ARRAY_MEMBER

	for (size_t i = 0; i < sizeof(kTable) / sizeof(kTable[0]); ++i) {
		int length = std::strlen(kTable[i].name);
		if (std::memcmp(kTable[i].name, expression + offset + 1,
				length) != 0) {
			continue;
		}
		char ch = expression[1 + length + offset];
		if ((ch <= 'z' && ch >= 'a') || (ch <= 'Z' && ch >= 'A')
				|| (ch >= '0' && ch <= '9') || ch == '_') {
			throw BadToken();
		}
		x->var = kTable[i].var;
		offset += length + 1;
		return kVariable;
	}
	throw BadToken();
}

Lexer::Token Lexer::get_float_literal(Extra* x) {
	char copy[100];
	int length = 0;
	char ch = expression[offset];
	while (ch == '.' || (ch >= '0' && ch <= '9')) {
		++length;
		ch = expression[offset + length];
	}
	if (std::sscanf(expression + offset, "%lg", &x->literal) != 1) {
		throw BadToken();
	}
	offset += length;
	return kFloatLiteral;
}

void JitExpr::push_value(X86Assembler& as, const X86XmmReg& src) {
	if (reg_index < kMaxRegIndex) {
		as.movsd(xmm[reg_index++], src);
		return;
	}
	stack_size += 8;
	as.movsd(x86::qword_ptr(x86::rsp, -stack_size), src);
}

void JitExpr::push_value(X86Assembler& as, double value) {
	literal_data[literal_count++] = value;
	int offset = (literal_count - 1) * 8;
	if (reg_index < kMaxRegIndex) {
		as.movsd(xmm[reg_index++], x86::qword_ptr(x86::rsi, offset));
		return;
	}
	as.mov(x86::rax, x86::qword_ptr(x86::rsi, offset));
	stack_size += 8;
	as.mov(x86::qword_ptr(x86::rsp, -stack_size), x86::rax);
}

void JitExpr::push_value(X86Assembler& as, const Lexer::Extra& extra) {
	if (extra.var.float_) {
		as.movsd(x86::xmm14, x86::qword_ptr(x86::rdi, extra.var.offset));
		push_value(as, x86::xmm14);
		return;
	}
	as.xor_(x86::rax, x86::rax);
	as.pxor(x86::xmm14, x86::xmm14);
	if (extra.var.size == 4) {
		as.mov(x86::eax, x86::dword_ptr(x86::rdi, extra.var.offset));
		as.cvtsi2sd(x86::xmm14, x86::eax);
		push_value(as, x86::xmm14);
		return;
	}
	Label label(as);
	as.mov(x86::al, x86::byte_ptr(x86::rdi, extra.var.offset));
	as.test(x86::eax, x86::eax);
	as.je(label);
	as.xor_(x86::rax, x86::rax);
	as.not_(x86::rax);
	as.movq(x86::xmm14, x86::rax);
	as.bind(label);
	push_value(as, x86::xmm14);
}

void JitExpr::pop_value(X86Assembler& as, const X86XmmReg& tgt) {
	if (stack_size > 0) {
		as.movsd(tgt, x86::qword_ptr(x86::rsp, -stack_size));
		stack_size -= 8;
		return;
	}
	if (no_value()) {
		throw BadExpr();
	}
	if (&tgt == &xmm[--reg_index]) {
		return;
	}
	as.movsd(tgt, xmm[reg_index]);
}

void JitExpr::emit_insn(X86Assembler& as, const X86XmmReg& first,
		Lexer::Token token, const X86XmmReg& second) {
	switch (token) {
	case Lexer::kPlus:
		as.addsd(first, second);
		break;
	case Lexer::kSubstract:
		as.subsd(first, second);
		break;
	case Lexer::kMultiply:
		as.mulsd(first, second);
		break;
	case Lexer::kDivide:
		as.divsd(first, second);
		break;
	case Lexer::kAnd: case Lexer::kOr:
		as.movq(x86::rax, first);
		as.movq(x86::rdx, second);
		if (token == Lexer::kAnd) {
			as.and_(x86::rax, x86::rdx);
		} else {
			as.or_(x86::rax, x86::rdx);
		}
		as.movq(first, x86::rax);
		break;
	case Lexer::kEqualTo:
		as.cmpsd(first, second, 0);
		break;
	case Lexer::kLessThan:
		as.cmpsd(first, second, 1);
		break;
	case Lexer::kGreaterThan:
		as.cmpsd(second, first, 1);
		break;
	default:
		throw BadExpr();
	}
}

void JitExpr::calculate(X86Assembler& as, Lexer::Token token) {
	if (stack_size == 0) {
		emit_insn(as, xmm[reg_index - 2], token, xmm[reg_index - 1]);
		--reg_index;
		return;
	}
	pop_value(as, x86::xmm15);
	pop_value(as, x86::xmm14);
	emit_insn(as, x86::xmm14, token, x86::xmm15);
	push_value(as, x86::xmm14);
}

bool JitExpr::compile(const char* expr) try {
	assert(function == NULL);

	X86Assembler as(&runtime);
	std::stack<Lexer::Token> op;
	Lexer lexer(expr);
	Lexer::Extra extra;
	bool has_literal = false;

	for (Lexer::Token token = lexer.get_token(&extra);
			token != Lexer::kEnd; token = lexer.get_token(&extra)) {
		if (Lexer::is_operator(token)) {
			if (op.empty()) {
				op.push(token);
				continue;
			}
			Lexer::Token top = op.top();
			while (Lexer::precedence(token) <= Lexer::precedence(top)) {
				op.pop();
				calculate(as, top);
				if (op.empty()) {
					break;
				}
				top = op.top();
			}
			op.push(token);
			continue;
		}
		if (token == Lexer::kLeftParenthesis) {
			op.push(token);
			continue;
		}
		if (token == Lexer::kRightParenthesis) {
			while (op.top() != Lexer::kLeftParenthesis) {
				calculate(as, op.top());
				op.pop();
			}
			op.pop();
			continue;
		}
		if (token == Lexer::kFloatLiteral) {
			if (!has_literal) {
				as.mov(x86::rsi, reinterpret_cast<intptr_t>(literal_data));
				has_literal = true;
			}
			push_value(as, extra.literal);
			continue;
		}

		assert(token == Lexer::kVariable);
		push_value(as, extra);
	}
	while (!op.empty()) {
		Lexer::Token top = op.top();
		calculate(as, top);
		op.pop();
	}
	if (reg_index != 1) {
		return false;
	}
	as.movq(x86::rax, x86::xmm0);
	as.test(x86::eax, x86::eax);
	as.setne(x86::al);
	as.ret();
	function = as.make();
	return true;
} catch (Lexer::BadToken&) {
	return false;
} catch (BadExpr&) {
	return false;
}
