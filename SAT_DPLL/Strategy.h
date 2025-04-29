#ifndef STRATEGY_H
#define STRATEGY_H
//#include "boolequation.h"

// Общий интерфейс всех стратегий
class IStrategy {
public:
    virtual int strat(BoolEquation &) = 0;
    virtual ~IStrategy() = default;
};
// Каждая конкретная стратегия реализует общий интерфейс своим способом
class Strategy1: public IStrategy {
public:
    int strat(BoolEquation &be) {
        return be.ChooseColForBranching();
    }
};

class Strategy2: public IStrategy {
public:
    int strat(BoolEquation &be) {
        return be.ChooseRowForBranching();
    }
};

// Контекст всегда работает со стратегиями через общий интерфейс
class Context {
    IStrategy* strategy;
public:
    Context (IStrategy *strat): strategy(strat) {}

    void setStrategy(IStrategy &strategy) {
        this->strategy = &strategy;
    }

    IStrategy* getStrategy() {
        return(strategy);
    }
};

#endif // STRATEGY_H
