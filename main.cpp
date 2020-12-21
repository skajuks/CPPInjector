#include "main.h"
#include "ui_main.h"
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <QFileDialog>
#include <QString>
#include <QApplication>
#include <wtsapi32.h>
#include "qprocessinfo.h"

Main::Main(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Main)
{
    ui->setupUi(this);
}

Main::~Main()
{
    delete ui;
}

int main(int argc, char** argv) {
    auto app = new QApplication(argc, argv);
    auto window = new Main;
    window->show();

   return app->exec();
}

void Main::on_findDLL_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select DLL"), "C://", "All (*.dll)");
    ui->DLLtxt->setText(filename);
}

DWORD Main::getProcID(QString filename)
{
    DWORD procID = 0;
    HANDLE h = NULL;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(Process32First(h, &pe))
    {
      do
      {
        if (QString::fromWCharArray(pe.szExeFile) == filename)
        {
            procID = pe.th32ProcessID;
            break;
        }
      } while(Process32Next(h, &pe));
    }
    CloseHandle(h);
    return procID;
}

void Main::updateProcessList()
{
    auto processes = QProcessInfo::enumerate();
    for(auto& listElem : processes){
        ui->ProcessList->addItem(listElem.name());
    }

}

void Main::on_ProcessList_itemDoubleClicked(QListWidgetItem *item)
{
    ui->Processtxt->setText(item->text());
}

void Main::on_refresh_clicked()
{
    updateProcessList();
}

void Main::on_InjectDLL_clicked()
{
    const char* dllPath = ui->DLLtxt->text().toStdString().c_str();
    QString procName = ui->Processtxt->text();
    DWORD procID = 0;

    while(!procID){
        procID = getProcID(procName);
        Sleep(30);
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);

    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {
        void * loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if (loc)
        {
            WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);
        }

        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

        if (hThread)
        {
            CloseHandle(hThread);
            ui->test->setText("Dll injected!");
        }

    }

    if (hProc)
    {
        CloseHandle(hProc);
    }
}
