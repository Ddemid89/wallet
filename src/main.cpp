#include "mainwindow.h"

#include <QApplication>
#include "model.h"
#include "first_window.h"
#include "add_account_window.h"
#include "add_category_window.h"
#include "add_inc_dec.h"
#include "add_transfer_window.h"
#include "transaction_editor.h"
#include "serialization.h"
#include "binary_loader.h"
#include "account_edit.h"
#include "cell_w.h"

const QString ACC_FILE = "../../data/accs.bin";
const QString CAT_FILE = "../../data/cats.bin";
const QString TRANSACTION_DIR = "../../data/transactions";

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    auto loader = std::make_unique<binary_loader::BinaryLoader>(TRANSACTION_DIR);

    Wallet wallet(*loader.get());
    MainWindow w(wallet);

    w.SetFirstWidget(new FirstWindow(wallet, w));
    w.SetAddAccountWidget(new AddAccountWindow(wallet, w));
    w.SetAddCategoryWidget(new AddCategoryWindow(wallet, w));
    w.SetAddIncomeWidget(new AddIncDec(wallet, w, true));
    w.SetAddExpenseWidget(new AddIncDec(wallet, w, false));
    w.SetAddTransferWidget(new AddTransferWindow(wallet, w));
    w.SetEditTransactsWidget(new TransactionEditor(wallet, w));
    w.SetEditAccountsWidget(new AccountEdit(wallet, w));
    w.SetCellIncDecWidget(new CellWindow(wallet, w));
    w.setFixedSize({800, 600});

    wallet.RestoreAccounts(model_serialization::DeserializeAccounts(ACC_FILE));
    wallet.RestoreCategories(model_serialization::DeserializeCategories(CAT_FILE));

    w.show();
    auto ret = a.exec();
    model_serialization::SerializeAccounts(ACC_FILE, wallet.GetAccountsRepresentation());
    model_serialization::SerializeCategories(CAT_FILE, wallet.GetCategoriesRepresentation());
    wallet.SaveTransacts();

    return ret;
}
