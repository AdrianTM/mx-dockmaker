/**********************************************************************
 *  mainwindow.cpp
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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>

#include "picklocation.h"
#include "version.h"
#include <about.h>
#include <cmd.h>
#include <sys/stat.h>

MainWindow::MainWindow(QWidget *parent, const QString &file)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    qDebug() << "Program Version:" << VERSION;
    ui->setupUi(this);
    setConnections();
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setup(file);
}

MainWindow::~MainWindow()
{
    cmd.close();
    delete ui;
}

bool MainWindow::isDockInMenu(const QString &file_name)
{
    if (dock_name.isEmpty())
        return false;
    return getDockName(file_name) == dock_name;
}

void MainWindow::displayIcon(const QString &app_name, int location)
{
    QString icon;
    if (ui->buttonSelectApp->text().endsWith(QLatin1String(".desktop"))) {
        QFile file("/usr/share/applications/" + app_name);
        if (file.open(QFile::ReadOnly)) {
            QString text = file.readAll();
            file.close();
            QRegularExpression re(QStringLiteral("^Icon=(.*)$"), QRegularExpression::MultilineOption);
            auto match = re.match(text);
            if (match.hasMatch()) {
                icon = match.captured(1);
            } else {
                qDebug() << "Could not find icon in file " << file.fileName();
            }
        } else {
            qDebug() << "Could not open file " << file.fileName();
        }
    } else {
        icon = ui->buttonSelectIcon->text();
    }
    const quint8 width = ui->comboSize->currentText().section(QStringLiteral("x"), 0, 0).toUShort();
    const QSize size(width, width);
    const QPixmap pix = findIcon(icon, size).scaled(size);
    if (location == list_icons.size()) {
        list_icons << new QLabel(this);
        ui->icons->addWidget(list_icons.constLast());
    }
    list_icons.at(location)->setPixmap(pix);
    list_icons.at(location)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    list_icons.at(location)->setStyleSheet(
        "background-color: " + ui->widgetBackground->palette().color(QWidget::backgroundRole()).name()
        + ";border: 4px solid " + ui->widgetBorder->palette().color(QWidget::backgroundRole()).name() + ";");
}

bool MainWindow::checkDoneEditing()
{
    if (!apps.isEmpty())
        ui->buttonDelete->setEnabled(true);

    if (ui->buttonSelectApp->text() != tr("Select...") || !ui->lineEditCommand->text().isEmpty()) {
        if (changed)
            ui->buttonSave->setEnabled(true);
        ui->buttonAdd->setEnabled(true);
        if (index != 0)
            ui->buttonPrev->setEnabled(true);
        if (index < apps.size() - 1 && apps.size() > 1)
            ui->buttonNext->setEnabled(true);
        return true;
    } else {
        ui->buttonSave->setEnabled(false);
        ui->buttonPrev->setEnabled(false);
        ui->buttonNext->setEnabled(false);
        ui->buttonAdd->setEnabled(false);
        return false;
    }
}

// setup versious items first time program runs
void MainWindow::setup(const QString &file)
{
    changed = false;
    this->setWindowTitle(QStringLiteral("MX Dockmaker"));
    ui->labelUsage->setText(tr("1. Add applications to the dock one at a time\n"
                               "2. Select a .desktop file or enter a command for the application you want\n"
                               "3. Select icon attributes for size, background (black is standard) and border\n"
                               "4. Press \"Add application\" to continue or \"Save\" to finish"));
    this->adjustSize();

    blockComboSignals(true);

    while (ui->icons->layout()->count() > 0)
        delete ui->icons->layout()->itemAt(0)->widget();

    ui->comboSize->setCurrentIndex(ui->comboSize->findText(QStringLiteral("48x48")));
    file_content.clear();

    // Write configs if not there
    settings.setValue(QStringLiteral("BackgroundColor"),
                      settings.value(QStringLiteral("BackgroundColor"), "black").toString());
    settings.setValue(QStringLiteral("BackgroundHoverColor"),
                      settings.value(QStringLiteral("BackgroundHoverColor"), "black").toString());
    settings.setValue(QStringLiteral("FrameColor"), settings.value(QStringLiteral("FrameColor"), "white").toString());
    settings.setValue(QStringLiteral("FrameHoverColor"),
                      settings.value(QStringLiteral("FrameHoverColor"), "white").toString());
    settings.setValue(QStringLiteral("Size"), settings.value(QStringLiteral("Size"), "48x48").toString());

    blockComboSignals(false);

    if (!file.isEmpty() && QFile::exists(file)) {
        editDock(file);
        return;
    }

    auto *mbox = new QMessageBox(nullptr);
    mbox->setText(tr("This tool allows you to create a new dock with one or more applications. "
                     "You can also edit or delete a dock created earlier."));
    mbox->setIcon(QMessageBox::Question);
    mbox->setWindowTitle(tr("Operation mode"));
    mbox->addButton(tr("&Close"), QMessageBox::NoRole);
    mbox->addButton(tr("&Move"), QMessageBox::NoRole);
    mbox->addButton(tr("&Delete"), QMessageBox::NoRole);
    mbox->addButton(tr("&Edit"), QMessageBox::NoRole);
    mbox->addButton(tr("C&reate"), QMessageBox::NoRole);

    this->hide();

    switch (enum {Close, Move, Delete, Edit, New}; mbox->exec()) {
    case Close:
        QTimer::singleShot(0, qApp, &QGuiApplication::quit);
        break;
    case Move:
        moveDock();
        break;
    case Delete:
        this->show();
        deleteDock();
        setup();
        break;
    case Edit:
        this->show();
        editDock();
        break;
    case New:
        newDock();
        break;
    }
    delete mbox;
}

// Find icon file by name
QPixmap MainWindow::findIcon(QString icon_name, QSize size)
{
    if (icon_name.isEmpty())
        return QPixmap();
    if (QFileInfo::exists("/" + icon_name))
        return QIcon(icon_name).pixmap(size);

    QString search_term = icon_name;
    if (!icon_name.endsWith(QLatin1String(".png")) && !icon_name.endsWith(QLatin1String(".svg"))
        && !icon_name.endsWith(QLatin1String(".xpm")))
        search_term = icon_name + ".*";

    icon_name.remove(QRegularExpression(QStringLiteral(R"(\.png$|\.svg$|\.xpm$")")));

    // return the icon from the theme if it exists
    if (!QIcon::fromTheme(icon_name).isNull())
        return QIcon::fromTheme(icon_name).pixmap(size);

    // Try to find in most obvious places
    QStringList search_paths {QDir::homePath() + "/.local/share/icons/", "/usr/share/pixmaps/",
                              "/usr/local/share/icons/", "/usr/share/icons/hicolor/48x48/apps/"};
    for (const QString &path : search_paths) {
        if (!QFileInfo::exists(path)) {
            search_paths.removeOne(path);
            continue;
        }
        for (const QString &ext : {".png", ".svg", ".xpm"}) {
            QString file = path + icon_name + ext;
            if (QFileInfo::exists(file))
                return QIcon(file).pixmap(QSize());
        }
    }

    // Search recursive
    search_paths.append(QStringLiteral("/usr/share/icons/hicolor/48x48/"));
    search_paths.append(QStringLiteral("/usr/share/icons/hicolor/"));
    search_paths.append(QStringLiteral("/usr/share/icons/"));
    const QString out = cmd.getCmdOut("find " + search_paths.join(QStringLiteral(" ")) + " -iname \"" + search_term
                                          + "\" -print -quit 2>/dev/null",
                                      true);
    return (!out.isEmpty()) ? QIcon(out).pixmap(size) : QPixmap();
}

QString MainWindow::getDockName(const QString &file_name)
{
    const QRegularExpression re_file(".*" + QFileInfo(file_name).fileName());
    const QRegularExpression re_name(QStringLiteral("\\(.*\\)"));

    // check if dock name is in /usr/share...
    QFile file(QStringLiteral("/usr/share/mxflux/menu/appearance"));

    if (file.open(QFile::Text | QFile::ReadOnly)) {
        QString text = file.readAll();
        file.close();

        text = re_file.match(text).captured();
        QString name = re_name.match(text).captured().mid(1);
        name.chop(1);
        if (!name.isEmpty())
            return name;
    }

    // find dock name in submenus file
    file.setFileName(QDir::homePath() + "/.fluxbox/submenus/appearance");

    if (!file.open(QFile::Text | QFile::ReadOnly))
        return QString();
    QString text = file.readAll();
    file.close();

    text = re_file.match(text).captured();

    QString name = re_name.match(text).captured().mid(1);
    name.chop(1);
    return name;
}

QString MainWindow::inputDockName()
{
    bool ok = false;
    QString text = QInputDialog::getText(nullptr, tr("Dock name"), tr("Enter the name to show in the Menu:"),
                                         QLineEdit::Normal, QString(), &ok);
    if (ok && !text.isEmpty())
        return text;
    return QString();
}

void MainWindow::allItemsChanged()
{
    changed = true;
    for (int i = 0; i < apps.size(); ++i) {
        if (index == i) // skip current element since we apply its style to all
            continue;
        const QStringList app_info = QStringList(
            {apps.at(i).at(Info::App), apps.at(i).at(Info::Command), apps.at(i).at(Info::Icon),
             ui->comboSize->currentText(), ui->widgetBackground->palette().color(QWidget::backgroundRole()).name(),
             ui->widgetHoverBackground->palette().color(QWidget::backgroundRole()).name(),
             ui->widgetBorder->palette().color(QWidget::backgroundRole()).name(),
             ui->widgetHoverBorder->palette().color(QWidget::backgroundRole()).name(), apps.at(i).at(Info::Extra)});
        const quint8 width = ui->comboSize->currentText().section(QStringLiteral("x"), 0, 0).toUShort();
        const QSize size(width, width);
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
        list_icons.at(i)->setPixmap(list_icons.at(i)->pixmap()->scaled(size));
#else
        list_icons.at(i)->setPixmap(list_icons.at(i)->pixmap(Qt::ReturnByValue).scaled(size));
#endif
        (i < apps.size()) ? apps.replace(i, app_info) : apps.push_back(app_info);
        list_icons.at(i)->setStyleSheet(
            "background-color: " + ui->widgetBackground->palette().color(QWidget::backgroundRole()).name()
            + ";border: 4px solid " + ui->widgetBorder->palette().color(QWidget::backgroundRole()).name() + ";");
    }
    checkDoneEditing();
}

QString MainWindow::pickSlitLocation()
{
    auto *const pick = new PickLocation(slit_location, this);
    pick->exec();
    return pick->button;
}

void MainWindow::itemChanged()
{
    changed = true;
    updateAppList(index);
    checkDoneEditing();
    displayIcon(ui->buttonSelectApp->text(), index);
    list_icons.at(index)->setStyleSheet(list_icons.at(index)->styleSheet() + "border-width: 10px;");
    if (ui->checkApplyStyleToAll->isChecked())
        allItemsChanged();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (this->childAt(event->pos()) == ui->widgetBackground)
        pickColor(ui->widgetBackground);
    else if (this->childAt(event->pos()) == ui->widgetHoverBackground)
        pickColor(ui->widgetHoverBackground);
    else if (this->childAt(event->pos()) == ui->widgetBorder)
        pickColor(ui->widgetBorder);
    else if (this->childAt(event->pos()) == ui->widgetHoverBorder)
        pickColor(ui->widgetHoverBorder);
    if (!checkDoneEditing())
        return;
    if (event->button() == Qt::LeftButton) {
        int i = ui->icons->layout()->indexOf(this->childAt(event->pos()));
        int old_idx = index;
        if (i >= 0)
            showApp(index = i, old_idx);
    }
}

void MainWindow::updateAppList(int idx)
{
    const QStringList app_info = QStringList(
        {ui->buttonSelectApp->text(), ui->lineEditCommand->text(), ui->buttonSelectIcon->text(),
         ui->comboSize->currentText(), ui->widgetBackground->palette().color(QWidget::backgroundRole()).name(),
         ui->widgetHoverBackground->palette().color(QWidget::backgroundRole()).name(),
         ui->widgetBorder->palette().color(QWidget::backgroundRole()).name(),
         ui->widgetHoverBorder->palette().color(QWidget::backgroundRole()).name(),
         ui->buttonSelectApp->property("extra_options").toString()});
    (idx < apps.size()) ? apps.replace(idx, app_info) : apps.push_back(app_info);
}

void MainWindow::addDockToMenu(const QString &file_name)
{
    cmd.run(R"(sed -i '/\[submenu\] (Docks)/a \\t\t\t[exec] ()" + dock_name + ") {" + file_name + "}' "
                + QDir::homePath() + "/.fluxbox/submenus/appearance",
            true);
}

void MainWindow::deleteDock()
{
    this->hide();
    const QString selected
        = QFileDialog::getOpenFileName(nullptr, tr("Select dock to delete"), QDir::homePath() + "/.fluxbox/scripts",
                                       tr("Dock Files (*.mxdk);;All Files (*.*)"));
    if (!selected.isEmpty()
        && QMessageBox::question(nullptr, tr("Confirmation"), tr("Are you sure you want to delete %1?").arg(selected),
                                 tr("&Delete"), tr("&Cancel"))
               == 0) {
        QFile::remove(selected);
        cmd.run("sed -ni '\\|" + selected + "|!p' " + QDir::homePath() + "/.fluxbox/submenus/appearance", true);
        cmd.run(QStringLiteral("pkill wmalauncher"), true);
    }
    this->show();
}

// block/unblock all relevant signals when loading stuff into GUI
void MainWindow::blockComboSignals(bool block) { ui->comboSize->blockSignals(block); }

void MainWindow::moveDock()
{
    this->hide();
    const QStringList possible_locations({"TopLeft", "TopCenter", "TopRight", "LeftTop", "RightTop", "LeftCenter",
                                          "RightCenter", "LeftBottom", "RightBottom", "BottomLeft", "BottomCenter",
                                          "BottomRight"});

    const QString selected_dock
        = QFileDialog::getOpenFileName(nullptr, tr("Select dock to move"), QDir::homePath() + "/.fluxbox/scripts",
                                       tr("Dock Files (*.mxdk);;All Files (*.*)"));
    if (selected_dock.isEmpty()) {
        setup();
        return;
    }

    QFile file(selected_dock);

    if (!file.open(QFile::Text | QFile::ReadOnly)) {
        qDebug() << "Could not open file:" << file.fileName();
        QMessageBox::warning(nullptr, tr("Could not open file"), tr("Could not open file"));
        setup();
        return;
    }
    QString text = file.readAll();
    file.close();

    // find location
    QRegularExpression re(possible_locations.join(QStringLiteral("|")));
    QRegularExpressionMatch match = re.match(text);
    if (match.hasMatch())
        slit_location = match.captured();

    // select location
    slit_location = pickSlitLocation();

    // replace string
    re.setPattern(QStringLiteral("^sed -i.*"));
    re.setPatternOptions(QRegularExpression::MultilineOption);
    const QString new_line = "sed -i 's/^session.screen0.slit.placement:.*/session.screen0.slit.placement: "
                             + slit_location + "/' $HOME/.fluxbox/init";

    if (!file.open(QFile::Text | QFile::ReadWrite | QFile::Truncate)) {
        qDebug() << "Could not open file:" << file.fileName();
        QMessageBox::warning(nullptr, tr("Could not open file"), tr("Could not open file"));
        setup();
        return;
    }
    QTextStream out(&file);

    // add shebang and pkill wmalauncher
    out << "#!/bin/bash\n\n";
    out << "pkill wmalauncher\n\n";

    // remove if already existent
    text.remove(QStringLiteral("#!/bin/bash\n\n"))
        .remove(QStringLiteral("#!/bin/bash\n"))
        .remove(QStringLiteral("pkill wmalauncher\n\n"))
        .remove(QStringLiteral("pkill wmalauncher\n"));

    // if location line not found add it at the beginning
    if (!re.match(text).hasMatch()) {
        out << "#set up slit location\n" + new_line + "\n";
        out << text.remove(QStringLiteral("#set up slit location\n"));
    } else {
        out << text.replace(re, new_line);
    }
    file.close();
    cmd.run("pkill wmalauncher;" + file.fileName() + "&disown", true);
    setup();
    this->show();
}

