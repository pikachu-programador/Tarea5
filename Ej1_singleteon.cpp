#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

class Account {
private:
    string id;
    double balance;

    void deposit(double amount) { balance += amount; }
    void withdraw(double amount) { balance -= amount; }
    friend class Transaction;

public:
    Account(const string& id, double initialBalance)
        : id(id), balance(initialBalance) {}

    double getBalance() const { return balance; }
};

enum class TransactionType { DEPOSIT, WITHDRAW };

class Transaction {
private:
    TransactionType type;
    double amount;
    bool success;

public:
    Transaction(TransactionType type, double amount, bool success)
        : type(type), amount(amount), success(success) {}

    string toString() const {
        string t = (type == TransactionType::DEPOSIT) ? "Deposito" : "Retiro";
        return t + " de " + to_string(amount) + (success ? " [OK]" : " [FALLIDA]");
    }

    static bool applyDeposit(Account* acc, double amount) {
        if (!acc || amount <= 0) return false;
        acc->deposit(amount);
        return true;
    }

    static bool applyWithdraw(Account* acc, double amount) {
        if (!acc || amount <= 0 || acc->getBalance() < amount) return false;
        acc->withdraw(amount);
        return true;
    }
};

class TransactionManager {
private:
    mutex mtx;
    vector<Transaction> history;   // registro de todas las operaciones
    TransactionManager() = default;

public:
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;

    static TransactionManager& getInstance() {
        static TransactionManager instance;
        return instance;
    }

    bool deposit(Account* account, double amount) {
        lock_guard<mutex> lock(mtx);
        bool ok = Transaction::applyDeposit(account, amount);
        history.emplace_back(TransactionType::DEPOSIT, amount, ok);
        return ok;
    }

    bool withdraw(Account* account, double amount) {
        lock_guard<mutex> lock(mtx);
        bool ok = Transaction::applyWithdraw(account, amount);
        history.emplace_back(TransactionType::WITHDRAW, amount, ok);
        return ok;
    }

    size_t getHistorySize() {
        lock_guard<mutex> lock(mtx);
        return history.size();
    }
};

int main() {
    auto& manager = TransactionManager::getInstance();
    Account account("A1", 1000);
    vector<thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            manager.deposit(&account, 100);
            manager.withdraw(&account, 50);
        });
    }

    for (auto& t : threads) t.join();

    cout << "Balance final: " << account.getBalance() << endl;
    cout << "Transacciones registradas: " << manager.getHistorySize() << endl;
    cout << "Direccion manager: " << &manager << endl;
    cout << "Direccion manager: " << &TransactionManager::getInstance() << endl;

    return 0;
}
