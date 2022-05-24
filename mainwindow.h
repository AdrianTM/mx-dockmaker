/**********************************************************************
 *  mainwindow.h
 **********************************************************************
 * Copyright (C) 2020 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>

#include "cmd.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, QString file = QString());
    ~MainWindow();

    bool checkDoneEditing();
    bool isDockInMenu(const QString &file_name);

    void addDockToMenu(const QString &file_name);
    void blockComboSignals(bool enable);
    void cleanup();
    void deleteDock();
    void displayIcon(const QString &app_name, int location);
    void editDock(QString file_arg = QString());
    void moveDock();
    void newDock();
    void parseFile(QFile &file);
    void resetAdd();
    void setConnections();
    void setup(QString file = QString());
    void showApp(int i, int old_idx);
    void updateAppList(int idx);

    QPixmap findIcon(QString icon_name, const QSize &size);
    QString getDockName(const QString &file_name);
    QString inputDockName();
    QString pickSlitLocation();

public slots:

private slots:
    void buttonAbout_clicked();
    void buttonAdd_clicked();
    void buttonDelete_clicked();
    void buttonHelp_clicked();
    void buttonMoveLeft_clicked();
    void buttonMoveRight_clicked();
    void buttonNext_clicked();
    void buttonPrev_clicked();
    void buttonSave_clicked();
    void buttonSelectApp_clicked();
    void buttonSelectIcon_clicked();
    void comboBgColor_currentTextChanged(const QString);
    void comboBorderColor_currentTextChanged(const QString);
    void comboSize_currentTextChanged(const QString);
    void itemChanged();
    void lineEditCommand_textEdited(const QString);
    void mousePressEvent(QMouseEvent *event);
    void radioDesktop_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QProcess proc;
    Cmd cmd;

    bool changed = false;
    bool parsing  = false;
    int index = 0;
    QList<QLabel *> list_icons;
    QString dock_name;
    QString file_content;
    QString file_name;
    QString slit_location;
    QSettings settings;
    QList<QStringList> apps;


};


#endif

