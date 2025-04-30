#ifndef STRATEGY_H
#define STRATEGY_H
#include "boolequation.h"

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
        //return be.ChooseColForBranching();
        vector<int> indexes;
        vector<int> values;
        bool rezInit = false;

        for (int i = 0; i < be.get_mask().getSize(); i++) {
            if (be.get_mask()[i] == 0) {
                indexes.push_back(i);
            }
        }

        for (int i = 0; i < be.get_cnfSize(); i++) {
            BoolInterval *interval = be.get_cnf()[i];

            if (interval != nullptr) {
                if (!rezInit) {
                    for (int k = 0; k < indexes.size(); k++) {
                        if (interval->getValue(indexes.at(k)) == '-') {
                            values.push_back(1);
                        } else {
                            values.push_back(0);
                        }
                    }

                    rezInit = true;
                } else {
                    for (int k = 0; k < indexes.size(); k++) {
                        if (interval->getValue(indexes.at(k)) == '-') {
                            //int val = values.at(k) + (interval->getValue(indexes.at(k)) - '0');
                            values.at(k)++;
                        }
                    }
                }
            }
        }

        int minElementIndex = std::min_element(values.begin(), values.end()) - values.begin();

        return indexes.at(minElementIndex);
    }
};

class Strategy2: public IStrategy {
public:
    int strat(BoolEquation &be) {
        //return be.ChooseRowForBranching();
        vector<int> nonEmptyRows;
        vector<int> rowWeights;

        // Собираем непустые строки (интервалы)
        for (int i = 0; i < be.get_cnfSize(); i++) {
            if (be.get_cnf()[i] != nullptr) {
                nonEmptyRows.push_back(i);

                // Вычисляем вес строки (количество незамаскированных переменных)
                int weight = 0;
                for (int j = 0; j < be.get_mask().getSize(); j++) {
                    if (be.get_mask()[j] == 0 && be.get_cnf()[i]->getValue(j) != '-') {
                        weight++;
                    }
                }
                rowWeights.push_back(weight);
            }
        }

        // Если нет строк, возвращаем -1 (ошибка)
        if (nonEmptyRows.empty()) {
            return -1;
        }

        // Выбираем строку с минимальным весом (но не нулевым)
        int minIndex = 0;
        int minWeight = INT_MAX;

        for (size_t i = 0; i < rowWeights.size(); i++) {
            if (rowWeights[i] > 0 && rowWeights[i] < minWeight) {
                minWeight = rowWeights[i];
                minIndex = i;
            }
        }

        // Для выбранной строки ищем индекс переменной (столбец) для ветвления
        int rowIndex = nonEmptyRows[minIndex];

        // Выбираем первый незамаскированный столбец в этой строке
        for (int j = 0; j < be.get_mask().getSize(); j++) {
            if (be.get_mask()[j] == 0 && be.get_cnf()[rowIndex]->getValue(j) != '-') {
                return j;
            }
        }
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
