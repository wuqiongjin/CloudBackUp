#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/qmessagebox.h>
#include "ui_QtCloudBackupClient.h"
#include "Monitor.hpp"

class QtCloudBackupClient : public QMainWindow
{
    Q_OBJECT

public:
    QtCloudBackupClient(QWidget *parent = nullptr);
    ~QtCloudBackupClient();
	void apply_on_clicked();
	void delete_on_clicked();
	//void exit_on_clicked();
	bool CheckPathValid(const std::string& path);
	bool Backup_Action(bool needCheck);

private:
    Ui::QtCloudBackupClientClass ui;
	CloudBackup::Monitor mon;
};
