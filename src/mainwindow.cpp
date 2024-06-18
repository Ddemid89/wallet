#include "mainwindow.h"

#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <map>

void MainWindow::SetFirstWidget(QWidget* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::First] = widgets_->count() - 1;
}

void MainWindow::SetAddAccountWidget(Widgets* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::AddAccount] = widgets_->count() - 1;
}

void MainWindow::SetAddCategoryWidget(Widgets* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::AddCategory] = widgets_->count() - 1;
}

void MainWindow::SetAddIncomeWidget(Widgets* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::AddIncome] = widgets_->count() - 1;
}

void MainWindow::SetAddExpenseWidget(Widgets* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::AddExpens] = widgets_->count() - 1;
}

void MainWindow::SetAddTransferWidget(Widgets* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::AddTransfer] = widgets_->count() - 1;
}

void MainWindow::SetEditTransactsWidget(Widgets* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::EditTransacts] = widgets_->count() - 1;
}

void MainWindow::SetCellIncDecWidget(Widgets* widget) {
    widgets_->addWidget(widget);
    widgets_index_[WidgetType::CellIncDec] = widgets_->count() - 1;
}



MainWindow::MainWindow(Wallet& wallet, QWidget* parent) : QMainWindow(parent)
                                                        , wallet_(wallet){
    setWindowTitle("Учет финансов (или типа того)");
    sbar_ = statusBar();
    mbar_ = menuBar();
    tbar_ = new QToolBar("Тулбар");

    addToolBar(tbar_);

    tbar_->addAction("1"); // можно и иконки
    tbar_->addAction("2"); // можно связать со слотом

    sbar_->showMessage("Здесь можно выводить какую-то информацию", 60000);

    QMenu* m1 = new QMenu("Здесь");
    QMenu* m2 = new QMenu("Счета");
    m2->addAction("Добавить", this, [this](){
        emit(this->change_window(WidgetType::AddAccount));
    });

    QMenu* m3 = new QMenu("Категории");
    m3->addAction("Добавить", this, [this]{
        emit(this->change_window(WidgetType::AddCategory));
    });
    m3->addAction("Редактировать");

    QMenu* m4 = new QMenu("Транзакции");
    m4->addAction("Добавить доходы", this, [this]{
        emit(this->change_window(WidgetType::AddIncome));
    });
    m4->addAction("Добавить расходы", this, [this]{
        emit(this->change_window(WidgetType::AddExpens));
    });
    m4->addAction("Добавить расходы/доходы/переводы", this, [this]{
        emit(this->change_window(WidgetType::CellIncDec));
    });
    m4->addAction("Редактировать", this, [this]{
        emit(this->change_window(WidgetType::EditTransacts));
    });

    QMenu* m5 = new QMenu("Анализ");
    QMenu* sm1 = new QMenu("Графики");
    QMenu* sm2 = new QMenu("Таблицы");
    QMenu* sm3 = new QMenu("Анализ");

    m5->addMenu(sm1);
    m5->addMenu(sm2);
    m5->addMenu(sm3);

    sm1->addAction("Такой-то график");
    sm1->addAction("Сякой график");

    sm2->addAction("Такая-то таблица");
    sm2->addAction("Сякая таблица");

    sm3->addAction("Такой-то анализ");
    sm3->addAction("Сякой анализ");

    QMenu* sm = new QMenu("Подменю");
    QMenu* ssm = new QMenu("Подподменю");

    ssm->addAction("Пункт 1");
    ssm->addAction("Пункт 2");
    ssm->addAction("Пункт 3");

    sm->addAction("Пункт 1");
    sm->addMenu(ssm);
    sm->addSeparator();
    sm->addAction("Пункт 2");
    sm->addAction("Пункт 3");

    //mbar_->addMenu(m1);
    mbar_->addMenu(m2);
    mbar_->addMenu(m3);
    mbar_->addMenu(m4);
    mbar_->addMenu(m5);


    m1->addAction(/* QIcon icon,*/ "Пункт 1" /* , SLOT(слот), */ /* QKeySequence("Ctrl+Q") */ );
    m1->addAction("Пункт 2");
    m1->addSeparator();
    m1->addMenu(sm);

    widgets_ = new QStackedWidget;

    setCentralWidget(widgets_);
    //widgets_->setCurrentIndex(0);
}

MainWindow::~MainWindow() {

}

void MainWindow::change_window(WidgetType type) {
    size_t idx = widgets_index_.at(type);

    if (static_cast<int>(idx) == widgets_->currentIndex()) {
        return;
    }

    auto cur_widget = qobject_cast<Widgets*>(widgets_->currentWidget());

    if (cur_widget) {
        cur_widget->Deactivate();
    }

    widgets_->setCurrentIndex(idx);
}

void MainWindow::go_to_first() {
    auto cur_widget = qobject_cast<Widgets*>(widgets_->currentWidget());

    if (cur_widget) {
        cur_widget->Deactivate();
    }
    widgets_->setCurrentIndex(0);
}

void MainWindow::show_status(const QString& txt) {
    sbar_->showMessage(txt, 60000);
}


void Widgets::Deactivate() {

}
