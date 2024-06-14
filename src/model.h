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
};

std::unique_ptr<AccountBase> MakeAccount(AccAdder acc, size_t idx);

class Wallet {
public:
    Wallet(transactions_manager::LoaderInterface&);

    const std::vector<std::unique_ptr<AccountBase>>& GetAccounts() const;
    QStringList GetAccountsNames() const;
    QString GetAccName(size_t idx) const;
    void AddAccount(AccAdder);

    QVector<CategoryInfo> GetCategories(bool arrive_) const;
    QVector<CategoryInfo> GetAllCategories() const;
    int AddCategory(const QString& name, int parent_idx, bool inc, bool dec);
    QString GetCatName(size_t idx) const;
    std::set<size_t> GetCategoryChilds(size_t cat_id) const;

    void AddTransaction(transactions_manager::TransactionAdder transact); //OK
    void AddTransfer(transactions_manager::TransferAdder transfer); //OK

    QVector<TransactBase*> GetTransacts(TransactType type = TransactType::All,
                                        size_t number = 10, bool late_to_early = true) const;

    QVector<TransactBase*> GetIncomes(size_t number = 10, bool late_to_early = true) const;
    QVector<TransactBase*> GetExpenses(size_t number = 10, bool late_to_early = true) const;
    QVector<TransactBase*> GetTransfers(size_t number = 10, bool late_to_early = true) const;
    QVector<TransactBase*> GetIncDecTransacts(bool arrive, size_t number = 10, bool late_to_early = true) const;

    QVector<TransactBase*> GetTransactFiltred
    (QDate from, QDate to, TransactType type, size_t acc_id, size_t cat_id) const;

    void SaveTransacts() {
        transacts_DELETE_THIS.Save();
    }

    model_representation::AccountsData GetAccountsRepresentation() const;
    model_representation::CategoriesData GetCategoriesRepresentation() const;

    void RestoreAccounts(model_representation::AccountsData&& accs);
    void RestoreCategories(model_representation::CategoriesData&& cats);

    void SetAccountDeleted(size_t acc_id, bool deleted);

private:
    using trans_ptr = std::unique_ptr<TransactBase>;

    class trans_comp{
    public:
        bool operator()(const trans_ptr& a, const trans_ptr& b) const;
    };

    using TransactIndex = std::set<trans_ptr, trans_comp>;

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

    mutable transactions_manager::TransactionsManager transacts_DELETE_THIS;
};

namespace detail {
class TransactInfo : public TransferVisitorInterface {
public:
    TransactInfo(Wallet& wallet) : wallet_(wallet) {}
    void Visit(TransactBase& tr) override;
    void Visit(Transaction& tr) override;
    void Visit(Transfer& tr) override;
    QString GetResult() const;
private:
    QString res_;
    Wallet& wallet_;
};

class TransactPredicat : public TransferVisitorInterface {
public:
    TransactPredicat(bool any_acc, bool any_cat, size_t acc, size_t cat, std::set<size_t>&& cat_childs);

    void Visit(TransactBase& tr) override;
    void Visit(Transaction& tr) override;
    void Visit(Transfer& tr) override;
    bool GetRes() const;
private:
    bool any_acc_;
    bool any_cat_;
    size_t acc_;
    size_t cat_;
    bool res_;
    std::set<size_t> cat_childs_;
};

} // namespace detail
#endif // MODEL_H