// move icon: pos -1 one to left, +1 one to right
void MainWindow::moveIcon(int pos)
{
    changed = true;
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    // swap instead of swapItemAt and don't use Qt::ReturnByValue for pixmap to make it work in Buster
    apps.swap(index, index + pos);
    QPixmap map = *list_icons.at(index)->pixmap();
    list_icons.at(index)->setPixmap(*list_icons.at(index + pos)->pixmap());
#else
    apps.swapItemsAt(index, index + pos);
    QPixmap map = list_icons.at(index)->pixmap(Qt::ReturnByValue);
    list_icons.at(index)->setPixmap(list_icons.at(index + pos)->pixmap(Qt::ReturnByValue));
#endif
    list_icons.at(index + pos)->setPixmap(map);
    list_icons.at(index)->setStyleSheet(list_icons.at(index + pos)->styleSheet());

    index += pos;
    showApp(index, index - 1);
    displayIcon(ui->buttonSelectApp->text(), index);
    list_icons.at(index)->setStyleSheet(list_icons.at(index)->styleSheet() + "border-width: 10px;");

    checkDoneEditing();
}

void MainWindow::parseFile(QFile &file)
{
    blockComboSignals(true);
    parsing = true;

    const QStringList possible_locations({"TopLeft", "TopCenter", "TopRight", "LeftTop", "RightTop", "LeftCenter",
                                          "RightCenter", "LeftBottom", "RightBottom", "BottomLeft", "BottomCenter",
                                          "BottomRight"});

    QString line;
    QStringList options;

    while (!file.atEnd()) {
        line = file.readLine().trimmed();

        if (line.startsWith(QLatin1String("sed -i"))) { // assume it's a slit relocation sed command
            for (const QString &location : possible_locations) {
                if (line.contains(location)) {
                    slit_location = location;
                    break;
                }
            }
            continue;
        }
        if (line.startsWith(QLatin1String("wmalauncher"))) {
            ui->buttonSelectApp->setProperty("extra_options", QString());
            line.remove(QRegularExpression(QStringLiteral("^wmalauncher")));
            line.remove(QRegularExpression(QStringLiteral("\\s*&.*sleep.*$")));

            options = line.split(QRegularExpression(QStringLiteral(" --| -")));
            options.removeAll(QString());

            for (const QString &option : qAsConst(options)) {
                QStringList tokens = option.split(QStringLiteral(" "));
                if (tokens.at(0) == QLatin1String("d") || tokens.at(0) == QLatin1String("desktop-file")) {
                    ui->radioDesktop->setChecked(true);
                    if (tokens.size() > 1)
                        ui->buttonSelectApp->setText(tokens.at(1));
                    ui->buttonSelectIcon->setToolTip(QString());
                    ui->buttonSelectIcon->setStyleSheet(QStringLiteral("text-align: center; padding: 3px;"));
                } else if (tokens.at(0) == QLatin1String("c") || tokens.at(0) == QLatin1String("command")) {
                    ui->radioCommand->setChecked(true);
                    if (tokens.size() > 1)
                        ui->lineEditCommand->setText(tokens.mid(1).join(QStringLiteral(" ")).trimmed());
                    ui->buttonSelectIcon->setStyleSheet(QStringLiteral("text-align: right; padding: 3px;"));
                } else if (tokens.at(0) == QLatin1String("i") || tokens.at(0) == QLatin1String("icon")) {
                    if (tokens.size() > 1) {
                        ui->buttonSelectIcon->setText(tokens.mid(1).join(QStringLiteral(" ")));
                        ui->buttonSelectIcon->setToolTip(tokens.mid(1).join(QStringLiteral(" ")));
                    }
                } else if (tokens.at(0) == QLatin1String("k") || tokens.at(0) == QLatin1String("background-color")) {
                    if (tokens.size() > 1) {
                        QString color = tokens.at(1);
                        color.remove("\"");
                        setColor(ui->widgetBackground, color);
                    }
                } else if (tokens.at(0) == QLatin1String("K")
                           || tokens.at(0) == QLatin1String("hover-background-color")) {
                    if (tokens.size() > 1) {
                        QString color = tokens.at(1);
                        color.remove("\"");
                        setColor(ui->widgetHoverBackground, color);
                    }
                } else if (tokens.at(0) == QLatin1String("b") || tokens.at(0) == QLatin1String("border-color")) {
                    if (tokens.size() > 1) {
                        QString color = tokens.at(1);
                        color.remove("\"");
                        setColor(ui->widgetBorder, color);
                    }
                } else if (tokens.at(0) == QLatin1String("B") || tokens.at(0) == QLatin1String("hover-border-color")) {
                    if (tokens.size() > 1) {
                        QString color = tokens.at(1);
                        color.remove("\"");
                        setColor(ui->widgetHoverBorder, color);
                    }
                } else if (tokens.at(0) == QLatin1String("w") || tokens.at(0) == QLatin1String("window-size")) {
                    if (tokens.size() > 1)
                        ui->comboSize->setCurrentIndex(ui->comboSize->findText(tokens.at(1) + "x" + tokens.at(1)));
                } else if (tokens.at(0) == QLatin1String("x") || tokens.at(0) == QLatin1String("exit-on-right-click")) {
                    // not used right now
                } else { // other not handled options, add them as a propriety to the app button
                    ui->buttonSelectApp->setProperty("extra_options",
                                                     ui->buttonSelectApp->property("extra_options").toString()
                                                         + ((tokens.at(0).length() > 1) ? " --" : " -")
                                                         + tokens.join(QStringLiteral(" ")));
                }
            }
            updateAppList(index);
            displayIcon(ui->buttonSelectApp->text(), index);
            index++;
        } else {
            file_content.append(line + "\n"); // add lines to the file_content, skipping wmalauncher lines
        }
    }
    ui->buttonSave->setDisabled(true);
    showApp(index = 0, -1);
    parsing = false;
}

