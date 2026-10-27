#include <iostream>
#include <memory>
#include <string>
#include <cmath>

using namespace std;

class Expression {
public:
    virtual ~Expression() = default;
    virtual double evaluate(double x) const = 0;
    virtual shared_ptr<Expression> derivative() const = 0;
    virtual string toString() const = 0;

    // Permiten saber si un nodo es constante sin usar dynamic_cast
    virtual bool isConstant() const { return false; }
    virtual double constValue() const { return 0; }
};

// Helpers que simplifican casos triviales (0 + a, 1 * a, a^1, ...)
shared_ptr<Expression> makeAdd(shared_ptr<Expression> a, shared_ptr<Expression> b);
shared_ptr<Expression> makeMul(shared_ptr<Expression> a, shared_ptr<Expression> b);
shared_ptr<Expression> makePow(shared_ptr<Expression> a, shared_ptr<Expression> b);

// Compara doubles con tolerancia en vez de usar ==
const double EPS = 1e-9;
bool isConstValue(const shared_ptr<Expression>& e, double v) {
    return e->isConstant() && fabs(e->constValue() - v) < EPS;
}

class Constant : public Expression {
    double value;
public:
    explicit Constant(double val) : value(val) {}
    double evaluate(double) const override { return value; }
    shared_ptr<Expression> derivative() const override { return make_shared<Constant>(0); }
    string toString() const override {
        if (value == static_cast<int>(value)) return to_string(static_cast<int>(value));
        return to_string(value);
    }
    bool isConstant() const override { return true; }
    double constValue() const override { return value; }
};

class Variable : public Expression {
public:
    double evaluate(double x) const override { return x; }
    shared_ptr<Expression> derivative() const override { return make_shared<Constant>(1); }
    string toString() const override { return "x"; }
};

class BinaryOperation : public Expression {
protected:
    shared_ptr<Expression> left, right;
public:
    BinaryOperation(shared_ptr<Expression> l, shared_ptr<Expression> r)
        : left(move(l)), right(move(r)) {}
};

class UnaryFunction : public Expression {
protected:
    shared_ptr<Expression> arg;
public:
    explicit UnaryFunction(shared_ptr<Expression> a) : arg(move(a)) {}
};

class Add : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) const override { return left->evaluate(x) + right->evaluate(x); }
    shared_ptr<Expression> derivative() const override;
    string toString() const override { return "(" + left->toString() + " + " + right->toString() + ")"; }
};

class Multiply : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) const override { return left->evaluate(x) * right->evaluate(x); }
    shared_ptr<Expression> derivative() const override;
    string toString() const override { return "(" + left->toString() + " * " + right->toString() + ")"; }
};

class Power : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) const override { return pow(left->evaluate(x), right->evaluate(x)); }
    shared_ptr<Expression> derivative() const override;
    string toString() const override { return "(" + left->toString() + "^" + right->toString() + ")"; }
};

class Sin : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override { return sin(arg->evaluate(x)); }
    shared_ptr<Expression> derivative() const override;
    string toString() const override { return "sin(" + arg->toString() + ")"; }
};

class Cos : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override { return cos(arg->evaluate(x)); }
    shared_ptr<Expression> derivative() const override;
    string toString() const override { return "cos(" + arg->toString() + ")"; }
};

class Ln : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override { return log(arg->evaluate(x)); }
    shared_ptr<Expression> derivative() const override;
    string toString() const override { return "ln(" + arg->toString() + ")"; }
};

// ---------- Helpers de simplificación ----------
shared_ptr<Expression> makeAdd(shared_ptr<Expression> a, shared_ptr<Expression> b) {
    if (a->isConstant() && b->isConstant())
        return make_shared<Constant>(a->constValue() + b->constValue());
    if (isConstValue(a, 0)) return b;
    if (isConstValue(b, 0)) return a;
    return make_shared<Add>(a, b);
}

shared_ptr<Expression> makeMul(shared_ptr<Expression> a, shared_ptr<Expression> b) {
    if (a->isConstant() && b->isConstant())
        return make_shared<Constant>(a->constValue() * b->constValue());
    if (isConstValue(a, 0) || isConstValue(b, 0))
        return make_shared<Constant>(0);
    if (isConstValue(a, 1)) return b;
    if (isConstValue(b, 1)) return a;
    return make_shared<Multiply>(a, b);
}

shared_ptr<Expression> makePow(shared_ptr<Expression> a, shared_ptr<Expression> b) {
    if (isConstValue(b, 0)) return make_shared<Constant>(1);
    if (isConstValue(b, 1)) return a;
    return make_shared<Power>(a, b);
}

// ---------- Reglas de derivación ----------
// (u + v)' = u' + v'
shared_ptr<Expression> Add::derivative() const {
    return makeAdd(left->derivative(), right->derivative());
}

// (u * v)' = u'v + uv'
shared_ptr<Expression> Multiply::derivative() const {
    return makeAdd(makeMul(left->derivative(), right),
                   makeMul(left, right->derivative()));
}

// Exponente constante: (u^n)' = n * u^(n-1) * u'
// Caso general:        (u^v)' = u^v * (v' * ln(u) + v * u' * u^(-1))
shared_ptr<Expression> Power::derivative() const {
    if (right->isConstant()) {
        double n = right->constValue();
        return makeMul(makeMul(make_shared<Constant>(n),
                               makePow(left, make_shared<Constant>(n - 1))),
                       left->derivative());
    }
    auto term1 = makeMul(right->derivative(), make_shared<Ln>(left));
    auto term2 = makeMul(makeMul(right, left->derivative()),
                         makePow(left, make_shared<Constant>(-1)));
    return makeMul(makePow(left, right), makeAdd(term1, term2));
}

// (sin u)' = cos(u) * u'
shared_ptr<Expression> Sin::derivative() const {
    return makeMul(make_shared<Cos>(arg), arg->derivative());
}

// (cos u)' = -sin(u) * u'
shared_ptr<Expression> Cos::derivative() const {
    return makeMul(makeMul(make_shared<Constant>(-1), make_shared<Sin>(arg)),
                   arg->derivative());
}

// (ln u)' = u' * u^(-1)
shared_ptr<Expression> Ln::derivative() const {
    return makeMul(arg->derivative(), makePow(arg, make_shared<Constant>(-1)));
}

int main() {
    auto x = make_shared<Variable>();
    auto expr = make_shared<Multiply>(
        make_shared<Add>(
            make_shared<Power>(
                x,
                make_shared<Constant>(2)
            ),
            make_shared<Multiply>(
                make_shared<Constant>(3),
                x
            )
        ),
        make_shared<Sin>(x)
    );

    auto deriv = expr->derivative();
    cout << "f(x) = " << expr->toString() << endl;
    cout << "f'(x) = " << deriv->toString() << endl;
    cout << "f(2) = " << expr->evaluate(2) << endl;
    cout << "f'(2) = " << deriv->evaluate(2) << endl;

    // Prueba extra: exponente variable, g(x) = x^x
    auto g = make_shared<Power>(x, x);
    cout << "\ng(x) = " << g->toString() << endl;
    cout << "g'(x) = " << g->derivative()->toString() << endl;
    cout << "g'(2) = " << g->derivative()->evaluate(2) << endl;

    return 0;
}
