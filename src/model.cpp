#include "model.h"

#include <QDebug>
#include <algorithm>

namespace {
class TransactionFilter{
public:
    TransactionFilter(size_t from_id, size_t to_id, std::set<size_t>&& childs) : from_{from_id}, to_{to_id}, childs_(std::move(childs)) {

    }
    bool operator()(const Transaction_DEL* trns) {
        return !(CheckAcc(trns) && CheckCat(trns));
    }
private:
    bool CheckAcc(const Transaction_DEL* trns) {
        return from_ == 0 || from_ == trns->AccountFromIdx();
    }
    bool CheckCat(const Transaction_DEL* trns) {
        return to_ == 0 || to_ == trns->ToIdx() || childs_.count(trns->ToIdx()) > 0;
    }

    const size_t from_;
    const size_t to_;
    const std::set<size_t> childs_;
};
}

Wallet::Wallet(transactions_manager::LoaderInterface& loader) : transacts_(loader){
    categories_.emplace_back(0, "Все категории", true, true);
    cats_index_[0] = &categories_.back();
}

const std::vector<std::unique_ptr<AccountBase>>& Wallet::GetAccounts() const {
    return accounts_;
}

AccountBase *Wallet::GetOneAccount(size_t id) const {
    return accs_index_.at(id);
}

QStringList Wallet::GetAccountsNames() const {
    QStringList result(accounts_.size());

    std::transform(accounts_.begin(), accounts_.end(), result.begin(),
                   [](const std::unique_ptr<AccountBase>& acc){
                        return acc->GetName();
                   }
    );

    return result;
}

QString Wallet::GetAccName(size_t idx) const {
    auto it = accs_index_.find(idx);

    if (it != accs_index_.end()) {
        return it->second->GetName();
    } else {
        return "Acc not found!";
    }
}

void Wallet::AddAccount(AccAdder account) {
    size_t new_index = GetAccIdx();
    accounts_.push_back(MakeAccount(account, new_index));
    accs_index_[new_index] = accounts_.back().get();
}

size_t Wallet::AccounsExist() const {
    size_t res = 0;
    for (auto& acc : accounts_) {
        res += !acc->IsDeleted();
    }

    return res;
}

void Wallet::ChangeAccountType(AccAdder acc, size_t acc_id) {
    std::unique_ptr<AccountBase>& acc_ptr = accounts_.at(acc_id - 1);

    bool deleted = acc_ptr->IsDeleted();

    if (acc_ptr->GetIndex() != acc_id) {
        throw std::runtime_error("Account index not equal index in vector in Wallet::ChangeAccountType.");
    }

    auto new_ptr = MakeAccount(acc, acc_id);
    auto tmp_ptr = new_ptr.get();
    acc_ptr.swap(new_ptr);
    accs_index_[acc_id] = tmp_ptr;

    tmp_ptr->SetDeleted(deleted);
}

QVector<CategoryInfo> Wallet::GetCategories(bool arrive) const {
    QVector<CategoryInfo> result;

    AddCatAndChilds(0, result, 0, arrive, !arrive);

    return result;
}

QVector<CategoryInfo> Wallet::GetAllCategories() const {
    QVector<CategoryInfo> result;

    AddCatAndChilds(0, result, 0, false, false);

    return result;
}

int Wallet::AddCategory(const QString& name, int parent_idx, bool inc, bool dec) {
    size_t new_idx = GetCatIdx();
    Category& new_cat = categories_.emplace_back(new_idx, name, inc, dec);
    cats_index_[new_idx] = &new_cat;

    Category& parent = *cats_index_.at(parent_idx);
    parent.AddChild(new_cat);
    return new_idx;
}

QString Wallet::GetCatName(size_t idx) const {
    auto it = cats_index_.find(idx);

    if (it != cats_index_.end()) {
        return it->second->GetName();
    } else {
        return "Cat not found!";
    }
}

std::set<size_t> Wallet::GetCategoryChilds(size_t cat_id) const {
    std::set<size_t> res;

    for (size_t child : cats_index_.at(cat_id)->GetChilds()) {
        GetCategoryChilds(res, child);
    }

    return res;
}