void MainWindow::buttonSave_clicked()
{
    slit_location = pickSlitLocation();

    // create "~/.fluxbox/scripts" if it doesn't exist
    if (!QFileInfo::exists(QDir::homePath() + "/.fluxbox/scripts"))
        QDir().mkpath(QDir::homePath() + "/.fluxbox/scripts");

    bool new_file = false;
    if (!QFileInfo::exists(file_name)
        || QMessageBox::No
               == QMessageBox::question(this, tr("Overwrite?"), tr("Do you want to overwrite the dock file?"))) {
        file_name = QFileDialog::getSaveFileName(this, tr("Save file"), QDir::homePath() + "/.fluxbox/scripts",
                                                 tr("Dock Files (*.mxdk);;All Files (*.*)"));
        if (file_name.isEmpty())
            return;
        if (!file_name.endsWith(QLatin1String(".mxdk")))
            file_name += QLatin1String(".mxdk");
        new_file = true;
    }
    QFile file(file_name);
    cmd.run("cp " + file_name + " " + file_name + ".~", true);
    if (!file.open(QFile::Text | QFile::WriteOnly)) {
        qDebug() << "Could not open file:" << file.fileName();
        return;
    }
    QTextStream out(&file);
    if (file_content.isEmpty()) {
        // build and write string
        out << "#!/bin/bash\n\n";
        out << "pkill wmalauncher\n\n";
        out << "#set up slit location\n";
        out << "sed -i 's/^session.screen0.slit.placement:.*/session.screen0.slit.placement: " + slit_location
                   + "/' $HOME/.fluxbox/init\n\n";
        out << "fluxbox-remote restart; sleep 1\n\n";
        out << "#commands for dock launchers\n";
    } else {
        // add shebang and pkill wmalauncher
        out << "#!/bin/bash\n\n";
        out << "pkill wmalauncher\n\n";

        // remove if already existent
        file_content.remove(QStringLiteral("#!/bin/bash\n\n"))
            .remove(QStringLiteral("#!/bin/bash\n"))
            .remove(QStringLiteral("pkill wmalauncher\n\n"))
            .remove(QStringLiteral("pkill wmalauncher\n"));
        QRegularExpression re(QStringLiteral("^sed -i.*"), QRegularExpression::MultilineOption);
        QString new_line = "sed -i 's/^session.screen0.slit.placement:.*/session.screen0.slit.placement: "
                           + slit_location + "/' $HOME/.fluxbox/init";

        // if location line not found add it at the beginning
        if (!re.match(file_content).hasMatch()) {
            out << "#set up slit location\n" + new_line + "\n";
            out << file_content.remove(QStringLiteral("#set up slit location\n"));
        } else {
            out << file_content.replace(re, new_line);
        }
    }
    for (const auto &app : qAsConst(apps)) {
        QString command = (app.at(Info::App).endsWith(QLatin1String(".desktop")))
                              ? "--desktop-file " + app.at(Info::App)
                              : "--command " + app.at(Info::Command) + " --icon " + app.at(Info::Command);
        out << "wmalauncher " + command + " --background-color \"" + app.at(Info::BgColor)
                   + "\" --hover-background-color \"" + app.at(Info::BgHoverColor) + "\" --border-color \""
                   + app.at(Info::BorderColor) + "\" --hover-border-color \"" + app.at(Info::BorderHoverColor)
                   + "\" --window-size " + app.at(Info::Size).section(QStringLiteral("x"), 0, 0) + app.at(Info::Extra)
                   + "& sleep 0.1\n";
    }
    file.close();
    QFile::setPermissions(file_name, QFlag(0x744));

    if (dock_name.isEmpty() || new_file)
        dock_name = inputDockName();

    if (dock_name.isEmpty())
        dock_name = QFileInfo(file).baseName();

    if (!isDockInMenu(file.fileName()))
        addDockToMenu(file.fileName());

    QMessageBox::information(this, tr("Dock saved"),
                             tr("The dock has been saved.\n\n"
                                "To edit the newly created dock please select 'Edit an existing dock'."));
    QProcess::execute(QStringLiteral("pkill"), {"wmalauncher"});
    QProcess::startDetached(file.fileName(), {});
    index = 0;
    apps.clear();
    list_icons.clear();
    resetAdd();
    ui->buttonSave->setEnabled(false);
    ui->buttonDelete->setEnabled(false);
    setup();
}

