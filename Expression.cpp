#include "Expression.hpp"
#include <stdexcept>


Const::Const(const Int128& v) : val_(v) {}

Int128 Const::eval(const std::map<std::string, Int128>&) const {
    return val_;
}

Expression* Const::clone() const {
    return new Const(val_);
}


Variable::Variable(std::string n) : var_name_(std::move(n)) {}

Int128 Variable::eval(const std::map<std::string, Int128>& vars) const {
    auto found = vars.find(var_name_);
    if (found != vars.end()) {
        return found->second;
    }
    return Int128(0);
}

Expression* Variable::clone() const {
    return new Variable(var_name_);
}


Negate::Negate(std::unique_ptr<Expression> operand) : expr_(operand.release()) {}

Negate::~Negate() {
    delete expr_;
}

Int128 Negate::eval(const std::map<std::string, Int128>& vars) const {
    return -(expr_->eval(vars));
}

Expression* Negate::clone() const {
    return new Negate(std::unique_ptr<Expression>(expr_->clone()));
}


Add::Add(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
    : lhs_(l.release()), rhs_(r.release()) {}

Add::~Add() {
    delete lhs_;
    delete rhs_;
}

Int128 Add::eval(const std::map<std::string, Int128>& vars) const {
    return lhs_->eval(vars) + rhs_->eval(vars);
}

Expression* Add::clone() const {
    return new Add(
        std::unique_ptr<Expression>(lhs_->clone()),
        std::unique_ptr<Expression>(rhs_->clone())
    );
}


Subtract::Subtract(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
    : lhs_(l.release()), rhs_(r.release()) {}

Subtract::~Subtract() {
    delete lhs_;
    delete rhs_;
}

Int128 Subtract::eval(const std::map<std::string, Int128>& vars) const {
    return lhs_->eval(vars) - rhs_->eval(vars);
}

Expression* Subtract::clone() const {
    return new Subtract(
        std::unique_ptr<Expression>(lhs_->clone()),
        std::unique_ptr<Expression>(rhs_->clone())
    );
}


Multiply::Multiply(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
    : lhs_(l.release()), rhs_(r.release()) {}

Multiply::~Multiply() {
    delete lhs_;
    delete rhs_;
}

Int128 Multiply::eval(const std::map<std::string, Int128>& vars) const {
    return lhs_->eval(vars) * rhs_->eval(vars);
}

Expression* Multiply::clone() const {
    return new Multiply(
        std::unique_ptr<Expression>(lhs_->clone()),
        std::unique_ptr<Expression>(rhs_->clone())
    );
}


Divide::Divide(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
    : lhs_(l.release()), rhs_(r.release()) {}

Divide::~Divide() {
    delete lhs_;
    delete rhs_;
}

Int128 Divide::eval(const std::map<std::string, Int128>& vars) const {
    return lhs_->eval(vars) / rhs_->eval(vars);
}

Expression* Divide::clone() const {
    return new Divide(
        std::unique_ptr<Expression>(lhs_->clone()),
        std::unique_ptr<Expression>(rhs_->clone())
    );
}


Add operator+(const Expression& a, const Expression& b) {
    return Add(std::unique_ptr<Expression>(a.clone()),
               std::unique_ptr<Expression>(b.clone()));
}

Subtract operator-(const Expression& a, const Expression& b) {
    return Subtract(std::unique_ptr<Expression>(a.clone()),
                    std::unique_ptr<Expression>(b.clone()));
}

Multiply operator*(const Expression& a, const Expression& b) {
    return Multiply(std::unique_ptr<Expression>(a.clone()),
                    std::unique_ptr<Expression>(b.clone()));
}

Divide operator/(const Expression& a, const Expression& b) {
    return Divide(std::unique_ptr<Expression>(a.clone()),
                  std::unique_ptr<Expression>(b.clone()));
}

Negate operator-(const Expression& a) {
    return Negate(std::unique_ptr<Expression>(a.clone()));
}


std::ostream& operator<<(std::ostream& os, const Expression& expr) {
    os << "Expression";
    return os;
}