void Wallet::AddTransaction(transactions_manager::TransactionAdder transact) {
    transacts_.AddTransaction(transact);

    auto& acc = *accs_index_.at(transact.from_idx);

    if ((transact.type == TransactionType::Income && transact.sum.Kopek() > 0) || (transact.type == TransactionType::Expense && transact.sum.Kopek() < 0)) {
        acc += transact.sum;
    } else {
        acc -= transact.sum;
    }

}

void Wallet::AddTransaction(transactions_manager::TransactionAdder transact, size_t idx) {
    transacts_.AddTransaction(transact, idx);

    auto& acc = *accs_index_.at(transact.from_idx);

    if ((transact.type == TransactionType::Income && transact.sum.Kopek() > 0) || (transact.type == TransactionType::Expense && transact.sum.Kopek() < 0)) {
        acc += transact.sum;
    } else {
        acc -= transact.sum;
    }

}

QVector<const Transaction_DEL*> Wallet::GetTransacts(TransactShowType type, size_t number, bool late_to_early) const {
    return transacts_.GetTransacts(type, number, late_to_early);
}

QVector<const Transaction_DEL*> Wallet::GetIncomes(size_t number, bool late_to_early) const {
    return transacts_.GetTransacts(TransactShowType::Income, number, late_to_early);
}

QVector<const Transaction_DEL*> Wallet::GetExpenses(size_t number, bool late_to_early) const {
    return transacts_.GetTransacts(TransactShowType::Expense, number, late_to_early);
}

QVector<const Transaction_DEL*> Wallet::GetTransfers(size_t number, bool late_to_early) const {
    return transacts_.GetTransacts(TransactShowType::Transfer, number, late_to_early);
}

QVector<const Transaction_DEL*> Wallet::GetIncDecTransacts(bool arrive, size_t number, bool late_to_early) const {
    if (arrive) {
        return transacts_.GetTransacts(TransactShowType::Income, number, late_to_early);
    } else {
        return transacts_.GetTransacts(TransactShowType::Expense, number, late_to_early);
    }
}

QVector<const Transaction_DEL*> Wallet::GetTransactFiltred
(QDate from, QDate to, TransactShowType type, size_t acc_id, size_t cat_id) const {
    auto trs = transacts_.GetTransactFiltred(from, to, type);

    TransactionFilter op(acc_id, cat_id, GetCategoryChilds(cat_id));

    auto it = std::remove_if(trs.begin(), trs.end(), op);

    trs.erase(it, trs.end());

    return trs;
}

model_representation::AccountsData Wallet::GetAccountsRepresentation() const {
    return {accounts_idx_, GetAccsRepresentation()};
}

model_representation::CategoriesData Wallet::GetCategoriesRepresentation() const {
    return {category_idx_, GetCatsRepresentation()};
}

void Wallet::RestoreAccounts(model_representation::AccountsData&& accs) {
    accounts_idx_ = accs.id;

    for (auto& acc : accs.accs) {
        RestoreOneAccount(std::move(acc));
    }
}

void Wallet::RestoreCategories(model_representation::CategoriesData&& cats) {
    category_idx_ = cats.id;

    for (auto& cat : cats.cats) {
        RestoreOneCategory(std::move(cat));
    }
}

bool Wallet::IsAccountDeleted(size_t acc_id) {
    auto it = accs_index_.find(acc_id);
    AccountBase* acc = it->second;

    return acc->IsDeleted();
}

void Wallet::SetAccountDeleted(size_t acc_id, bool deleted) {
    auto it = accs_index_.find(acc_id);

    if (it != accs_index_.end()) {
        it->second->SetDeleted(deleted);
    }
}

void Wallet::DeleteTransaction(size_t idx) {
    auto trns = transacts_.FindTransact(idx);

    AccountBase* acc = accs_index_.at(trns->AccountFromIdx());

    if (trns->Type() == TransactionType::Income) {
        *acc -= trns->Sum();
    } else if (trns->Type() == TransactionType::Expense) {
        *acc += trns->Sum();
    } else {
        AccountBase* to = accs_index_.at(trns->ToIdx());
        *acc += trns->Sum();
        *to  -= trns->Sum();
    }

    transacts_.DeleteTransact(idx);
}