// About button clicked
void MainWindow::buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(
        tr("About %1").arg(tr("MX Dockmaker")),
        R"(<p align="center"><b><h2>MX Dockmaker</h2></b></p><p align="center">)" + tr("Version: ") + VERSION
            + "</p><p align=\"center\"><h3>" + tr("Description goes here")
            + "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"
              "<p align=\"center\">"
            + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
        QStringLiteral("/usr/share/doc/mx-dockmaker/license.html"), tr("%1 License").arg(this->windowTitle()));

    this->show();
}

// Help button clicked
void MainWindow::buttonHelp_clicked()
{
    const QString url = QStringLiteral("https://mxlinux.org/wiki/help-files/help-mx-dockmaker/");
    displayDoc(url, tr("%1 Help").arg(this->windowTitle()));
}

void MainWindow::comboSize_currentTextChanged() { itemChanged(); }

void MainWindow::comboBgColor_currentTextChanged() { itemChanged(); }

void MainWindow::comboBorderColor_currentTextChanged() { itemChanged(); }

void MainWindow::buttonNext_clicked()
{
    blockComboSignals(true);
    updateAppList(index);
    index++;
    showApp(index, index - 1);
    blockComboSignals(false);
}

void MainWindow::buttonDelete_clicked()
{
    if (!apps.isEmpty()) {
        blockComboSignals(true);
        delete ui->icons->layout()->itemAt(index)->widget();
        list_icons.removeAt(index);
        apps.removeAt(index);
        blockComboSignals(false);
    }
    if (apps.isEmpty()) {
        index = 0;
        ui->buttonDelete->setEnabled(false);
        resetAdd();
    } else if (index == 0) {
        ui->buttonPrev->setEnabled(false);
        showApp(index, -1);
    } else {
        ui->buttonSave->setEnabled(true);
        showApp(--index, -1);
    }
    updateAppList(index);
    ui->buttonSave->setEnabled(true);
}

