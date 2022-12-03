#include "Evalvisitor.h"
#include "AnyOpt.h"
#include "func.h"
#include <string>
#include <unordered_map>

NameSpace scope;

antlrcpp::Any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx) {
	string func_name = ctx->NAME()->getText();
	scope.find(func_name)->second = std::shared_ptr<func>(std::make_shared<func_user>(*this, ctx));
	return {};
}

antlrcpp::Any EvalVisitor::visitTfpdef(Python3Parser::TfpdefContext *ctx) {
	return ctx->getText();
}

antlrcpp::Any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) {
	return visitSmall_stmt(ctx->small_stmt());
}

antlrcpp::Any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
	if (!ctx->ASSIGN(0) && !ctx->augassign()) return visitChildren(ctx);
	auto var_list = ctx->testlist();
	int n = static_cast<int>(var_list.size()) - 1;
	std::vector<antlrcpp::Any> rvalues = visitTestlist(var_list.back()).as<std::vector<antlrcpp::Any>>();
	int m = static_cast<int>(rvalues.size());
	for (auto &x : rvalues)
		testVar(x);
	AugassignType opt = AugassignType::equal;
	if (auto agu = ctx->augassign()) opt = visitAugassign(agu).as<AugassignType>();
	for (int i = 0; i < n; ++i) {
		std::vector<antlrcpp::Any> lvalues = visitTestlist(var_list[i]).as<std::vector<antlrcpp::Any>>();
		int mm = static_cast<int>(lvalues.size());
		for (int j = 0; j < m && j < mm; ++j) {
			antlrcpp::Any &var = lvalues[j];
			antlrcpp::Any const &val = rvalues[j];
			if (!var.is<VarType>())
				continue;
			if (opt == AugassignType::equal)
				var.as<VarType>()->second = val;
			else if (opt == AugassignType::add)
				var.as<VarType>()->second += val;
			else if (opt == AugassignType::minus)
				var.as<VarType>()->second -= val;
			else if (opt == AugassignType::mult)
				var.as<VarType>()->second *= val;
			else if (opt == AugassignType::div)
				var.as<VarType>()->second = DivFloat(var.as<VarType>()->second, val);
			else if (opt == AugassignType::idiv)
				var.as<VarType>()->second = DivInt(var.as<VarType>()->second, val);
			else if (opt == AugassignType::mod)
				var.as<VarType>()->second %= val;
		}
	}
	return {};
}

antlrcpp::Any EvalVisitor::visitAugassign(Python3Parser::AugassignContext *ctx) {
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

antlrcpp::Any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) {
	if (ctx->break_stmt()) return FlowStmt(FlowStmt::FlowWord::break_stmt);
	else if (ctx->continue_stmt()) return FlowStmt(FlowStmt::FlowWord::continue_stmt);
	else if (auto ret = ctx->return_stmt()) {
		if (auto test = ret->testlist()) {
			antlrcpp::Any &&res = visitTestlist(test);
			std::vector<antlrcpp::Any> &vals = res.as<std::vector<antlrcpp::Any>>();
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

antlrcpp::Any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx) {
	static std::map<Python3Parser::If_stmtContext *, std::vector<Python3Parser::TestContext *>> TESTS;
	if (!ctx->IF()) return {};
	if (TESTS.find(ctx) == TESTS.end())
		TESTS[ctx] = ctx->test();
	std::vector<Python3Parser::TestContext *> const &tests = TESTS[ctx];
	int n = static_cast<int>(tests.size());
	int kth = 0;
	while (kth < n) {
		antlrcpp::Any &&val = visitTest(tests[kth]);
		testVar(val);
		if (toBool(val))
			return visitSuite(ctx->suite(kth));
		else
			++kth;
	}
	if (ctx->suite(kth)) return visitSuite(ctx->suite(kth));
	return {};
}

antlrcpp::Any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx) {
	if (!ctx->WHILE()) return {};
	while (true) {
		antlrcpp::Any &&val = visitTest(ctx->test());
		testVar(val);
		if (toBool(val)) {
			antlrcpp::Any &&res = visitSuite(ctx->suite());
			if (res.isNull()) continue;
			if (res.is<FlowStmt>()) {
				if (res.as<FlowStmt>().word == FlowStmt::FlowWord::break_stmt)
					break;
				else if (res.as<FlowStmt>().word == FlowStmt::FlowWord::continue_stmt)
					continue;
				else if (res.as<FlowStmt>().word == FlowStmt::FlowWord::return_stmt)
					return res;
			}
		}
		else
			break;
	}
	return {};
}

antlrcpp::Any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx) {
	if (ctx->simple_stmt()) {
		antlrcpp::Any &&val = visitSimple_stmt(ctx->simple_stmt());
		if (val.is<FlowStmt>()) return val;
	}
	for (int i = 0; ctx->stmt(i); ++i) {
		antlrcpp::Any &&val = visitStmt(ctx->stmt(i));
		if (val.is<FlowStmt>()) return val;
	}
	return {};
}

