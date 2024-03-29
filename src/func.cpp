#include "func.h"
#include "AnyOpt.h"
#include <iostream>

std::string func::getName() const { return "__builtin_base__"; }

std::any func::call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const {
	return {};
}

std::string func_print::getName() const { return "__builtin_print__"; }

std::any func_print::call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const {
	std::vector<Python3Parser::ArgumentContext *> list;
	if (ctx) list = ctx->argument();
	if (!list.empty()) {
		std::cout << vis.visit(list[0]);
		int _n = static_cast<int>(list.size());
		for (int i = 1; i < _n; ++i)
			std::cout << ' ' << vis.visit(list[i]);
	}
	std::cout << std::endl;
	return {};
}

std::string func_int::getName() const { return "__builtin_int__"; }

std::any func_int::call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const {
	return toInt(vis.visit(ctx));
}

std::string func_float::getName() const { return "__builtin_float__"; }

std::any func_float::call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const {
	return toFloat(vis.visit(ctx));
}

std::string func_str::getName() const { return "__builtin_str__"; }

std::any func_str::call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const {
	return toStr(vis.visit(ctx));
}

std::string func_bool::getName() const { return "__builtin_bool__"; }

std::any func_bool::call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const {
	return toBool(vis.visit(ctx));
}

func_user::func_user(EvalVisitor &vis, Python3Parser::FuncdefContext *_node) : node(_node) {
	auto tlist = node->parameters()->typedargslist();
	if (!tlist) return;
	auto need = tlist->tfpdef();
	auto test = tlist->test();
	int n = static_cast<int>(need.size()), m = static_cast<int>(test.size());
	int k = n - m;
	paraLists.reserve(n);
	for (int i = 0; i < k; ++i)
		paraLists.emplace_back(need[i]->getText(), std::any{});
	for (int i = k; i < n; ++i) {
		std::any &&val = vis.visit(test[i - k]);
		testVar(val);
		paraLists.emplace_back(need[i]->getText(), std::move(val));
	}
}

std::string func_user::getName() const {
	if (node && node->NAME())
		return node->NAME()->getText();
	else
		return "__empty_function__";
}

std::any func_user::call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const {
	std::unordered_map<std::string, std::any> new_scope;
	if (ctx) {
		for (int i = 0; ctx->argument(i); ++i) {
			std::any &&res = vis.visit(ctx->argument(i));
			if (is<Variable>(res)) {
				Variable &var = as<Variable>(res);
				new_scope.insert(std::move(var));
			}
			else
				new_scope.emplace(paraLists[i].first, std::move(res));
		}
	}
	for (auto const &p : paraLists)
		if (new_scope.find(p.first) == new_scope.end())
			new_scope.insert(p);
	scope.in(std::move(new_scope));
	std::any &&val = vis.visit(node->suite());
	scope.out();

	if (is<FlowStmt>(val)) {
		FlowStmt &fs = as<FlowStmt>(val);
		if (fs.word == FlowStmt::FlowWord::return_stmt) {
			if (is<std::vector<std::any>>(fs.val)) {
				std::vector<std::any> &ret = as<std::vector<std::any>>(fs.val);
				for (auto &x : ret)
					testVar(x);
				if (ret.size() == 1) return std::move(ret[0]);
				else return std::move(ret);
			}
			else return std::move(fs.val);
		}
	}
	return {};
}

NameSpace::NameSpace() : scope(1) {
	scope[0]["print"] = std::shared_ptr<func>(new func_print());
	scope[0]["int"] = std::shared_ptr<func>(new func_int());
	scope[0]["float"] = std::shared_ptr<func>(new func_float());
	scope[0]["str"] = std::shared_ptr<func>(new func_str());
	scope[0]["bool"] = std::shared_ptr<func>(new func_bool());
}

VarType NameSpace::find(string const &name) {
	auto p = scope.back().find(name);
	if (p != scope.back().end()) return p;
	p = scope.front().find(name);
	if (p != scope.front().end()) return p;
	return scope.back().insert({name, {}}).first;
}