void MainWindow::resetAdd()
{
    ui->buttonSelectApp->setText(tr("Select..."));
    ui->buttonSelectApp->setProperty("extra_options", QString());
    ui->radioDesktop->click();
    emit ui->radioDesktop->toggled(true);
    ui->buttonAdd->setDisabled(true);

    QString size;
    if (ui->checkApplyStyleToAll->isChecked() && index != 0) { // set style according to the first item
        size = apps.at(0).at(Info::Size);
    } else {
        size = settings.value(QStringLiteral("Size"), "48x48").toString();
    }

    blockComboSignals(true);
    ui->comboSize->setCurrentIndex(ui->comboSize->findText(size));
    blockComboSignals(false);

    ui->buttonSelectIcon->setToolTip(QString());
    ui->buttonSelectIcon->setStyleSheet(QStringLiteral("text-align: center; padding: 3px;"));
}

void MainWindow::setConnections()
{
    connect(ui->buttonAbout, &QPushButton::clicked, this, &MainWindow::buttonAbout_clicked);
    connect(ui->buttonAdd, &QPushButton::clicked, this, &MainWindow::buttonAdd_clicked);
    connect(ui->buttonDelete, &QPushButton::clicked, this, &MainWindow::buttonDelete_clicked);
    connect(ui->buttonHelp, &QPushButton::clicked, this, &MainWindow::buttonHelp_clicked);
    connect(ui->buttonMoveLeft, &QPushButton::clicked, this, &MainWindow::buttonMoveLeft_clicked);
    connect(ui->buttonMoveRight, &QPushButton::clicked, this, &MainWindow::buttonMoveRight_clicked);
    connect(ui->buttonNext, &QPushButton::clicked, this, &MainWindow::buttonNext_clicked);
    connect(ui->buttonPrev, &QPushButton::clicked, this, &MainWindow::buttonPrev_clicked);
    connect(ui->buttonSave, &QPushButton::clicked, this, &MainWindow::buttonSave_clicked);
    connect(ui->buttonSelectApp, &QPushButton::clicked, this, &MainWindow::buttonSelectApp_clicked);
    connect(ui->buttonSelectIcon, &QPushButton::clicked, this, &MainWindow::buttonSelectIcon_clicked);
    connect(ui->checkApplyStyleToAll, &QCheckBox::stateChanged, this, &MainWindow::checkApplyStyleToAll_stateChanged);
    connect(ui->comboSize, &QComboBox::currentTextChanged, this, &MainWindow::comboSize_currentTextChanged);
    connect(ui->lineEditCommand, &QLineEdit::textEdited, this, &MainWindow::lineEditCommand_textEdited);
    connect(ui->radioDesktop, &QRadioButton::toggled, this, &MainWindow::radioDesktop_toggled);
    connect(ui->toolBackground, &QToolButton::clicked, this, [this] { pickColor(ui->widgetBackground); });
    connect(ui->toolBorder, &QToolButton::clicked, this, [this] { pickColor(ui->widgetBorder); });
    connect(ui->toolHoverBackground, &QToolButton::clicked, this, [this] { pickColor(ui->widgetHoverBackground); });
    connect(ui->toolHoverBorder, &QToolButton::clicked, this, [this] { pickColor(ui->widgetHoverBorder); });
}

