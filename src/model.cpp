#include "model.h"

#include <QDebug>
#include <algorithm>

Wallet::Wallet(transactions_manager::LoaderInterface& loader) : transacts_DELETE_THIS(loader){
    categories_.emplace_back(0, "Все категории", true, true);
    cats_index_[0] = &categories_.back();
}

const std::vector<std::unique_ptr<AccountBase>>& Wallet::GetAccounts() const {
    return accounts_;
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
    transacts_DELETE_THIS.AddTransaction(transact);

    auto& acc = *accs_index_.at(transact.acc_idx);
    acc += transact.sum;
}

void Wallet::AddTransfer(transactions_manager::TransferAdder transfer) {
    transacts_DELETE_THIS.AddTransfer(transfer);

    auto& from = *accs_index_.at(transfer.from_idx);
    auto& to = *accs_index_.at(transfer.to_idx);

    from -= transfer.sum;
    to += transfer.sum;

}

QVector<TransactBase*> Wallet::GetTransacts(TransactType type, size_t number, bool late_to_early) const {
    return transacts_DELETE_THIS.GetTransacts(type, number, late_to_early);
}

QVector<TransactBase*> Wallet::GetIncomes(size_t number, bool late_to_early) const {
    return transacts_DELETE_THIS.GetTransacts(TransactType::Income, number, late_to_early);
}

QVector<TransactBase*> Wallet::GetExpenses(size_t number, bool late_to_early) const {
    return transacts_DELETE_THIS.GetTransacts(TransactType::Expense, number, late_to_early);
}

QVector<TransactBase*> Wallet::GetTransfers(size_t number, bool late_to_early) const {
    return transacts_DELETE_THIS.GetTransacts(TransactType::Transfer, number, late_to_early);
}

QVector<TransactBase*> Wallet::GetIncDecTransacts(bool arrive, size_t number, bool late_to_early) const {
    if (arrive) {
        return transacts_DELETE_THIS.GetTransacts(TransactType::Income, number, late_to_early);
    } else {
        return transacts_DELETE_THIS.GetTransacts(TransactType::Expense, number, late_to_early);
    }
}

QVector<TransactBase*> Wallet::GetTransactFiltred
(QDate from, QDate to, TransactType type, size_t acc_id, size_t cat_id) const {
    auto trs = transacts_DELETE_THIS.GetTransactFiltred(from, to, type);

    bool any_acc = acc_id == 0;
    bool any_cat = cat_id == 0;

    detail::TransactPredicat tr(any_acc, any_cat, acc_id, cat_id, GetCategoryChilds(cat_id));

    auto op = [&tr](TransactBase* trs) {
        trs->Visit(tr);
        return !tr.GetRes();
    };

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

void Wallet::SetAccountDeleted(size_t acc_id, bool deleted) {
    auto it = accs_index_.find(acc_id);

    if (it != accs_index_.end()) {
        it->second->SetDeleted(deleted);
    }
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

//QVector<TransactBase*> Wallet::GetTransacts(const TransactIndex& index, size_t number, bool late_to_early) const {
//    number = std::min(index.size(), number);

//    QVector<TransactBase*> result(number);

//    auto op = [](const std::unique_ptr<TransactBase>& ptr){
//        return ptr.get();
//    };

//    if (late_to_early) {
//        auto end = index.cbegin();
//        std::advance(end, number);
//        std::transform(index.cbegin(), end, result.begin(), op);
//    } else {
//        auto end = index.crbegin();
//        std::advance(end, number);
//        std::transform(index.crbegin(), end, result.begin(), op);
//    }

//    return result;
//}

//QVector<TransactBase*> Wallet::GetTransactFiltred
//    (const TransactIndex& transacts, QDate from, QDate to, size_t acc, size_t cat) const {
//    QVector<TransactBase*> result;

//    bool any_acc = acc == 0;
//    bool any_cat = cat == 0;

//    std::unique_ptr<TransactBase> from_ptr = std::make_unique<Transfer>(0, 0, from, Money{0}, 0);

//    detail::TransactPredicat pred(any_acc, any_cat, acc, cat, GetCategoryChilds(cat));

//    auto it = transacts.lower_bound(from_ptr);

//    while(it != transacts.end() && (*it)->Date() <= to) {
//        auto& tr = *it;
//        tr->Visit(pred);
//        if (pred.GetRes()) {
//            result.push_back(tr.get());
//        }
//        it++;
//    }

//    return result;
//}

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

bool Wallet::trans_comp::operator()(const trans_ptr& a, const trans_ptr& b) const {
    return (a->Date() < b->Date())
            || (a->Date() == b->Date() && a->Index() < b->Index());
}

void detail::TransactInfo::Visit(TransactBase& tr) {
    tr.Visit(*this);
}

void detail::TransactInfo::Visit(Transaction& tr) {
    res_ = tr.Date().toString("dd.MM.yyyy") + ": "
            + wallet_.GetAccName(tr.AccountFromIdx())
            + (tr.IsIncome() ? " <--(" : " ---(")
            + tr.Sum().StringAbs() + " руб."
            + (tr.IsIncome() ? ")--- " : ")--> ")
            + wallet_.GetCatName(tr.CategoryIdx());
}

void detail::TransactInfo::Visit(Transfer& tr) {
    res_ = tr.Date().toString("dd.MM.yyyy") + ": "
            + wallet_.GetAccName(tr.AccountFromIdx())
            + " ---["
            + tr.Sum().String() + " руб."
            + "]--> "
            + wallet_.GetAccName(tr.AccountToIdx());
}

QString detail::TransactInfo::GetResult() const {
    return res_;
}

detail::TransactPredicat::TransactPredicat(bool any_acc, bool any_cat, size_t acc, size_t cat, std::set<size_t>&& cat_childs)
    : any_acc_(any_acc), any_cat_(any_cat), acc_(acc), cat_(cat), cat_childs_(std::move(cat_childs)) {}

void detail::TransactPredicat::Visit(TransactBase& tr) {
    tr.Visit(*this);
}

void detail::TransactPredicat::Visit(Transaction& tr) {
    if ((any_acc_ || tr.AccountFromIdx() == acc_)
            && (any_cat_ || tr.CategoryIdx() == cat_ || cat_childs_.count(tr.CategoryIdx()) != 0)) {
        res_ = true;
    } else {
        res_ = false;
    }
}

void detail::TransactPredicat::Visit(Transfer& tr) {
    if (any_cat_ && (any_acc_ || tr.AccountToIdx() == acc_ || tr.AccountFromIdx() == acc_)) {
        res_ = true;
    } else {
        res_ = false;
    }
}

bool detail::TransactPredicat::GetRes() const {
    return res_;
}