antlrcpp::Any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
	if (ctx->OR(0)) {
		static std::map<Python3Parser::Or_testContext *, std::vector<Python3Parser::And_testContext *>> TESTS;
		if (TESTS.find(ctx) == TESTS.end()) TESTS[ctx] = ctx->and_test();
		auto const &tests = TESTS[ctx];
		for (auto *const p : tests) {
			antlrcpp::Any &&val = visitAnd_test(p);
			testVar(val);
			if (toBool(val))
				return true;
		}
		return false;
	}
	else
		return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx) {
	if (ctx->AND(0)) {
		static std::map<Python3Parser::And_testContext *, std::vector<Python3Parser::Not_testContext *>> TESTS;
		if (TESTS.find(ctx) == TESTS.end()) TESTS[ctx] = ctx->not_test();
		auto const &tests = TESTS[ctx];
		for (auto *const p : tests) {
			antlrcpp::Any &&val = visitNot_test(p);
			testVar(val);
			if (!toBool(val))
				return false;
		}
		return true;
	}
	else
		return visitChildren(ctx);
}

antlrcpp::Any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx) {
	if (ctx->NOT()) {
		antlrcpp::Any &&val = visitNot_test(ctx->not_test());
		testVar(val);
		return !toBool(val);
	}
	else
		return visitComparison(ctx->comparison());
}

antlrcpp::Any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
	if (ctx->comp_op(0)) {
		static std::map<Python3Parser::ComparisonContext *, std::vector<Python3Parser::Arith_exprContext *>> EXPRS;
		static std::map<Python3Parser::ComparisonContext *, std::vector<Python3Parser::Comp_opContext *>> COMPS;
		if (EXPRS.find(ctx) == EXPRS.end()) EXPRS[ctx] = ctx->arith_expr();
		if (COMPS.find(ctx) == COMPS.end()) COMPS[ctx] = ctx->comp_op();

		auto const &nodes = EXPRS[ctx];
		auto const &comps = COMPS[ctx];
		antlrcpp::Any &&last = visitArith_expr(nodes[0]);
		testVar(last);
		int _n = static_cast<int>(nodes.size());
		for (int i = 1; i < _n; ++i) {
			antlrcpp::Any &&val = visitArith_expr(nodes[i]);
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

antlrcpp::Any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx) {
	auto terms = ctx->term();
	auto opt = ctx->addorsub_op();
	antlrcpp::Any &&res = visitTerm(terms[0]);
	int n = static_cast<int>(terms.size());
	if (n <= 1) return res;
	testVar(res);
	for (int i = 1; i < n; ++i) {
		antlrcpp::Any &&val = visitTerm(terms[i]);
		testVar(val);
		if (opt[i - 1]->ADD())
			res += val;
		else if (opt[i - 1]->MINUS())
			res -= val;
	}
	return res;
}

antlrcpp::Any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
	auto factors = ctx->factor();
	auto opt = ctx->muldivmod_op();
	antlrcpp::Any &&res = visitFactor(factors[0]);
	int n = static_cast<int>(factors.size());
	if (n <= 1) return res;
	testVar(res);
	for (int i = 1; i < n; ++i) {
		antlrcpp::Any &&val = visitFactor(factors[i]);
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

antlrcpp::Any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
	if (ctx->atom_expr())
		return visitAtom_expr(ctx->atom_expr());
	antlrcpp::Any &&val = visitFactor(ctx->factor());
	testVar(val);
	if (ctx->MINUS()) setAnyNegative(val);
	return val;
}

antlrcpp::Any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
	antlrcpp::Any &&res = visitAtom(ctx->atom());
	if (ctx->trailer()) {
		testVar(res);
		if (res.is<std::shared_ptr<func>>()) {
			auto &f = res.as<std::shared_ptr<func>>();
			return f->call(*this, ctx->trailer()->arglist());
		}
		else
			return res;
	}
	else
		return res;
}

antlrcpp::Any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
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

antlrcpp::Any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx) {
	std::vector<antlrcpp::Any> vals;
	for (int i = 0; auto p = ctx->test(i); ++i) {
		antlrcpp::Any &&res = visitTest(p);
		if (res.is<std::vector<antlrcpp::Any>>()) {
			std::vector<antlrcpp::Any> &v = res.as<std::vector<antlrcpp::Any>>();
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
antlrcpp::Any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx) {
	if (ctx->test(1)) {
		antlrcpp::Any &&val = visitTest(ctx->test(1));
		testVar(val);
		return std::make_pair<std::string, antlrcpp::Any>(ctx->test(0)->getText(), std::move(val));
	}
	else {
		antlrcpp::Any &&val = visitTest(ctx->test(0));
		testVar(val);
		return val;
	}
}