void MainWindow::showApp(int idx, int old_idx)
{
    ui->radioCommand->blockSignals(true);
    ui->radioDesktop->blockSignals(true);

    ui->buttonSelectApp->setText(apps.at(idx).at(Info::App));
    if (apps.at(idx).at(0).endsWith(QLatin1String(".desktop")) || apps.at(idx).at(1).isEmpty()) {
        ui->radioDesktop->setChecked(true);
        ui->lineEditCommand->clear();
        ui->buttonSelectApp->setEnabled(true);
        ui->lineEditCommand->setEnabled(false);
        ui->buttonSelectIcon->setEnabled(false);
        ui->buttonSelectIcon->setText(tr("Select icon..."));
        ui->buttonSelectIcon->setStyleSheet(QStringLiteral("text-align: center; padding: 3px;"));
    } else {
        ui->radioCommand->setChecked(true);
        ui->buttonSelectApp->setText(tr("Select..."));
        ui->buttonSelectApp->setEnabled(false);
        ui->lineEditCommand->setEnabled(true);
        ui->buttonSelectIcon->setEnabled(true);
        ui->lineEditCommand->setText(apps.at(idx).at(Info::Command));
        ui->buttonSelectIcon->setText(apps.at(idx).at(Info::Icon));
        ui->buttonSelectIcon->setToolTip(apps.at(idx).at(Info::Icon));
        ui->buttonSelectIcon->setStyleSheet(QStringLiteral("text-align: right; padding: 3px;"));
    }

    ui->radioCommand->blockSignals(false);
    ui->radioDesktop->blockSignals(false);

    blockComboSignals(true);
    ui->comboSize->setCurrentIndex(ui->comboSize->findText(apps.at(idx).at(Info::Size)));
    setColor(ui->widgetBackground, apps.at(idx).at(Info::BgColor));
    setColor(ui->widgetBackground, apps.at(idx).at(Info::BgHoverColor));
    setColor(ui->widgetBorder, apps.at(idx).at(Info::BorderColor));
    setColor(ui->widgetBorder, apps.at(idx).at(Info::BorderHoverColor));
    ui->buttonSelectApp->setProperty("extra_options", apps.at(idx).at(Info::Extra));
    blockComboSignals(false);

    // set buttons
    ui->buttonNext->setDisabled(idx == apps.size() - 1 || apps.size() == 1);
    ui->buttonPrev->setDisabled(idx == 0);
    ui->buttonAdd->setDisabled(apps.isEmpty());
    ui->buttonDelete->setEnabled(true);
    ui->buttonMoveLeft->setDisabled(idx == 0);
    ui->buttonMoveRight->setDisabled(idx == apps.size() - 1);

    if (old_idx != -1)
        list_icons.at(old_idx)->setStyleSheet(list_icons.at(old_idx)->styleSheet() + "border-width: 4px;");
    list_icons.at(idx)->setStyleSheet(list_icons.at(idx)->styleSheet() + "border-width: 10px;");
}