const Transaction_DEL* Wallet::FindTransact(size_t idx) {
    return transacts_.FindTransact(idx);
}

void Wallet::EditTransact(size_t idx, transactions_manager::TransactionAdder& adder) {
    DeleteTransaction(idx);
    AddTransaction(adder, idx);
}


void Wallet::RestoreOneAccount(model_representation::AccountRepresentation&& acc) {
    AccountType type = static_cast<AccountType>(acc.type);

    switch (type) {
    case AccountType::Debet:
        accounts_.push_back(std::make_unique<Debet>(acc));
        break;
    case AccountType::Credit:
        accounts_.push_back(std::make_unique<Credit>(acc));
        break;
    case AccountType::Deposit:
        accounts_.push_back(std::make_unique<Deposit>(acc));
        break;
    case AccountType::Overdraft:
        accounts_.push_back(std::make_unique<OverdraftCard>(acc));
        break;
    }

    accs_index_[acc.id] = accounts_.back().get();
}

void Wallet::RestoreOneCategory(model_representation::CategoryRepresentation&& cat) {
    Category* parent = &GetCategoryById(cat.parent_id);

    categories_.emplace_back(cat, parent);

    cats_index_[cat.id] = &categories_.back();
}

QVector<model_representation::AccountRepresentation> Wallet::GetAccsRepresentation() const {
    QVector<model_representation::AccountRepresentation> res;
    res.reserve(accounts_.size());

    for (auto& acc : accounts_) {
        res.push_back(acc->GetRepresentation());
    }

    return res;
}

QVector<model_representation::CategoryRepresentation> Wallet::GetCatsRepresentation() const {
    QVector<model_representation::CategoryRepresentation> res;
    res.reserve(categories_.size());

    for (auto it = ++categories_.begin(); it != categories_.end(); ++it) {
        res.push_back(it->GetRepresentation());
    }

    return res;
}

size_t Wallet::GetAccIdx() {
    return accounts_idx_++;
}

size_t Wallet::GetCatIdx() {
    return category_idx_++;
}

Category& Wallet::GetCategoryById(size_t id) {
    auto it = cats_index_.find(id);

    if (it != cats_index_.end()) {
        return *it->second;
    }

    throw std::invalid_argument("Нет категории с таким ID!");
}

void Wallet::AddCatAndChilds(size_t index, QVector<CategoryInfo>& result, int indent, bool inc, bool dec) const {
    const Category& cat = *cats_index_.at(index);

    bool ok = (!inc || cat.isInc()) && (!dec || cat.isDec());

    if (!ok) {
        return;
    }

    CategoryInfo info;
    info.idx = index;
    info.name = cat.GetName();
    info.indent = indent;

    result.push_back(std::move(info));

    for (size_t child : cat.GetChilds()) {
        AddCatAndChilds(child, result, indent + 1, inc, dec);
    }
}

void Wallet::GetCategoryChilds(std::set<size_t>& res, size_t cat_id) const {
    res.insert(cat_id);

    for (size_t child : cats_index_.at(cat_id)->GetChilds()) {
        GetCategoryChilds(res, child);
    }
}

std::unique_ptr<AccountBase> MakeAccount(AccAdder acc, size_t idx) {
    SimpleAcc s_acc;
    s_acc.idx = idx;
    s_acc.name = std::move(acc.name);
    s_acc.sum = acc.balance;
    s_acc.consider = acc.consider;
    if (acc.type == AccountType::Debet) {
        return std::make_unique<Debet>(
            std::move(s_acc)
        );
    } else if (acc.type == AccountType::Deposit) {
        return std::make_unique<Deposit>(
            std::move(s_acc),
            acc.perc_rate,
            acc.payday
        );
    } else if (acc.type == AccountType::Credit) {
        return std::make_unique<Credit>(
            std::move(s_acc),
            acc.perc_rate,
            acc.payday,
            acc.payment
        );
    } else {
        return std::make_unique<OverdraftCard>(
            std::move(s_acc),
            acc.perc_rate,
            acc.payday,
            acc.overdraft
        );
    }
}
