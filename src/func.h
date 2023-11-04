#pragma once
#include "Evalvisitor.h"

#ifndef KUPI_FUNC_HEADER
#define KUPI_FUNC_HEADER
#include "base.h"

using Variable = std::pair<std::string, std::any>;
using VarType = std::unordered_map<std::string, std::any>::iterator;

struct func {
	virtual std::string getName() const;
	virtual std::any call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const;
	virtual ~func() = default;
};

struct func_user : func {
	Python3Parser::FuncdefContext *node;
	std::vector<Variable> paraLists;
	func_user(EvalVisitor &vis, Python3Parser::FuncdefContext *_node);
	virtual std::string getName() const override;
	virtual std::any call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const override;
};

struct func_print : func {
	virtual std::string getName() const override;
	virtual std::any call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const override;
};

struct func_int : func {
	virtual std::string getName() const override;
	virtual std::any call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const override;
};

struct func_float : func {
	virtual std::string getName() const override;
	virtual std::any call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const override;
};

struct func_str : func {
	virtual std::string getName() const override;
	virtual std::any call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const override;
};

struct func_bool : func {
	virtual std::string getName() const override;
	virtual std::any call(EvalVisitor &vis, Python3Parser::ArglistContext *ctx) const override;
};

struct NameSpace {
	NameSpace();
	void in() { scope.emplace_back(); }
	void in(std::unordered_map<std::string, std::any> &&new_scope) { scope.emplace_back(std::move(new_scope)); }
	void out() { scope.pop_back(); }
	/**
	 * @attention find in the last and global space, create in the last
	 */
	VarType find(std::string const &name);
	std::vector<std::unordered_map<std::string, std::any>> scope;
};

extern NameSpace scope;

struct FlowStmt {
	enum class FlowWord { break_stmt,
												continue_stmt,
												return_stmt };
	FlowStmt(FlowWord _word) : word(_word), val() {}
	FlowStmt(FlowWord _word, std::any const &v) : word(_word), val(v) {}
	FlowStmt(FlowWord _word, std::any &&v) : word(_word), val(std::forward<std::any>(v)) {}
	FlowWord word;
	std::any val;
};

enum class AugassignType { none,
													 equal,
													 add,
													 minus,
													 mult,
													 div,
													 idiv,
													 mod };

#endif