void MainWindow::buttonSelectApp_clicked()
{
    const QString selected
        = QFileDialog::getOpenFileName(nullptr, tr("Select .desktop file"), QStringLiteral("/usr/share/applications"),
                                       tr("Desktop Files (*.desktop)"));
    const QString file = QFileInfo(selected).fileName();
    if (!file.isEmpty()) {
        file_name = file;
        ui->buttonSelectApp->setText(file);
        ui->buttonAdd->setEnabled(true);
        ui->buttonSelectApp->setProperty("extra_options", QString()); // reset extra options when changing the app.
        itemChanged();
    }
}

void MainWindow::editDock(const QString &file_arg)
{
    this->hide();

    QString selected_dock;
    if (!file_arg.isEmpty() && QFile::exists(file_arg))
        selected_dock = file_arg;
    else
        selected_dock
            = QFileDialog::getOpenFileName(nullptr, tr("Select a dock file"), QDir::homePath() + "/.fluxbox/scripts",
                                           tr("Dock Files (*.mxdk);;All Files (*.*)"));

    if (!QFileInfo::exists(selected_dock)) {
        QMessageBox::warning(nullptr, tr("No file selected"),
                             tr("You haven't selected any dock file to edit.\nCreating a new dock instead."));
        return;
    }
    QFile file(selected_dock);
    if (!file.open(QFile::Text | QFile::ReadOnly)) {
        qDebug() << "Could not open file:" << file.fileName();
        QMessageBox::warning(nullptr, tr("Could not open file"),
                             tr("Could not open selected file.\nCreating a new dock instead."));
        return;
    }
    file_name = file.fileName();
    dock_name = getDockName(file_name);

    parseFile(file);
    file.close();
    ui->labelUsage->setText(tr("1. Edit applications one at a time using the Back and Next buttons\n"
                               "2. Add or delete applications as you like\n"
                               "3. When finished click Save"));
    this->show();
}

