#include "Evalvisitor.h"
#include "AnyOpt.h"
#include "func.h"
#include <string>
#include <unordered_map>

NameSpace scope;

std::any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx) {
	string func_name = ctx->NAME()->getText();
	scope.find(func_name)->second = std::shared_ptr<func>(std::make_shared<func_user>(*this, ctx));
	return {};
}

std::any EvalVisitor::visitTfpdef(Python3Parser::TfpdefContext *ctx) {
	return ctx->getText();
}

std::any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) {
	return visitSmall_stmt(ctx->small_stmt());
}

std::any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
	if (!ctx->ASSIGN(0) && !ctx->augassign()) return visitChildren(ctx);
	auto var_list = ctx->testlist();
	int n = static_cast<int>(var_list.size()) - 1;
	std::vector<std::any> rvalues = as<std::vector<std::any>>(visitTestlist(var_list.back()));
	int m = static_cast<int>(rvalues.size());
	for (auto &x : rvalues)
		testVar(x);
	AugassignType opt = AugassignType::equal;
	if (auto agu = ctx->augassign()) opt = as<AugassignType>(visitAugassign(agu));
	for (int i = 0; i < n; ++i) {
		std::vector<std::any> lvalues = as<std::vector<std::any>>(visitTestlist(var_list[i]));
		int mm = static_cast<int>(lvalues.size());
		for (int j = 0; j < m && j < mm; ++j) {
			std::any &var = lvalues[j];
			std::any const &val = rvalues[j];
			if (!is<VarType>(var))
				continue;
			if (opt == AugassignType::equal)
				as<VarType>(var)->second = val;
			else if (opt == AugassignType::add)
				as<VarType>(var)->second += val;
			else if (opt == AugassignType::minus)
				as<VarType>(var)->second -= val;
			else if (opt == AugassignType::mult)
				as<VarType>(var)->second *= val;
			else if (opt == AugassignType::div)
				as<VarType>(var)->second = DivFloat(as<VarType>(var)->second, val);
			else if (opt == AugassignType::idiv)
				as<VarType>(var)->second = DivInt(as<VarType>(var)->second, val);
			else if (opt == AugassignType::mod)
				as<VarType>(var)->second %= val;
		}
	}
	return {};
}

std::any EvalVisitor::visitAugassign(Python3Parser::AugassignContext *ctx) {
	if (ctx->ADD_ASSIGN())
		return AugassignType::add;
	else if (ctx->SUB_ASSIGN())
		return AugassignType::minus;
	else if (ctx->MULT_ASSIGN())
		return AugassignType::mult;
	else if (ctx->DIV_ASSIGN())
		return AugassignType::div;
	else if (ctx->IDIV_ASSIGN())
		return AugassignType::idiv;
	else if (ctx->MOD_ASSIGN())
		return AugassignType::mod;
	else
		return AugassignType::none;
}

std::any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) {
	if (ctx->break_stmt()) return FlowStmt(FlowStmt::FlowWord::break_stmt);
	else if (ctx->continue_stmt()) return FlowStmt(FlowStmt::FlowWord::continue_stmt);
	else if (auto ret = ctx->return_stmt()) {
		if (auto test = ret->testlist()) {
			std::any &&res = visitTestlist(test);
			std::vector<std::any> &vals = as<std::vector<std::any>>(res);
			for (auto &x : vals)
				testVar(x);
			if (vals.size() == 1)
				return FlowStmt(FlowStmt::FlowWord::return_stmt, std::move(vals[0]));
			else return FlowStmt(FlowStmt::FlowWord::return_stmt, std::move(vals));
		}
		else return FlowStmt(FlowStmt::FlowWord::return_stmt);
	}
	return {};
}

std::any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx) {
	static std::map<Python3Parser::If_stmtContext *, std::vector<Python3Parser::TestContext *>> TESTS;
	if (!ctx->IF()) return {};
	if (TESTS.find(ctx) == TESTS.end())
		TESTS[ctx] = ctx->test();
	std::vector<Python3Parser::TestContext *> const &tests = TESTS[ctx];
	int n = static_cast<int>(tests.size());
	int kth = 0;
	while (kth < n) {
		std::any &&val = visitTest(tests[kth]);
		testVar(val);
		if (toBool(val))
			return visitSuite(ctx->suite(kth));
		else
			++kth;
	}
	if (ctx->suite(kth)) return visitSuite(ctx->suite(kth));
	return {};
}

