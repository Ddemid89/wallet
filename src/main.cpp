#include "mainwindow.h"

#include <QApplication>
#include "model.h"
#include "first_window.h"
#include "add_account_window.h"
#include "add_category_window.h"
#include "transaction_editor.h"
#include "serialization.h"
#include "json_serialization.h"
#include "binary_loader.h"
#include "json_loader.h"
#include "cell_w.h"
#include "file_system.h"

// TODO Доделать функционал:
//          * Начисление процентов
//          * Выбор даты оплаты
//          * Сообщение об оплате исоздание автоматических транзакций

// TODO Подумать о главном экране:
//          * Отображение общего баланса
//          * Прокрутка для счетов
//          * Дополнительная информация

// TODO Подумать о стеке возврата для кнопки "назад"

// TODO Подумать об отображении какой-либо статистики

// TODO Разобраться с документированием

const QString DATA_DIR = QString::fromStdString(QDir::currentPath().toStdString()) + "/data";
const QString ACC_FILE = DATA_DIR + "/accs";
const QString CAT_FILE = DATA_DIR + "/cats";
const QString TRANSACTION_DIR = DATA_DIR + "/transactions";

int main(int argc, char *argv[]) {
    file_system::CheckAndMakeDir(DATA_DIR);
    file_system::CheckAndMakeDir(TRANSACTION_DIR);


    QApplication a(argc, argv);

    auto loader = std::make_unique<binary_loader::BinaryLoader>(TRANSACTION_DIR);

    json_loader::JsonLoader j_ldr(TRANSACTION_DIR);

    //Wallet wallet(*loader.get());
    Wallet wallet(j_ldr);

    MainWindow w(wallet);

    w.SetFirstWidget(new FirstWindow(wallet, w));
    w.SetAddAccountWidget(new AddAccountWindow(wallet, w));
    w.SetAddCategoryWidget(new AddCategoryWindow(wallet, w));
    w.SetEditTransactsWidget(new TransactionEditor(wallet, w));
    w.SetCellIncDecWidget(new CellWindow(wallet, w));
    w.setFixedSize({800, 600});

    wallet.RestoreAccounts(json_serialization::DeserializeAccounts(ACC_FILE));
    wallet.RestoreCategories(json_serialization::DeserializeCategories(CAT_FILE));

    w.show();
    auto ret = a.exec();
    json_serialization::SerializeAccounts(ACC_FILE, wallet.GetAccountsRepresentation());
    json_serialization::SerializeCategories(CAT_FILE, wallet.GetCategoriesRepresentation());
    wallet.SaveTransacts();

    return ret;
}
