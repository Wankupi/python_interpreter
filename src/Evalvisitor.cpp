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
	return visit(ctx->small_stmt());
}

antlrcpp::Any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
	if (!ctx->ASSIGN(0) && !ctx->augassign()) return visitChildren(ctx);
	auto var_list = ctx->testlist();
	int n = static_cast<int>(var_list.size()) - 1;
	std::vector<antlrcpp::Any> rvalues = visit(var_list.back()).as<std::vector<antlrcpp::Any>>();
	int m = static_cast<int>(rvalues.size());
	for (auto &x : rvalues)
		testVar(x);
	AugassignType opt = AugassignType::equal;
	if (auto agu = ctx->augassign()) opt = visit(agu).as<AugassignType>();
	for (int i = 0; i < n; ++i) {
		std::vector<antlrcpp::Any> lvalues = visit(var_list[i]).as<std::vector<antlrcpp::Any>>();
		int mm = static_cast<int>(lvalues.size());
		for (int j = 0; j < m && j < mm; ++j) {
			antlrcpp::Any &var = lvalues[j];
			antlrcpp::Any const& val = rvalues[j];
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
			antlrcpp::Any &&res = visit(test);
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
	if (!ctx->IF()) return {};
	int kth = 0;
	while (ctx->test(kth)) {
		antlrcpp::Any &&val = visit(ctx->test(kth));
		testVar(val);
		if (toBool(val))
			return visit(ctx->suite(kth));
		else
			++kth;
	}
	if (ctx->suite(kth)) return visit(ctx->suite(kth));
	return {};
}

antlrcpp::Any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx) {
	if (!ctx->WHILE()) return {};
	while (true) {
		antlrcpp::Any &&val = visit(ctx->test());
		testVar(val);
		if (toBool(val)) {
			antlrcpp::Any &&res = visit(ctx->suite());
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
		antlrcpp::Any &&val = visit(ctx->simple_stmt());
		if (val.is<FlowStmt>()) return std::move(val);
	}
	for (int i = 0; ctx->stmt(i); ++i) {
		antlrcpp::Any &&val = visit(ctx->stmt(i));
		if (val.is<FlowStmt>()) return std::move(val);
	}
	return {};
}

antlrcpp::Any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
	if (ctx->OR(0)) {
		auto v2 = ctx->and_test();
		for (auto p : v2) {
			antlrcpp::Any &&val = visit(p);
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
		auto v2 = ctx->not_test();
		for (auto p : v2) {
			antlrcpp::Any &&val = visit(p);
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
		antlrcpp::Any &&val = visit(ctx->not_test());
		testVar(val);
		return !toBool(val);
	}
	else
		return visit(ctx->comparison());
}

antlrcpp::Any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
	if (ctx->comp_op(0)) {
		auto nodes = ctx->arith_expr();
		antlrcpp::Any &&last = visit(nodes[0]);
		testVar(last);
		int _n = static_cast<int>(nodes.size());
		for (int i = 1; i < _n; ++i) {
			antlrcpp::Any &&val = visit(nodes[i]);
			testVar(val);
			auto *p = ctx->comp_op(i - 1);
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
	antlrcpp::Any &&res = visit(terms[0]);
	int n = static_cast<int>(terms.size());
	if (n <= 1) return std::move(res);
	testVar(res);
	for (int i = 1; i < n; ++i) {
		antlrcpp::Any &&val = visit(terms[i]);
		testVar(val);
		if (opt[i - 1]->ADD())
			res += val;
		else if (opt[i - 1]->MINUS())
			res -= val;
	}
	return std::move(res);
}

antlrcpp::Any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
	auto factors = ctx->factor();
	auto opt = ctx->muldivmod_op();
	antlrcpp::Any &&res = visit(factors[0]);
	int n = static_cast<int>(factors.size());
	if (n <= 1) return std::move(res);
	testVar(res);
	for (int i = 1; i < n; ++i) {
		antlrcpp::Any &&val = visit(factors[i]);
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
	return std::move(res);
}

antlrcpp::Any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
	if (ctx->atom_expr())
		return visit(ctx->atom_expr());
	antlrcpp::Any &&val = visit(ctx->factor());
	testVar(val);
	if (ctx->MINUS()) setAnyNegative(val);
	return std::move(val);
}

antlrcpp::Any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
	antlrcpp::Any &&res = visit(ctx->atom());
	if (ctx->trailer()) {
		testVar(res);
		if (res.is<std::shared_ptr<func>>()) {
			auto &f = res.as<std::shared_ptr<func>>();
			return f->call(*this, ctx->trailer()->arglist());
		}
		else
			return std::move(res);
	}
	else
		return std::move(res);
}

antlrcpp::Any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
	if (ctx->NAME())
		return scope.find(ctx->getText());
	else if (ctx->NUMBER()) {
		string &&s = ctx->getText();
		if (s.find('.') == string::npos)
			return Int(ctx->getText());
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
		return visit(ctx->test());
	else {
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
		return std::move(res);
	}
}

antlrcpp::Any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx) {
	std::vector<antlrcpp::Any> vals;
	for (int i = 0; auto p = ctx->test(i); ++i) {
		antlrcpp::Any &&res = visit(p);
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
		antlrcpp::Any &&val = visit(ctx->test(1));
		testVar(val);
		return std::make_pair<std::string, antlrcpp::Any>(ctx->test(0)->getText(), std::move(val));
	}
	else {
		antlrcpp::Any &&val = visit(ctx->test(0));
		testVar(val);
		return std::move(val);
	}
}