std::any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx) {
	if (!ctx->WHILE()) return {};
	while (true) {
		std::any &&val = visitTest(ctx->test());
		testVar(val);
		if (toBool(val)) {
			std::any &&res = visitSuite(ctx->suite());
			if (isNull(res)) continue;
			if (is<FlowStmt>(res)) {
				if (as<FlowStmt>(res).word == FlowStmt::FlowWord::break_stmt)
					break;
				else if (as<FlowStmt>(res).word == FlowStmt::FlowWord::continue_stmt)
					continue;
				else if (as<FlowStmt>(res).word == FlowStmt::FlowWord::return_stmt)
					return res;
			}
		}
		else
			break;
	}
	return {};
}

std::any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx) {
	if (ctx->simple_stmt()) {
		std::any &&val = visitSimple_stmt(ctx->simple_stmt());
		if (is<FlowStmt>(val)) return val;
	}
	for (int i = 0; ctx->stmt(i); ++i) {
		std::any &&val = visitStmt(ctx->stmt(i));
		if (is<FlowStmt>(val)) return val;
	}
	return {};
}

std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
	if (ctx->OR(0)) {
		static std::map<Python3Parser::Or_testContext *, std::vector<Python3Parser::And_testContext *>> TESTS;
		if (TESTS.find(ctx) == TESTS.end()) TESTS[ctx] = ctx->and_test();
		auto const &tests = TESTS[ctx];
		for (auto *const p : tests) {
			std::any &&val = visitAnd_test(p);
			testVar(val);
			if (toBool(val))
				return true;
		}
		return false;
	}
	else
		return visitChildren(ctx);
}

std::any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx) {
	if (ctx->AND(0)) {
		static std::map<Python3Parser::And_testContext *, std::vector<Python3Parser::Not_testContext *>> TESTS;
		if (TESTS.find(ctx) == TESTS.end()) TESTS[ctx] = ctx->not_test();
		auto const &tests = TESTS[ctx];
		for (auto *const p : tests) {
			std::any &&val = visitNot_test(p);
			testVar(val);
			if (!toBool(val))
				return false;
		}
		return true;
	}
	else
		return visitChildren(ctx);
}

std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx) {
	if (ctx->NOT()) {
		std::any &&val = visitNot_test(ctx->not_test());
		testVar(val);
		return !toBool(val);
	}
	else
		return visitComparison(ctx->comparison());
}

std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
	if (ctx->comp_op(0)) {
		static std::map<Python3Parser::ComparisonContext *, std::vector<Python3Parser::Arith_exprContext *>> EXPRS;
		static std::map<Python3Parser::ComparisonContext *, std::vector<Python3Parser::Comp_opContext *>> COMPS;
		if (EXPRS.find(ctx) == EXPRS.end()) EXPRS[ctx] = ctx->arith_expr();
		if (COMPS.find(ctx) == COMPS.end()) COMPS[ctx] = ctx->comp_op();

		auto const &nodes = EXPRS[ctx];
		auto const &comps = COMPS[ctx];
		std::any &&last = visitArith_expr(nodes[0]);
		testVar(last);
		int _n = static_cast<int>(nodes.size());
		for (int i = 1; i < _n; ++i) {
			std::any &&val = visitArith_expr(nodes[i]);
			testVar(val);
			auto *const p = comps[i - 1];
			bool res = true;
			if (p->LESS_THAN())
				res = last < val;
			else if (p->GREATER_THAN())
				res = last > val;
			else if (p->EQUALS())
				res = last == val;
			else if (p->GT_EQ())
				res = last >= val;
			else if (p->LT_EQ())
				res = last <= val;
			else if (p->NOT_EQ_2())
				res = last != val;
			if (!res) return false;
			last = std::move(val);
		}
		return true;
	}
	else
		return visitChildren(ctx);
}

