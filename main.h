#ifndef MAIN_H
#define MAIN_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Main; }
QT_END_NAMESPACE

class Main : public QMainWindow
{
    Q_OBJECT

public:
    Main(QWidget *parent = nullptr);
    ~Main();

private slots:
    void on_findDLL_clicked();
    void updateProcessList();
    DWORD getProcID(QString filename);

    void on_ProcessList_itemDoubleClicked(QListWidgetItem *item);

    void on_refresh_clicked();

    void on_InjectDLL_clicked();

    void on_autoSelectCSGO_clicked();

    void on_searchButton_clicked();

    void on_ClearProc_clicked();

private:
    Ui::Main *ui;
};
#endif // MAIN_H
