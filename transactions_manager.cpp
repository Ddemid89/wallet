#include "transactions_manager.h"

namespace transactions_manager {

TransactionsManager::TransactionsManager(LoaderInterface& loader) : loader_(loader) {
    Load(QDate::currentDate().addYears(-1), QDate::currentDate());
}

TransactionsManager::~TransactionsManager() {

}

void TransactionsManager::AddTransaction(TransactionAdder& trs) {
    Load(trs.date);
    size_t new_id = GetNewId();

    transacts_.emplace(std::make_unique<Transaction>(
                           new_id,
                           trs.acc_idx,
                           trs.date,
                           trs.sum,
                           trs.cat_idx,
                           trs.inc
                           ));
    qDebug() << transacts_.size();
}

void TransactionsManager::AddTransfer(TransferAdder& trs) {
    Load(trs.date);
    size_t new_id = GetNewId();

    transacts_.emplace(std::make_unique<Transfer>(
                           new_id,
                           trs.from_idx,
                           trs.date,
                           trs.sum,
                           trs.to_idx
                           ));
}

QVector<TransactBase*> TransactionsManager::GetTransacts(TransactType type, size_t number, bool late_to_early) {
    if (late_to_early) {
        return GetTransacts(transacts_.cbegin(), transacts_.cend(), number, type);
    } else {
        return GetTransacts(transacts_.crbegin(), transacts_.crend(), number, type);
    }
}

QVector<TransactBase*> TransactionsManager::GetTransactFiltred(QDate from, QDate to, TransactType type) {
    Load(from, to);

    QVector<TransactBase*> result;
    TypeChecker tc(type);
    std::unique_ptr<TransactBase> l_bound = std::make_unique<Transfer>(0, 0, from, 0, 0);

    auto it = transacts_.lower_bound(l_bound);

    while(it != transacts_.end() && (*it)->Date() <= to) {
        (*it)->Visit(tc);
        if (tc.Result()) {
            result.push_back(it->get());
        }
        ++it;
    }

    return result;
}


bool TransactionComp::operator()(const std::unique_ptr<TransactBase>& a, const std::unique_ptr<TransactBase>& b) const {
    return a->Date() < b->Date()
            || (a->Date() == b->Date() && a->Index() < b->Index());
}

void TypeChecker::Visit(TransactBase& trs) {
    trs.Visit(*this);
}

void TypeChecker::Visit(Transaction& trs) {
    bool inc = trs.IsIncome() && type_ == TransactType::Income;
    bool exp = !trs.IsIncome() && type_ == TransactType::Expense;
    res_ = inc || exp || type_ == TransactType::IncExp || type_ == TransactType::All;
}

void TypeChecker::Visit(Transfer&) {
    res_ = type_ == TransactType::Transfer || type_ == TransactType::All;
}

bool TypeChecker::Result() {
    return res_;
}

bool Date::operator<(Date other) const {
    return year < other.year
            || (year == other.year && month < other.month);
}

bool Date::operator<=(Date other) const {
    return year < other.year
            || (year == other.year && !(month > other.month));
}

QString Date::String() const {
    return QString::number(month) + "_" + QString::number(year);
}

} // namespace transactions_manager