std::any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx) {
	auto terms = ctx->term();
	auto opt = ctx->addorsub_op();
	std::any &&res = visitTerm(terms[0]);
	int n = static_cast<int>(terms.size());
	if (n <= 1) return res;
	testVar(res);
	for (int i = 1; i < n; ++i) {
		std::any &&val = visitTerm(terms[i]);
		testVar(val);
		if (opt[i - 1]->ADD())
			res += val;
		else if (opt[i - 1]->MINUS())
			res -= val;
	}
	return res;
}

std::any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
	auto factors = ctx->factor();
	auto opt = ctx->muldivmod_op();
	std::any &&res = visitFactor(factors[0]);
	int n = static_cast<int>(factors.size());
	if (n <= 1) return res;
	testVar(res);
	for (int i = 1; i < n; ++i) {
		std::any &&val = visitFactor(factors[i]);
		testVar(val);
		if (opt[i - 1]->STAR())
			res *= val;
		else if (opt[i - 1]->DIV())
			res = DivFloat(res, val);
		else if (opt[i - 1]->IDIV())
			res = DivInt(res, val);
		else if (opt[i - 1]->MOD())
			res %= val;
	}
	return res;
}

std::any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
	if (ctx->atom_expr())
		return visitAtom_expr(ctx->atom_expr());
	std::any &&val = visitFactor(ctx->factor());
	testVar(val);
	if (ctx->MINUS()) setAnyNegative(val);
	return val;
}

std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
	std::any &&res = visitAtom(ctx->atom());
	if (ctx->trailer()) {
		testVar(res);
		if (is<std::shared_ptr<func>>(res)) {
			auto &f = as<std::shared_ptr<func>>(res);
			return f->call(*this, ctx->trailer()->arglist());
		}
		else
			return res;
	}
	else
		return res;
}

std::any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
	static std::map<Python3Parser::AtomContext *, std::string> TEXT;
	if (TEXT.find(ctx) == TEXT.end()) TEXT[ctx] = ctx->getText();
	if (ctx->NAME())
		return scope.find(TEXT[ctx]);
	else if (ctx->NUMBER()) {
		string const &s = TEXT[ctx];
		if (s.find('.') == string::npos)
			return Int(s);
		else
			return std::stod(s);
	}
	else if (ctx->NONE())
		return {};
	else if (ctx->TRUE())
		return true;
	else if (ctx->FALSE())
		return false;
	else if (ctx->test())
		return visitTest(ctx->test());
	else {
		static std::map<Python3Parser::AtomContext *, std::string> STR;
		if (STR.find(ctx) != STR.end()) return STR[ctx];
		auto v = ctx->STRING();
		int totalLength = 1; // 1 is because \0
		for (auto p : v) {
			auto symbol = p->getSymbol();
			totalLength += static_cast<int>(symbol->getStopIndex()) - static_cast<int>(symbol->getStartIndex()) - 2;
		}
		string res;
		res.reserve(totalLength);
		for (auto p : v) {
			string tmp = p->getText();
			res.append(p->getText(), 1, tmp.length() - 2);
		}
		return STR[ctx] = res;
	}
}

std::any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx) {
	std::vector<std::any> vals;
	for (int i = 0; auto p = ctx->test(i); ++i) {
		std::any &&res = visitTest(p);
		if (is<std::vector<std::any>>(res)) {
			std::vector<std::any> &v = as<std::vector<std::any>>(res);
			for (auto &x : v)
				vals.emplace_back(std::move(x));
		}
		else vals.emplace_back(std::move(res));
	}
	return vals;
}

/**
 * @param ctx
 * @return VarType if "a = b"
 * @return ValType if "b"
 */
std::any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx) {
	if (ctx->test(1)) {
		std::any &&val = visitTest(ctx->test(1));
		testVar(val);
		return std::make_pair<std::string, std::any>(ctx->test(0)->getText(), std::move(val));
	}
	else {
		std::any &&val = visitTest(ctx->test(0));
		testVar(val);
		return val;
	}
}
