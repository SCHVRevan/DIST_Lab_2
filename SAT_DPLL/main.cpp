#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QStack>
#include <string>
#include <cstring>
#include <stack>
#include <ostream>
#include <chrono>
#include <assert.h>
#include <new>
#include <iostream>

#include "NodeBoolTree.h"
#include "boolinterval.h"
#include "boolequation.h"
#include "BBV.h"
#include "Allocator.h"
#include "Strategy.h"

using namespace std;
using namespace std::chrono;

// Для BoolEquation
Allocator equationAllocator(sizeof(BoolEquation), 1000, NULL, "EquationAllocator");
// Для NodeBoolTree
Allocator nodeAllocator(sizeof(NodeBoolTree), 2000, NULL, "NodeAllocator");
// Для BoolInterval
Allocator intervalAllocator(sizeof(BoolInterval), 500, NULL, "IntervalAllocator");
// Для BBV
Allocator bbvAllocator(sizeof(BBV), 1000, NULL, "BBVAllocator");

#ifdef USE_CUSTOM_ALLOCATOR
BoolEquation* allocateEquation(BoolInterval** cnf, BoolInterval* root, int cnfSize, int count, BBV mask) {
    void* memory = equationAllocator.Allocate(sizeof(BoolEquation));
    return new(memory) BoolEquation(cnf, root, cnfSize, count, mask);
}

BoolEquation* allocateEquationCopy(BoolEquation& equation) {
    void* memory = equationAllocator.Allocate(sizeof(BoolEquation));
    return new(memory) BoolEquation(equation);
}

NodeBoolTree* allocateNode(BoolEquation* eq) {
    void* memory = nodeAllocator.Allocate(sizeof(NodeBoolTree));
    return new(memory) NodeBoolTree(eq);
}

BoolInterval* allocateInterval(BBV& vec, BBV& dnc) {
    void* memory = intervalAllocator.Allocate(sizeof(BoolInterval));
    return new(memory) BoolInterval(vec, dnc);
}

BoolInterval* allocateInterval(const char* str) {
    void* memory = intervalAllocator.Allocate(sizeof(BoolInterval));
    return new(memory) BoolInterval(str);
}

BBV* allocateBBV(const char* str) {
    void* memory = bbvAllocator.Allocate(sizeof(BBV));
    return new(memory) BBV(str);
}

void deallocateNode(NodeBoolTree* node) {
    if (node) {
        node->~NodeBoolTree();
        nodeAllocator.Deallocate(node);
    }
}

void deallocateEquation(BoolEquation* eq) {
    if (eq) {
        eq->~BoolEquation();
        equationAllocator.Deallocate(eq);
    }
}

void deallocateInterval(BoolInterval* interval) {
    if (interval) {
        interval->~BoolInterval();
        intervalAllocator.Deallocate(interval);
    }
}

void deallocateBBV(BBV* bbv) {
    if (bbv) {
        bbv->~BBV();
        bbvAllocator.Deallocate(bbv);
    }
}
#endif

static void out_of_memory() {
    // new-handler function called by Allocator when pool is out of memory
    cout << "Out of memory!\n";
    assert(0);
}

