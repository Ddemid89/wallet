#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QVector>
#include <QDate>

#include "domain.h"
#include <memory>
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>
#include "transactions_manager.h"

struct AccAdder {
    AccountType type;
    QString name;
    Money balance;
    double perc_rate;
    int payday;
    Money overdraft;
    bool consider;
    Money payment;
};

struct Balance{
    Money inc;
    Money dec;
};

struct CategoryInfo {
    QString name;
    size_t idx;
    int indent;
    CategoryTypeClass type_;
};

std::unique_ptr<AccountBase> MakeAccount(AccAdder acc, size_t idx);

class Wallet {
public:
    Wallet(transactions_manager::LoaderInterface&);

    const std::vector<std::unique_ptr<AccountBase>>& GetAccounts() const;
    AccountBase* GetOneAccount(size_t id) const;
    QStringList GetAccountsNames() const;
    QString GetAccName(size_t idx) const;
    void AddAccount(AccAdder);
    size_t AccounsExist() const;
    void ChangeAccountType(AccAdder acc, size_t acc_id);

    QVector<CategoryInfo> GetCategories(bool arrive_) const;
    QVector<CategoryInfo> GetAllCategories() const;
    int AddCategory(const QString& name, int parent_idx, bool inc, bool dec);
    QString GetCatName(size_t idx) const;
    std::set<size_t> GetCategoryChilds(size_t cat_id) const;
    const Category* GetCategory(size_t cat_id) const;
    std::pair<bool, bool> GetCategoryChildsType(size_t cat_id) const;
    void EditCategory(size_t idx, QString new_name, size_t new_parent, bool inc, bool dec);

    void AddTransaction(transactions_manager::TransactionAdder transact); //OK
    void AddTransaction(transactions_manager::TransactionAdder transact, size_t idx); //OK

    QVector<const Transaction*> GetTransacts(TransactShowType type = TransactShowType::All,
                                        size_t number = 10, bool late_to_early = true) const;

    QVector<const Transaction*> GetIncomes(size_t number = 10, bool late_to_early = true) const;
    QVector<const Transaction*> GetExpenses(size_t number = 10, bool late_to_early = true) const;
    QVector<const Transaction*> GetTransfers(size_t number = 10, bool late_to_early = true) const;
    QVector<const Transaction*> GetIncDecTransacts(bool arrive, size_t number = 10, bool late_to_early = true) const;

    QVector<const Transaction*> GetTransactFiltred
    (QDate from, QDate to, TransactShowType type, size_t acc_id, size_t cat_id) const;

    void SaveTransacts() {
        transacts_.Save();
    }

    model_representation::AccountsData GetAccountsRepresentation() const;
    model_representation::CategoriesData GetCategoriesRepresentation() const;

    void RestoreAccounts(model_representation::AccountsData&& accs);
    void RestoreCategories(model_representation::CategoriesData&& cats);

    bool IsAccountDeleted(size_t acc_id);
    void SetAccountDeleted(size_t acc_id, bool deleted);

    void DeleteTransaction(size_t idx);
    const Transaction* FindTransact(size_t idx);
    void EditTransact(size_t idx, transactions_manager::TransactionAdder& adder);

private:
    void RestoreOneAccount(model_representation::AccountRepresentation&& acc);
    void RestoreOneCategory(model_representation::CategoryRepresentation&& cat);
    void AddCatAndChilds(size_t index, QVector<CategoryInfo>& result, int indent, bool inc, bool dec) const;
    void GetCategoryChilds(std::set<size_t>& res, size_t cat_id) const;
    Category& GetCategoryById(size_t id);

    QVector<model_representation::AccountRepresentation> GetAccsRepresentation() const;
    QVector<model_representation::CategoryRepresentation> GetCatsRepresentation() const;

    size_t GetAccIdx();
    size_t accounts_idx_ = 1; // <-- 1
    std::vector<std::unique_ptr<AccountBase>> accounts_; // <-- 2
    std::unordered_map<size_t, AccountBase*> accs_index_;

    size_t GetCatIdx();
    size_t category_idx_ = 1; // <-- 3
    std::deque<Category> categories_; // <-- 4
    std::unordered_map<size_t, Category*> cats_index_;

    mutable transactions_manager::TransactionsManager transacts_;
};

#endif // MODEL_H
