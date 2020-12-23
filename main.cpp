#include "main.h"
#include "ui_main.h"
#include "injection.h"
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <QFileDialog>
#include <QString>
#include <QApplication>
#include <wtsapi32.h>
#include "qprocessinfo.h"
#include <QMessageBox>
#include <QDebug>


bool autoFindCS;

QString defaultLocation = "C://Windows//cheat";

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
    QString filename = QFileDialog::getOpenFileName(this, tr("Select DLL"), defaultLocation, "Dynamic linked library (*.dll)");
    ui->DLLtxt->setText(filename);
}


bool Main::IsCorrectTargetArchitecture(HANDLE hProc)
{
    BOOL bTarget = FALSE;
    if (!IsWow64Process(hProc, &bTarget))
    {
        printf("Can't confirm target process architecture: 0x%X\n", GetLastError());
        return false;
    }

    BOOL bHost = FALSE;
    IsWow64Process(GetCurrentProcess(), &bHost);

    return (bTarget == bHost);
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
        if (autoFindCS && listElem.name() == "csgo.exe")
        {
            ui->Processtxt->setText(listElem.name());
        }
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
    // Getting path and process variables from UI
    const char* dllPath = ui->DLLtxt->text().toStdString().c_str();
    QString procName = ui->Processtxt->text();
    DWORD procID = 0;

    // Loops while process == NULL
    while(!procID){
        procID = getProcID(procName);
        Sleep(30);
    }

    // Changes UI process name to process number
    ui->Processtxt->setText(QString::number(procID));

    // Opens handle to process
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);

    // Checks if process value is valid
    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {
        // Allocating memory for the loader code
        void * loc = VirtualAllocEx(hProc, 0, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

        if (loc)
        {
            // Write the loader information to target process
            WriteProcessMemory(hProc, loc, (LPVOID)dllPath, strlen(dllPath) + 1, 0);
        }

        // Create a remote thread to execute the loader code
        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"), loc, 0, 0);

        // Waits for thread to be created
        WaitForSingleObject(hThread, INFINITE);

        if (hThread)
        {
            CloseHandle(hThread);
            //VirtualFreeEx(hProc, loc, strlen(dllPath) + 1, MEM_RELEASE);
            ui->test->setText("Dll injected!");
        }

    }

    if (hProc)
    {
       CloseHandle(hProc);
    }
}

void Main::on_autoSelectCSGO_clicked()
{
    autoFindCS = true;
}

void Main::on_searchButton_clicked()
{
    QString filter = ui->Filter->text();
    int listSize = ui->ProcessList->count();

    for (int i = 0; i < listSize; i++)
    {
        if (ui->ProcessList->item(i)->text().startsWith(filter))
        {
            ui->ProcessList->item(i)->setHidden(false);
        }
        else
        {
            ui->ProcessList->item(i)->setHidden(true);
        }
    }


}

void Main::on_ClearProc_clicked()
{
    ui->DLLtxt->setText(NULL);
    ui->Processtxt->setText(NULL);
    ui->ProcessList->clear();
}

void Main::on_InjectDLL_ManMap_clicked()
{
    // Getting path and process variables from UI
    LPCSTR dllPath = ui->DLLtxt->text().toStdString().c_str();
    QString procName = ui->Processtxt->text();
    DWORD procID = 0;


    // Loops while process == NULL
    while(!procID){
        procID = getProcID(procName);
        Sleep(30);
    }

    // Changes UI process name to process number
    ui->Processtxt->setText(QString::number(procID));

    loaderdata LoaderParams;

    // Open the DLL
    HANDLE hFile = CreateFileA(dllPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL);

    DWORD FileSize = GetFileSize(hFile, NULL);
    PVOID FileBuffer = VirtualAlloc(NULL, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    // Read the DLL
    ReadFile(hFile, FileBuffer, FileSize, NULL, NULL);

    // Target Dll's DOS Header
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;

    // Target Dll's NT Headers
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)FileBuffer + pDosHeader->e_lfanew);

    // Opening target process.
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);

    // Allocating memory for the DLL
    PVOID ExecutableImage = VirtualAllocEx(hProcess, NULL, pNtHeaders->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // Copy the headers to target process
    WriteProcessMemory(hProcess, ExecutableImage, FileBuffer, pNtHeaders->OptionalHeader.SizeOfHeaders, NULL);

    // Target Dll's Section Header
    PIMAGE_SECTION_HEADER pSectHeader = (PIMAGE_SECTION_HEADER)(pNtHeaders + 1);

    // Copying sections of the dll to the target process
    for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
    {
        WriteProcessMemory(hProcess, (PVOID)((LPBYTE)ExecutableImage + pSectHeader[i].VirtualAddress),
            (PVOID)((LPBYTE)FileBuffer + pSectHeader[i].PointerToRawData), pSectHeader[i].SizeOfRawData, NULL);
    }

    // Allocating memory for the loader code.
    PVOID LoaderMemory = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    LoaderParams.ImageBase = ExecutableImage;
    LoaderParams.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)ExecutableImage + pDosHeader->e_lfanew);

    LoaderParams.BaseReloc = (PIMAGE_BASE_RELOCATION)((LPBYTE)ExecutableImage
        + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

    LoaderParams.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)ExecutableImage
        + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    LoaderParams.fnLoadLibraryA = LoadLibraryA;
    LoaderParams.fnGetProcAddress = GetProcAddress;

    // Write the loader information to target process
    WriteProcessMemory(hProcess, LoaderMemory, &LoaderParams, sizeof(loaderdata), NULL);

    // Write the loader code to target process
    WriteProcessMemory(hProcess, (PVOID)((loaderdata*)LoaderMemory + 1), (LPCVOID)LibraryLoader, (DWORD)stub - (DWORD)LibraryLoader, NULL);

    // Create a remote thread to execute the loader code
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((loaderdata*)LoaderMemory + 1), LoaderMemory, 0, NULL);

    // Wait for the loader to finish executing
    WaitForSingleObject(hThread, INFINITE);

    std::cin.get();

    // free the allocated loader code
    VirtualFreeEx(hProcess, LoaderMemory, 0, MEM_RELEASE);

    ui->test->setText("Dll injected!");

}