int main() {
    set_new_handler(out_of_memory);

    Strategy1 str1;
    Strategy2 str2;

    Context strategy(&str1);

    if (dynamic_cast<Strategy1*>(strategy.getStrategy())) {
        cout << "Choose Col For Branching\n";
    }

    else if (dynamic_cast<Strategy2*>(strategy.getStrategy())) {
        cout << "Choose Row For Branching\n";
    }

#ifdef USE_CUSTOM_ALLOCATOR
    cout << "Custom allocators are used for different types of classes\n";
#else
    cout << "A standard allocator is used\n";
#endif

    //auto totalStart = high_resolution_clock::now();
    auto start_time = high_resolution_clock::now();

    QStringList full_file_list;
    QList<QStringList> Elements;
    std::string filepath;
    QStringList inputs;
    //std::cout << "Input file path...\n";
    //std::cin >> filepath;
    // Hardcode input
    //	filepath = "sat_ex_2.pla";
    //filepath = "Sat_ex11_3.pla";
    filepath = "/home/schvrevan/Documents/DIST/lab2/Project/SAT_DPLL/SatExamples/Sat_ex14_3.pla";
    QFile file(QString::fromUtf8(filepath.c_str()));

    //считываем весь файл
    if ((file.exists()) && (file.open(QIODevice::ReadOnly))) {
        while (!file.atEnd()) {
            full_file_list << file.readLine().replace("\r\n", "");
        }

        int cnfSize = full_file_list.length();
        BoolInterval **CNF = new BoolInterval*[cnfSize];

        int rangInterval = -1; // error

        if (cnfSize) {
            rangInterval = full_file_list[0].toUtf8().trimmed().length();
        }

        for (int i = 0; i < cnfSize; i++) { // Заполняем массив
            QString strv = full_file_list[i];

            #ifdef USE_CUSTOM_ALLOCATOR
                CNF[i] = allocateInterval(strv.toUtf8().trimmed().data());
            #else
                CNF[i] = new BoolInterval(strv.toUtf8().trimmed().data());
            #endif
        }

        QString rootvec = "";
        QString rootdnc = "";

        // Строим интервал в котором все компоненты принимают значение '-',
        // который представляет собой корень уравнения, пока пустой.
        // В процессе поиска корня, компоненты интервала буду заменены на конкретные значения.

        for (int i = 0; i < rangInterval; i++) {
            rootvec += "0";
            rootdnc += "1";
        }

        #ifdef USE_CUSTOM_ALLOCATOR
            BBV* vec_ptr = allocateBBV(rootvec.toUtf8().trimmed().data());
            BBV* dnc_ptr = allocateBBV(rootdnc.toUtf8().trimmed().data());
            BBV& vec = *vec_ptr;
            BBV& dnc = *dnc_ptr;

            // Создаем пустой корень уравнения
            BoolInterval *root = allocateInterval(vec, dnc);
        #else
            QByteArray v = rootvec.toUtf8();

            BBV vec(v.data());
            QByteArray d = rootdnc.toUtf8();
            BBV dnc(d.data());

            // Создаем пустой корень уравнения
            BoolInterval *root = new BoolInterval(vec, dnc);
        #endif
            // Создаем пустой корень уравнения;
            //BoolInterval *root = new BoolInterval(vec, dnc);

            // Создаем уравнение и начальный узел в зависимости от типа аллокации
            BoolEquation *boolequation;
            NodeBoolTree *startNode;

        #ifdef USE_CUSTOM_ALLOCATOR
            // Используем специализированные аллокаторы
            boolequation = allocateEquation(CNF, root, cnfSize, cnfSize, vec);
            startNode = allocateNode(boolequation);
        #else
            // Используем стандартный new
            boolequation = new BoolEquation(CNF, root, cnfSize, cnfSize, vec, strategy);
            startNode = new NodeBoolTree(boolequation);
        #endif

        // Алгоритм поиска корня. Работаем всегда с верхушкой стека.
        // Шаг 1. Правила выполняются? Нет - Ветвление Шаг 5. Да - Упрощаем Шаг 2.
        // Шаг 2. Строки закончились? Нет - Шаг1, Да - Корень найден? Да - Успех КОНЕЦ, Нет - Шаг 3.
        // Шаг 3. Кол-во узлов в стеке > 1? Нет - Корня нет КОНЕЦ, Да - Шаг 4.
        // Шаг 4. Текущий узел выталкиваем из стека, попадаем в новый узел. У нового узла lt rt отличны от NULL? Нет - Шаг 1. Да - Шаг 3.
        // Шаг 5. Выбор компоненты ветвления, создание двух новых узлов, добавление их в стек сначала с 1 потом с 0. Шаг 1.

        // Алгоритм CheckRules.
        // Цикл по строкам КНФ.
        // 1. Проверка правила 2. Выполнилось? Да - Корня нет, Нет - Идем дальше.
        // 2. Проверка правила 1. Выполнилось? Да - Упрощаем, Нет - Идем дальше.

        // Создаем стек под узлы булева дерева
        // QStack<NodeBoolTree> BoolTree;

        bool rootIsFinded = false;
        stack<NodeBoolTree *> BoolTree;
        BoolTree.push(startNode);

        do {
            NodeBoolTree *currentNode(BoolTree.top());

            if (currentNode->lt == nullptr &&
                currentNode->rt == nullptr) { // Если вернулись в обработанный узел
                BoolEquation *currentEquation = currentNode->eq;
                bool flag = true;

                // Цикл для упрощения по правилам.
                while (flag) {
                    int a = currentEquation->CheckRules(); // Проверка выполнения правил

                    switch (a) {
                    case 0: { // Корня нет.
                        BoolTree.pop();
                        flag = false;
                        break;
                    }

                    case 1: { // Правило выполнилось, корень найден или продолжаем упрощать.
                        if (currentEquation->get_count() == 0 ||
                            currentEquation->get_mask().getWeight() ==
                                currentEquation->get_mask().getSize()) { // Если кончились строки или столбцы, корень найден.
                            flag = false;
                            rootIsFinded =
                                true; // Полагаем, что корень найден, выполняем проверку корня

                            for (int i = 0; i < cnfSize; i++) {

                                if (!CNF[i]->isEqualComponent(*currentEquation->get_root())) {
                                    rootIsFinded = false;//Корень не найден. Продолжаем искать дальше.
                                    BoolTree.pop();
                                    break;
                                }
                            }
                        }

                        break;
                    }

                    case 2: { // Правила не выполнились, ветвление.
                        // Ветвление, создание новых узлов.

                        //int indexBranching = currentEquation->ChooseColForBranching();
                        int indexBranching = strategy.getStrategy()->strat(*currentEquation);

                        // Создаем новые уравнения и узлы в зависимости от типа аллокации
                        BoolEquation *Equation0, *Equation1;
                        NodeBoolTree *Node0, *Node1;

                        #ifdef USE_CUSTOM_ALLOCATOR
                        Equation0 = allocateEquationCopy(*currentEquation);
                        Equation1 = allocateEquationCopy(*currentEquation);

                        Equation0->Simplify(indexBranching, '0');
                        Equation1->Simplify(indexBranching, '1');

                        Node0 = allocateNode(Equation0);
                        Node1 = allocateNode(Equation1);
                        #else
                        Equation0 = new BoolEquation(*currentEquation);
                        Equation1 = new BoolEquation(*currentEquation);

                        Equation0->Simplify(indexBranching, '0');
                        Equation1->Simplify(indexBranching, '1');

                        Node0 = new NodeBoolTree(Equation0);
                        Node1 = new NodeBoolTree(Equation1);
                        #endif

                        currentNode->lt = Node0;
                        currentNode->rt = Node1;

                        BoolTree.push(Node1);
                        BoolTree.push(Node0);

                        flag = false;
                        break;
                    }
                    }
                }
            } else {
                BoolTree.pop();
            }

        } while (BoolTree.size() > 1 && !rootIsFinded);

        auto end_time = high_resolution_clock::now();

        if (rootIsFinded) {
            cout << "Root is:\n ";
            BoolInterval *finded_root = BoolTree.top()->eq->get_root();
            cout << string(*finded_root);
        } else {
            cout << "Root is not exists!\n";
        }
        cout << "\nTotal time: " << duration_cast<microseconds>(end_time - start_time).count() << " mcs\n";

/*        #ifdef USE_CUSTOM_ALLOCATOR
            cout << "\n\n===== Allocator statistics =====\n\n";
            cout << "Allocator for BoolEquation:\n";
            cout << "  Block Size: " << equationAllocator.GetBlockSize() << " bytes\n";
            cout << "  Blocks count: " << equationAllocator.GetBlockCount() << "\n";
            cout << "  Blocks in use: " << equationAllocator.GetBlocksInUse() << "\n";
            cout << "  Allocations: " << equationAllocator.GetAllocations() << "\n";
            cout << "  Deallocations: " << equationAllocator.GetDeallocations() << "\n";

            cout << "\nAllocator for NodeBoolTree:\n";
            cout << "  Block Size: " << nodeAllocator.GetBlockSize() << " bytes\n";
            cout << "  Blocks count: " << nodeAllocator.GetBlockCount() << "\n";
            cout << "  Blocks in use: " << nodeAllocator.GetBlocksInUse() << "\n";
            cout << "  Allocations: " << nodeAllocator.GetAllocations() << "\n";
            cout << "  Deallocations: " << nodeAllocator.GetDeallocations() << "\n";

            cout << "\nAllocator for BoolInterval:\n";
            cout << "  Block Size: " << intervalAllocator.GetBlockSize() << " bytes\n";
            cout << "  Blocks count: " << intervalAllocator.GetBlockCount() << "\n";
            cout << "  Blocks in use: " << intervalAllocator.GetBlocksInUse() << "\n";
            cout << "  Allocations: " << intervalAllocator.GetAllocations() << "\n";
            cout << "  Deallocations: " << intervalAllocator.GetDeallocations() << "\n";

            cout << "\nAllocator for BBV:\n";
            cout << "  Block Size: " << bbvAllocator.GetBlockSize() << " bytes\n";
            cout << "  Blocks count: " << bbvAllocator.GetBlockCount() << "\n";
            cout << "  Blocks in use: " << bbvAllocator.GetBlocksInUse() << "\n";
            cout << "  Allocations: " << bbvAllocator.GetAllocations() << "\n";
            cout << "  Deallocations: " << bbvAllocator.GetDeallocations() << "\n";
        #endif
*/
        #ifdef USE_CUSTOM_ALLOCATOR
            while (!BoolTree.empty()) {
                NodeBoolTree* node = BoolTree.top();
                BoolTree.pop();

                if (node->eq) {
                    deallocateEquation(node->eq);
                }

                deallocateNode(node);
            }

            for (int i = 0; i < cnfSize; i++) {
                deallocateInterval(CNF[i]);
            }

            deallocateInterval(root);
            deallocateBBV(vec_ptr);
            deallocateBBV(dnc_ptr);
        #else
            while (!BoolTree.empty()) {
                NodeBoolTree* node = BoolTree.top();
                BoolTree.pop();
                delete node->eq;
                delete node;
            }

            for (int i = 0; i < cnfSize; i++) {
                delete CNF[i];
            }
            delete root;
        #endif

        delete[] CNF;

    } else {
        cout << "File does not exists.\n";
    }

    return 0;
}
