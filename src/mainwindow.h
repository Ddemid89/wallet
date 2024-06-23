#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QVBoxLayout>
#include <QPushButton>
#include <QMainWindow>
#include <QStackedWidget>
#include "model.h"
#include <map>

enum class WidgetType {
    First,
    AddAccount,
    AddCategory,
    EditTransacts,
    CellIncDec
};

class Widgets;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    void SetFirstWidget(QWidget* widget);
    void SetAddAccountWidget(Widgets* widget);
    void SetAddCategoryWidget(Widgets* widget);
    void SetEditTransactsWidget(Widgets* widget);
    void SetEditAccountsWidget(Widgets* widget);
    void SetCellIncDecWidget(Widgets* widget);

    MainWindow(Wallet& wallet, QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void go_to_first();
    void show_status(const QString& txt);
    void change_window(WidgetType type);
    void closeEvent(QCloseEvent*) override;
private:
    Wallet& wallet_;

    std::map<WidgetType, size_t> widgets_index_;

    QStatusBar* sbar_;
    QMenuBar*   mbar_;
    QToolBar*   tbar_;
    QStackedWidget* widgets_;
};


class Widgets : public QWidget {
Q_OBJECT
public:
    Widgets(Wallet& wallet, MainWindow& m_window, QWidget* parent = nullptr)
        : QWidget(parent)
        , wallet_(wallet)
        , m_window_(m_window)
    {
        layout_->addWidget(back_);
        setLayout(layout_);

        connect(back_, SIGNAL(clicked()), &m_window_, SLOT(go_to_first()));
    }
    virtual void Deactivate();
protected:
    Wallet& wallet_;
    MainWindow& m_window_;
    QVBoxLayout* layout_ = new QVBoxLayout;
    QPushButton* back_   = new QPushButton("<---");
};
#endif // MAINWINDOW_H
