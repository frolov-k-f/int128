#pragma once

#include "Int128.hpp"
#include <map>
#include <string>
#include <memory>
#include <ostream>

class Expression {
public:
    virtual ~Expression() = default;
    virtual Int128 eval(const std::map<std::string, Int128>& vars) const = 0;
    virtual Expression* clone() const = 0;
};

std::ostream& operator<<(std::ostream& os, const Expression& expr);

class Const : public Expression {
public:
    explicit Const(const Int128& v);
    Int128 eval(const std::map<std::string, Int128>&) const override;
    Expression* clone() const override;
private:
    Int128 val_;
};

class Variable : public Expression {
public:
    explicit Variable(std::string n);
    Int128 eval(const std::map<std::string, Int128>& vars) const override;
    Expression* clone() const override;
private:
    std::string var_name_;
};

class Negate : public Expression {
public:
    explicit Negate(std::unique_ptr<Expression> operand);
    ~Negate() override;
    Int128 eval(const std::map<std::string, Int128>& vars) const override;
    Expression* clone() const override;
private:
    Expression* expr_;
};

class Add : public Expression {
public:
    Add(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
    ~Add() override;
    Int128 eval(const std::map<std::string, Int128>& vars) const override;
    Expression* clone() const override;
private:
    Expression* lhs_;
    Expression* rhs_;
};

class Subtract : public Expression {
public:
    Subtract(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
    ~Subtract() override;
    Int128 eval(const std::map<std::string, Int128>& vars) const override;
    Expression* clone() const override;
private:
    Expression* lhs_;
    Expression* rhs_;
};

class Multiply : public Expression {
public:
    Multiply(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
    ~Multiply() override;
    Int128 eval(const std::map<std::string, Int128>& vars) const override;
    Expression* clone() const override;
private:
    Expression* lhs_;
    Expression* rhs_;
};

class Divide : public Expression {
public:
    Divide(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
    ~Divide() override;
    Int128 eval(const std::map<std::string, Int128>& vars) const override;
    Expression* clone() const override;
private:
    Expression* lhs_;
    Expression* rhs_;
};

Add operator+(const Expression& a, const Expression& b);
Subtract operator-(const Expression& a, const Expression& b);
Multiply operator*(const Expression& a, const Expression& b);
Divide operator/(const Expression& a, const Expression& b);
Negate operator-(const Expression& a);