void MainWindow::newDock()
{
    this->show();
    apps.clear();
    list_icons.clear();

    resetAdd();
    ui->buttonSave->setEnabled(false);
    ui->buttonDelete->setEnabled(false);
}

void MainWindow::buttonPrev_clicked()
{
    blockComboSignals(true);
    updateAppList(index);
    int old_idx = index;
    showApp(--index, old_idx);
    blockComboSignals(false);
}

void MainWindow::radioDesktop_toggled(bool checked)
{
    ui->lineEditCommand->clear();
    ui->buttonSelectApp->setEnabled(checked);
    ui->lineEditCommand->setEnabled(!checked);
    ui->buttonSelectIcon->setEnabled(!checked);
    if (checked) {
        ui->buttonSelectIcon->setText(tr("Select icon..."));
        ui->buttonSelectIcon->setStyleSheet(QStringLiteral("text-align: center; padding: 3px;"));
    } else {
        ui->buttonSelectApp->setText(tr("Select..."));
    }
    if (!parsing)
        checkDoneEditing();
}

void MainWindow::pickColor(QWidget *widget)
{
    QColor color = QColorDialog::getColor(widget->palette().color(QWidget::backgroundRole()));
    if (color.isValid()) {
        setColor(widget, color);
        itemChanged();
        if (widget == ui->widgetBackground)
            setColor(ui->widgetHoverBackground, color);
        else if (widget == ui->widgetBorder)
            setColor(ui->widgetHoverBorder, color);
    }
}

void MainWindow::setColor(QWidget *widget, const QColor &color)
{
    if (color.isValid()) {
        QPalette pal = palette();
        pal.setColor(QPalette::Window, color);
        widget->setAutoFillBackground(true);
        widget->setPalette(pal);
    }
}

void MainWindow::buttonSelectIcon_clicked()
{
    ui->buttonSave->setDisabled(ui->buttonNext->isEnabled());
    QString default_folder;
    if (ui->buttonSelectIcon->text() != tr("Select icon...") && QFileInfo::exists(ui->buttonSelectIcon->text())) {
        QFileInfo f_info(ui->buttonSelectIcon->text());
        default_folder = f_info.canonicalPath();
    } else {
        default_folder = QStringLiteral("/usr/share/icons/");
    }
    QString selected = QFileDialog::getOpenFileName(nullptr, tr("Select icon"), default_folder,
                                                    tr("Icons (*.png *.jpg *.bmp *.xpm *.svg)"));
    QString file = QFileInfo(selected).fileName();
    if (!file.isEmpty()) {
        ui->buttonSelectIcon->setText(selected);
        ui->buttonSelectIcon->setToolTip(selected);
        ui->buttonSelectIcon->setStyleSheet(QStringLiteral("text-align: right; padding: 3px;"));
        updateAppList(index);
        displayIcon(ui->buttonSelectIcon->text(), index);
        list_icons.at(index)->setStyleSheet(list_icons.at(index)->styleSheet() + "border-width: 10px;");
    }
    if (!parsing)
        checkDoneEditing();
}

void MainWindow::lineEditCommand_textEdited()
{
    ui->buttonSave->setDisabled(ui->buttonNext->isEnabled());
    if (ui->buttonSelectIcon->text() != tr("Select icon...")) {
        ui->buttonSelectApp->setProperty("extra_options", QString()); // reset extra options when changing the command
        ui->buttonNext->setEnabled(true);
        changed = true;
    } else {
        ui->buttonNext->setEnabled(false);
        return;
    }
    if (!parsing)
        checkDoneEditing();
}

void MainWindow::buttonAdd_clicked()
{
    index++;
    resetAdd();
    list_icons.insert(index, new QLabel());
    const QStringList app_info {ui->buttonSelectApp->text(),
                                ui->lineEditCommand->text(),
                                ui->buttonSelectIcon->text(),
                                ui->comboSize->currentText(),
                                ui->widgetBackground->palette().color(QWidget::backgroundRole()).name(),
                                ui->widgetHoverBackground->palette().color(QWidget::backgroundRole()).name(),
                                ui->widgetBorder->palette().color(QWidget::backgroundRole()).name(),
                                ui->widgetHoverBorder->palette().color(QWidget::backgroundRole()).name(),
                                ui->buttonSelectApp->property("extra_options").toString()};
    apps.insert(index, app_info);
    ui->icons->insertWidget(index, list_icons.at(index));
    showApp(index, index - 1);
    itemChanged();
}

void MainWindow::buttonMoveLeft_clicked()
{
    if (index == 0)
        return;
    moveIcon(-1);
}

void MainWindow::buttonMoveRight_clicked()
{
    if (index == apps.size() - 1)
        return;
    moveIcon(1);
}

void MainWindow::checkApplyStyleToAll_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
        allItemsChanged();
}
